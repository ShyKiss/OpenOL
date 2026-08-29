/*=============================================================================
    OLImGui_Spawns.cpp — "Spawns" tab: enemy spawn interface via SpawnEnemy cheat.
=============================================================================*/
#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// Enemy mesh table — matches GEnemyNames order exactly.
// Package and ObjectPath as used by LoadObjectFromModPackage.
// ---------------------------------------------------------------------------

// Mesh + AnimSet lookup table — matches GEnemyNames order.
// Package "" = already in memory. AnimSetObject = "Package.Object" for FindObject.
struct FEnemyMeshEntry
{
    const char* Package;
    const char* ObjectPath;
    const char* AnimSetPackage;       // package for LoadObjectFromModPackage
    const char* AnimSetObject;        // animset used when no weapon selected
    const char* AnimSetWeaponObject;  // animset used when a weapon is selected (NULL = same as above)
    const char* IdleAnim;             // animation sequence name to play looping
};
// AnimSetWeaponObject: only NpcMedium enemies switch to the weapon animset.
// Soldier/Groom/NanoCloud use Soldier_V2 which has no weapon variant — leave NULL.
static const FEnemyMeshEntry GEnemyMeshTable[] =
{
    { "",                        "02_Cannibal.Cannibal",              "", "03_NPCMedium_Generic.NpcMediumGeneric-01_AS","03_NPCMedium_Generic.NpcMediumGenericWeapon-01_AS","Idle" }, // Cannibal
    { "",                        "02_Soldier.Pawn.Soldier-03",        "", "03_Soldier.Soldier_V2",                      NULL,                                                "Idle" }, // Soldier
    { "DLC_Build2Exterior-01_LD","02_Groom.Groom_Shirt",             "", "03_Soldier.Soldier_V2",                      NULL,                                                "Idle" }, // Groom
    { "",                        "02_Priest.Pawn.Priest-01",          "", "03_NPCMedium_Generic.NpcMediumGeneric-01_AS","03_NPCMedium_Generic.NpcMediumGenericWeapon-01_AS","Idle" }, // Priest
    { "Male_ward_SE",            "02_Surgeon.Mesh.Surgeon",           "", "03_NPCMedium_Generic.NpcMediumGeneric-01_AS","03_NPCMedium_Generic.NpcMediumGenericWeapon-01_AS","Idle" }, // Surgeon
    { "",                        "02_NanoCloud.Pawn.Nano_Swarm_body", "", "03_Soldier.Soldier_V2",                      NULL,                                                "Idle" }, // NanoCloud
};

// ---------------------------------------------------------------------------
// Spawn state
// ---------------------------------------------------------------------------

static int  GSpawnEnemyIdx   = 1; // default: Soldier
static int  GSpawnWeaponIdx  = 0; // default: None
static bool GSpawnShouldAtk  = true;
static int  GSpawnCount      = 1; // how many to spawn at once

static const char* GEnemyNames[] =
{
    "Cannibal", "Soldier", "Groom", "Priest", "Surgeon", "NanoCloud"
};
static const char* GWeaponNames[] =
{
    "None", "Knife", "Butcher Knife", "Bone Shear",
    "Machete", "Night Stick", "Pipe", "Wood Plank", "Cannibal Drill"
};

// ---------------------------------------------------------------------------
// Weapon static mesh table — matches GWeaponNames order (Weapon_None first).
// Package "" = already in memory. ObjectPath = Package.Object inside UE3.
// ---------------------------------------------------------------------------

struct FWeaponMeshEntry { const char* Package; const char* ObjectPath; };
static const FWeaponMeshEntry GWeaponMeshTable[] =
{
    { "",  ""                                  }, // None
    { "",  "Weapons.Knife-01"                  }, // Knife
    { "",  "Weapons.ButcherKnife-01"           }, // Butcher Knife
    { "",  "Weapons.bone_shear-01"             }, // Bone Shear
    { "",  "Weapons.Machete-01"                }, // Machete
    { "",  "Weapons.Nightstick-01"             }, // Night Stick
    { "",  "Weapons.PipeWeapon-01"             }, // Pipe
    { "",  "Weapons.WoodPlankWeapon-01"        }, // Wood Plank
    { "",  "02_Cannibal.cannibal_drill"        }, // Cannibal Drill
};

// ---------------------------------------------------------------------------
// Enemy preview mesh cache — loaded lazily, indexed by GEnemyNames.
// NULL = not yet queried, (USkeletalMesh*)-1 = not found.
// All cached pointers are pinned with AddToRoot so GC never collects them.
// ShutdownPreviews() calls RemoveFromRoot before clearing.
// ---------------------------------------------------------------------------

static USkeletalMesh* GPreviewMeshCache[ARRAY_COUNT(GEnemyNames)] = {};
static int            GPreviewLoadedIdx  = -1;
static int            GPreviewPendingIdx = -1;

// ---------------------------------------------------------------------------
// Weapon preview mesh cache — indexed by GWeaponNames.
// NULL = not queried, (UStaticMesh*)-1 = not found.
// ---------------------------------------------------------------------------

static UStaticMesh*     GWeaponMeshCache[ARRAY_COUNT(GWeaponNames)] = {};
static int              GWeaponAttachedIdx      = -1; // weapon currently attached to GPreviewMain
static volatile int     GWeaponHoverIdx         = -1; // weapon hovered in combo (render thread → game thread)

