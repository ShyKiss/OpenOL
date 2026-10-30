/*=============================================================================
    OLImGui_Spawns_Doors.cpp — "Doors" sub-tab: spawn OLDoor actors with
    configurable parameters and tag "CustomSpawnedDoor".
=============================================================================*/
#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// DoorMeshType enum labels — loaded once from UEnum at runtime.
// ANSI strings are heap-allocated and never freed (process lifetime).
// ---------------------------------------------------------------------------
static TArray<const char*> GDoorMeshPtrs;  // heap-allocated ANSI strings
static int                 GDoorMeshCount = 0;

// Call once on game thread after the engine has loaded OLDoor package.
// Reads EOLDoorMeshType names from reflection; no fixed upper limit.
static void EnsureDoorMeshNames()
{
    if (GDoorMeshCount > 0)
        return;

    UEnum* E = FindObject<UEnum>(ANY_PACKAGE, TEXT("EOLDoorMeshType"));
    if (!E)
        return; // retry next tick

    int N = E->NumEnums();
    GDoorMeshPtrs.Empty(N);
    for (int i = 0; i < N; ++i)
    {
        FString Raw = E->GetEnum(i).ToString();
        // Strip "DMesh_" prefix for readability.
        const TCHAR* Src = *Raw;
        if (Src[0]=='D' && Src[1]=='M' && Src[2]=='e' &&
            Src[3]=='s' && Src[4]=='h' && Src[5]=='_')
            Src += 6;

        // Convert to ANSI and store on heap so the pointer is stable.
        int Len = 0;
        while (Src[Len]) ++Len;
        char* Buf = (char*)appMalloc(Len + 1);
        for (int ci = 0; ci < Len; ++ci)
            Buf[ci] = (char)Src[ci];
        Buf[Len] = '\0';
        GDoorMeshPtrs.AddItem((const char*)Buf);
    }
    GDoorMeshCount = N;
}

// ---------------------------------------------------------------------------
// Spawn form state
// ---------------------------------------------------------------------------
static int   GDoorMeshIdx        = 1;    // DMesh_Wooden
static int   GDoorTypeIdx        = 0;    // DT_Normal
static bool  GDoorLocked         = false;
static bool  GDoorReverse        = false;
static bool  GDoorAutoClose      = false;
static bool  GDoorAICanUse       = true;
static float GDoorOpenAngle      = 0.f;
static float GDoorOpeningSpeed   = 150.f;
static float GDoorClosingSpeed   = 240.f;

// ---------------------------------------------------------------------------
// Door list snapshot — double-buffer, built each tick on game thread.
// ---------------------------------------------------------------------------
struct FDoorEntry
{
    char  Name[64];
    int   MeshIdx;  // EOLDoorMeshType value (for display)
    bool  bLocked;
    float LocX, LocY, LocZ;
};

static const int MAX_DOOR_SNAP = 128;

struct FDoorListSnap
{
    FDoorEntry Entries[MAX_DOOR_SNAP];
    int        Count;
};

static FDoorListSnap GDoorBuf[2];
static volatile int  GDoorReadIdx  = -1;
static int           GDoorWriteIdx = 0;

static void RebuildDoorList()
{
    EnsureDoorMeshNames();

    if (!GWorld || !GWorld->PersistentLevel)
        return;

    ULevel* PL = GWorld->PersistentLevel;
    FDoorListSnap Tmp;
    Tmp.Count = 0;

    static const FName TagDoor(TEXT("CustomSpawnedDoor"));
    for (int ai = 0; ai < PL->Actors.Num(); ++ai)
    {
        AActor* A = PL->Actors(ai);
        if (!A || A->IsPendingKill()) continue;
        if (A->Tag != TagDoor) continue;

        // Only consider OLDoor actors (may also be in the list as AActor if cast fails).
        if (Tmp.Count >= MAX_DOOR_SNAP) break;

        FDoorEntry& De = Tmp.Entries[Tmp.Count++];
        De.bLocked = false;
        De.MeshIdx = 0;

        // Try to read OLDoor-specific fields via UProperty to stay robust.
        // Simpler than casting to generated C++ class (avoids include dependency).
        {
            UClass* Cls = A->GetClass();
            UProperty* PBL = FindField<UProperty>(Cls, TEXT("bLocked"));
            if (PBL) De.bLocked = *(BITFIELD*)((BYTE*)A + PBL->Offset) != 0;

            UProperty* PMT = FindField<UProperty>(Cls, TEXT("DoorMeshType"));
            if (PMT) De.MeshIdx = (int)(*(BYTE*)((BYTE*)A + PMT->Offset));
        }

        De.LocX = A->Location.X;
        De.LocY = A->Location.Y;
        De.LocZ = A->Location.Z;

        const FString AN = A->GetFName().ToString();
        const TCHAR* N = *AN;
        int ni = 0;
        while (ni < 63 && N[ni]) { De.Name[ni] = (char)N[ni]; ++ni; }
        De.Name[ni] = '\0';
    }

    GDoorBuf[GDoorWriteIdx] = Tmp;
    GDoorReadIdx  = GDoorWriteIdx;
    GDoorWriteIdx = 1 - GDoorWriteIdx;
}

