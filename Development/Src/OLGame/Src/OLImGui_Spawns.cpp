/*=============================================================================
    OLImGui_Spawns.cpp — Spawns hub tab.

    Grid of category tiles (3 per row); each tile opens a dedicated sub-tab.
    Currently implemented: Enemies (OLImGui_Spawns_Enemies.cpp).
=============================================================================*/

#include "OLImGui_Tabs.h"
#include "OLImageLoader.h"

// ---------------------------------------------------------------------------
// Hub state
// ---------------------------------------------------------------------------

// NULL  = show the grid hub
// other = pointer to a sub-tab draw function currently open
static void (*GSpawnsSubTab)() = NULL;

// ---------------------------------------------------------------------------
// Tile layout constants
// ---------------------------------------------------------------------------

static const int   TILES_PER_ROW  = 3;
static const float TILE_GAP       = 8.f;   // gap between tiles (horizontal and vertical)
static const float TILE_LABEL_H   = 22.f;  // height reserved for the text label below the image
static const float TILE_BORDER_R  = 6.f;   // corner radius

// Compute tile width from the current window's content region.
// All 3 tiles + 2 gaps fit exactly into the region's full width.
//
// Must be based on the width at the START of the row, not the width remaining
// at the current cursor position: GetContentRegionAvail().x shrinks after each
// ImGui::SameLine() as tiles eat into the row, so calling this per-tile (as
// before) made every tile after the first in a row progressively smaller.
// Cached once per row instead — see DrawTile's RowStartWidth parameter.
static float TileWidth(float RowStartWidth)
{
    return (RowStartWidth - TILE_GAP * (TILES_PER_ROW - 1)) / (float)TILES_PER_ROW;
}

// ---------------------------------------------------------------------------
// DrawTile — draw one tile at the current cursor position.
// img          — preview texture (NULL → grey placeholder with "..." text)
// label        — centred text below the image
// RowStartWidth — content region width at the START of this tile's row (same
//                 value for every tile in the row — see TileWidth() comment).
// Returns true if the tile was clicked this frame.
// ---------------------------------------------------------------------------
static bool DrawTile(const char* id, ImTextureID img, const char* label, float RowStartWidth)
{
    ImGui::PushID(id);

    const float TW    = TileWidth(RowStartWidth);
    const float TH    = TW + TILE_LABEL_H;   // square image + label strip
    const float ImgH  = TW;                  // image is square

    const ImVec2 Pos = ImGui::GetCursorScreenPos();
    const ImVec2 TileMax = ImVec2(Pos.x + TW, Pos.y + TH);

    const bool Hovered = ImGui::IsMouseHoveringRect(Pos, TileMax);
    const bool Clicked = Hovered && ImGui::IsMouseClicked(0);

    // Background + border
    const ImU32 BgCol = Hovered
        ? IM_COL32(60, 60, 70, 220)
        : IM_COL32(35, 35, 42, 200);
    const ImU32 BdCol = Hovered
        ? IM_COL32(140, 140, 180, 255)
        : IM_COL32(70, 70, 85, 200);

    ImDrawList* DL = ImGui::GetWindowDrawList();
    DL->AddRectFilled(Pos, TileMax, BgCol, TILE_BORDER_R);
    DL->AddRect      (Pos, TileMax, BdCol, TILE_BORDER_R, 0, 1.5f);

    // Image area (square, inset 3 px)
    const ImVec2 ImgMin = ImVec2(Pos.x + 3.f, Pos.y + 3.f);
    const ImVec2 ImgMax = ImVec2(Pos.x + TW  - 3.f, Pos.y + ImgH - 3.f);

    if (img)
    {
        DL->AddImage(img, ImgMin, ImgMax);
    }
    else
    {
        DL->AddRectFilled(ImgMin, ImgMax, IM_COL32(25, 25, 30, 255), 4.f);
        const char* Ph = "...";
        ImVec2 TS = ImGui::CalcTextSize(Ph);
        DL->AddText(
            ImVec2(ImgMin.x + (ImgMax.x - ImgMin.x - TS.x) * 0.5f,
                   ImgMin.y + (ImgMax.y - ImgMin.y - TS.y) * 0.5f),
            IM_COL32(80, 80, 80, 255), Ph);
    }

    // Label — centred horizontally, centred inside the label strip
    ImVec2 LS = ImGui::CalcTextSize(label);
    const float LabelStripTop = Pos.y + ImgH;
    DL->AddText(
        ImVec2(Pos.x + (TW - LS.x) * 0.5f,
               LabelStripTop + (TILE_LABEL_H - LS.y) * 0.5f),
        Hovered ? IM_COL32(230, 230, 255, 255) : IM_COL32(180, 180, 190, 255),
        label);

    // Advance ImGui cursor past the tile
    ImGui::Dummy(ImVec2(TW, TH));

    ImGui::PopID();
    return Clicked;
}

