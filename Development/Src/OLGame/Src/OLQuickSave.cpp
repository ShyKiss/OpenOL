/*=============================================================================
    OLQuickSave.cpp -- debug-only quick save/load (exact position + progress +
    live enemy positions, not the real OLCheckpoint/checkpoint-actor system).
=============================================================================*/
#include "OLGame.h"
#include "OLUtilities.h"
#include "OLQuickSave.h"

#ifndef UCONST_LOADING_MOVIE
#define UCONST_LOADING_MOVIE TEXT("LoadingScreen")
#endif

// ---------------------------------------------------------------------------
// FName is serialized as (string, number) here rather than the raw index,
// same reasoning as FCheckpointActorRecordWriter/Reader in OLCheckpoint.cpp:
// the index isn't guaranteed to persist across game sessions. Duplicated
// rather than reused because that pair of classes is private to that .cpp.
// ---------------------------------------------------------------------------
static inline void QuickSaveSerializeName(FArchive& Ar, FName& N)
{
    FString NameText;
    if (Ar.IsSaving()) NameText = N.GetNameString();
    INT Number = N.GetNumber();
    Ar << NameText;
    Ar << Number;
    if (Ar.IsLoading()) N = FName(*NameText, Number);
}

class FQuickSaveRecordWriter : public FMemoryWriter
{
public:
    FQuickSaveRecordWriter(TArray<BYTE>& InBytes) : FMemoryWriter(InBytes) {}
    virtual FArchive& operator<<(FName& N) { QuickSaveSerializeName(*this, N); return *this; }
};
class FQuickSaveRecordReader : public FMemoryReader
{
public:
    FQuickSaveRecordReader(TArray<BYTE>& InBytes) : FMemoryReader(InBytes) {}
    virtual FArchive& operator<<(FName& N) { QuickSaveSerializeName(*this, N); return *this; }
};

// ---------------------------------------------------------------------------
// Enemy type name <-> class mapping — same 6 types SpawnEnemy/SpawnEnemyAt
// (OLCheatManager.uc) understand. Kept in sync manually since there's no
// shared table to pull from on the C++ side.
// ---------------------------------------------------------------------------
static const char* GQuickSaveEnemyTypes[] = { "Cannibal", "Soldier", "Groom", "Priest", "Surgeon", "NanoCloud" };
static const int GQuickSaveEnemyTypeCount = ARRAY_COUNT(GQuickSaveEnemyTypes);

static const char* EnemyClassToTypeName(UClass* Class)
{
    if (!Class) return NULL;
    FString ClassName = Class->GetName(); // e.g. "OLEnemyCannibal"
    for (int i = 0; i < GQuickSaveEnemyTypeCount; ++i)
        if (ClassName.InStr(ANSI_TO_TCHAR(GQuickSaveEnemyTypes[i])) != INDEX_NONE)
            return GQuickSaveEnemyTypes[i];
    return NULL;
}

// ---------------------------------------------------------------------------
// Path — lives next to bindings.ini, not part of the real save-slot system.
// ---------------------------------------------------------------------------
static void GetQuickSavePath(char* Out, int Size)
{
    char ExeDir[512] = {};
    GetModuleFileNameA(NULL, ExeDir, sizeof(ExeDir));
    char* Slash = strrchr(ExeDir, '\\');
    if (Slash) Slash[1] = '\0';
    _snprintf(Out, Size, "%sOpenOL\\quicksave.dat", ExeDir);
}

struct FQuickSaveEnemyEntry
{
    FString Type;
    FVector Location;
    FRotator Rotation;
};

// ---------------------------------------------------------------------------
// Pending load — set by OLQuickSave_Load() when the saved map differs from
// the current one; consumed by OLQuickSave_TickPendingLoad() once the new
// map has finished loading and the player pawn exists (mirrors the
// PendingCheckpointName pattern in AOLPlayerController::Tick).
// ---------------------------------------------------------------------------
static bool     GQuickLoadPending = false;
static FName    GQuickLoadMapName;
static FVector  GQuickLoadLocation;
static FRotator GQuickLoadRotation;
static TArray<BYTE> GQuickLoadCheckpointRecord;

// TEMP diagnostic for QuickLoad position-gets-overwritten investigation
// (read from AOLPlayerController::Tick — see OLPlayerController.cpp).
bool   GQuickLoadTraceActive    = false;
DOUBLE GQuickLoadTraceStartTime = 0.0;
static TArray<FQuickSaveEnemyEntry> GQuickLoadEnemies;

