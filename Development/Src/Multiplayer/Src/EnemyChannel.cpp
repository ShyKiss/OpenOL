#include "Multiplayer.h"
#include "EnemyChannel.h"
#include "EnemyChannelPackets.h"
#include "HeroChannel.h"    // PutU8/ReadU8/etc. helpers
#include "DoorChannel.h"    // FindIndexedDoor / DoorChannel_OnDoorDeny

// Writes [len(1)][bytes] into B at offset N, returns new offset. Max 63 chars.
static INT PutStr(BYTE* B, INT N, const FString& S, INT MaxLen = 63)
{
    INT Len = Min(S.Len(), MaxLen);
    N = PutU8(B, N, Len);
    for (INT i = 0; i < Len; i++)
        N = PutU8(B, N, (BYTE)((*S)[i] & 0x7F));
    return N;
}

// Reads [len(1)][bytes] from Data at offset Off into OutStr, returns new offset.
static INT ReadStr(const BYTE* Data, INT Off, INT DataLen, FString& OutStr)
{
    INT Len = 0;
    Off = ReadU8(Data, Off, Len);
    TCHAR Buf[65];
    INT   i;
    for (i = 0; i < Len && i < 64 && Off < DataLen; i++, Off++)
        Buf[i] = (TCHAR)Data[Off];
    Buf[i] = 0;
    OutStr = FString(Buf);
    return Off;
}

// ============================================================================
// Native IndexEnemies — called by EnemyChannel to keep CachedEnemies in sync.
// ============================================================================

void AMultiplayerController::IndexEnemies()
{
    // Build a pointer set from the current cache for O(1) duplicate detection.
    TMap<AOLEnemyPawn*, INT> CacheSet;
    for (INT i = 0; i < CachedEnemies.Num(); i++)
        if (CachedEnemies(i))
            CacheSet.Set(CachedEnemies(i), i);

    // Walk world actors once; add enemies not yet in the cache.
    static const FName DummyTag(TEXT("MultiplayerDummyEnemy"));
    for (FActorIterator It; It; ++It)
    {
        AOLEnemyPawn* E = Cast<AOLEnemyPawn>(*It);
        if (!E || E->bDeleteMe || E->bPendingDelete)
            continue;
        if (E->Tag == DummyTag)
            continue;
        if (CacheSet.Find(E))
            continue;

        CachedEnemies.AddItem(E);
        LastEnemyAlive.AddItem(E->Health > 0);
        LastEnemySMT.AddItem(0);
        LastBashLoopSentTime.AddItem(0.0f);
        CacheSet.Set(E, CachedEnemies.Num() - 1);
    }

    // Purge stale (NULL) entries from the cache (back-to-front to keep indices stable).
    for (INT i = CachedEnemies.Num() - 1; i >= 0; i--)
    {
        if (!CachedEnemies(i))
        {
            CachedEnemies.Remove(i, 1);
            LastEnemyAlive.Remove(i, 1);
            LastEnemySMT.Remove(i, 1);
            LastBashLoopSentTime.Remove(i, 1);
        }
    }
}

// ============================================================================
// Thin forwarding helpers — delegate to native AOLEnemyPawn methods
// ============================================================================

static void CallSetNetMesh(AOLEnemyPawn* E, const FString& MeshName)       { E->NativeSetNetMesh(MeshName); }
static void CallSetNetWeapon(AOLEnemyPawn* E, INT WeaponIdx)                { E->NativeSetNetWeapon(WeaponIdx); }
static void CallBuildScriptAnimSetList(AOLEnemyPawn* E)                     { E->NativeBuildScriptAnimSetList(); }
static void CallSetEnemyMode(AOLEnemyPawn* E, INT Mode)                     { E->NativeSetEnemyMode(Mode); }
static void CallSetNetEnvironment(AOLEnemyPawn* E, INT Env)                 { E->NativeSetNetEnvironment(Env); }
static void CallPlayNetSMT(AOLEnemyPawn* E, INT S, INT P1, INT P2)         { E->NativePlayNetSMT(S, P1, P2); }
static INT  CallGetDoorSMTFlags(AOLEnemyPawn* E)                            { return E->NativeGetDoorSMTFlags(); }

// ============================================================================
// FindEnemyByName
// ============================================================================

AOLEnemyPawn* UEnemyChannel::FindEnemyByName(const FString& EnemyName)
{
    if (!ControllerOwner) return NULL;
    for (INT i = 0; i < ControllerOwner->CachedEnemies.Num(); i++)
    {
        AOLEnemyPawn* E = ControllerOwner->CachedEnemies(i);
        if (E && E->GetName() == EnemyName)
            return E;
    }
    ControllerOwner->IndexEnemies();
    for (INT i = 0; i < ControllerOwner->CachedEnemies.Num(); i++)
    {
        AOLEnemyPawn* E = ControllerOwner->CachedEnemies(i);
        if (E && E->GetName() == EnemyName)
            return E;
    }
    return NULL;
}

// ============================================================================
// SendSpawn
// ============================================================================

