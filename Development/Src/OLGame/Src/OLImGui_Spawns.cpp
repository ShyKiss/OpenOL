/*=============================================================================
    OLImGui_Spawns.cpp — Spawns hub tab.

    Grid of category tiles (3 per row); each tile opens a dedicated sub-tab.
    Currently implemented: Enemies (OLImGui_Spawns_Enemies.cpp).
=============================================================================*/

#include "OLImGui_Tabs.h"

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
// All 3 tiles + 2 gaps fit exactly into GetContentRegionAvail().x.
static float TileWidth()
{
    return (ImGui::GetContentRegionAvail().x - TILE_GAP * (TILES_PER_ROW - 1)) / (float)TILES_PER_ROW;
}

// ---------------------------------------------------------------------------
// DrawTile — draw one tile at the current cursor position.
// img   — preview texture (NULL → grey placeholder with "..." text)
// label — centred text below the image
// Returns true if the tile was clicked this frame.
// ---------------------------------------------------------------------------
static bool DrawTile(const char* id, ImTextureID img, const char* label)
{
    ImGui::PushID(id);

    const float TW    = TileWidth();
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
// Hub grid
// ---------------------------------------------------------------------------

static void DrawSpawnsHub()
{
    ImGui::Spacing();

    // Tile 0 — Enemies
    void* EnemyImg = OLPreview_GetImTextureID();
    if (DrawTile("enemies", (ImTextureID)EnemyImg, OLLocale_T("spawns.tile.enemies")))
        GSpawnsSubTab = OLImGui_TabSpawns_Enemies;

    // Next tile on the same row — uncomment and add new sub-tabs here:
    // ImGui::SameLine(0.f, TILE_GAP);
    // if (DrawTile("doors", NULL, "Doors"))
    //     GSpawnsSubTab = OLImGui_TabSpawns_Doors;

    // Tiles 2 and beyond — same pattern with SameLine between them.
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
