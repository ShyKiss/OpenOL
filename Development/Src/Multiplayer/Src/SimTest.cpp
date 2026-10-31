/*=============================================================================
    SimTest.cpp - headless check that enemy AI can run without a renderer or a
    player. Run with: OLGame.exe server <map>?Game=Multiplayer.MultiplayerGame
    -simtest -simcp=<checkpoint>. Loads all streaming levels, places a dummy hero
    at the checkpoint and a soldier in front of it, and logs them once a second.
=============================================================================*/
#include "Multiplayer.h"
#include "OLGameClasses.h"
#include "OLUtilities.h"
#include "EngineAnimClasses.h"
#include "UDKBaseAnimationClasses.h"

// Used by AActor::GetALocalPlayerController when the engine has no local players.
extern APlayerController* GHeadlessController;

class FSimTestTicker : public FTickableObject
{
public:
    FSimTestTicker()
        : Phase(0), PhaseStart(0.0), NextReport(0.0), Frames(0)
        , Hero(NULL), Enemy(NULL), Bot(NULL)
    {}

    virtual UBOOL IsTickable() const        { return GWorld && GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->bBegunPlay; }
    virtual UBOOL IsTickableWhenPaused() const { return FALSE; }

    virtual void Tick(FLOAT DeltaTime)
    {
        const DOUBLE Now = appSeconds();
        Frames++;
        switch (Phase)
        {
        case 0: StartStreaming(Now); break;
        case 1: WaitForLevels(Now); break;
        case 2: SpawnActors(Now); break;
        default: Report(Now); break;
        }
    }

private:
    void StartStreaming(DOUBLE Now)
    {
        AWorldInfo* WI = GWorld->GetWorldInfo();
        for (INT i = 0; i < WI->StreamingLevels.Num(); i++)
        {
            ULevelStreaming* Level = WI->StreamingLevels(i);
            if (Level)
            {
                Level->bShouldBeLoaded = TRUE;
                Level->bShouldBeVisible = TRUE;
            }
        }
        debugf(NAME_Log, TEXT("[SimTest] Loading %d streaming levels"), WI->StreamingLevels.Num());
        PhaseStart = Now;
        Phase = 1;
    }

    void WaitForLevels(DOUBLE Now)
    {
        AWorldInfo* WI = GWorld->GetWorldInfo();
        INT Visible = 0;
        for (INT i = 0; i < WI->StreamingLevels.Num(); i++)
        {
            ULevelStreaming* Level = WI->StreamingLevels(i);
            if (!Level || Level->bIsVisible)
                Visible++;
        }
        if (Visible == WI->StreamingLevels.Num() || Now - PhaseStart > 90.0)
        {
            debugf(NAME_Log, TEXT("[SimTest] %d/%d levels visible after %.1f s"),
                Visible, WI->StreamingLevels.Num(), (FLOAT)(Now - PhaseStart));
            Phase = 2;
        }
    }