// Applies GQuickLoadCheckpointRecord to PC via the same ApplyCheckpointRecord
// event OLCheckpointSave::LoadData() uses — never touches the real save file.
static void ApplyCheckpointRecordBytes(AOLPlayerController* PC, TArray<BYTE>& RecordBytes)
{
    if (RecordBytes.Num() == 0) return;

    UFunction* Function = PC->FindFunction(FName(TEXT("ApplyCheckpointRecord")));
    UScriptStruct* CheckpointStruct = FindField<UScriptStruct>(PC->GetClass(), TEXT("CheckpointRecord"));
    if (!Function || !CheckpointStruct) return;

    TArray<BYTE> EventData;
    EventData.AddZeroed(Function->ParmsSize);
    {
        FQuickSaveRecordReader Reader(RecordBytes);
        Reader.SetPortFlags(PPF_ForceBinarySerialization);
        CheckpointStruct->SerializeBin(Reader, EventData.GetTypedData(), Function->ParmsSize);
    }
    PC->ProcessEvent(Function, EventData.GetData(), NULL);
    for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
    {
        if ((P->GetClass()->ClassCastFlags & CASTCLASS_UStructProperty) && ((UStructProperty*)P)->Struct == CheckpointStruct)
        {
            P->DestroyValue(EventData.GetData());
            break;
        }
    }
}

// Destroys every live AOLEnemyPawn and re-spawns the saved ones in their
// saved spots via OLCheatManager::SpawnEnemyAt, mirroring how the Spawns
// Enemies debug tab already spawns enemies.
static void RestoreEnemies(AOLPlayerController* PC, const TArray<FQuickSaveEnemyEntry>& Enemies)
{
    for (FActorIterator It; It; ++It)
    {
        AOLEnemyPawn* E = Cast<AOLEnemyPawn>(*It);
        if (E && !E->IsPendingKill())
        {
            if (E->Controller) GWorld->DestroyActor(E->Controller);
            GWorld->DestroyActor(E);
        }
    }

    UOLCheatManager* CM = PC->CheatManager ? Cast<UOLCheatManager>(PC->CheatManager) : NULL;
    if (!CM) return;

    UFunction* SpawnFn = CM->FindFunction(FName(TEXT("SpawnEnemyAt")));
    if (!SpawnFn) return;

    for (INT i = 0; i < Enemies.Num(); ++i)
    {
        struct { FString EnemyType; BYTE WeaponToUse; UBOOL ShouldAttack; FVector SpawnLoc; FRotator SpawnRot; } Parms;
        Parms.EnemyType    = Enemies(i).Type;
        Parms.WeaponToUse  = 0;
        Parms.ShouldAttack = TRUE;
        Parms.SpawnLoc     = Enemies(i).Location;
        Parms.SpawnRot     = Enemies(i).Rotation;
        CM->ProcessEvent(SpawnFn, &Parms);
    }
}