// Separate preview instance for weapon hover tooltip only.
static FOLPreviewInstance GPreviewWeapon;
static int              GWeaponPreviewLoadedIdx = -1;

// Load mesh for GPreviewPendingIdx into cache and push to preview system.
static void LoadAndSetPreviewMesh()
{
    int Idx = GPreviewPendingIdx;
    if (Idx < 0 || Idx >= (int)ARRAY_COUNT(GEnemyNames))
        return;

    if (!GPreviewMeshCache[Idx])
    {
        const FEnemyMeshEntry& E = GEnemyMeshTable[Idx];
        UObject* Loaded = Utils::LoadObjectFromModPackage(
            FString(ANSI_TO_TCHAR(E.Package)),
            FString(ANSI_TO_TCHAR(E.ObjectPath)),
            USkeletalMesh::StaticClass());
        USkeletalMesh* M = Loaded ? Cast<USkeletalMesh>(Loaded) : NULL;
        if (M) M->AddToRoot(); // pin against GC for the lifetime of this cache slot
        GPreviewMeshCache[Idx] = M ? M : (USkeletalMesh*)(PTRINT)(-1);
    }

    USkeletalMesh* Mesh = GPreviewMeshCache[Idx];
    if ((PTRINT)Mesh == (PTRINT)(-1))
        Mesh = NULL;

    const FEnemyMeshEntry& E2 = GEnemyMeshTable[Idx];
    const bool bWeapon = (GSpawnWeaponIdx > 0) && (E2.AnimSetWeaponObject != NULL);
    const char* AnimSetObj = bWeapon ? E2.AnimSetWeaponObject : E2.AnimSetObject;
    GPreviewMain.SetSkeletalMeshWithAnim(Mesh, E2.AnimSetPackage, AnimSetObj, E2.IdleAnim);
    GPreviewLoadedIdx  = Idx;
    GWeaponAttachedIdx = -1; // force weapon re-attach after mesh change
}

// Hovered index in combo — -1 means no hover (written render thread, read game thread).
static volatile int GPreviewHoverIdx     = -1;
static int          GPreviewHoverLoaded  = -1; // index currently in hover instance

// Separate preview instance for hover tooltip.
static FOLPreviewInstance GPreviewHover;

// Called from game thread each tick to keep both preview instances up to date.
// Load weapon static mesh for index Idx into cache; returns mesh or NULL.
static UStaticMesh* LoadWeaponMesh(int Idx)
{
    if (Idx <= 0 || Idx >= (int)ARRAY_COUNT(GWeaponNames))
        return NULL; // Weapon_None or out of range

    if (!GWeaponMeshCache[Idx])
    {
        const FWeaponMeshEntry& E = GWeaponMeshTable[Idx];
        if (!E.ObjectPath[0])
        {
            GWeaponMeshCache[Idx] = (UStaticMesh*)(PTRINT)(-1);
        }
        else
        {
            UObject* Loaded = Utils::LoadObjectFromModPackage(
                FString(ANSI_TO_TCHAR(E.Package)),
                FString(ANSI_TO_TCHAR(E.ObjectPath)),
                UStaticMesh::StaticClass());
            UStaticMesh* M = Loaded ? Cast<UStaticMesh>(Loaded) : NULL;
            if (M) M->AddToRoot(); // pin against GC
            GWeaponMeshCache[Idx] = M ? M : (UStaticMesh*)(PTRINT)(-1);
        }
    }

    UStaticMesh* M = GWeaponMeshCache[Idx];
    return ((PTRINT)M == (PTRINT)(-1)) ? NULL : M;
}

// ---------------------------------------------------------------------------
// Enemy list snapshot — built on game thread, read on render thread.
// Double-buffer with atomic pointer swap: game thread writes to the inactive
// buffer and publishes it via a volatile pointer. Render thread snapshots the
// pointer once per frame into a local — no partial reads possible.
// ---------------------------------------------------------------------------

struct FEnemyEntry
{
    char  Name[64];
    int   Health;
    int   HealthMax;
    float LocX, LocY, LocZ;
};

static const int MAX_ENEMY_SNAP = 128;

struct FEnemyListSnap
{
    FEnemyEntry Entries[MAX_ENEMY_SNAP];
    int         Count;
};

// Two buffers; game thread always writes to the one NOT currently published.
static FEnemyListSnap          GEnemyBuf[2];
static volatile int            GEnemyReadIdx  = -1; // -1 = no data yet
static int                     GEnemyWriteIdx = 0;  // game thread writes here


