/*=============================================================================
    SimNode.cpp - headless process that runs enemy AI for a room.

    OLGame.exe server OLEntry?Game=Multiplayer.MultiplayerGame -simnode
        -simaddr=<ip> -simport=<port> -simroom=<code>
    The relay and this process must share OPENOL_SIM_SECRET in their environment.
    OPENOL_SERVER_KEY, if set, is the key the relay must have.

    Behaves as an ordinary client that follows the room's world, so triggers and
    checkpoints run the same Kismet as in the game. Differences: nothing renders,
    there is no local player, and its own hero exists only for code that expects one.
    Start on OLEntry, not a story map: a map cannot later be streamed in over itself.
=============================================================================*/
#include "Multiplayer.h"
#include "OLGameClasses.h"
#include "EngineAnimClasses.h"

UBOOL GIsSimNode = FALSE;

// Used by AActor::GetALocalPlayerController when the engine has no local players.
extern APlayerController* GHeadlessController;

// Triggers that arrived while the world was still loading.
static TArray<FString> GDeferredTriggers;

static UBOOL SimNode_IsWorldReady()
{
    if (!GMultiplayerController || GMultiplayerController->bTravellingToCheckpoint)
        return FALSE;
    const AWorldInfo* WI = GWorld->GetWorldInfo();
    for (INT i = 0; i < WI->StreamingLevels.Num(); i++)
    {
        const ULevelStreaming* Level = WI->StreamingLevels(i);
        if (Level && !Level->bIsVisible)
            return FALSE;
    }
    return TRUE;
}

UBOOL SimNode_DeferTrigger(USeqEvent_Touch* TouchEvent, const FString& EventPath)
{
    if (SimNode_IsWorldReady() && GDeferredTriggers.Num() == 0)
        return FALSE;
    GDeferredTriggers.AddItem(EventPath);
    debugf(NAME_Log, TEXT("[SimNode] Trigger waits for the level: %s"), *EventPath);
    return TRUE;
}

void SimNode_OnHeroSpawned(AOLHero* Hero)
{
    // A new hero means a new world: whatever was waiting belonged to the old one.
    GDeferredTriggers.Empty();

    // Bots skip ghosts; without collision it cannot set off anything in the world.
    Hero->bIsGhost = TRUE;
    Hero->setPhysics(PHYS_None);
    Hero->SetCollision(FALSE, FALSE, FALSE);
    Hero->bCollideWorld = FALSE;
    debugf(NAME_Log, TEXT("[SimNode] Hero %s at %s"), *Hero->GetName(), *Hero->Location.ToString());
}

class FSimNodeTicker : public FTickableObject
{
public:
    FSimNodeTicker() : LastTick(0.0), NextReport(0.0), Frames(0) {}

    virtual UBOOL IsTickable() const           { return GWorld && GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->bBegunPlay; }
    // Loading a checkpoint pauses the world, and levels must keep coming in meanwhile.
    virtual UBOOL IsTickableWhenPaused() const { return TRUE; }

    virtual void Tick(FLOAT DeltaTime)
    {
        LimitTickRate();
        KeepLevelsLoaded();

        if (!GMultiplayerController)
        {
            SpawnController();
            return;
        }
        ApplyDeferredTriggers();
        TickEnemies();
        FollowPlayers();
        Report();
    }

private:
    // The engine caps a dedicated server by its net driver, and this process has none.
    void LimitTickRate()
    {
        const DOUBLE Spare = 1.0 / 30.0 - (appSeconds() - LastTick);
        if (Spare > 0.0)
            appSleep((FLOAT)Spare);
        LastTick = appSeconds();
    }

    // Players can be anywhere in the map, and a trigger needs its level to be there.
    void KeepLevelsLoaded()
    {
        AWorldInfo* WI = GWorld->GetWorldInfo();
        for (INT i = 0; i < WI->StreamingLevels.Num(); i++)
        {
            ULevelStreaming* Level = WI->StreamingLevels(i);
            // After a map change the old map's levels are on their way out.
            if (Level && !Level->bIsRequestingUnloadAndRemoval)
            {
                Level->bShouldBeLoaded = TRUE;
                Level->bShouldBeVisible = TRUE;
            }
        }
    }

    void SpawnController()
    {
        AGameInfo* Game = GWorld->GetGameInfo();
        if (!Game)
            return;
        AMultiplayerController* Controller = Cast<AMultiplayerController>(
            GWorld->SpawnActor(AMultiplayerController::StaticClass(), NAME_None, FVector(0.f), FRotator(0, 0, 0)));
        GHeadlessController = Controller;
        if (Controller)
        {
            // Never drawn, but gameplay code reports to it all over the place.
            Controller->HUD = Cast<AOLHUD>(GWorld->SpawnActor(Game->HUDType, NAME_None, FVector(0.f), FRotator(0, 0, 0), NULL, FALSE, FALSE, Controller));
            Controller->myHUD = Controller->HUD;
        }
        debugf(NAME_Log, TEXT("[SimNode] Controller: %s"), Controller ? *Controller->GetName() : TEXT("FAILED"));
    }

    void ApplyDeferredTriggers()
    {
        if (GDeferredTriggers.Num() == 0 || !SimNode_IsWorldReady())
            return;
        extern void SimNode_ApplyTrigger(AMultiplayerController*, const FString&);
        const TArray<FString> Triggers = GDeferredTriggers;
        GDeferredTriggers.Empty();
        for (INT i = 0; i < Triggers.Num(); i++)
            SimNode_ApplyTrigger(GMultiplayerController, Triggers(i));
    }