void OLQuickSave_Save()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->HeroPawn)
    {
        if (PC) PC->eventClientMessage(TEXT("QuickSave: no player pawn"));
        return;
    }

    const FString MapName = GWorld->GetOutermost()->GetName();
    const FVector Loc = PC->HeroPawn->Location;
    const FRotator Rot = PC->HeroPawn->Rotation;

    // Player progress — same struct/event the real checkpoint save uses.
    TArray<BYTE> CheckpointRecordBytes;
    {
        UFunction* Function = PC->FindFunction(FName(TEXT("CreateCheckpointRecord")));
        UScriptStruct* CheckpointStruct = FindField<UScriptStruct>(PC->GetClass(), TEXT("CheckpointRecord"));
        if (Function && CheckpointStruct)
        {
            TArray<BYTE> BaseData;
            BaseData.AddZeroed(CheckpointStruct->GetPropertiesSize());
            PC->ProcessEvent(Function, BaseData.GetData(), NULL);
            {
                FQuickSaveRecordWriter Writer(CheckpointRecordBytes);
                Writer.SetPortFlags(PPF_ForceBinarySerialization);
                CheckpointStruct->SerializeBin(Writer, BaseData.GetTypedData(), CheckpointStruct->GetPropertiesSize());
            }
            for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
            {
                if ((P->GetClass()->ClassCastFlags & CASTCLASS_UStructProperty) && ((UStructProperty*)P)->Struct == CheckpointStruct)
                {
                    P->DestroyValue(BaseData.GetData());
                    break;
                }
            }
        }
    }

    // Live enemies — position/rotation only (see OLQuickSave.h).
    TArray<FQuickSaveEnemyEntry> Enemies;
    INT DbgTotal = 0, DbgSkippedPendingKill = 0, DbgSkippedHealth = 0, DbgSkippedNoType = 0;
    for (FActorIterator It; It; ++It)
    {
        AOLEnemyPawn* E = Cast<AOLEnemyPawn>(*It);
        if (!E) continue;
        ++DbgTotal;
        if (E->IsPendingKill()) { ++DbgSkippedPendingKill; continue; }
        if (E->Health <= 0) { ++DbgSkippedHealth; continue; }
        const char* TypeName = EnemyClassToTypeName(E->GetClass());
        if (!TypeName) { ++DbgSkippedNoType; debugf(TEXT("QuickSave: unrecognized enemy class '%s'"), *E->GetClass()->GetName()); continue; }

        FQuickSaveEnemyEntry Entry;
        Entry.Type     = FString(ANSI_TO_TCHAR(TypeName));
        Entry.Location = E->Location;
        Entry.Rotation = E->Rotation;
        Enemies.AddItem(Entry);
    }
    debugf(TEXT("QuickSave: enemies total=%d pendingKill=%d deadHealth=%d unrecognizedType=%d saved=%d"),
        DbgTotal, DbgSkippedPendingKill, DbgSkippedHealth, DbgSkippedNoType, Enemies.Num());

    char Path[512];
    GetQuickSavePath(Path, sizeof(Path));

    FILE* F = fopen(Path, "wb");
    if (!F)
    {
        PC->eventClientMessage(TEXT("QuickSave: failed to open file"));
        return;
    }

    // Fixed-size header line (ANSI text) followed by raw CheckpointRecord bytes.
    fprintf(F, "%s %f %f %f %d %d %d %d %d\n",
        TCHAR_TO_ANSI(*MapName), Loc.X, Loc.Y, Loc.Z, Rot.Pitch, Rot.Yaw, Rot.Roll,
        CheckpointRecordBytes.Num(), Enemies.Num());
    if (CheckpointRecordBytes.Num() > 0)
        fwrite(CheckpointRecordBytes.GetData(), 1, CheckpointRecordBytes.Num(), F);
    for (INT i = 0; i < Enemies.Num(); ++i)
    {
        const FQuickSaveEnemyEntry& E = Enemies(i);
        fprintf(F, "%s %f %f %f %d %d %d\n",
            TCHAR_TO_ANSI(*E.Type), E.Location.X, E.Location.Y, E.Location.Z,
            E.Rotation.Pitch, E.Rotation.Yaw, E.Rotation.Roll);
    }
    fclose(F);

    PC->eventClientMessage(FString::Printf(TEXT("QuickSave: saved (%d enemies)"), Enemies.Num()));
}

// Applies the parsed player position + checkpoint record + enemies to the
// current world — shared by the same-map (immediate) and pending-load
// (after travel) paths below.
static void ApplyLoadedState(AOLPlayerController* PC, const FVector& Loc, const FRotator& Rot,
    TArray<BYTE>& CheckpointRecordBytes, const TArray<FQuickSaveEnemyEntry>& Enemies)
{
    if (PC->HeroPawn)
    {
        debugf(TEXT("QuickLoad: ApplyLoadedState — HeroPawn=%s before=(%.0f,%.0f,%.0f) target=(%.0f,%.0f,%.0f)"),
            *PC->HeroPawn->GetName(), PC->HeroPawn->Location.X, PC->HeroPawn->Location.Y, PC->HeroPawn->Location.Z,
            Loc.X, Loc.Y, Loc.Z);
        UBOOL bMoved = GWorld->FarMoveActor(PC->HeroPawn, Loc, FALSE, FALSE, FALSE);
        debugf(TEXT("QuickLoad: FarMoveActor returned %d, HeroPawn now at (%.0f,%.0f,%.0f)"),
            bMoved, PC->HeroPawn->Location.X, PC->HeroPawn->Location.Y, PC->HeroPawn->Location.Z);
        PC->HeroPawn->SetRotation(Rot);
    }
    ApplyCheckpointRecordBytes(PC, CheckpointRecordBytes);
    RestoreEnemies(PC, Enemies);
    PC->eventClientMessage(TEXT("QuickLoad: restored"));
}