// Called on game thread each tick to build a safe render-thread snapshot.
// Iterates only PersistentLevel->Actors directly instead of using FActorIterator
// to avoid any interaction with GWorld->Levels (which may be in transition).
static void RebuildEnemyList()
{
    if (!GWorld || !GWorld->PersistentLevel)
        return;

    ULevel* PL = GWorld->PersistentLevel;
    const int NumActors = PL->Actors.Num();
    if (NumActors <= 0)
        return;

    FEnemyListSnap Tmp;
    Tmp.Count = 0;

    static const FName TagDummy(TEXT("MultiplayerDummyEnemy"));
    for (int ai = 0; ai < PL->Actors.Num(); ++ai)
    {
        AActor* A = PL->Actors(ai);
        if (!A || A->IsPendingKill()) continue;
        AOLEnemyPawn* E = Cast<AOLEnemyPawn>(A);
        if (!E || E->Tag == TagDummy) continue;
        if (Tmp.Count >= MAX_ENEMY_SNAP) break;

        FEnemyEntry& En = Tmp.Entries[Tmp.Count++];
        En.Health    = E->Health;
        En.HealthMax = E->HealthMax;
        En.LocX      = E->Location.X;
        En.LocY      = E->Location.Y;
        En.LocZ      = E->Location.Z;
        const FString EName = E->GetFName().ToString();
        const TCHAR* N = *EName;
        int ni = 0;
        while (ni < 63 && N[ni]) { En.Name[ni] = (char)N[ni]; ++ni; }
        En.Name[ni] = '\0';
    }

    GEnemyBuf[GEnemyWriteIdx] = Tmp;
    GEnemyReadIdx  = GEnemyWriteIdx;
    GEnemyWriteIdx = 1 - GEnemyWriteIdx;
}

static void DoSpawnEnemyAtGhost(AOLPlayerController* PC);

// ---------------------------------------------------------------------------
// Placement overlay snapshot — written on game thread, read on render thread.
// ---------------------------------------------------------------------------
struct FPlacementSnapshot
{
    bool    bActive;
    FVector Center;
    float   Radius;
    float   FloorZ;
    FMatrix ViewProjMatrix;
    float   ScreenSizeX;
    float   ScreenSizeY;
};
static FPlacementSnapshot GPlacementSnapshot; // written game thread, read render thread

void OLSpawns_TickPlacement()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC && PC->bPlacementConfirmPending)
    {
        PC->bPlacementConfirmPending = FALSE;
        DoSpawnEnemyAtGhost(PC);
    }

    // Update placement overlay snapshot for render thread.
    FPlacementSnapshot Snap;
    Snap.bActive = false;
    if (PC && PC->bPlacementMode && PC->PlacementGhost && !PC->PlacementGhost->bDeleteMe)
    {
        // Find skeletal mesh component to get real world bounds (same as Show Bounds).
        FBoxSphereBounds SkelBounds(PC->PlacementGhost->Location, FVector(60.f, 60.f, 90.f), 90.f);
        for (INT i = 0; i < PC->PlacementGhost->Components.Num(); ++i)
        {
            USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(PC->PlacementGhost->Components(i));
            if (Skel && Skel->SkeletalMesh)
            {
                SkelBounds = Skel->Bounds;
                break;
            }
        }

        FVector CamLoc; FRotator CamRot;
        PC->eventGetPlayerViewPoint(CamLoc, CamRot);

        UINT SX = 0, SY = 0;
        if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
        {
            SX = GEngine->GameViewport->Viewport->GetSizeX();
            SY = GEngine->GameViewport->Viewport->GetSizeY();
        }

        const float FOVRad  = PC->FOVAngle * (float)PI / 180.f;
        const float AspectR = SY > 0 ? (float)SX / (float)SY : 1.f;
        const float Near    = 10.f, Far = 100000.f;
        const float T       = appTan(FOVRad * 0.5f);

        FMatrix View = FTranslationMatrix(-CamLoc) *
                       FRotationMatrix(CamRot).Inverse() *
                       FMatrix(FPlane(0,0,1,0),FPlane(1,0,0,0),FPlane(0,1,0,0),FPlane(0,0,0,1));
        FMatrix Proj(
            FPlane(1.f/(AspectR*T), 0,       0,                    0),
            FPlane(0,               1.f/T,   0,                    0),
            FPlane(0,               0,       Far/(Far-Near),       1),
            FPlane(0,               0,      -Near*Far/(Far-Near),  0));

        // Ellipse at bottom of bounds (floor level), radius = XY extent.
        Snap.bActive        = true;
        Snap.Center         = SkelBounds.Origin;
        Snap.Radius         = Max(SkelBounds.BoxExtent.X, SkelBounds.BoxExtent.Y);
        Snap.FloorZ         = SkelBounds.Origin.Z - SkelBounds.BoxExtent.Z;
        Snap.ViewProjMatrix = View * Proj;
        Snap.ScreenSizeX    = (float)SX;
        Snap.ScreenSizeY    = (float)SY;
    }
    GPlacementSnapshot = Snap;
}

// Project a world point to ImGui screen coords. Returns false if behind camera.
static bool WorldToImGui(const FVector& WorldPt,
                          const FMatrix& ViewProjMatrix,
                          float SX, float SY,
                          ImVec2& OutScreen)
{
    FPlane Clip = ViewProjMatrix.TransformFVector4(FVector4(WorldPt.X, WorldPt.Y, WorldPt.Z, 1.f));
    if (Clip.W <= 0.f) return false;
    float InvW = 1.f / Clip.W;
    // NDC [-1,1] -> pixel
    OutScreen.x = (Clip.X * InvW * 0.5f + 0.5f) * SX;
    OutScreen.y = (1.f - (Clip.Y * InvW * 0.5f + 0.5f)) * SY;
    return true;
}