    void SpawnActors(DOUBLE Now)
    {
        Phase = 3;
        NextReport = Now + 1.0;
        Frames = 0;

        FString CpName;
        Parse(appCmdLine(), TEXT("simcp="), CpName);

        AOLCheckpoint* Checkpoint = NULL;
        INT NumCheckpoints = 0;
        for (FActorIterator It; It; ++It)
        {
            AOLCheckpoint* CP = Cast<AOLCheckpoint>(*It);
            if (!CP) continue;
            NumCheckpoints++;
            if (CP->CheckpointName.ToString() == CpName)
                Checkpoint = CP;
        }
        debugf(NAME_Log, TEXT("[SimTest] %d checkpoints in world, '%s': %s"), NumCheckpoints, *CpName, Checkpoint ? TEXT("found") : TEXT("NOT FOUND"));
        if (!Checkpoint)
            return;

        // The bot scripts look for a player controller to report through.
        AMultiplayerController* Controller = Cast<AMultiplayerController>(
            GWorld->SpawnActor(AMultiplayerController::StaticClass(), NAME_None, Checkpoint->Location, Checkpoint->Rotation));
        GHeadlessController = Controller;
        debugf(NAME_Log, TEXT("[SimTest] Controller: %s"), Controller ? *Controller->GetName() : TEXT("FAILED"));

        Hero = Cast<AOLHero>(GWorld->SpawnActor(AMultiplayerHero::StaticClass(), NAME_None,
            Checkpoint->Location, Checkpoint->Rotation, NULL, TRUE, FALSE));
        if (Hero)
        {
            Hero->InitDummyMesh();
            Hero->bIsDummyPawn = TRUE;
            Hero->DummyOwnerID = 1;
            Hero->setPhysics(PHYS_None);
            // Audio and gameplay code ask the controller for "the" hero; it is the listener.
            if (Controller)
                Controller->HeroPawn = Hero;
        }

        const FVector EnemyLoc = Checkpoint->Location + Checkpoint->Rotation.Vector() * 500.f;
        Enemy = Cast<AOLEnemyPawn>(GWorld->SpawnActor(AOLEnemySoldier::StaticClass(), NAME_None,
            EnemyLoc, (-Checkpoint->Rotation.Vector()).Rotation(), NULL, TRUE, FALSE));
        if (Enemy)
        {
            Bot = Cast<AOLBot>(GWorld->SpawnActor(AOLBot::StaticClass()));
            if (Bot)
                Bot->eventPossess(Enemy, FALSE);
            Enemy->Modifiers.bShouldAttack = TRUE;
            Enemy->eventApplyModifiers(Enemy->Modifiers);
            Enemy->BehaviorTree = Cast<UOLBTBehaviorTree>(Utils::LoadObjectFromModPackage(
                TEXT(""), TEXT("02_AI_Behaviors.Soldier_BT"), UOLBTBehaviorTree::StaticClass()));
            if (Enemy->Mesh)
            {
                // Nothing ever renders here; animation must tick anyway.
                Enemy->Mesh->bUpdateSkelWhenNotRendered = TRUE;
                Enemy->Mesh->bTickAnimNodesWhenNotRendered = TRUE;
                Enemy->Mesh->bIgnoreControllersWhenNotRendered = FALSE;
            }
        }
        debugf(NAME_Log, TEXT("[SimTest] Hero=%s Enemy=%s Bot=%s BehaviorTree=%s"),
            Hero ? *Hero->GetName() : TEXT("FAILED"),
            Enemy ? *Enemy->GetName() : TEXT("FAILED"),
            Bot ? *Bot->GetName() : TEXT("FAILED"),
            Enemy && Enemy->BehaviorTree ? *Enemy->BehaviorTree->GetName() : TEXT("NONE"));
    }

    void Report(DOUBLE Now)
    {
        // Normally set by the camera, which a dummy does not have.
        if (Hero)
        {
            Hero->EyeLocation = Hero->Location + FVector(0.f, 0.f, 60.f);
            Hero->EyeRotation = Hero->Rotation;
            Hero->EyeForward  = Hero->Rotation.Vector();
        }
        if (Now < NextReport)
            return;
        const FLOAT Fps = (FLOAT)Frames / (FLOAT)(Now - NextReport + 1.0);
        NextReport = Now + 1.0;
        Frames = 0;
        if (!Enemy || Enemy->bDeleteMe || !Bot)
        {
            debugf(NAME_Log, TEXT("[SimTest] fps=%.1f no enemy"), Fps);
            return;
        }

        FString Anim(TEXT("-"));
        if (Enemy->FullBodyAnimSlot && Enemy->FullBodyAnimSlot->bIsPlayingCustomAnim)
        {
            UAnimNodeSequence* Seq = Enemy->FullBodyAnimSlot->GetCustomAnimNodeSeq();
            if (Seq)
                Anim = FString::Printf(TEXT("%s@%.2f"), *Seq->AnimSeqName.ToString(), Seq->CurrentTime);
        }
        const FLOAT Dist = Hero ? (Enemy->Location - Hero->Location).Size() : 0.f;
        debugf(NAME_Log, TEXT("[SimTest] fps=%.1f state=%s mode=%d smt=%d phys=%d speed=%.0f dist=%.0f target=%s anim=%s"),
            Fps,
            Bot->GetStateFrame() && Bot->GetStateFrame()->StateNode ? *Bot->GetStateFrame()->StateNode->GetName() : TEXT("none"),
            (INT)Enemy->EnemyMode, (INT)Enemy->SpecialMove, (INT)Enemy->Physics, Enemy->Velocity.Size(), Dist,
            Bot->TargetPlayer ? *Bot->TargetPlayer->GetName() : TEXT("none"), *Anim);
    }

    INT             Phase;
    DOUBLE          PhaseStart;
    DOUBLE          NextReport;
    INT             Frames;
    AOLHero*        Hero;
    AOLEnemyPawn*   Enemy;
    AOLBot*         Bot;
};

void SimTest_Init()
{
    if (ParseParam(appCmdLine(), TEXT("simtest")))
        new FSimTestTicker();
}