// ---------------------------------------------------------------------------
// Hub tile previews — Doors and Props (each a FOLPreviewInstance).
// Refreshed once per menu open with a random asset.
// ---------------------------------------------------------------------------

static FOLPreviewInstance GHubEnemyPreview;
static FOLPreviewInstance GHubDoorPreview;
static FOLPreviewInstance GHubPropPreview;
static bool               GHubPreviewsLoaded = false; // set after first load attempt

// Lighting and Fog have no representative UStaticMesh to render a live 3D preview
// from, so their tiles use static images instead (same FOLImageLoader used by the
// Checkpoints tab). Files: Binaries/Win64/OpenOL/res/tiles/tile_spawns_*.png
static FOLImageLoader GHubTileLoader;
static bool           GHubTileLoaderInited = false;

static IDirect3DTexture9* GetHubTileTexture(const char* Name)
{
    if (!GHubTileLoaderInited)
    {
        GHubTileLoaderInited = true;
        FString Base = appBaseDir();
        FString Dir  = Base + TEXT("OpenOL\\res\\tiles\\");
        char DirA[512] = {};
        const TCHAR* D = *Dir;
        for (int i = 0; i < 511 && D[i]; ++i) DirA[i] = (char)D[i];
        GHubTileLoader.SetBaseDir(DirA);
    }
    return GHubTileLoader.Get(Name);
}

void OLSpawns_TickHubPreviews()
{
    if (!GHubEnemyPreview.IsInited()) GHubEnemyPreview.Init();
    if (!GHubDoorPreview.IsInited())  GHubDoorPreview.Init();
    if (!GHubPropPreview.IsInited())  GHubPropPreview.Init();

    if (!GHubPreviewsLoaded)
    {
        // Enemy — random from table using shared mesh cache.
        FHubEnemyMesh EM;
        if (OLSpawns_Enemies_GetRandomMesh(EM))
            GHubEnemyPreview.SetSkeletalMeshWithAnim(EM.Mesh, EM.AnimSetPkg, EM.AnimSetObj, EM.IdleAnim);

        // Door — random non-Undefined type from CDO's DoorMeshData table.
        AOLDoor* CDO = Cast<AOLDoor>(AOLDoor::StaticClass()->GetDefaultObject());
        if (CDO)
        {
            TArray<int> ValidIdx;
            for (int i = 1; i < 24; ++i)
                if (CDO->DoorMeshData[i].NormalData.MainMesh)
                    ValidIdx.AddItem(i);
            if (ValidIdx.Num() > 0)
            {
                int Pick = ValidIdx(appRand() % ValidIdx.Num());
                GHubDoorPreview.SetStaticMesh(CDO->DoorMeshData[Pick].NormalData.MainMesh);
            }
        }

        // Prop — random mesh from the scanned list (may be empty before scan completes).
        UStaticMesh* PropMesh = OLSpawns_Props_GetRandomMesh();
        if (PropMesh)
            GHubPropPreview.SetStaticMesh(PropMesh);

        // Static meshes don't change — render every 30 frames to save GPU time.
        GHubDoorPreview.SetRenderInterval(30);
        GHubPropPreview.SetRenderInterval(30);

        GHubPreviewsLoaded = true;
    }

    if (!OLSpawns_IsHubVisible())
        return;

    GHubEnemyPreview.Tick(GOLPreviewYaw, 50.f);
    GHubDoorPreview.Tick(GOLPreviewYaw, 50.f);
    GHubPropPreview.Tick(GOLPreviewYaw, 50.f);
}

bool OLSpawns_IsHubVisible()     { return GSpawnsSubTab == NULL; }
bool OLSpawns_IsEnemyTabActive() { return GSpawnsSubTab == OLImGui_TabSpawns_Enemies; }
bool OLSpawns_IsFogTabActive()   { return GSpawnsSubTab == OLImGui_TabSpawns_Fog; }

// Called from D3D9Drv on overlay open — next tick picks a new random asset.
void OLSpawns_ResetHubPreviews()
{
    GHubPreviewsLoaded = false;
}

void OLSpawns_ShutdownHubPreviews()
{
    GHubEnemyPreview.Shutdown();
    GHubDoorPreview.Shutdown();
    GHubPropPreview.Shutdown();
    GHubPreviewsLoaded = false;
}

// ---------------------------------------------------------------------------
// Hub grid
// ---------------------------------------------------------------------------