void OLSpawns_DrawPlacementOverlay()
{
    const FPlacementSnapshot Snap = GPlacementSnapshot;
    if (!Snap.bActive || Snap.ScreenSizeX <= 0.f || Snap.ScreenSizeY <= 0.f)
        return;

    const int   N      = 48;
    const float TWO_PI = 6.2831853f;
    ImDrawList* DL     = ImGui::GetBackgroundDrawList();
    const ImU32 Col    = IM_COL32(255, 220, 60, 200);

    ImVec2 pts[N];
    bool   valid[N];
    for (int i = 0; i < N; ++i)
    {
        float   Angle   = (TWO_PI * i) / N;
        FVector WorldPt = FVector(Snap.Center.X + Snap.Radius * appCos(Angle),
                                  Snap.Center.Y + Snap.Radius * appSin(Angle),
                                  Snap.FloorZ);
        valid[i] = WorldToImGui(WorldPt, Snap.ViewProjMatrix, Snap.ScreenSizeX, Snap.ScreenSizeY, pts[i]);
    }

    for (int i = 0; i < N; ++i)
    {
        int j = (i + 1) % N;
        if (valid[i] && valid[j])
            DL->AddLine(pts[i], pts[j], Col, 2.f);
    }
}

void OLSpawns_TickPreviews()
{
    // Rebuild enemy list snapshot on game thread each tick.
    RebuildEnemyList();

    // Lazy init of main preview.
    if (!GPreviewMain.IsInited())
        GPreviewMain.Init();

    // Main preview — selected enemy + weapon (animset changes when weapon toggled).
    static int GLastWeaponIdx = -1;
    if (GPreviewLoadedIdx != GSpawnEnemyIdx || GLastWeaponIdx != GSpawnWeaponIdx)
    {
        GLastWeaponIdx     = GSpawnWeaponIdx;
        GPreviewPendingIdx = GSpawnEnemyIdx;
        LoadAndSetPreviewMesh();
    }
    // Attach weapon to main preview bone (or detach if none selected).
    if (GWeaponAttachedIdx != GSpawnWeaponIdx)
    {
        UStaticMesh* WMesh = (GSpawnWeaponIdx > 0) ? LoadWeaponMesh(GSpawnWeaponIdx) : NULL;
        // Only attach if this enemy supports weapon animset (Soldier_V2 enemies excluded).
        const bool bSupportsWeapon = (GEnemyMeshTable[GSpawnEnemyIdx].AnimSetWeaponObject != NULL);
        GPreviewMain.AttachWeaponToBone(
            (WMesh && bSupportsWeapon) ? WMesh : NULL,
            "NPCMedium-R-Hand_aux");
        GWeaponAttachedIdx = GSpawnWeaponIdx;
    }
    GPreviewMain.Tick(GOLPreviewYaw, GOLPreviewFOV);

    // Hover preview — hovered enemy in combo (no weapon attached, just idle pose).
    int HoverIdx = GPreviewHoverIdx;
    GPreviewHoverIdx = -1;
    if (HoverIdx >= 0)
    {
        if (!GPreviewHover.IsInited())
            GPreviewHover.Init();

        if (GPreviewHoverLoaded != HoverIdx)
        {
            if (!GPreviewMeshCache[HoverIdx])
            {
                const FEnemyMeshEntry& E = GEnemyMeshTable[HoverIdx];
                UObject* Loaded = Utils::LoadObjectFromModPackage(
                    FString(ANSI_TO_TCHAR(E.Package)),
                    FString(ANSI_TO_TCHAR(E.ObjectPath)),
                    USkeletalMesh::StaticClass());
                USkeletalMesh* M = Loaded ? Cast<USkeletalMesh>(Loaded) : NULL;
                if (M) M->AddToRoot(); // pin against GC
                GPreviewMeshCache[HoverIdx] = M ? M : (USkeletalMesh*)(PTRINT)(-1);
            }
            USkeletalMesh* HovMesh = GPreviewMeshCache[HoverIdx];
            if ((PTRINT)HovMesh == (PTRINT)(-1)) HovMesh = NULL;
            GPreviewHover.SetSkeletalMeshWithAnim(HovMesh,
                GEnemyMeshTable[HoverIdx].AnimSetPackage,
                GEnemyMeshTable[HoverIdx].AnimSetObject,
                GEnemyMeshTable[HoverIdx].IdleAnim);
            GPreviewHoverLoaded = HoverIdx;
        }
        GPreviewHover.Tick(GOLPreviewYaw, GOLPreviewFOV);
    }

    // Weapon hover tooltip — separate preview instance, only ticked when hovering.
    int WDesired = GWeaponHoverIdx;
    GWeaponHoverIdx = -1;
    if (WDesired > 0)
    {
        if (!GPreviewWeapon.IsInited())
            GPreviewWeapon.Init();

        if (GWeaponPreviewLoadedIdx != WDesired)
        {
            UStaticMesh* WMesh = LoadWeaponMesh(WDesired);
            GPreviewWeapon.SetStaticMesh(WMesh);
            GWeaponPreviewLoadedIdx = WDesired;
        }
        GPreviewWeapon.Tick(GOLPreviewYaw, GOLPreviewFOV);
    }
}

// ---------------------------------------------------------------------------
// Game-thread spawn action
// ---------------------------------------------------------------------------