void UEnemyChannel::SendSpawn(AOLEnemyPawn* E)
{
    if (!E || !GMpConn.bIsConnected) return;

    // Dedup: never send the same enemy twice per session
    FString EName = E->GetName();
    for (INT i = 0; i < ControllerOwner->SentEnemySpawns.Num(); i++)
        if (ControllerOwner->SentEnemySpawns(i) == EName) return;
    ControllerOwner->SentEnemySpawns.AddItem(EName);

    FString ClassName = E->GetClass()->GetName();
    FString MeshPath  = E->NetMeshPath.Len() > 0
        ? FString(E->NetMeshPath)
        : (E->Mesh && E->Mesh->SkeletalMesh) ? FString(E->Mesh->SkeletalMesh->GetPathName()) : FString(TEXT(""));

    // Layout: [type(1)][name][class][X(4)][Y(4)][Z(4)][Yaw(2)][mesh][weapon(1)][bColor(1)]
    //         if bColor: [R(2)][G(2)][B(2)][A(2)]  (each * 1000, as I16)
    BYTE B[512];
    INT  N = 0;
    N = PutU8 (B, N, CH_ENEMY);
    N = PutU8 (B, N, ENEMY_SPAWN);
    N = PutStr(B, N, EName);
    N = PutStr(B, N, ClassName);
    N = PutF32(B, N, E->Location.X);
    N = PutF32(B, N, E->Location.Y);
    N = PutF32(B, N, E->Location.Z);
    N = PutU16(B, N, E->Rotation.Yaw & 0xFFFF);
    N = PutStr(B, N, MeshPath, 127);
    N = PutU8 (B, N, (BYTE)E->CurrentWeapon);
    if (E->bHasUniformColorOverride)
    {
        N = PutU8 (B, N, 1);
        N = PutI16(B, N, appRound(E->UniformColorOverride.R * 1000.0f));
        N = PutI16(B, N, appRound(E->UniformColorOverride.G * 1000.0f));
        N = PutI16(B, N, appRound(E->UniformColorOverride.B * 1000.0f));
        N = PutI16(B, N, appRound(E->UniformColorOverride.A * 1000.0f));
    }
    else
    {
        N = PutU8(B, N, 0);
    }
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// SendLocBinary (internal)
// ============================================================================

static void SendEnemyLocBinary(AOLEnemyPawn* E,
    INT EnvIdx, FVector BotLookAt, UBOOL bBotLookActive, BYTE BotFlags)
{
    BYTE B[256];
    INT  N = 0;

    FString EName  = E->GetName();
    INT     NameLen = Min(EName.Len(), 31);

    N = PutU8(B, N, CH_ENEMY);
    N = PutU8(B, N, ENEMY_LOC);
    N = PutU8(B, N, NameLen);
    for (INT c = 0; c < NameLen; c++)
        N = PutU8(B, N, (BYTE)((*EName)[c] & 0x7F));
    N = PutU8(B, N, 0);  // null terminator

    N = PutF32(B, N, E->Location.X);
    N = PutF32(B, N, E->Location.Y);
    N = PutF32(B, N, E->Location.Z);
    N = PutU16(B, N, E->Rotation.Yaw & 0xFFFF);
    N = PutI16(B, N, (INT)E->Velocity.X);
    N = PutI16(B, N, (INT)E->Velocity.Y);
    N = PutU8 (B, N, (INT)E->EnemyMode);
    N = PutU8 (B, N, EnvIdx);
    N = PutF32(B, N, BotLookAt.X);
    N = PutF32(B, N, BotLookAt.Y);
    N = PutF32(B, N, BotLookAt.Z);
    N = PutU8 (B, N, bBotLookActive ? 1 : 0);
    N = PutU8 (B, N, BotFlags);

    GMpConn.SendBinary(B, N);
}

// ============================================================================
// SendSMT
// ============================================================================

void UEnemyChannel::SendSMT(AOLEnemyPawn* E, INT Idx)
{
    if (!E || !GMpConn.bIsConnected) return;

    FString EName = E->GetName();
    FLOAT   Now   = GWorld->GetWorldInfo()->TimeSeconds;

    // SMT 90 (bash loop) periodic re-send
    if (E->SpecialMove == 90
        && Idx < ControllerOwner->LastEnemySMT.Num()
        && ControllerOwner->LastEnemySMT(Idx) == 90
        && Idx < ControllerOwner->LastBashLoopSentTime.Num()
        && Now - ControllerOwner->LastBashLoopSentTime(Idx) >= 1.2f)
    {
        BYTE B[256]; INT N = 0;
        N = PutU8(B, N, CH_ENEMY);
        N = PutU8(B, N, ENEMY_SMT);
        N = PutStr(B, N, EName);
        const FEnpcSmtBody Body = { 90, 0, 0, 0, 0, 0 };
        appMemcpy(B + N, &Body, sizeof(Body));  N += sizeof(Body);
        GMpConn.SendBinary(B, N);
        ControllerOwner->LastBashLoopSentTime(Idx) = Now;
    }

    if (E->SpecialMove == 0)
    {
        // Notify receiver to cancel SMT if one was active
        if (ControllerOwner->LastEnemySMT(Idx) != 0)
        {
            BYTE B[256]; INT N = 0;
            N = PutU8(B, N, CH_ENEMY);
            N = PutU8(B, N, ENEMY_SMT);
            N = PutStr(B, N, EName);
            const FEnpcSmtBody Body = { 0, 0, 0, 0, 0, 0 };
            appMemcpy(B + N, &Body, sizeof(Body)); N += sizeof(Body);
            GMpConn.SendBinary(B, N);
        }
        ControllerOwner->LastEnemySMT(Idx) = 0;
        return;
    }
    if (E->SpecialMove == ControllerOwner->LastEnemySMT(Idx)) return;

    INT  SMTParam1 = 0, SMTParam2 = 0;
    UBOOL bSend   = TRUE;

    switch ((INT)E->SpecialMove)
    {
    case 71: SMTParam1 = (INT)E->AttackSide; SMTParam2 = E->bUsesDirectionalAttacks ? 1 : 0; break;
    case 72: break;
    case 73: SMTParam1 = (E->Bot && E->IsA(AOLEnemyNanoCloud::StaticClass())) ? 1 : 0;
             SMTParam2 = E->bCanThrow ? 1 : 0; break;
    case 74: SMTParam1 = E->Bot ? (INT)E->Bot->CurrentBedSide : 0;
             SMTParam2 = E->bCanThrow ? 1 : 0; break;
    case 85: SMTParam1 = (E->bUsingWeapon ? 1 : 0) | (E->WeaponType == 2 ? 2 : 0) | (E->bBackAnim ? 4 : 0); break;
    case 86: break;
    case 87: SMTParam1 = E->Bot ? (INT)E->Bot->CurrentBedSide : 0; break;
    case 89: SMTParam1 = (E->bUsingWeapon && !E->bHasWeaponEquipped) ? 1 : 0; break;
    case 90: ControllerOwner->LastBashLoopSentTime(Idx) = Now; break;
    case 91: SMTParam1 = (E->bUsingWeapon && !E->bHasWeaponEquipped) ? 1 : 0; break;
    case 95: SMTParam1 = (INT)(E->TurnAmount * 1000.0f); break;
    case 96: break;
    case 98: SMTParam1 = E->bLeftAnim ? 1 : 0; SMTParam2 = (INT)(E->SpecialMoveBlendAlpha * 1000.0f); break;
    case 28: SMTParam1 = CallGetDoorSMTFlags(E); break;
    default: bSend = FALSE; break;
    }

    if (bSend)
    {
        SendSMTDirect(E, (INT)E->SpecialMove, SMTParam1, SMTParam2);
    }
}

// ============================================================================
// SendSMTDirect
// ============================================================================

void UEnemyChannel::SendSMTDirect(AOLEnemyPawn* E, INT SMTType, INT Param1, INT Param2)
{
    if (!E || !GMpConn.bIsConnected) return;
    FString EName = E->GetName();

    AOLDoor* ActiveDoor = (E->Bot && E->Bot->ActiveDoor) ? E->Bot->ActiveDoor : NULL;

    BYTE B[256]; INT N = 0;
    N = PutU8(B, N, CH_ENEMY);
    N = PutU8(B, N, ENEMY_SMT);
    N = PutStr(B, N, EName);
    FEnpcSmtBody Body;
    Body.SMTType = (BYTE)SMTType;
    Body.Param1  = Param1;
    Body.Param2  = Param2;
    Body.DoorX   = ActiveDoor ? (INT)ActiveDoor->Location.X : 0;
    Body.DoorY   = ActiveDoor ? (INT)ActiveDoor->Location.Y : 0;
    Body.DoorZ   = ActiveDoor ? (INT)ActiveDoor->Location.Z : 0;
    appMemcpy(B + N, &Body, sizeof(Body)); N += sizeof(Body);
    GMpConn.SendBinary(B, N);

    for (INT i = 0; i < ControllerOwner->CachedEnemies.Num(); i++)
    {
        if (ControllerOwner->CachedEnemies(i) == E)
        {
            ControllerOwner->LastEnemySMT(i) = SMTType;
            break;
        }
    }
}

// ============================================================================
// SendEnemyDoorOpen / SendEnemyDoorDone
// ============================================================================

void UEnemyChannel::SendEnemyDoorOpen(AOLEnemyPawn* Enemy, AOLDoor* D, FLOAT Speed, FLOAT Angle)
{
    debugf(TEXT("### SendEnemyDoorOpen: Enemy=%s Door=%s Speed=%.1f Angle=%.1f Connected=%d"),
        Enemy ? *Enemy->GetName() : TEXT("NULL"),
        D ? *D->GetName() : TEXT("NULL"),
        Speed, Angle, (INT)GMpConn.bIsConnected);
    if (!Enemy || !D || !GMpConn.bIsConnected) return;
    FString EName = Enemy->GetName();
    BYTE B[256]; INT N = 0;
    N = PutU8(B, N, CH_ENEMY);
    N = PutU8(B, N, ENEMY_DOOR_OPEN);
    N = PutStr(B, N, EName);
    FEnpcDoorBody Body;
    Body.DoorX   = (INT)D->Location.X;
    Body.DoorY   = (INT)D->Location.Y;
    Body.DoorZ   = (INT)D->Location.Z;
    Body.Speed10 = appIsFinite(Speed) ? (short)Clamp((INT)(Speed * 10.f), -32768, 32767) : 32767;
    Body.Angle10 = (short)Clamp((INT)(Angle * 10.f), -32768, 32767);
    appMemcpy(B + N, &Body, sizeof(Body)); N += sizeof(Body);
    GMpConn.SendBinary(B, N);
}

void UEnemyChannel::SendEnemyDoorDone(AOLEnemyPawn* Enemy, AOLDoor* D, FLOAT CloseSpeed)
{
    if (!Enemy || !D || !GMpConn.bIsConnected) return;
    FString EName = Enemy->GetName();
    BYTE B[256]; INT N = 0;
    N = PutU8(B, N, CH_ENEMY);
    N = PutU8(B, N, ENEMY_DOOR_DONE);
    N = PutStr(B, N, EName);
    FEnpcDoorBody Body;
    Body.DoorX   = (INT)D->Location.X;
    Body.DoorY   = (INT)D->Location.Y;
    Body.DoorZ   = (INT)D->Location.Z;
    Body.Speed10 = appIsFinite(CloseSpeed) ? (short)Clamp((INT)(CloseSpeed * 10.f), -32768, 32767) : 32767;
    appMemcpy(B + N, &Body, sizeof(Body)); N += sizeof(Body);
    GMpConn.SendBinary(B, N);
}

void UEnemyChannel::SendEnemyDoorBash(AOLEnemyPawn* Enemy, AOLDoor* D, UBOOL bReversed)
{
    debugf(TEXT("### SendEnemyDoorBash: Enemy=%s Door=%s"), Enemy ? *Enemy->GetName() : TEXT("NULL"), D ? *D->GetName() : TEXT("NULL"));
    if (!Enemy || !D || !GMpConn.bIsConnected) return;
    FString EName = Enemy->GetName();
    BYTE B[256]; INT N = 0;
    N = PutU8(B, N, CH_ENEMY);
    N = PutU8(B, N, ENEMY_DOOR_BASH);
    N = PutStr(B, N, EName);
    FEnpcDoorBashBody Body; Body.DoorX = (INT)D->Location.X; Body.DoorY = (INT)D->Location.Y; Body.DoorZ = (INT)D->Location.Z; Body.bReversed = bReversed ? 1 : 0;
    appMemcpy(B + N, &Body, sizeof(Body)); N += sizeof(Body);
    GMpConn.SendBinary(B, N);
}

void UEnemyChannel::SendEnemyDoorBreak(AOLEnemyPawn* Enemy, AOLDoor* D, UBOOL bReversed)
{
    debugf(TEXT("### SendEnemyDoorBreak: Enemy=%s Door=%s"), Enemy ? *Enemy->GetName() : TEXT("NULL"), D ? *D->GetName() : TEXT("NULL"));
    if (!Enemy || !D || !GMpConn.bIsConnected) return;
    FString EName = Enemy->GetName();
    BYTE B[256]; INT N = 0;
    N = PutU8(B, N, CH_ENEMY);
    N = PutU8(B, N, ENEMY_DOOR_BREAK);
    N = PutStr(B, N, EName);
    FEnpcDoorBashBody Body; Body.DoorX = (INT)D->Location.X; Body.DoorY = (INT)D->Location.Y; Body.DoorZ = (INT)D->Location.Z; Body.bReversed = bReversed ? 1 : 0;
    appMemcpy(B + N, &Body, sizeof(Body)); N += sizeof(Body);
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// TickSend — called every game tick while connected
// ============================================================================

void UEnemyChannel::TickSend(FLOAT DeltaTime)
{
    if (!GMpConn.bIsConnected || !GMpConn.SyncEnemies || !HeroPawn)
        return;

    ControllerOwner->IndexEnemies();

    for (INT i = 0; i < ControllerOwner->CachedEnemies.Num(); i++)
    {
        AOLEnemyPawn* E = ControllerOwner->CachedEnemies(i);
        if (!E) continue;

        if (E->Health > 0 && !E->bDeleteMe && !E->bPendingDelete)
        {
            ControllerOwner->LastEnemyAlive(i) = TRUE;
            SendSpawn(E);

            // Gather bot look-at data and state flags
            INT     EnvIdx       = 0;
            FVector BotLookAt    = FVector(0.f, 0.f, 0.f);
            UBOOL   bLookActive  = FALSE;
            BYTE    BotFlags     = 0;
            if (E->Bot)
            {
                AOLBot* Bot = E->Bot;
                EnvIdx = (INT)Bot->CurrentEnvironment;
                if (Bot->TargetPlayer)
                {
                    BotLookAt   = Bot->TargetPlayer->EyeLocation;
                    bLookActive = (Bot->bEnableHeadTracking && Bot->SightComponent && Bot->SightComponent->CanSeeTarget) ? TRUE : FALSE;
                }
                if (Bot->bTurning)    BotFlags |= 1;
                if (Bot->bDisturbed)  BotFlags |= 2;
                if (Bot->bAnimating)  BotFlags |= 4;
            }

            SendEnemyLocBinary(E, EnvIdx, BotLookAt, bLookActive, BotFlags);
            SendSMT(E, i);
        }
        else if (ControllerOwner->LastEnemyAlive(i))
        {
            ControllerOwner->LastEnemyAlive(i) = FALSE;
            {
                FString EName = E->GetName();
                BYTE B[256]; INT N = 0;
                N = PutU8(B, N, CH_ENEMY);
                N = PutU8(B, N, ENEMY_DEL);
                N = PutStr(B, N, EName);
                GMpConn.SendBinary(B, N);
            }

            if (E->bDeleteMe || E->bPendingDelete)
            {
                ControllerOwner->CachedEnemies.Remove(i, 1);
                ControllerOwner->LastEnemyAlive.Remove(i, 1);
                ControllerOwner->LastEnemySMT.Remove(i, 1);
                ControllerOwner->LastBashLoopSentTime.Remove(i, 1);
                i--;
            }
        }
    }
}

// ============================================================================
// SendAllDeletes
// ============================================================================

void UEnemyChannel::SendAllDeletes()
{
    if (!GMpConn.bIsConnected) return;
    for (INT i = 0; i < ControllerOwner->CachedEnemies.Num(); i++)
    {
        AOLEnemyPawn* E = ControllerOwner->CachedEnemies(i);
        if (!E) continue;
        FString EName = E->GetName();
        BYTE B[256]; INT N = 0;
        N = PutU8(B, N, CH_ENEMY);
        N = PutU8(B, N, ENEMY_DEL);
        N = PutStr(B, N, EName);
        GMpConn.SendBinary(B, N);
    }
}

// ============================================================================
// BroadcastSpawns — re-sends all live enemies to a newly joined player
// ============================================================================

void UEnemyChannel::BroadcastSpawns()
{
    if (!GMpConn.bIsConnected || !HeroPawn) return;

    // Clear dedup so everything goes out
    ControllerOwner->SentEnemySpawns.Empty();

    for (INT i = 0; i < ControllerOwner->CachedEnemies.Num(); i++)
    {
        AOLEnemyPawn* E = ControllerOwner->CachedEnemies(i);
        if (!E || E->Health <= 0 || E->bDeleteMe || E->bPendingDelete) continue;
        SendSpawn(E);
    }
}

// ============================================================================
// Internal helpers shared by OnBinaryPacket
// ============================================================================

static void ApplySpawn(UEnemyChannel* Ch, INT SenderID,
    const FString& EnemyName, const FString& ClassName,
    FLOAT IX, FLOAT IY, FLOAT IZ, INT Yaw,
    const FString& MeshPath, INT WeaponIdx,
    UBOOL bHasColor, FLOAT CR, FLOAT CG, FLOAT CB, FLOAT CA)
{
    AMultiplayerController* Ctrl = Ch->ControllerOwner;
    if (!Ctrl || !GMpConn.SyncEnemies) return;

    for (INT j = 0; j < Ctrl->RemoteEnemies.Num(); j++)
        if (Ctrl->RemoteEnemies(j).NetName == EnemyName
            && Ctrl->RemoteEnemies(j).OwnerID == SenderID)
            return;

    FVector  NewLoc(IX, IY, IZ);
    FRotator NewRot(0, Yaw, 0);

    UClass* EClass = FindObject<UClass>(NULL, *(FString(TEXT("OLGame.")) + ClassName), FALSE);
    if (!EClass)
        EClass = LoadObject<UClass>(NULL, *(FString(TEXT("OLGame.")) + ClassName), NULL, LOAD_None, NULL);

    AOLEnemyPawn* NetEnemy = NULL;
    if (EClass && GWorld)
    {
        NetEnemy = Cast<AOLEnemyPawn>(GWorld->SpawnActor(EClass, NAME_None, NewLoc, NewRot,
            NULL, TRUE, FALSE, NULL, NULL));
        if (NetEnemy)
        {
            NetEnemy->Tag = FName(TEXT("MultiplayerDummyEnemy"));
            if (NetEnemy->Controller)
                GWorld->DestroyActor(NetEnemy->Controller);
            NetEnemy->SetCollision(FALSE, FALSE, FALSE);
            NetEnemy->bCollideWorld = FALSE;
            NetEnemy->Physics = PHYS_None;

            CallSetNetMesh(NetEnemy, MeshPath);
            CallSetNetWeapon(NetEnemy, WeaponIdx);

            if (bHasColor)
            {
                FLinearColor LC; LC.R = CR; LC.G = CG; LC.B = CB; LC.A = CA;
                NetEnemy->ApplyUniformColorOverride(LC);
            }

            CallBuildScriptAnimSetList(NetEnemy);
        }
    }

    FRemoteEnemyState S;
    S.NetName    = EnemyName;
    S.DummyEnemy = NetEnemy;
    S.OwnerID    = SenderID;
    S.TargetLoc  = NewLoc;
    S.TargetRot  = NewRot;
    S.TargetVel  = FVector(0.f, 0.f, 0.f);
    Ctrl->RemoteEnemies.AddItem(S);
}

static void ApplyDel(UEnemyChannel* Ch, INT SenderID, const FString& EnemyName)
{
    AMultiplayerController* Ctrl = Ch->ControllerOwner;
    if (!Ctrl) return;
    for (INT j = Ctrl->RemoteEnemies.Num() - 1; j >= 0; j--)
    {
        if (Ctrl->RemoteEnemies(j).NetName == EnemyName
            && Ctrl->RemoteEnemies(j).OwnerID == SenderID)
        {
            if (Ctrl->RemoteEnemies(j).DummyEnemy)
                GWorld->DestroyActor(Ctrl->RemoteEnemies(j).DummyEnemy);
            Ctrl->RemoteEnemies.Remove(j, 1);
            break;
        }
    }
}

static void ApplySmt(UEnemyChannel* Ch, INT SenderID,
    const FString& EnemyName, INT SMTType, INT Param1, INT Param2)
{
    AMultiplayerController* Ctrl = Ch->ControllerOwner;
    if (!Ctrl) return;
    for (INT j = 0; j < Ctrl->RemoteEnemies.Num(); j++)
    {
        FRemoteEnemyState& RS = Ctrl->RemoteEnemies(j);
        if (RS.NetName == EnemyName && RS.OwnerID == SenderID && RS.DummyEnemy)
        {
            // On SMT cancel, release any door reservation
            if (SMTType == 0)
            {
                for (INT di = 0; di < Ctrl->CachedDoors.Num(); di++)
                {
                    AOLDoor* D = Cast<AOLDoor>(Ctrl->CachedDoors(di));
                    if (D && D->DoorUser == RS.DummyEnemy && D->DoorState == DS_Idle)
                    {
                        D->DoorUser = NULL;
                        D->bNetDrivenMove = FALSE;
                        break;
                    }
                }
            }
            CallPlayNetSMT(RS.DummyEnemy, SMTType, Param1, Param2);
            break;
        }
    }
}

static void ApplyDoorEvent(UEnemyChannel* Ch, INT SenderID,
    const FString& EnemyName, INT DX, INT DY, INT DZ, UBOOL bDone, FLOAT Speed, FLOAT Angle)
{
    AMultiplayerController* Ctrl = Ch->ControllerOwner;
    if (!Ctrl) return;

    if (!Ctrl->bDoorsIndexed || Ctrl->CachedDoors.Num() == 0)
        Ctrl->IndexDoors();

    // Find door
    AOLDoor* Door = NULL;
    for (INT i = 0; i < Ctrl->CachedDoors.Num(); i++)
    {
        AOLDoor* D = Cast<AOLDoor>(Ctrl->CachedDoors(i));
        if (D && (INT)D->Location.X == DX && (INT)D->Location.Y == DY && (INT)D->Location.Z == DZ)
        {
            Door = D;
            break;
        }
    }

    debugf(TEXT("### ApplyDoorEvent: Enemy=%s bDone=%d Speed=%.1f Door=%s CachedDoors=%d Seek=(%d,%d,%d)"),
        *EnemyName, (INT)bDone, Speed, Door ? *Door->GetName() : TEXT("NULL"), Ctrl->CachedDoors.Num(), DX, DY, DZ);
    if (!Door) return;

    // Find dummy enemy
    AOLEnemyPawn* Dummy = NULL;
    for (INT j = 0; j < Ctrl->RemoteEnemies.Num(); j++)
    {
        if (Ctrl->RemoteEnemies(j).NetName == EnemyName
            && Ctrl->RemoteEnemies(j).OwnerID == SenderID)
        {
            Dummy = Ctrl->RemoteEnemies(j).DummyEnemy;
            break;
        }
    }

    if (!bDone)
    {
        // ENPC_DOOR_OPEN: open door and assign DoorUser = dummy enemy
        UBOOL bCanOpen =
            Door->DoorUser == NULL ||
            Door->DoorUser == Dummy ||
            (Cast<AOLHero>(Door->DoorUser) && ((AOLHero*)Door->DoorUser)->bIsDummyPawn);
        debugf(TEXT("### ApplyDoorEvent OPEN: SyncInteractable=%d bCanOpen=%d DoorState=%d Dummy=%s"),
            (INT)GMpConn.SyncInteractable, (INT)bCanOpen, (INT)Door->DoorState,
            Dummy ? *Dummy->GetName() : TEXT("NULL"));
        if (GMpConn.SyncInteractable && bCanOpen
            && (Door->DoorState == DS_Idle || Door->DoorState == DS_Closing
                || Door->DoorState == DS_PlayerInteracting))
        {
            Door->bNetDrivenMove = TRUE;
            FLOAT TargetAngle = Angle > 0.f ? Angle : Door->MaxOpenAngle;
            Door->Open(Dummy, Speed > 0.f ? Speed : Door->OpeningSpeed, TargetAngle, TRUE);
            Door->DoorUser = Dummy;
        }
    }
    else
    {
        // ENPC_DOOR_DONE: AnimNotify_Door DI_Close fires close on sender — replicate here
        UBOOL bOwnedByDummy = Door->DoorUser && Cast<AOLEnemyPawn>(Door->DoorUser)
            && Cast<AOLEnemyPawn>(Door->DoorUser)->Tag == FName(TEXT("MultiplayerDummyEnemy"));
        debugf(TEXT("### ApplyDoorEvent DONE: Speed=%.1f bOwnedByDummy=%d DoorState=%d DoorUser=%s"),
            Speed, (INT)bOwnedByDummy, (INT)Door->DoorState,
            Door->DoorUser ? *Door->DoorUser->GetName() : TEXT("NULL"));
        if (bOwnedByDummy)
        {
            if (Speed > 0.f)
                Door->Close(Dummy, Speed, AOLDoor::CST_NoSound);
            Door->DoorUser = NULL;
        }
    }
}

static void ApplyDoorBashBreak(UEnemyChannel* Ch, INT SenderID,
    const FString& EnemyName, INT DX, INT DY, INT DZ, UBOOL bReversed, UBOOL bBreak)
{
    debugf(TEXT("### ApplyDoorBashBreak: bBreak=%d DX=%d DY=%d DZ=%d"), bBreak, DX, DY, DZ);
    AMultiplayerController* Ctrl = Ch->ControllerOwner;
    if (!Ctrl) return;

    AOLDoor* Door = NULL;
    for (INT i = 0; i < Ctrl->CachedDoors.Num(); i++)
    {
        AOLDoor* D = Cast<AOLDoor>(Ctrl->CachedDoors(i));
        if (D && (INT)D->Location.X == DX && (INT)D->Location.Y == DY && (INT)D->Location.Z == DZ)
        {
            Door = D;
            break;
        }
    }
    debugf(TEXT("### ApplyDoorBashBreak: Door=%s CachedDoors=%d"), Door ? *Door->GetName() : TEXT("NULL"), Ctrl->CachedDoors.Num());
    if (!Door) return;

    AOLEnemyPawn* Dummy = NULL;
    for (INT j = 0; j < Ctrl->RemoteEnemies.Num(); j++)
    {
        if (Ctrl->RemoteEnemies(j).NetName == EnemyName
            && Ctrl->RemoteEnemies(j).OwnerID == SenderID)
        {
            Dummy = Ctrl->RemoteEnemies(j).DummyEnemy;
            break;
        }
    }

    if (bBreak)
        Door->eventBreakDoor(Dummy, bReversed);
    else
        Door->eventBashDoor(bReversed);
}

// ============================================================================
// OnRequestEnemies — received when another client joins and asks for state
// ============================================================================

void UEnemyChannel::OnRequestEnemies(INT SenderID)
{
    BroadcastSpawns();
}

// ============================================================================
// OnBinaryLoc — MPKT_ENPC_LOC from remote client
// ============================================================================

void UEnemyChannel::OnBinaryLoc(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < 6) return;
    if (!GMpConn.SyncEnemies) return;

    INT Off = 0;
    INT NameLen = 0;
    Off = ReadU8(Data, Off, NameLen);

    // Read name bytes
    if (Off + NameLen + 1 > DataLen) return;
    TCHAR NameBuf[33];
    INT   c;
    for (c = 0; c < NameLen && c < 32; c++)
        NameBuf[c] = (TCHAR)Data[Off + c];
    NameBuf[c] = 0;
    FString EnemyName(NameBuf);
    Off += NameLen + 1;  // skip name bytes + null terminator

    if (Off + 23 > DataLen) return;

    INT Yaw, VX, VY, EMode, EnvIdx, LookActive, BotFlags;
    FLOAT IX, IY, IZ, LX, LY, LZ;
    Off = ReadF32(Data, Off, IX);
    Off = ReadF32(Data, Off, IY);
    Off = ReadF32(Data, Off, IZ);
    Off = ReadU16(Data, Off, Yaw);
    Off = ReadI16(Data, Off, VX);
    Off = ReadI16(Data, Off, VY);
    Off = ReadU8 (Data, Off, EMode);
    Off = ReadU8 (Data, Off, EnvIdx);
    Off = ReadF32(Data, Off, LX);
    Off = ReadF32(Data, Off, LY);
    Off = ReadF32(Data, Off, LZ);
    Off = ReadU8 (Data, Off, LookActive);
    Off = ReadU8 (Data, Off, BotFlags);

    AOLEnemyPawn* NetEnemy = NULL;
    INT           FoundJ   = -1;
    for (INT j = 0; j < ControllerOwner->RemoteEnemies.Num(); j++)
    {
        if (ControllerOwner->RemoteEnemies(j).NetName == EnemyName
            && ControllerOwner->RemoteEnemies(j).OwnerID == SenderID)
        {
            NetEnemy = ControllerOwner->RemoteEnemies(j).DummyEnemy;
            FoundJ   = j;
            break;
        }
    }

    if (!NetEnemy || FoundJ < 0) return;

    FRemoteEnemyState& RS = ControllerOwner->RemoteEnemies(FoundJ);
    RS.TargetLoc  = FVector(IX, IY, IZ);
    RS.TargetRot  = FRotator(0, Yaw, 0);
    RS.TargetVel  = FVector(VX, VY, 0.f);

    UBOOL bWasAnimating = RS.bAnimating;
    RS.bAnimating = (BotFlags & 4) ? TRUE : FALSE;

    // Sender interrupted the animation (e.g. spotted player mid-anim) — stop it on dummy too
    if (bWasAnimating && !RS.bAnimating
        && NetEnemy->FullBodyAnimSlot != NULL && NetEnemy->bIsAnimatingFullBody)
    {
        NetEnemy->FullBodyAnimSlot->StopCustomAnim(0.1f);
        NetEnemy->bIsAnimatingFullBody = FALSE;
    }

    if (EMode != (INT)NetEnemy->EnemyMode
        && !((EMode == 1 || EMode == 2) && NetEnemy->bInSqueezeAttackChain))
    {
        CallSetEnemyMode(NetEnemy, EMode);
    }

    CallSetNetEnvironment(NetEnemy, EnvIdx);
    NetEnemy->SetNetLookAt(FVector(LX, LY, LZ), LookActive != 0);
}

// ============================================================================
// OnBinaryPacket — dispatch for all MPKT_ENPC_* except LOC
// ============================================================================

void UEnemyChannel::OnBinaryPacket(INT SenderID, BYTE PktType, BYTE* Data, INT DataLen)
{
    if (!ControllerOwner) return;

    INT     Off = 0;
    FString EnemyName;

    switch (PktType)
    {
    case ENEMY_SPAWN:
    {
        // [name][class][X(4)][Y(4)][Z(4)][Yaw(2)][mesh][weapon(1)][bColor(1)]
        // if bColor: [R(2)][G(2)][B(2)][A(2)]
        FString ClassName, MeshPath;
        FLOAT IX = 0, IY = 0, IZ = 0;
        INT Yaw = 0, Weapon = 0, HasColor = 0;
        Off = ReadStr(Data, Off, DataLen, EnemyName);
        Off = ReadStr(Data, Off, DataLen, ClassName);
        Off = ReadF32(Data, Off, IX);
        Off = ReadF32(Data, Off, IY);
        Off = ReadF32(Data, Off, IZ);
        Off = ReadU16(Data, Off, Yaw);
        Off = ReadStr(Data, Off, DataLen, MeshPath);
        Off = ReadU8 (Data, Off, Weapon);
        Off = ReadU8 (Data, Off, HasColor);
        FLOAT CR = 0, CG = 0, CB = 0, CA = 1;
        if (HasColor)
        {
            INT IR = 0, IG = 0, IB = 0, IA = 0;
            Off = ReadI16(Data, Off, IR);
            Off = ReadI16(Data, Off, IG);
            Off = ReadI16(Data, Off, IB);
            Off = ReadI16(Data, Off, IA);
            CR = IR / 1000.0f; CG = IG / 1000.0f;
            CB = IB / 1000.0f; CA = IA / 1000.0f;
        }
        ApplySpawn(this, SenderID, EnemyName, ClassName,
            IX, IY, IZ, Yaw, MeshPath, Weapon,
            HasColor != 0, CR, CG, CB, CA);
        break;
    }
    case ENEMY_DEL:
    {
        Off = ReadStr(Data, Off, DataLen, EnemyName);
        ApplyDel(this, SenderID, EnemyName);
        break;
    }
    case ENEMY_SMT:
    {
        // [name][SMTType(1)][Param1(4)][Param2(4)][DoorX(4)][DoorY(4)][DoorZ(4)]
        Off = ReadStr(Data, Off, DataLen, EnemyName);
        if (Off + (INT)sizeof(FEnpcSmtBody) > DataLen) break;
        const FEnpcSmtBody* Body = (const FEnpcSmtBody*)(Data + Off);
        ApplySmt(this, SenderID, EnemyName, (INT)Body->SMTType, Body->Param1, Body->Param2);
        // Reserve the door for the duration of the SMT to block player interaction
        if (Body->DoorX != 0 || Body->DoorY != 0 || Body->DoorZ != 0)
        {
            if (!ControllerOwner->bDoorsIndexed || ControllerOwner->CachedDoors.Num() == 0)
                ControllerOwner->IndexDoors();
            for (INT di = 0; di < ControllerOwner->CachedDoors.Num(); di++)
            {
                AOLDoor* D = Cast<AOLDoor>(ControllerOwner->CachedDoors(di));
                if (D && (INT)D->Location.X == Body->DoorX
                      && (INT)D->Location.Y == Body->DoorY
                      && (INT)D->Location.Z == Body->DoorZ)
                {
                    // Find dummy enemy to set as DoorUser
                    for (INT j = 0; j < ControllerOwner->RemoteEnemies.Num(); j++)
                    {
                        FRemoteEnemyState& RS = ControllerOwner->RemoteEnemies(j);
                        if (RS.NetName == EnemyName && RS.OwnerID == SenderID && RS.DummyEnemy)
                        {
                            D->DoorUser = RS.DummyEnemy;
                            D->bNetDrivenMove = TRUE;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        break;
    }
    case ENEMY_DOOR_OPEN:
    case ENEMY_DOOR_DONE:
    {
        // [name][DoorX(4)][DoorY(4)][DoorZ(4)][Speed10(2)][Angle10(2)]
        Off = ReadStr(Data, Off, DataLen, EnemyName);
        if (Off + (INT)sizeof(FEnpcDoorBody) > DataLen) break;
        const FEnpcDoorBody* Body = (const FEnpcDoorBody*)(Data + Off);
        FLOAT Speed = (Body->Speed10 != 0) ? (FLOAT)Body->Speed10 / 10.f : 0.f;
        FLOAT Angle = (Body->Angle10 != 0) ? (FLOAT)Body->Angle10 / 10.f : 0.f;
        ApplyDoorEvent(this, SenderID, EnemyName,
            Body->DoorX, Body->DoorY, Body->DoorZ,
            PktType == ENEMY_DOOR_DONE, Speed, Angle);
        break;
    }
    case ENEMY_DOOR_BASH:
    case ENEMY_DOOR_BREAK:
    {
        // [name][DoorX(4)][DoorY(4)][DoorZ(4)][bReversed(1)]
        Off = ReadStr(Data, Off, DataLen, EnemyName);
        if (Off + (INT)sizeof(FEnpcDoorBashBody) > DataLen) break;
        const FEnpcDoorBashBody* Body = (const FEnpcDoorBashBody*)(Data + Off);
        ApplyDoorBashBreak(this, SenderID, EnemyName,
            Body->DoorX, Body->DoorY, Body->DoorZ,
            Body->bReversed != 0, PktType == ENEMY_DOOR_BREAK);
        break;
    }
    default:
        break;
    }
}
