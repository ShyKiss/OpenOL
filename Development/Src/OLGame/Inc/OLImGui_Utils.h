#pragma once
// OLImGui_Utils.h — shared utilities for OLImGui tab/overlay files.
// Included by OLImGui_Tabs.h after OLGame.h — all UE3 types are available here.

#include <math.h>
#include <ctype.h>
#include "imgui.h"

// ---------------------------------------------------------------------------
// OLImGui_AddTiltedText(DL, Font, FontSize, Pos, Col, Text, TiltRad)
//   Draw text rotated around its own center.
//   TiltRad — clockwise angle in radians.
//   For a smooth ease-in-out pendulum use:
//       sinf(TimeSeconds * (2*PI / Period)) * (MaxDeg * PI/180)
//
//   Example:
//       float Tilt = sinf(T * (2.f*3.14159f/3.5f)) * 2.f * (3.14159f/180.f);
//       OLImGui_AddTiltedText(DL, Font, FontSize, Pos, Col, "Hello", Tilt);
// ---------------------------------------------------------------------------
static void OLImGui_AddTiltedText(ImDrawList* DL, ImFont* Font, float FontSize,
                                  ImVec2 Pos, ImU32 Col, const char* Text, float TiltRad)
{
    ImVec2 Sz       = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text);
    int    VtxStart = DL->VtxBuffer.Size;
    DL->AddText(Font, FontSize, Pos, Col, Text);

    float cx = Pos.x + Sz.x * 0.5f;
    float cy = Pos.y + Sz.y * 0.5f;
    float cs = cosf(TiltRad), sn = sinf(TiltRad);
    for (int vi = VtxStart; vi < DL->VtxBuffer.Size; ++vi)
    {
        ImDrawVert& v = DL->VtxBuffer[vi];
        float dx = v.pos.x - cx, dy = v.pos.y - cy;
        v.pos.x = cx + dx * cs - dy * sn;
        v.pos.y = cy + dx * sn + dy * cs;
    }
}

// ---------------------------------------------------------------------------
// OLImGui_AddJitterText(DL, Font, FontSize, Pos, Col, Text, T)
//   Draw text with a nervous jitter effect: small irregular trembling with
//   occasional sudden lurches, as if the text is frightened.
//   T — time in seconds (e.g. S.TimeSeconds).
//
//   Example:
//       OLImGui_AddJitterText(DL, Font, FontSize, ImVec2(TextX, TextY), Col, "Connecting", T, 1.f);
// ---------------------------------------------------------------------------
static void OLImGui_AddJitterText(ImDrawList* DL, ImFont* Font, float FontSize,
                                  ImVec2 Pos, ImU32 Col, const char* Text, float T,
                                  float Strength = 1.0f)
{
    // Pseudo-random trembling: many incommensurable frequencies so the pattern
    // never visibly repeats. Amplitudes are large enough to look like a puppet shaking.
    float TiltRad = Strength * (
        sinf(T * 29.0f  + 0.00f) * 0.040f +
        sinf(T * 53.7f  + 1.13f) * 0.030f +
        sinf(T * 41.3f  + 2.71f) * 0.025f +
        sinf(T * 67.1f  + 3.14f) * 0.018f +
        sinf(T * 83.9f  + 5.55f) * 0.012f +
        sinf(T * 37.2f  + 1.41f) * 0.020f);

    float ShakeX = Strength * (
        sinf(T * 43.1f  + 0.77f) * 3.0f +
        sinf(T * 71.9f  + 2.22f) * 2.0f +
        sinf(T * 59.3f  + 4.88f) * 1.5f);

    float ShakeY = Strength * (
        sinf(T * 37.7f  + 1.57f) * 2.0f +
        sinf(T * 61.1f  + 3.33f) * 1.5f +
        sinf(T * 47.3f  + 0.99f) * 1.0f);

    ImVec2 ShiftedPos = ImVec2(Pos.x + ShakeX, Pos.y + ShakeY);

    ImVec2 Sz       = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text);
    int    VtxStart = DL->VtxBuffer.Size;
    DL->AddText(Font, FontSize, ShiftedPos, Col, Text);

    float cx = ShiftedPos.x + Sz.x * 0.5f;
    float cy = ShiftedPos.y + Sz.y * 0.5f;
    float cs = cosf(TiltRad), sn = sinf(TiltRad);
    for (int vi = VtxStart; vi < DL->VtxBuffer.Size; ++vi)
    {
        ImDrawVert& v = DL->VtxBuffer[vi];
        float dx = v.pos.x - cx, dy = v.pos.y - cy;
        v.pos.x = cx + dx * cs - dy * sn;
        v.pos.y = cy + dx * sn + dy * cs;
    }
}

// ---------------------------------------------------------------------------
// OLImGui_MatchFilter(Name, Filter)
//   Case-insensitive ASCII substring search.
//   Returns true when Filter is empty or is a substring of Name.
//   Use for filter input boxes instead of strstr().
//
//   Example:
//       if (OLImGui_MatchFilter(ActorName, GSceneFilter))
//           ImGui::Selectable(ActorName, false);
// ---------------------------------------------------------------------------
FORCEINLINE bool OLImGui_MatchFilter(const char* Name, const char* Filter)
{
    if (!Filter || !Filter[0]) return true;
    for (int i = 0; Name[i]; ++i)
    {
        int j = 0;
        while (Filter[j] && (tolower((unsigned char)Name[i+j]) == tolower((unsigned char)Filter[j])))
            ++j;
        if (!Filter[j]) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// OLImGui_ExecConsole(Cmd)
//   Execute a narrow-char UE3 console command on the game thread.
//   Must be called from a function already dispatched via OLImGui_EnqueueCall.
//
//   Example:
//       static void ExecStat() { OLImGui_ExecConsole("Stat FPS"); }
//       if (ImGui::Button("FPS")) OLImGui_EnqueueCall(ExecStat);
// ---------------------------------------------------------------------------
FORCEINLINE void OLImGui_ExecConsole(const char* Cmd)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC) PC->ConsoleCommand(FString(ANSI_TO_TCHAR(Cmd)));
}