// Called from placement confirm (LMB) — spawns enemy at Ghost location/rotation.
static void DoSpawnEnemyAtGhost(AOLPlayerController* PC)
{
    if (!PC || !PC->CheatManager || !PC->PlacementGhost)
        return;
    UOLCheatManager* CM = Cast<UOLCheatManager>(PC->CheatManager);
    if (!CM)
        return;

    const FVector  SpawnLoc = PC->PlacementGhost->Location;
    const FRotator SpawnRot = PC->PlacementGhost->Rotation;

    UFunction* SpawnFn = CM->FindFunctionChecked(FName(TEXT("SpawnEnemyAt")));
    const int Count = GSpawnCount > 0 ? GSpawnCount : 1;
    for (int i = 0; i < Count; ++i)
    {
        struct { FString EnemyType; BYTE WeaponToUse; UBOOL ShouldAttack; FVector SpawnLoc; FRotator SpawnRot; } P;
        P.EnemyType    = FString(ANSI_TO_TCHAR(GEnemyNames[GSpawnEnemyIdx]));
        P.WeaponToUse  = (BYTE)GSpawnWeaponIdx;
        P.ShouldAttack = GSpawnShouldAtk ? TRUE : FALSE;
        P.SpawnLoc     = SpawnLoc;
        P.SpawnRot     = SpawnRot;
        CM->ProcessEvent(SpawnFn, &P);
    }
}

static void Action_SpawnEnemy()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->CheatManager)
        return;
    UOLCheatManager* CM = Cast<UOLCheatManager>(PC->CheatManager);
    if (!CM)
        return;

    UFunction* Fn = CM->FindFunctionChecked(FName(TEXT("SpawnEnemy")));
    const int Count = GSpawnCount > 0 ? GSpawnCount : 1;
    for (int i = 0; i < Count; ++i)
    {
        // Re-create P each iteration: ProcessEvent may call the FString destructor
        // on the out-param frame, corrupting a reused struct on the next call.
        struct { FString EnemyType; BYTE WeaponToUse; UBOOL ShouldAttack; } P;
        P.EnemyType    = FString(ANSI_TO_TCHAR(GEnemyNames[GSpawnEnemyIdx]));
        P.WeaponToUse  = (BYTE)GSpawnWeaponIdx;
        P.ShouldAttack = GSpawnShouldAtk ? TRUE : FALSE;
        CM->ProcessEvent(Fn, &P);
    }
}

static void Action_BeginEnemyPlacement()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;

    // Spawn a placement ghost actor — no collision, no physics.
    UClass* GhostClass = FindObject<UClass>(ANY_PACKAGE, TEXT("OLPlacementGhost"));
    if (!GhostClass) return;
    AActor* Ghost = GWorld->SpawnActor(GhostClass, NAME_None,
        PC->Location, PC->Rotation);
    if (!Ghost) return;

    // Set skeletal mesh on the ghost's mesh component.
    const FEnemyMeshEntry& E = GEnemyMeshTable[GSpawnEnemyIdx];
    USkeletalMesh* SkelMesh = NULL;
    {
        UObject* Loaded = Utils::LoadObjectFromModPackage(
            FString(ANSI_TO_TCHAR(E.Package)),
            FString(ANSI_TO_TCHAR(E.ObjectPath)),
            USkeletalMesh::StaticClass());
        SkelMesh = Loaded ? Cast<USkeletalMesh>(Loaded) : NULL;
    }

    if (SkelMesh)
    {
        // Ghost is a bare Actor — attach a fresh SkeletalMeshComponent.
        USkeletalMeshComponent* SkelComp = ConstructObject<USkeletalMeshComponent>(
            USkeletalMeshComponent::StaticClass(), Ghost);
        SkelComp->bCastDynamicShadow = FALSE;
        SkelComp->CollideActors      = FALSE;
        SkelComp->BlockActors        = FALSE;

        // Load AnimSet before attaching so the tree is ready on the first rendered frame.
        const bool bWeapon = (GSpawnWeaponIdx > 0) && (E.AnimSetWeaponObject != NULL);
        const char* AnimSetObj = bWeapon ? E.AnimSetWeaponObject : E.AnimSetObject;
        UObject* ASLoaded = Utils::LoadObjectFromModPackage(
            FString(ANSI_TO_TCHAR(E.AnimSetPackage)),
            FString(ANSI_TO_TCHAR(AnimSetObj)),
            UAnimSet::StaticClass());
        UAnimSet* AnimSet = ASLoaded ? Cast<UAnimSet>(ASLoaded) : NULL;
        if (AnimSet)
        {
            UAnimNodeSequence* AnimNode = ConstructObject<UAnimNodeSequence>(
                UAnimNodeSequence::StaticClass(), SkelComp);
            AnimNode->bLooping                       = TRUE;
            AnimNode->bDisableWarningWhenAnimNotFound = TRUE;

            SkelComp->AnimSets.Empty();
            SkelComp->AnimSets.AddItem(AnimSet);
            SkelComp->Animations = AnimNode;

            SkelComp->SetSkeletalMesh(SkelMesh);
            Ghost->AttachComponent(SkelComp);

            SkelComp->UpdateAnimations();
            SkelComp->InitAnimTree(TRUE);
            AnimNode->SetAnim(FName(ANSI_TO_TCHAR(E.IdleAnim)));
            AnimNode->PlayAnim(TRUE, 1.0f, 0.0f);
        }
        else
        {
            SkelComp->SetSkeletalMesh(SkelMesh);
            Ghost->AttachComponent(SkelComp);
        }

        // Attach weapon StaticMesh to hand bone if selected and supported.
        const bool bSupportsWeapon = (E.AnimSetWeaponObject != NULL);
        if (GSpawnWeaponIdx > 0 && bSupportsWeapon)
        {
            UStaticMesh* WMesh = LoadWeaponMesh(GSpawnWeaponIdx);
            if (WMesh)
            {
                UStaticMeshComponent* WComp = ConstructObject<UStaticMeshComponent>(
                    UStaticMeshComponent::StaticClass(), Ghost);
                WComp->SetStaticMesh(WMesh);
                WComp->CollideActors  = FALSE;
                WComp->BlockActors    = FALSE;
                WComp->bCastDynamicShadow = FALSE;
                SkelComp->AttachComponent(WComp, FName(TEXT("NPCMedium-R-Hand_aux")));
            }
        }
    }

    PC->eventBeginPlacementMode(Ghost);
}