// ---------------------------------------------------------------------------
// Pending game-thread action
// ---------------------------------------------------------------------------
static char GPendingDoorAction[64] = {};

static AActor* FindDoorByName(const char* name)
{
    if (!GWorld || !GWorld->PersistentLevel) return NULL;
    FName Target(ANSI_TO_TCHAR(name));
    ULevel* PL = GWorld->PersistentLevel;
    for (int i = 0; i < PL->Actors.Num(); ++i)
    {
        AActor* A = PL->Actors(i);
        if (!A || A->IsPendingKill()) continue;
        if (A->GetFName() == Target) return A;
    }
    return NULL;
}

static void Action_TeleportDoor()
{
    AActor* D = FindDoorByName(GPendingDoorAction);
    if (!D) return;
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->Pawn) return;
    FVector Loc = PC->Pawn->Location + PC->Pawn->Rotation.Vector() * 200.f;
    D->SetLocation(Loc);
}

static void Action_DeleteDoor()
{
    AActor* D = FindDoorByName(GPendingDoorAction);
    if (!D) return;
    GWorld->DestroyActor(D);
}

// ---------------------------------------------------------------------------
// Placement
// ---------------------------------------------------------------------------

// Apply current form params to a just-spawned OLDoor actor via UProperty writes.
static void ApplyDoorParams(AActor* A)
{
    if (!A) return;
    UClass* Cls = A->GetClass();

    // Helper: write BYTE property
    UProperty* PMT = FindField<UProperty>(Cls, TEXT("DoorMeshType"));
    if (PMT) *(BYTE*)((BYTE*)A + PMT->Offset) = (BYTE)GDoorMeshIdx;

    UProperty* PDT = FindField<UProperty>(Cls, TEXT("DoorType"));
    if (PDT) *(BYTE*)((BYTE*)A + PDT->Offset) = (BYTE)GDoorTypeIdx;

    auto WriteBool = [&](const TCHAR* Name, bool Val) {
        UBoolProperty* P = FindField<UBoolProperty>(Cls, Name);
        if (!P) return;
        BITFIELD* BitPtr = (BITFIELD*)((BYTE*)A + P->Offset);
        if (Val) *BitPtr |= P->BitMask;
        else     *BitPtr &= ~P->BitMask;
    };
    WriteBool(TEXT("bLocked"),           GDoorLocked);
    WriteBool(TEXT("bReverseDirection"), GDoorReverse);
    WriteBool(TEXT("bAutoClose"),        GDoorAutoClose);
    WriteBool(TEXT("bAICanUseDoor"),     GDoorAICanUse);

    auto WriteFloat = [&](const TCHAR* Name, float Val) {
        UProperty* P = FindField<UProperty>(Cls, Name);
        if (!P) return;
        *(float*)((BYTE*)A + P->Offset) = Val;
    };
    WriteFloat(TEXT("InitialOpenAngle"), GDoorOpenAngle);
    WriteFloat(TEXT("OpeningSpeed"),     GDoorOpeningSpeed);
    WriteFloat(TEXT("ClosingSpeed"),     GDoorClosingSpeed);
}

// DoorMainMesh defaults to LightingChannels=(Static=TRUE,Dynamic=FALSE), so the DLE
// (which feeds the Dynamic channel) never reaches it. Enable Dynamic so the DLE works.
static void FixDoorLighting(AOLDoor* Door)
{
    if (!Door || !Door->DoorMainMesh)
        return;

    Door->DoorMainMesh->LightingChannels.Dynamic = TRUE;
    Door->DoorMainMesh->LightingChannels.bInitialized = TRUE;
    Door->ReattachComponent(Door->DoorMainMesh);
}