static void DrawSpawnsHub()
{
    ImGui::Spacing();

    // Row width must be captured once at the start of each row: it shrinks
    // after every ImGui::SameLine() as tiles eat into it, so re-querying it
    // per-tile would make later tiles in the row smaller than the first.
    const float Row1Width = ImGui::GetContentRegionAvail().x;

    // Tile 0 — Enemies
    void* EnemyImg = GHubEnemyPreview.IsInited() ? GHubEnemyPreview.GetImTextureID() : NULL;
    if (DrawTile("enemies", (ImTextureID)EnemyImg, OLLocale_T("spawns.tile.enemies"), Row1Width))
        GSpawnsSubTab = OLImGui_TabSpawns_Enemies;

    // Tile 1 — Lighting (static image, no representative mesh to preview)
    void* LightingImg = GetHubTileTexture("tile_spawns_lighting");
    ImGui::SameLine(0.f, TILE_GAP);
    if (DrawTile("lighting", (ImTextureID)LightingImg, OLLocale_T("spawns.tile.lighting"), Row1Width))
        GSpawnsSubTab = OLImGui_TabSpawns_Lighting;

    // Tile 2 — Doors
    void* DoorImg = GHubDoorPreview.IsInited() ? GHubDoorPreview.GetImTextureID() : NULL;
    ImGui::SameLine(0.f, TILE_GAP);
    if (DrawTile("doors", (ImTextureID)DoorImg, OLLocale_T("spawns.tile.doors"), Row1Width))
        GSpawnsSubTab = OLImGui_TabSpawns_Doors;

    // Tile 3 — Props (new row — width must be re-queried here)
    const float Row2Width = ImGui::GetContentRegionAvail().x;
    void* PropImg = GHubPropPreview.IsInited() ? GHubPropPreview.GetImTextureID() : NULL;
    if (DrawTile("props", (ImTextureID)PropImg, OLLocale_T("spawns.tile.props"), Row2Width))
        GSpawnsSubTab = OLImGui_TabSpawns_Props;

    // Tile 4 — Fog (static image, no representative mesh to preview)
    void* FogImg = GetHubTileTexture("tile_spawns_fog");
    ImGui::SameLine(0.f, TILE_GAP);
    if (DrawTile("fog", (ImTextureID)FogImg, OLLocale_T("spawns.tile.fog"), Row2Width))
        GSpawnsSubTab = OLImGui_TabSpawns_Fog;
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------

void OLImGui_TabSpawns()
{
    if (GSpawnsSubTab)
        GSpawnsSubTab();
    else
        DrawSpawnsHub();
}

// Called by sub-tabs that want to return to the hub (e.g. DrawEnemyList "< Back").
void OLSpawns_BackToHub()
{
    GSpawnsSubTab = NULL;
}

// ---------------------------------------------------------------------------
// Despawn all test actors placed via the Spawns editor tabs — called on map
// change (CALLBACK_PreLoadMap) and on player death (AOLHero::Die), so testing
// props/doors/lights/enemies never carries over across a reload. Each tab
// tags its spawned actors on creation; one pass over PersistentLevel::Actors
// is enough to catch all four kinds.
// ---------------------------------------------------------------------------
void OLSpawns_DespawnTestActors()
{
    if (!GWorld || !GWorld->PersistentLevel)
        return;

    static const FName TagProp    (TEXT("CustomSpawnedProp"));
    static const FName TagDoor    (TEXT("CustomSpawnedDoor"));
    static const FName TagLight   (TEXT("CustomSpawnedLight"));
    static const FName TagEnemy   (TEXT("CustomEnemy"));

    ULevel* PL = GWorld->PersistentLevel;
    for (int i = PL->Actors.Num() - 1; i >= 0; --i)
    {
        AActor* A = PL->Actors(i);
        if (!A || A->IsPendingKill()) continue;
        if (A->Tag == TagProp || A->Tag == TagDoor || A->Tag == TagLight || A->Tag == TagEnemy)
            GWorld->DestroyActor(A);
    }
}

// ---------------------------------------------------------------------------
// Placement confirm dispatch — centralised handler registry.
//
// Each sub-tab registers a confirm handler via OLSpawns_RegisterConfirmHandler.
// OLSpawns_DispatchConfirm iterates the list and calls each handler in order;
// the first one that returns true owns the ghost and the loop stops.
//
// Handler signature: bool(AOLPlayerController*)
//   - Return true  → ghost was yours; you must clear bPlacementConfirmPending.
//   - Return false → not your ghost; leave it for the next handler.
// ---------------------------------------------------------------------------

typedef bool (*FPlacementConfirmFn)(AOLPlayerController*);

static const int         MAX_CONFIRM_HANDLERS = 16;
static FPlacementConfirmFn GConfirmHandlers[MAX_CONFIRM_HANDLERS];
static int                 GConfirmHandlerCount = 0;

void OLSpawns_RegisterConfirmHandler(FPlacementConfirmFn Fn)
{
    if (GConfirmHandlerCount >= MAX_CONFIRM_HANDLERS) return;
    // Avoid double-registration.
    for (int i = 0; i < GConfirmHandlerCount; ++i)
        if (GConfirmHandlers[i] == Fn) return;
    GConfirmHandlers[GConfirmHandlerCount++] = Fn;
}

// Call once per tick (always, not just when menu is open).
// Finds the first registered handler that accepts the pending ghost.
void OLSpawns_DispatchConfirm()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->bPlacementConfirmPending || !PC->PlacementGhost)
        return;
    for (int i = 0; i < GConfirmHandlerCount; ++i)
    {
        if (GConfirmHandlers[i](PC))
            return;
    }
}