// ---------------------------------------------------------------------------
// Tab view state
// ---------------------------------------------------------------------------

enum ESpawnsView { SV_List, SV_New, SV_Enemy };
static ESpawnsView GSpawnsView         = SV_List;
static int         GSelectedEnemyIdx   = -1; // index into GEnemyList snapshot

// ---------------------------------------------------------------------------
// Enemy actions — executed on game thread via EnqueueCall.
// Actor is found by FName to avoid holding a stale pointer from render thread.
// ---------------------------------------------------------------------------

static char GPendingEnemyAction[64] = {}; // actor name for pending action

// Find enemy pawn in PersistentLevel by actor FName.
static AOLEnemyPawn* FindEnemyByName(const char* name)
{
    if (!GWorld || !GWorld->PersistentLevel) return NULL;
    FName Target(ANSI_TO_TCHAR(name));
    ULevel* PL = GWorld->PersistentLevel;
    for (int i = 0; i < PL->Actors.Num(); ++i)
    {
        AActor* A = PL->Actors(i);
        if (!A || A->IsPendingKill()) continue;
        AOLEnemyPawn* E = Cast<AOLEnemyPawn>(A);
        if (E && E->GetFName() == Target) return E;
    }
    return NULL;
}

static void Action_TeleportEnemy()
{
    AOLEnemyPawn* E = FindEnemyByName(GPendingEnemyAction);
    if (!E) return;
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->Pawn) return;
    FVector Loc = PC->Pawn->Location + PC->Pawn->Rotation.Vector() * 200.f;
    E->SetLocation(Loc);
}

static void Action_DeleteEnemy()
{
    AOLEnemyPawn* E = FindEnemyByName(GPendingEnemyAction);
    if (!E) return;
    GWorld->DestroyActor(E);
}

void OLSpawns_InvalidateMeshCache()
{
    // After CollectGarbage, raw pointers in our caches may point to freed objects.
    // Clear every slot so LoadAndSetPreviewMesh / LoadWeaponMesh reload them next time.
    for (int i = 0; i < (int)ARRAY_COUNT(GEnemyNames); ++i)
        GPreviewMeshCache[i] = NULL;
    for (int i = 0; i < (int)ARRAY_COUNT(GWeaponNames); ++i)
        GWeaponMeshCache[i] = NULL;

    // Drop AnimSet references from hover/weapon instances before GC runs.
    // GC may free the AnimSet that SkeletalComp->AnimSets[0] points to;
    // the next Tick() would then crash inside TickAnimNodes.
    GPreviewMain.ClearMesh();
    GPreviewHover.ClearMesh();
    GPreviewWeapon.ClearMesh();

    // Force preview re-load on the next tick.
    GPreviewLoadedIdx       = -1;
    GWeaponAttachedIdx      = -1;
    GPreviewHoverLoaded     = -1;
    GWeaponPreviewLoadedIdx = -1;

    // Clear both buffers so render thread sees an empty list immediately.
    GEnemyBuf[0].Count = 0;
    GEnemyBuf[1].Count = 0;
    GEnemyReadIdx  = -1;
    GEnemyWriteIdx = 0;

    GSelectedEnemyIdx = -1;
    GSpawnsView       = SV_List;
}

// Shut down all preview instances completely on map change.
// Called from OLPreview_ShutdownSystem() via CALLBACK_PreLoadMap — at this point
// old UObjects are still alive so Shutdown() can safely call Detach/RemoveFromRoot.
// The instances are lazy-Init()'d again on the next tick that needs them.
void OLSpawns_ReleaseD3DObjects()
{
    // Release only D3DPOOL_DEFAULT surfaces across all preview instances.
    GPreviewMain.ReleaseD3DObjects();
    GPreviewHover.ReleaseD3DObjects();
    GPreviewWeapon.ReleaseD3DObjects();
}

