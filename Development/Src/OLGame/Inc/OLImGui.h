/*=============================================================================
    OLImGui.h — OLGame-side ImGui declarations.

    Included only by OLGame code (OLGame/Src/OLImGui.cpp and callers).
    Do NOT include from Engine or D3D9Drv.
=============================================================================*/

#pragma once

// Runtime tuning vectors — written/read by the ImGui debug overlay (OLGame side).
// Defined in D3D9Drv/Src/ImGuiLinker.cpp (render-thread owner).
extern volatile float GDebugVec0[3];
extern volatile float GDebugVec1[3];