static void Action_BeginDoorPlacement()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !GWorld) return;

    UClass* DoorClass = FindObject<UClass>(ANY_PACKAGE, TEXT("OLDoor"));
    if (!DoorClass) return;

    FVector CamLoc; FRotator CamRot;
    PC->eventGetPlayerViewPoint(CamLoc, CamRot);
    FVector SpawnLoc = CamLoc + FRotationMatrix(CamRot).GetAxis(0) * 150.f;

    // Spawn the door as the ghost. bNoCollisionFail=TRUE so it can overlap geometry.
    AActor* Ghost = GWorld->SpawnActor(DoorClass, NAME_None, SpawnLoc, CamRot,
        NULL, TRUE, FALSE, NULL, NULL, TRUE);
    if (!Ghost) return;

    Ghost->bNoDelete = FALSE;
    Ghost->SetCollision(FALSE, FALSE, Ghost->bIgnoreEncroachers);

    // Apply params then call ForceInit() so the correct mesh is visible during placement.
    ApplyDoorParams(Ghost);
    AOLDoor* Door = Cast<AOLDoor>(Ghost);
    if (Door) { Door->ForceInit(); FixDoorLighting(Door); }

    // Clear any stale pending confirm before entering placement mode.
    PC->bPlacementConfirmPending = FALSE;
    PC->eventBeginPlacementMode(Ghost);
}

static bool ConfirmDoorPlacement(AOLPlayerController* PC)
{
    AActor* Ghost = PC->PlacementGhost;
    if (!FindField<UProperty>(Ghost->GetClass(), TEXT("DoorMeshType")))
        return false;

    PC->bPlacementConfirmPending = FALSE;

    // Save pointer before DetachPlacementMode clears PC->PlacementGhost.
    AActor* Placed = Ghost;
    PC->eventDetachPlacementMode();

    if (!Placed || Placed->bDeleteMe) return true;

    Placed->Tag = FName(TEXT("CustomSpawnedDoor"));
    Placed->SetCollision(TRUE, TRUE, Placed->bIgnoreEncroachers);

    // Re-apply params (may have changed during placement) and reinit mesh.
    ApplyDoorParams(Placed);
    AOLDoor* Door = Cast<AOLDoor>(Placed);
    if (Door) { Door->ForceInit(); FixDoorLighting(Door); }

    // Immediately start a new placement ghost so the user stays in placement mode.
    OLImGui_EnqueueCall(Action_BeginDoorPlacement);

    return true;
}

void OLSpawns_TickDoors()
{
    OLSpawns_RegisterConfirmHandler(ConfirmDoorPlacement);
    RebuildDoorList();
}

// ---------------------------------------------------------------------------
// Views
// ---------------------------------------------------------------------------

enum EDoorView { DV_List, DV_New };
static EDoorView GDoorView = DV_List;

