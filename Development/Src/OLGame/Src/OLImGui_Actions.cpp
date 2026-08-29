/*=============================================================================
    OLImGui_Actions.cpp — "Actions" tab: debug actions + tuning sliders.
    Add new game-thread actions as static functions, render their widgets here.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "UnPath.h"

#if WITH_EDITOR

static void BuildPathsOnGameThread()
{
    FPathBuilder::Exec(TEXT("PREDEFINEPATHS"));
    FPathBuilder::Exec(TEXT("DEFINEPATHS REVIEWPATHS=1 SHOWMAPCHECK=0 UNDEFINEPATHS=1"));
    FPathBuilder::Exec(TEXT("BUILDCOVER FROMDEFINEPATHS=0"));
    FPathBuilder::Exec(TEXT("POSTDEFINEPATHS"));
    FPathBuilder::Exec(TEXT("BUILDNETWORKIDS"));
    FPathBuilder::Exec(TEXT("FINISHPATHBUILD"));
}

static void SpawnPylonOnGameThread()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC)
        return;

    FVector ViewLoc;
    FRotator ViewRot;
    PC->eventGetPlayerViewPoint(ViewLoc, ViewRot);

    FVector TraceEnd = ViewLoc + ViewRot.Vector() * 2000.f;
    FCheckResult Hit(1.f);
    FVector SpawnLoc = TraceEnd;
    if (!GWorld->SingleLineCheck(Hit, PC->Pawn, TraceEnd, ViewLoc, TRACE_World | TRACE_StopAtAnyHit))
        SpawnLoc = Hit.Location;

    UClass* PylonClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Pylon"));
    if (!PylonClass)
        return;

    // bNoFail=TRUE skips the bStatic/bNoDelete runtime spawn guard
    APylon* Pylon = Cast<APylon>(GWorld->SpawnActor(PylonClass, NAME_None, SpawnLoc, FRotator(0,0,0), NULL, TRUE, FALSE, NULL, NULL, TRUE));
    if (Pylon)
    {
        Pylon->ExpansionRadius = 1024.f;
        debugf(TEXT("Spawned Pylon at %s"), *SpawnLoc.ToString());
    }
}

#endif

void OLImGui_TabActions()
{
    OL_T_SECTION("actions.section.dummyhead");
    ImGui::DragFloat3(OLLocale_T("actions.origin"), (float*)GDebugVec0, 0.1f);
    ImGui::DragFloat3(OLLocale_T("actions.attach"), (float*)GDebugVec1, 0.1f);

#if WITH_EDITOR
    ImGui::Separator();
    if (ImGui::Button("Build Paths"))
        OLImGui_EnqueueCall(BuildPathsOnGameThread);
    ImGui::SameLine();
    if (ImGui::Button("Spawn Pylon"))
        OLImGui_EnqueueCall(SpawnPylonOnGameThread);
#endif
}