void OLSpawns_ShutdownPreviews()
{
    GPreviewMain.Shutdown();   // already called by OLPreview_ShutdownSystem, but idempotent
    GPreviewHover.Shutdown();
    GPreviewWeapon.Shutdown();

    // Unpin and clear mesh caches. AddToRoot was called when each entry was cached;
    // RemoveFromRoot here lets GC collect them if no other references exist.
    for (int i = 0; i < (int)ARRAY_COUNT(GEnemyNames); ++i)
    {
        USkeletalMesh* M = GPreviewMeshCache[i];
        if (M && (PTRINT)M != (PTRINT)(-1)) M->RemoveFromRoot();
        GPreviewMeshCache[i] = NULL;
    }
    for (int i = 0; i < (int)ARRAY_COUNT(GWeaponNames); ++i)
    {
        UStaticMesh* M = GWeaponMeshCache[i];
        if (M && (PTRINT)M != (PTRINT)(-1)) M->RemoveFromRoot();
        GWeaponMeshCache[i] = NULL;
    }

    // Force full reload on next tick.
    GPreviewLoadedIdx       = -1;
    GWeaponAttachedIdx      = -1;
    GPreviewHoverLoaded     = -1;
    GWeaponPreviewLoadedIdx = -1;

    GEnemyBuf[0].Count = 0;
    GEnemyBuf[1].Count = 0;
    GEnemyReadIdx  = -1;
    GEnemyWriteIdx = 0;

    GSelectedEnemyIdx = -1;
    GSpawnsView       = SV_List;
}

// ---------------------------------------------------------------------------
// Sub-views
// ---------------------------------------------------------------------------

static void DrawSpawnNew()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        GSpawnsView = SV_List;
        return;
    }

    OL_T_SECTION("spawns.new.section.enemy");

    // Type combo — manual BeginCombo so we can detect hover per item.
    ImGui::TextUnformatted(OLLocale_T("spawns.new.type")); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::BeginCombo("##enemytype", GEnemyNames[GSpawnEnemyIdx]))
    {
        for (int i = 0; i < (int)ARRAY_COUNT(GEnemyNames); ++i)
        {
            bool bSelected = (i == GSpawnEnemyIdx);
            if (ImGui::Selectable(GEnemyNames[i], bSelected))
                GSpawnEnemyIdx = i;
            if (bSelected)
                ImGui::SetItemDefaultFocus();

            if (!bSelected && ImGui::IsItemHovered())
            {
                GPreviewHoverIdx = i;
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(GEnemyNames[i]);
                void* _tid = GPreviewHover.GetImTextureID();
                if (_tid)
                    ImGui::Image((ImTextureID)_tid,
                        ImVec2((float)OL_PREVIEW_SIZE, (float)OL_PREVIEW_SIZE),
                        ImVec2(0,0), ImVec2(1,1));
                else
                    ImGui::TextDisabled(OLLocale_T("common.loading"));
                ImGui::EndTooltip();
            }
        }
        ImGui::EndCombo();
    }

    // Weapon combo — with hover tooltip preview.
    ImGui::TextUnformatted(OLLocale_T("spawns.new.weapon")); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::BeginCombo("##weapontype", GWeaponNames[GSpawnWeaponIdx]))
    {
        for (int i = 0; i < (int)ARRAY_COUNT(GWeaponNames); ++i)
        {
            bool bSel = (i == GSpawnWeaponIdx);
            if (ImGui::Selectable(GWeaponNames[i], bSel))
                GSpawnWeaponIdx = i;
            if (bSel)
                ImGui::SetItemDefaultFocus();

            if (i > 0 && ImGui::IsItemHovered())
            {
                if (!GWeaponMeshCache[i])
                    LoadWeaponMesh(i);
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(GWeaponNames[i]);
                void* _wtid = GPreviewWeapon.GetImTextureID();
                if (_wtid && GWeaponPreviewLoadedIdx == i)
                    ImGui::Image((ImTextureID)_wtid,
                        ImVec2((float)OL_PREVIEW_SIZE, (float)OL_PREVIEW_SIZE),
                        ImVec2(0,0), ImVec2(1,1));
                else
                    ImGui::TextDisabled(OLLocale_T("common.loading"));
                ImGui::EndTooltip();
                GWeaponHoverIdx = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::TextUnformatted(OLLocale_T("spawns.new.count")); ImGui::SameLine();
    ImGui::SetNextItemWidth(120.f);
    ImGui::InputInt("##spawncount", &GSpawnCount, 1, 5);
    if (GSpawnCount < 1) GSpawnCount = 1;
    ImGui::SameLine();
    ImGui::TextUnformatted(OLLocale_T("spawns.new.should_attack")); ImGui::SameLine();
    ImGui::Checkbox("##shouldatk", &GSpawnShouldAtk);

    ImGui::SeparatorText(OLLocale_T("spawns.new.section.preview"));
    {
        void* _etid = GPreviewMain.GetImTextureID();
        if (_etid)
            ImGui::Image((ImTextureID)_etid,
                ImVec2((float)OL_PREVIEW_SIZE, (float)OL_PREVIEW_SIZE),
                ImVec2(0,0), ImVec2(1,1));
        else
        {
            ImGui::Dummy(ImVec2((float)OL_PREVIEW_SIZE, (float)OL_PREVIEW_SIZE));
            ImVec2 mn = ImGui::GetItemRectMin(), mx = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(mn, mx, IM_COL32(30,30,30,200));
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(mn.x+8.f, mn.y+(float)OL_PREVIEW_SIZE*0.5f-8.f),
                IM_COL32(120,120,120,255), OLLocale_T("common.no_preview"));
        }
    }

    ImGui::Spacing();
    if (ImGui::Button(OLLocale_T("spawns.new.spawn_btn"), ImVec2(-1.f, 0.f)))
        OLImGui_EnqueueCall(Action_BeginEnemyPlacement);
}

static void DrawEnemyDetail()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        GSpawnsView      = SV_List;
        GSelectedEnemyIdx = -1;
        return;
    }

    // Validate index — but only kick back if the list is non-empty and the index
    // is genuinely out of range. An empty list may be a transient snapshot race;
    // don't eject the user in that case.
    const int _ri = GEnemyReadIdx;
    if (_ri < 0) return;
    const FEnemyListSnap& Snap = GEnemyBuf[_ri];

    if (GSelectedEnemyIdx < 0 ||
        (Snap.Count > 0 && GSelectedEnemyIdx >= Snap.Count))
    {
        GSpawnsView       = SV_List;
        GSelectedEnemyIdx = -1;
        return;
    }

    if (Snap.Count == 0)
        return;

    const FEnemyEntry& En = Snap.Entries[GSelectedEnemyIdx];

    ImGui::SameLine();
    ImGui::TextUnformatted(En.Name);

    ImGui::Separator();
    ImGui::Text(OLLocale_T("spawns.detail.health"), En.Health, En.HealthMax);
    ImGui::Text(OLLocale_T("spawns.detail.location"),
        En.LocX, En.LocY, En.LocZ);
}