static void DrawDoorNew()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        GDoorView = DV_List;
        return;
    }

    OL_T_SECTION("spawns.doors.section.params");

    // Mesh type combo
    ImGui::TextUnformatted(OLLocale_T("spawns.doors.mesh_type")); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    const char* MeshLabel = (GDoorMeshIdx >= 0 && GDoorMeshIdx < GDoorMeshCount)
        ? GDoorMeshPtrs(GDoorMeshIdx) : "?";
    if (ImGui::BeginCombo("##doormesh", MeshLabel))
    {
        for (int i = 0; i < GDoorMeshCount; ++i)
        {
            bool bSel = (i == GDoorMeshIdx);
            if (ImGui::Selectable(GDoorMeshPtrs(i), bSel))
                GDoorMeshIdx = i;
            if (bSel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Door type combo
    ImGui::TextUnformatted(OLLocale_T("spawns.doors.door_type")); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    static const char* DoorTypeNames[] = { "Normal", "Locker" };
    if (ImGui::BeginCombo("##doortype", DoorTypeNames[GDoorTypeIdx & 1]))
    {
        for (int i = 0; i < 2; ++i)
        {
            bool bSel = (i == GDoorTypeIdx);
            if (ImGui::Selectable(DoorTypeNames[i], bSel))
                GDoorTypeIdx = i;
            if (bSel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    ImGui::Checkbox(OLLocale_T("spawns.doors.locked"),      &GDoorLocked);      ImGui::SameLine();
    ImGui::Checkbox(OLLocale_T("spawns.doors.reverse"),     &GDoorReverse);
    ImGui::Checkbox(OLLocale_T("spawns.doors.autoclose"),   &GDoorAutoClose);   ImGui::SameLine();
    ImGui::Checkbox(OLLocale_T("spawns.doors.ai_can_use"),  &GDoorAICanUse);

    ImGui::Separator();

    ImGui::SetNextItemWidth(120.f);
    ImGui::InputFloat(OLLocale_T("spawns.doors.open_angle"),   &GDoorOpenAngle,   1.f, 10.f, "%.1f");
    ImGui::SetNextItemWidth(120.f);
    ImGui::InputFloat(OLLocale_T("spawns.doors.open_speed"),   &GDoorOpeningSpeed, 5.f, 50.f, "%.1f");
    ImGui::SetNextItemWidth(120.f);
    ImGui::InputFloat(OLLocale_T("spawns.doors.close_speed"),  &GDoorClosingSpeed, 5.f, 50.f, "%.1f");

    ImGui::Spacing();
    if (ImGui::Button(OLLocale_T("spawns.doors.place_btn"), ImVec2(-1.f, 0.f)))
        OLImGui_EnqueueCall(Action_BeginDoorPlacement);
}

static char GDoorFilter[128] = {};

static FOLRowMultiSelect GDoorSel;

static void DrawDoorList()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        OLSpawns_BackToHub();
        return;
    }
    ImGui::SameLine();
    OL_SEARCH("##dfilter", "spawns.doors.search", GDoorFilter, sizeof(GDoorFilter));

    ImGui::Spacing();

    const int _ri = GDoorReadIdx;
    if (_ri < 0)
    {
        ImGui::TextDisabled(OLLocale_T("common.loading"));
        return;
    }

    const FDoorListSnap& Snap = GDoorBuf[_ri];

    static int GFilteredIdx[MAX_DOOR_SNAP];
    int FilteredCount = 0;
    for (int i = 0; i < Snap.Count; ++i)
    {
        if (OLImGui_MatchFilter(Snap.Entries[i].Name, GDoorFilter))
            GFilteredIdx[FilteredCount++] = i;
    }

    const float PlusH = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("##doorlist", ImVec2(0, -PlusH), false);

    if (ImGui::BeginTable("##dtbl", 3,
        ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn(OLLocale_T("spawns.col.name"),    ImGuiTableColumnFlags_WidthStretch, 3.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.col.mesh"),    ImGuiTableColumnFlags_WidthStretch, 2.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.col.actions"), ImGuiTableColumnFlags_WidthFixed,  60.f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        OL_ROW_MULTISELECT_BEGIN(GDoorSel, FilteredCount);

        ImGuiListClipper Clipper;
        Clipper.Begin(FilteredCount);
        OL_ROW_MULTISELECT_INCLUDE_RANGE_SRC(Clipper);
        while (Clipper.Step())
        {
            for (int ci = Clipper.DisplayStart; ci < Clipper.DisplayEnd; ++ci)
            {
                const int i          = GFilteredIdx[ci];
                const FDoorEntry& De = Snap.Entries[i];
                ImGui::TableNextRow();

                ImGui::PushID(i);

                ImGui::TableSetColumnIndex(0);
                const char* MLabel = (De.MeshIdx >= 0 && De.MeshIdx < GDoorMeshCount)
                    ? GDoorMeshPtrs(De.MeshIdx) : "?";
                OL_ROW_SELECTABLE(GDoorSel, De.Name, ci);

                ImGui::TableSetColumnIndex(1);
                if (De.bLocked)
                    ImGui::TextColored(ImVec4(1.f, 0.5f, 0.2f, 1.f), "%s [L]", MLabel);
                else
                    ImGui::TextUnformatted(MLabel);

                ImGui::TableSetColumnIndex(2);
                if (OL_ROW_MENU_BEGIN("##dmenu"))
                {
                    OL_ROW_MENU_ITEM("spawns.action.teleport", GPendingDoorAction, sizeof(GPendingDoorAction), De.Name, Action_TeleportDoor);
                    OL_ROW_MENU_SEPARATOR;
                    OL_ROW_MENU_ITEM_DANGER("spawns.action.delete", GPendingDoorAction, sizeof(GPendingDoorAction), De.Name, Action_DeleteDoor);
                    OL_ROW_MENU_END;
                }

                ImGui::PopID();
            }
        }
        Clipper.End();
        OL_ROW_MULTISELECT_END(GDoorSel);

        if (OL_ROW_MULTIMENU_BEGIN(GDoorSel, "##dbulkmenu"))
        {
            OL_ROW_MULTIMENU_ITEM_DANGER("spawns.action.delete", GDoorSel, _r, Snap.Entries[GFilteredIdx[_r]].Name, GPendingDoorAction, sizeof(GPendingDoorAction), Action_DeleteDoor);
            OL_ROW_MENU_END;
        }
        ImGui::EndTable();
    }

    ImGui::EndChild();

    if (ImGui::Button("+", ImVec2(-1.f, 0.f)))
        GDoorView = DV_New;
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------

void OLImGui_TabSpawns_Doors()
{
    switch (GDoorView)
    {
    case DV_New:  DrawDoorNew();  break;
    default:      DrawDoorList(); break;
    }
}