    void TickEnemies()
    {
        AMultiplayerController* Ctrl = GMultiplayerController;
        for (INT i = 0; i < Ctrl->CachedEnemies.Num(); i++)
        {
            AOLEnemyPawn* Enemy = Ctrl->CachedEnemies(i);
            if (Enemy && Enemy->Mesh && !Enemy->Mesh->bUpdateSkelWhenNotRendered)
            {
                // Nothing ever renders here; animation must tick anyway.
                Enemy->Mesh->bUpdateSkelWhenNotRendered = TRUE;
                Enemy->Mesh->bTickAnimNodesWhenNotRendered = TRUE;
                Enemy->Mesh->bIgnoreControllersWhenNotRendered = FALSE;
                debugf(NAME_Log, TEXT("[SimNode] Enemy %s"), *Enemy->GetName());
            }
        }
    }

    // Enemies read paths and distances from "the" hero, so it stays with a real player.
    void FollowPlayers()
    {
        AMultiplayerController* Ctrl = GMultiplayerController;
        if (!GMultiplayerHero)
            return;
        for (INT i = 0; i < Ctrl->RemotePlayers.Num(); i++)
        {
            URemotePlayer* Player = Ctrl->RemotePlayers(i);
            AOLHero* Dummy = Player ? Cast<AOLHero>(Player->DummyPlayer) : NULL;
            if (Dummy && !Dummy->bDeleteMe && Dummy->Health > 0)
            {
                GWorld->FarMoveActor(GMultiplayerHero, Dummy->Location, FALSE, TRUE);
                return;
            }
        }
    }

    void Report()
    {
        Frames++;
        const DOUBLE Now = appSeconds();
        if (Now < NextReport)
            return;
        NextReport = Now + 5.0;

        AMultiplayerController* Ctrl = GMultiplayerController;
        AWorldInfo* WI = GWorld->GetWorldInfo();
        INT Visible = 0;
        for (INT i = 0; i < WI->StreamingLevels.Num(); i++)
            if (WI->StreamingLevels(i) && WI->StreamingLevels(i)->bIsVisible)
                Visible++;
        debugf(NAME_Log, TEXT("[SimNode] fps=%.1f paused=%d conn=%d hs=%d id=%d owner=%d epoch=%d levels=%d/%d hero=%d players=%d enemies=%d"),
            Frames / 5.f, (INT)GWorld->IsPaused(), (INT)GMpConn.bIsConnected, (INT)GMpConn.bIsHandshaked, GMpConn.LocalPlayerID, GMpConn.SimOwnerID,
            (INT)GMpConn.WorldEpoch, Visible, WI->StreamingLevels.Num(), GMultiplayerHero ? 1 : 0,
            Ctrl->RemotePlayers.Num(), Ctrl->CachedEnemies.Num());
        Frames = 0;

        for (INT i = 0; i < Ctrl->CachedEnemies.Num(); i++)
        {
            AOLEnemyPawn* Enemy = Ctrl->CachedEnemies(i);
            if (!Enemy || Enemy->bDeleteMe || !Enemy->Bot)
                continue;
            AOLBot* Bot = Enemy->Bot;
            debugf(NAME_Log, TEXT("[SimNode]   %s state=%s smt=%d speed=%.0f loc=%s target=%s"),
                *Enemy->GetName(),
                Bot->GetStateFrame() && Bot->GetStateFrame()->StateNode ? *Bot->GetStateFrame()->StateNode->GetName() : TEXT("none"),
                (INT)Enemy->SpecialMove, Enemy->Velocity.Size(), *Enemy->Location.ToString(),
                Bot->TargetPlayer ? *Bot->TargetPlayer->GetName() : TEXT("none"));
        }
    }

    DOUBLE LastTick;
    DOUBLE NextReport;
    INT    Frames;
};

void SimNode_Init()
{
    const TCHAR* CmdLine = appCmdLine();
    if (!ParseParam(CmdLine, TEXT("simnode")))
        return;
    GIsSimNode = TRUE;

    FString Value;
    GMpConn.IP       = Parse(CmdLine, TEXT("simaddr="), Value) ? Value : FString(TEXT("127.0.0.1"));
    GMpConn.UdpPort  = Parse(CmdLine, TEXT("simport="), Value) ? Value : FString(TEXT("7777"));
    GMpConn.RoomCode = Parse(CmdLine, TEXT("simroom="), Value) ? Value : FString(TEXT("DEFAULT"));
    // Taken from the environment so that it stays out of the process list.
    TCHAR Secret[128] = {0};
    appGetEnvironmentVariable(TEXT("OPENOL_SIM_SECRET"), Secret, ARRAY_COUNT(Secret));
    GMpConn.Password = Secret;
    // Optional: the relay's key, for a relay on another machine.
    TCHAR ServerKey[80] = {0};
    appGetEnvironmentVariable(TEXT("OPENOL_SERVER_KEY"), ServerKey, ARRAY_COUNT(ServerKey));
    GMpConn.ConfiguredServerKey = ServerKey;
    GMpConn.Username = TEXT("SimNode");
    // Same as hosting: the caller's address counts, not the player's ini.
    GMpConn.bHostMode = TRUE;

    new FSimNodeTicker();
}