void OLQuickSave_Load()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;

    char Path[512];
    GetQuickSavePath(Path, sizeof(Path));

    FILE* F = fopen(Path, "rb");
    if (!F)
    {
        PC->eventClientMessage(TEXT("QuickLoad: no save file"));
        return;
    }

    char MapNameAnsi[256] = {};
    double X = 0, Y = 0, Z = 0;
    int Pitch = 0, Yaw = 0, Roll = 0;
    int RecordSize = 0, EnemyCount = 0;
    int Parsed = fscanf(F, "%255s %lf %lf %lf %d %d %d %d %d\n",
        MapNameAnsi, &X, &Y, &Z, &Pitch, &Yaw, &Roll, &RecordSize, &EnemyCount);
    if (Parsed != 9)
    {
        fclose(F);
        PC->eventClientMessage(TEXT("QuickLoad: save file corrupt"));
        return;
    }

    TArray<BYTE> RecordBytes;
    if (RecordSize > 0)
    {
        RecordBytes.Add(RecordSize);
        fread(RecordBytes.GetData(), 1, RecordSize, F);
    }

    TArray<FQuickSaveEnemyEntry> Enemies;
    for (int i = 0; i < EnemyCount; ++i)
    {
        char TypeAnsi[64] = {};
        double EX = 0, EY = 0, EZ = 0;
        int EPitch = 0, EYaw = 0, ERoll = 0;
        if (fscanf(F, "%63s %lf %lf %lf %d %d %d\n", TypeAnsi, &EX, &EY, &EZ, &EPitch, &EYaw, &ERoll) != 7)
            break;
        FQuickSaveEnemyEntry Entry;
        Entry.Type     = FString(ANSI_TO_TCHAR(TypeAnsi));
        Entry.Location = FVector((FLOAT)EX, (FLOAT)EY, (FLOAT)EZ);
        Entry.Rotation = FRotator(EPitch, EYaw, ERoll);
        Enemies.AddItem(Entry);
    }
    fclose(F);

    const FName SavedMap(ANSI_TO_TCHAR(MapNameAnsi));
    const FVector Loc((FLOAT)X, (FLOAT)Y, (FLOAT)Z);
    const FRotator Rot(Pitch, Yaw, Roll);

    GQuickLoadTraceActive    = true;
    GQuickLoadTraceStartTime = appSeconds();

    const FName CurrentMap(*GWorld->GetOutermost()->GetName());
    if (SavedMap == CurrentMap)
    {
        ApplyLoadedState(PC, Loc, Rot, RecordBytes, Enemies);
    }
    else
    {
        GQuickLoadPending          = true;
        GQuickLoadMapName          = SavedMap;
        GQuickLoadLocation         = Loc;
        GQuickLoadRotation         = Rot;
        GQuickLoadCheckpointRecord = RecordBytes;
        GQuickLoadEnemies          = Enemies;

        PC->eventClientMessage(TEXT("QuickLoad: traveling..."));
        PC->ConsoleCommand(FString::Printf(TEXT("open %s"), *SavedMap.ToString()));
    }
}

void OLQuickSave_TickPendingLoad(AOLPlayerController* PC)
{
    if (!GQuickLoadPending || !PC || !PC->HeroPawn) return;

    // Wait for the loading movie to actually stop before teleporting —
    // HeroPawn exists (and even bTravellingToCheckpoint can already be
    // FALSE) well before the standard travel/checkpoint-spawn flow has
    // finished placing the player at the level's start spot; applying our
    // position any earlier just gets overwritten once that settles. The
    // loading movie hiding is the same "travel is actually done" signal
    // OLLoadingOverlay uses.
    if (GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE)) return;

    const FName CurrentMap(*GWorld->GetOutermost()->GetName());
    if (CurrentMap != GQuickLoadMapName) return;

    GQuickLoadPending = false;
    ApplyLoadedState(PC, GQuickLoadLocation, GQuickLoadRotation, GQuickLoadCheckpointRecord, GQuickLoadEnemies);
    GQuickLoadCheckpointRecord.Empty();
    GQuickLoadEnemies.Empty();
}