// ---------------------------------------------------------------------------
// List view
// ---------------------------------------------------------------------------

static char GEnemyFilter[128] = {};

static void DrawEnemyList()
{
    // --- Top bar: Back + search filter ---
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        OLSpawns_BackToHub();
        return;
    }
    ImGui::SameLine();
    OL_SEARCH("##efilter", "spawns.enemies.search", GEnemyFilter, sizeof(GEnemyFilter));

    ImGui::Spacing();

    const int _ri2 = GEnemyReadIdx;
    if (_ri2 < 0)
    {
        ImGui::TextDisabled(OLLocale_T("common.loading"));
        return;
    }

    const FEnemyListSnap& Snap = GEnemyBuf[_ri2];

    // Collect filtered indices so the clipper knows the real row count.
    static int GFilteredIdx[MAX_ENEMY_SNAP];
    int FilteredCount = 0;
    for (int i = 0; i < Snap.Count; ++i)
    {
        if (OLImGui_MatchFilter(Snap.Entries[i].Name, GEnemyFilter))
            GFilteredIdx[FilteredCount++] = i;
    }

    // --- Scrollable list (leave room for the + button at the bottom) ---
    const float PlusH = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("##enemylist", ImVec2(0, -PlusH), false);

    if (ImGui::BeginTable("##etbl", 3,
        ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn(OLLocale_T("spawns.col.name"),    ImGuiTableColumnFlags_WidthStretch, 3.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.col.health"),  ImGuiTableColumnFlags_WidthStretch, 1.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.col.actions"), ImGuiTableColumnFlags_WidthFixed,  60.f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        ImGuiListClipper Clipper;
        Clipper.Begin(FilteredCount);
        while (Clipper.Step())
        {
            for (int ci = Clipper.DisplayStart; ci < Clipper.DisplayEnd; ++ci)
            {
                const int i           = GFilteredIdx[ci];
                const FEnemyEntry& En = Snap.Entries[i];
                ImGui::TableNextRow();

                // Col 0 — name / row selector
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(i);
                if (ImGui::Selectable(En.Name, (i == GSelectedEnemyIdx),
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                {
                    GSelectedEnemyIdx = i;
                    GSpawnsView       = SV_Enemy;
                }
                ImGui::PopID();

                // Col 1 — HP
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", En.Health);

                // Col 2 — "⋮" button → popup with Teleport / Delete
                ImGui::TableSetColumnIndex(2);
                ImGui::PushID(i + 10000);
                char PopupId[32];
                _snprintf(PopupId, sizeof(PopupId), "##emenu%d", i);
                if (ImGui::Button("...", ImVec2(-1.f, 0.f)))
                    ImGui::OpenPopup(PopupId);
                if (ImGui::BeginPopup(PopupId))
                {
                    if (ImGui::MenuItem(OLLocale_T("spawns.action.teleport")))
                    {
                        _snprintf(GPendingEnemyAction, sizeof(GPendingEnemyAction), "%s", En.Name);
                        OLImGui_EnqueueCall(Action_TeleportEnemy);
                    }
                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.35f, 0.35f, 1.f));
                    if (ImGui::MenuItem(OLLocale_T("spawns.action.delete")))
                    {
                        _snprintf(GPendingEnemyAction, sizeof(GPendingEnemyAction), "%s", En.Name);
                        OLImGui_EnqueueCall(Action_DeleteEnemy);
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
        }
        Clipper.End();
        ImGui::EndTable();
    }

    ImGui::EndChild();

    // --- + button always visible at the bottom of the window ---
    if (ImGui::Button("+", ImVec2(-1.f, 0.f)))
        GSpawnsView = SV_New;
}

// ---------------------------------------------------------------------------
// Tab entry point — called from OLImGui_Spawns.cpp hub
// ---------------------------------------------------------------------------

void OLImGui_TabSpawns_Enemies()
{
    switch (GSpawnsView)
    {
    case SV_New:   DrawSpawnNew();    break;
    case SV_Enemy: DrawEnemyDetail(); break;
    default:       DrawEnemyList();   break;
    }
}
