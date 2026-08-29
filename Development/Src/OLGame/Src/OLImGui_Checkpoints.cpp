/*=============================================================================
    OLImGui_Checkpoints.cpp — "Checkpoints" tab.

    Autonomous checkpoint list — never touches GMasterCheckpointList /
    GMasterGameStateList and never spawns actors in GWorld for scanning.

    Sources (scanned every tick, append-only, no duplicates):
      - X_Checkpoints  (GT_Outlast)   — loaded via LoadModPackage if not seen
      - DLC_Checkpoints (GT_Whistleblower) — same
      - GWorld->Levels — any level whose package contains an AOLCheckpointList
      - Mods/Persistent/*.upk — same

    Each AOLCheckpointList carries PersistentLevelName, so we never need to
    find individual AOLCheckpoint actors just to fill the list.

    Execution:
      - AOLCheckpoint in current world → StartNewGameAtCheckpoint directly.
      - Not in world → spawn a temporary AOLCheckpoint with the right
        CheckpointName + PersistentLevelName, call StartNewGameAtCheckpoint;
        the engine does PrepareMapChange and the actor is discarded on travel.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "OLUtilities.h"
#include "OLImageLoader.h"

// ---------------------------------------------------------------------------
// FCPEntry
// ---------------------------------------------------------------------------

struct FCPEntry
{
    char Name[128];
    char Tag[64];
    char PkgName[64];
    char PersistentLevel[128];
    BYTE GT;
};

static TArray<FCPEntry> GCPList;

// GT seen flags — skip scanning a source we already have complete data for.
static bool GHaveOL  = false;
static bool GHaveDLC = false;

// Set of package names already scanned (to avoid re-scanning every tick).
static TArray<FString> GScannedPkgs;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void FNameToAnsi(char* Dst, INT Len, const FName& N)
{
    FString S = N.ToString();
    INT i = 0;
    while (i < Len - 1 && (*S)[i]) { Dst[i] = (char)(*S)[i]; ++i; }
    Dst[i] = '\0';
}

static void TagFromName(char* Dst, INT Len, const char* CPName)
{
    appStrcpyANSI(Dst, Len, CPName);
    char* u = strchr(Dst, '_');
    if (u) *u = '\0';
    if (!Dst[0]) appStrcpyANSI(Dst, Len, CPName);
}

static bool IsScanned(const FString& PkgName)
{
    for (INT i = 0; i < GScannedPkgs.Num(); ++i)
        if (GScannedPkgs(i) == PkgName) return true;
    return false;
}

// ---------------------------------------------------------------------------
// ScanList — read all checkpoints from one AOLCheckpointList into GCPList.
// PersistentLevel is filled from AOLCheckpoint actors in the provided level
// (may be NULL — field stays empty until FillPersistentLevels is called).
// ---------------------------------------------------------------------------

static void ScanList(AOLCheckpointList* L, BYTE GT, const char* PkgName, ULevel* ActorLevel)
{
    if (!L || L->IsPendingKill()) return;

    for (INT i = 0; i < L->CheckpointList.Num(); ++i)
    {
        FName CPName = L->CheckpointList(i);
        if (CPName == NAME_None) continue;

        char NameA[128];
        FNameToAnsi(NameA, sizeof(NameA), CPName);

        // Skip duplicates by name.
        bool Dup = false;
        for (INT j = 0; j < GCPList.Num(); ++j)
            if (appStrcmpANSI(GCPList(j).Name, NameA) == 0) { Dup = true; break; }
        if (Dup) continue;

        FCPEntry E;
        appStrcpyANSI(E.Name,    sizeof(E.Name),    NameA);
        appStrcpyANSI(E.PkgName, sizeof(E.PkgName), PkgName);
        E.PersistentLevel[0] = '\0';
        TagFromName(E.Tag, sizeof(E.Tag), NameA);
        E.GT = GT;

        // Try to fill PersistentLevel from the provided level's actors.
        if (ActorLevel)
        {
            for (INT ai = 0; ai < ActorLevel->Actors.Num(); ++ai)
            {
                AOLCheckpoint* CP = Cast<AOLCheckpoint>(ActorLevel->Actors(ai));
                if (!CP || CP->IsPendingKill() || CP->CheckpointName != CPName) continue;
                FNameToAnsi(E.PersistentLevel, sizeof(E.PersistentLevel), CP->PersistentLevelName);
                break;
            }
        }

        GCPList.AddItem(E);
    }
}

// Fill PersistentLevel for entries that still have it empty.
// Uses TObjectIterator so it finds AOLCheckpoint actors in packages that were
// loaded via LoadModPackage but are not streamed into GWorld (e.g. X_Checkpoints
// on a custom map). Safe because we only write PersistentLevelName, not spawn.
static void FillPersistentLevels()
{
    for (INT i = 0; i < GCPList.Num(); ++i)
    {
        if (GCPList(i).PersistentLevel[0]) continue;
        FName CPName(ANSI_TO_TCHAR(GCPList(i).Name));
        for (TObjectIterator<AOLCheckpoint> It; It; ++It)
        {
            if (It->IsPendingKill() || It->CheckpointName != CPName) continue;
            if (It->PersistentLevelName == NAME_None) continue;
            FNameToAnsi(GCPList(i).PersistentLevel, sizeof(GCPList(i).PersistentLevel),
                It->PersistentLevelName);
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// ScanPackage — find AOLCheckpointList objects in a UPackage and scan them.
// ActorLevel: if non-NULL, actors in that level are used to fill PersistentLevel.
// ---------------------------------------------------------------------------

static void ScanPackage(UPackage* Pkg, BYTE DefaultGT, ULevel* ActorLevel = NULL)
{
    if (!Pkg) return;
    for (TObjectIterator<AOLCheckpointList> It; It; ++It)
    {
        if (It->GetOutermost() != Pkg || It->IsPendingKill()) continue;
        BYTE GT = (It->GameType < 2) ? (BYTE)It->GameType : DefaultGT;
        ScanList(*It, GT, TCHAR_TO_ANSI(*Pkg->GetName()), ActorLevel);
        if (GT == 0) GHaveOL  = true;
        if (GT == 1) GHaveDLC = true;
    }
}

// ---------------------------------------------------------------------------
// UpdateCPList — called every PostTick. Appends any new sources found.
// ---------------------------------------------------------------------------

static void UpdateCPList_PostTick()
{
    if (!GWorld || !GWorld->HasBegunPlay()) return;
    if (GIsEditor) return;

    bool bLoadedNew = false;

    // 1. Standard packages — load once per GT if not yet seen.
    if (!GHaveOL)
    {
        FString PkgName(TEXT("X_Checkpoints"));
        if (!IsScanned(PkgName))
        {
            UPackage* Pkg = Utils::LoadModPackage(PkgName);
            if (Pkg)
            {
                GScannedPkgs.AddItem(PkgName);
                ScanPackage(Pkg, 0);
                bLoadedNew = true;
            }
        }
    }
    if (!GHaveDLC)
    {
        FString PkgName(TEXT("DLC_Checkpoints"));
        if (!IsScanned(PkgName))
        {
            UPackage* Pkg = Utils::LoadModPackage(PkgName);
            if (Pkg)
            {
                GScannedPkgs.AddItem(PkgName);
                ScanPackage(Pkg, 1);
                bLoadedNew = true;
            }
        }
    }
    // Fill PersistentLevel for standard package entries using actors currently
    // in GWorld (x_checkpoints / dlc_checkpoints streaming sublevels).
    if (bLoadedNew)
        FillPersistentLevels();

    // 2. Levels currently in GWorld — catches custom maps and DLC sublevels.
    // AOLCheckpointList may live in the level itself; pass the level for actor lookup.
    for (INT li = 0; li < GWorld->Levels.Num(); ++li)
    {
        ULevel* Level = GWorld->Levels(li);
        if (!Level) continue;
        FString PkgName = Level->GetOutermost()->GetName();
        if (IsScanned(PkgName)) continue;
        GScannedPkgs.AddItem(PkgName);
        ScanPackage(Level->GetOutermost(), 0, Level);
    }

    // 3. Mods/Persistent/*.upk — scan any we haven't seen yet.
    FString CookedPath;
    appGetCookedContentPath(appGetPlatformType(), CookedPath);
    FString ModsDir = CookedPath + TEXT("Mods\\Persistent\\");
    TArray<FString> UPKFiles;
    GFileManager->FindFiles(UPKFiles, *(ModsDir + TEXT("*.upk")), TRUE, FALSE);

    bool bNewMod = false;
    for (INT fi = 0; fi < UPKFiles.Num(); ++fi)
    {
        FString BaseName = FFilename(UPKFiles(fi)).GetBaseFilename();
        if (IsScanned(BaseName)) continue;
        UPackage* Pkg = Utils::LoadModPackage(BaseName);
        if (!Pkg) continue;
        GScannedPkgs.AddItem(BaseName);
        ScanPackage(Pkg, 0);
        bNewMod = true;
    }
    if (bNewMod)
        FillPersistentLevels();
}

// ---------------------------------------------------------------------------
// Texture loaders
// ---------------------------------------------------------------------------

static FOLImageLoader GTexLoaderOL;
static FOLImageLoader GTexLoaderDLC;
static bool           GTexLoadersInited = false;

static void InitTexLoaders()
{
    if (GTexLoadersInited) return;
    GTexLoadersInited = true;

    FString Base   = appBaseDir();
    FString OLDir  = Base + TEXT("OpenOL\\res\\checkpoints\\main\\");
    FString DLCDir = Base + TEXT("OpenOL\\res\\checkpoints\\dlc_wb\\");

    char OLDirA[512]  = {};
    char DLCDirA[512] = {};
    const TCHAR* O = *OLDir;
    const TCHAR* D = *DLCDir;
    for (int i = 0; i < 511 && O[i]; ++i) OLDirA[i]  = (char)O[i];
    for (int i = 0; i < 511 && D[i]; ++i) DLCDirA[i] = (char)D[i];

    GTexLoaderOL.SetBaseDir(OLDirA);
    GTexLoaderDLC.SetBaseDir(DLCDirA);
}

static FOLImageLoader* LoaderForEntry(const FCPEntry& E)
{
    if (E.GT == 1) return &GTexLoaderDLC;
    return &GTexLoaderOL;
}

static IDirect3DTexture9* GetCPTexture(const FCPEntry& E)
{
    InitTexLoaders();
    return LoaderForEntry(E)->Get(E.Name);
}

// ---------------------------------------------------------------------------
// Checkpoint execution
// ---------------------------------------------------------------------------

static char GCPPendingName[128]    = "";
static char GCPPendingPersist[128] = "";

static void ExecCP_GameThread()
{
    if (!GCPPendingName[0]) return;
    FName   CPName(ANSI_TO_TCHAR(GCPPendingName));
    FName   PersistName(ANSI_TO_TCHAR(GCPPendingPersist));
    GCPPendingName[0]    = '\0';
    GCPPendingPersist[0] = '\0';


    AOLPlayerController* OLPC = Utils::GetOLPC();
    if (!OLPC || !GWorld) return;

    // If the AOLCheckpoint actor is already in the current world, use it directly.
    for (FActorIterator It; It; ++It)
    {
        AOLCheckpoint* CP = Cast<AOLCheckpoint>(*It);
        if (!CP || CP->IsPendingKill()) continue;
        if (CP->CheckpointName == CPName)
        {
            debugf(TEXT("ExecCP: '%s' found in world — direct"), *CPName.ToString());
            OLPC->eventStartNewGameAtCheckpoint(CPName.ToString(), FALSE);
            return;
        }
    }

    // Not in current world — use the same flow as StartNewGameAtCheckpoint:
    // save the checkpoint data, set CurrentCheckpointName, then call
    // StartTravelToCheckpoint which sets bTravelCheckPersistent and shows the
    // loading screen. UpdateTravel then calls PrepareMapChange using the
    // PersistentLevelName read from the AOLCheckpoint actor found via
    // TObjectIterator (X_Checkpoints / DLC_Checkpoints loaded by LoadModPackage).
    AOLGame* OLGame = Cast<AOLGame>(GWorld->GetGameInfo());
    if (!OLGame) return;

    UOLEngine* OLEngine = Cast<UOLEngine>(GEngine);
    if (!OLEngine) return;

    debugf(TEXT("ExecCP: cross-map travel '%s' → '%s'"),
        *CPName.ToString(), *PersistName.ToString());

    // We cannot find the AOLCheckpoint actor (it lives in the destination map's
    // streaming sublevel, not loaded here). Instead we travel directly and set
    // PendingCheckpointName so PlayerController::Tick fires StartNewGameAtCheckpoint
    // once the real actor is present on the destination map.
    OLGame->PendingCheckpointName = CPName;

    UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
    if (!GameEngine) return;

    TArray<FName> LevelNames;
    LevelNames.AddItem(PersistName);
    GameEngine->PrepareMapChange(LevelNames);
    GameEngine->bShouldCommitPendingMapChange = TRUE;
}

// ---------------------------------------------------------------------------
// Tab availability
// ---------------------------------------------------------------------------

bool OLImGui_CheckpointsAvailable()
{
    return GOLPlayerState.bCheckpointListAvailable;
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void OLImGui_Checkpoints_Tick()
{
    if (GWorld && GWorld->HasBegunPlay())
        OLImGui_EnqueuePostTickCall(UpdateCPList_PostTick);
}

// ---------------------------------------------------------------------------
// Tooltip
// ---------------------------------------------------------------------------

static const float kTooltipW = 640.f;
static const float kTooltipH = 360.f;

static void DrawCPTooltip(INT idx)
{
    ImGui::BeginTooltip();
    IDirect3DTexture9* Tex = GetCPTexture(GCPList(idx));
    if (Tex)
        ImGui::Image((ImTextureID)Tex, ImVec2(kTooltipW, kTooltipH));
    else
        ImGui::TextDisabled(OLLocale_T("common.no_image"));
    ImGui::EndTooltip();
}

// ---------------------------------------------------------------------------
// Tab render (render thread)
// ---------------------------------------------------------------------------

static const char* kGroupTitles[2] = { "checkpoints.group.outlast", "checkpoints.group.whistleblower" };

void OLImGui_TabCheckpoints()
{
    if (GCPList.Num() == 0)
    {
        ImGui::TextDisabled(OLLocale_T("checkpoints.empty"));
        return;
    }

    static char Filter[128] = "";
    OL_SEARCH("##cpfilter", "checkpoints.search", Filter, sizeof(Filter));
    ImGui::Spacing();

    for (int gi = 0; gi < 2; ++gi)
    {
        bool AnyVisible = false;
        for (INT i = 0; i < GCPList.Num() && !AnyVisible; ++i)
            if (GCPList(i).GT == gi && OLImGui_MatchFilter(GCPList(i).Name, Filter))
                AnyVisible = true;
        if (!AnyVisible) continue;

        if (Filter[0])
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.20f, 0.18f, 0.10f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.26f, 0.13f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.40f, 0.34f, 0.16f, 1.f));
        bool GroupOpen = ImGui::CollapsingHeader(OLLocale_T(kGroupTitles[gi]));
        ImGui::PopStyleColor(3);
        if (!GroupOpen) continue;

        ImGui::PushID(gi);

        static const INT kMaxTags = 64;
        char Tags[kMaxTags][64];
        INT  TagCount = 0;
        for (INT i = 0; i < GCPList.Num(); ++i)
        {
            if (GCPList(i).GT != gi) continue;
            if (!OLImGui_MatchFilter(GCPList(i).Name, Filter)) continue;
            bool Found = false;
            for (INT ti = 0; ti < TagCount; ++ti)
                if (appStrcmpANSI(Tags[ti], GCPList(i).Tag) == 0) { Found = true; break; }
            if (!Found && TagCount < kMaxTags)
                appStrcpyANSI(Tags[TagCount++], 64, GCPList(i).Tag);
        }

        for (INT ti = 0; ti < TagCount; ++ti)
        {
            if (Filter[0])
                ImGui::SetNextItemOpen(true, ImGuiCond_Always);

            ImGui::PushID(ti);
            bool ChOpen = ImGui::TreeNodeEx(Tags[ti], ImGuiTreeNodeFlags_SpanAvailWidth);
            if (ChOpen)
            {
                for (INT i = 0; i < GCPList.Num(); ++i)
                {
                    if (GCPList(i).GT != gi) continue;
                    if (appStrcmpANSI(GCPList(i).Tag, Tags[ti]) != 0) continue;
                    if (!OLImGui_MatchFilter(GCPList(i).Name, Filter)) continue;

                    ImGui::PushID(i);
                    char Label[256];
                    _snprintf(Label, sizeof(Label), "%s (%s)",
                        GCPList(i).Name,
                        GCPList(i).PersistentLevel[0] ? GCPList(i).PersistentLevel : "???");
                    if (ImGui::Selectable(Label))
                    {
                        _snprintf(GCPPendingName,    sizeof(GCPPendingName),    "%s", GCPList(i).Name);
                        _snprintf(GCPPendingPersist, sizeof(GCPPendingPersist), "%s", GCPList(i).PersistentLevel);
                        OLImGui_EnqueuePostTickCall(ExecCP_GameThread);
                    }
                    if (ImGui::IsItemHovered())
                        DrawCPTooltip(i);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::Spacing();
    }
}
