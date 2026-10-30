/*=============================================================================
    OLImGui.cpp — ImGui UI content for the Outlast debug overlay.
    Owns: scene/detail snapshots, property write queue, FOLImGuiTicker,
          action registry, and OLImGui_BuildUI() called each frame by D3D9Drv.
    D3D9Drv owns only: device init, NewFrame, Render(), device objects.
=============================================================================*/

// ImGui headers must come before UE3 (UE3 overrides operator new/delete).
#include "imgui_compat_msvc2012.h"
#include "imgui.h"

// UE3 core.
#include "OLGame.h"

// Shared types and extern declarations.
#include "ImGuiShared.h"
#include "OLImGui.h"

// Tab entry points.
#include "OLImGui_Tabs.h"


// Asset preview system.
#include "OLImGui_AssetPreview.h"

// Console window.
#include "OLImGui_Console.h"

// Player state snapshot.
#include "OLCmd_Player.h"

// MP HUD snapshot filler (needs Multiplayer types, isolated in MultiplayerHUD.cpp).
#include "..\..\Multiplayer\Inc\MultiplayerHUD.h"
#include "OLImGui_LoadingOverlay.h"

// ---------------------------------------------------------------------------
// Shared globals — defined here, declared extern in ImGuiShared.h
// ---------------------------------------------------------------------------

volatile int GSceneObjCount = 0;

FDetailSnap*  GDetailFront = NULL;
FDetailSnap*  GDetailBack  = NULL;
volatile int  GDetailReady = 0;

UObject* volatile GDetailRequest = NULL;

FPropWrite   GWriteQueue[WRITE_QUEUE_SIZE];
volatile int GWriteCount = 0;

volatile OLImGuiCallFn GCallQueue[GCallQueueCapacity];
volatile int           GCallCount = 0;

volatile OLImGuiCallFn GPostTickQueue[GPostTickQueueCapacity];
volatile int           GPostTickCount = 0;

const char* GOLPendingCmd = NULL;
void OLImGui_ExecPendingCmd() { if (GOLPendingCmd) OLImGui_ExecConsole(GOLPendingCmd); }

int GOLWinX = 0, GOLWinY = 0, GOLWinW = 0, GOLWinH = 0;

FMpHudSnapshot GMpHudFront;
FMpHudSnapshot GMpHudBack;
volatile int   GMpHudReady = 0;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void TCHARToCharBuf(char* Dst, int Len, const TCHAR* Src)
{
    int i = 0;
    while (i < Len - 1 && Src[i]) { Dst[i] = (char)Src[i]; ++i; }
    Dst[i] = '\0';
}

// Enqueue a zero-arg function to be called on the game thread next tick (inside InTick).
// Safe to call from the render thread.
void OLImGui_EnqueueCall(OLImGuiCallFn Fn)
{
    int Slot = GCallCount;
    if (Slot < GCallQueueCapacity)
    {
        GCallQueue[Slot] = Fn;
        GCallCount = Slot + 1;
    }
}

static void OLImGui_DrainCalls()
{
    int Count = GCallCount;
    GCallCount = 0;
    for (int i = 0; i < Count; ++i)
    {
        OLImGuiCallFn Fn = GCallQueue[i];
        if (Fn) Fn();
    }
}

// Enqueue a function to run AFTER GWorld::Tick (InTick==0).
// Use for operations forbidden during InTick (Streammap, GC-triggering code).
void OLImGui_EnqueuePostTickCall(OLImGuiCallFn Fn)
{
    int Slot = GPostTickCount;
    if (Slot < GPostTickQueueCapacity)
    {
        GPostTickQueue[Slot] = Fn;
        GPostTickCount = Slot + 1;
    }
}

// FCallbackEventObserver that drains GPostTickQueue after each world tick.
class FOLImGuiPostTickObserver : public FCallbackEventObserver
{
public:
    virtual void Send(ECallbackEventType InType)
    {
        int Count = GPostTickCount;
        GPostTickCount = 0;
        for (int i = 0; i < Count; ++i)
        {
            OLImGuiCallFn Fn = GPostTickQueue[i];
            if (Fn) Fn();
        }
    }
};
static FOLImGuiPostTickObserver GPostTickObserver;

// ---------------------------------------------------------------------------
// OLImGui_BuildUI — called from D3D9Drv each frame between NewFrame/Render
// ---------------------------------------------------------------------------

void OLImGui_BuildUI()
{
    // MP HUD overlay — always drawn regardless of debug window state.
    MpHud_DrawOverlay();

    // Loading progress overlay — always drawn regardless of debug window state.
    OLLoadingOverlay_Draw();

    // Placement mode overlay — ellipse on floor under ghost.
    OLSpawns_DrawPlacementOverlay();

    // Key-bind popup (must be called every frame to render the BeginPopup).
    OLImGui_Bindings_DrawPopup();
    OLImGui_Bindings_RenderPopup();

    // Fade state: single alpha value that moves toward the target each frame.
    // Open duration ~0.12s, close ~0.07s — time-based, independent of FPS.
    static float GFadeAlpha = 0.f;

    const float dt           = ImGui::GetIO().DeltaTime;
    const float FADE_IN_SPD  = 1.f / 0.12f; // alpha units per second
    const float FADE_OUT_SPD = 1.f / 0.07f;

    if (GImGuiShowDemoWindow)
    {
        GFadeAlpha += FADE_IN_SPD * dt;
        if (GFadeAlpha > 1.f) GFadeAlpha = 1.f;
    }
    else
    {
        GFadeAlpha -= FADE_OUT_SPD * dt;
        if (GFadeAlpha <= 0.f)
        {
            GFadeAlpha = 0.f;
            return; // fully faded out — nothing to draw
        }
    }

    // On first open: auto-size to content and center the window.
    static bool   bFirstOpen     = true;
    static ImVec2 GSavedMousePos = ImVec2(-1.f, -1.f);

    ImGuiIO& WIO = ImGui::GetIO();

    if (bFirstOpen)
    {
        ImGui::SetNextWindowSize(ImVec2(630.f, 730.f));
        ImGui::SetNextWindowPos(
            ImVec2(WIO.DisplaySize.x * 0.5f, WIO.DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }
    else if (GSavedMousePos.x >= 0.f)
    {
        WIO.MousePos   = GSavedMousePos;
        GSavedMousePos = ImVec2(-1.f, -1.f);
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 200.f), ImVec2(FLT_MAX, FLT_MAX));

    ImGuiWindowFlags WinFlags = ImGuiWindowFlags_NoCollapse;

    // Title centered via style — no manual TextUnformatted needed.
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);

    // Apply fade alpha to the entire window.
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, GFadeAlpha);

    bool bOpen = true;
    ImGui::Begin("OpenOL", &bOpen, WinFlags);

    // Export window rect so D3D9Drv can sample the backbuffer for blur.
    {
        ImVec2 WPos  = ImGui::GetWindowPos();
        ImVec2 WSize = ImGui::GetWindowSize();
        GOLWinX = (int)WPos.x;
        GOLWinY = (int)WPos.y;
        GOLWinW = (int)WSize.x;
        GOLWinH = (int)WSize.y;
    }

    if (!bOpen)
    {
        GSavedMousePos       = WIO.MousePos;
        GImGuiShowDemoWindow = false;
        WIO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ::SetCursor(NULL);
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    if (bFirstOpen)
        bFirstOpen = false;

    ImGui::Text("FPS: %.1f", WIO.Framerate);

    const float ConsoleH = ImGui::GetTextLineHeightWithSpacing() * 3.f + ImGui::GetStyle().WindowPadding.y * 2.f;

    ImGui::BeginChild("##tabs_region", ImVec2(0.f, -ConsoleH), false, ImGuiWindowFlags_NoScrollbar);
    if (ImGui::BeginTabBar("##tabs"))
    {
        if (ImGui::BeginTabItem(OLLocale_T("tab.actions")))
        {
            OLImGui_TabActions();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(OLLocale_T("tab.player")))
        {
            OLImGui_TabPlayer();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(OLLocale_T("tab.visuals")))
        {
            OLImGui_TabVisuals();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(OLLocale_T("tab.spawns")))
        {
            OLImGui_TabSpawns();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(OLLocale_T("tab.scene")))
        {
            OLImGui_TabScene();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(OLLocale_T("tab.relay")))
        {
            OLImGui_TabRelay();
            ImGui::EndTabItem();
        }
        if (OLImGui_CheckpointsAvailable())
        {
            if (ImGui::BeginTabItem(OLLocale_T("tab.checkpoints")))
            {
                OLImGui_TabCheckpoints();
                ImGui::EndTabItem();
            }
        }
        if (ImGui::BeginTabItem(OLLocale_T("tab.system")))
        {
            OLImGui_TabSystem();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();

    OLImGui_Console_Draw();

    ImGui::End();

    ImGui::PopStyleVar(); // fade alpha
}

// ---------------------------------------------------------------------------
// FOLImGuiTicker — game thread: drains write queue, rebuilds snapshots
// ---------------------------------------------------------------------------

class FOLImGuiTicker : public FTickableObject
{
public:
    virtual void Tick(FLOAT /*DeltaTime*/)
    {


        // 1. Apply pending property writes from the rendering thread.
        int WriteCount = GWriteCount;
        GWriteCount = 0;
        for (int i = 0; i < WriteCount; ++i)
        {
            const FPropWrite& W = GWriteQueue[i];
            BYTE* Base = (BYTE*)W.Actor + W.Offset;
            switch (W.Type)
            {
            case PWT_Bool:
                if (W.NewValue.Bool) *(BITFIELD*)Base |=  W.BitMask;
                else                 *(BITFIELD*)Base &= ~W.BitMask;
                break;
            case PWT_Int:   *(INT*)Base   = W.NewValue.Int;   break;
            case PWT_Float: *(FLOAT*)Base = W.NewValue.Float; break;
            case PWT_Byte:  *(BYTE*)Base  = W.NewValue.Byte;  break;
            case PWT_Str:   *(FString*)Base = FString(ANSI_TO_TCHAR(W.StrValue)); break;
            case PWT_Name:  *(FName*)Base   = FName(ANSI_TO_TCHAR(W.StrValue));   break;
            case PWT_Vector:
                ((FVector*)Base)->X = W.NewValue.Vec[0];
                ((FVector*)Base)->Y = W.NewValue.Vec[1];
                ((FVector*)Base)->Z = W.NewValue.Vec[2];
                break;
            case PWT_Rotator:
                ((FRotator*)Base)->Pitch = W.NewValue.Rot[0];
                ((FRotator*)Base)->Yaw   = W.NewValue.Rot[1];
                ((FRotator*)Base)->Roll  = W.NewValue.Rot[2];
                break;
            case PWT_Color:
                ((FColor*)Base)->R = (BYTE)(W.NewValue.Col[0] * 255.f);
                ((FColor*)Base)->G = (BYTE)(W.NewValue.Col[1] * 255.f);
                ((FColor*)Base)->B = (BYTE)(W.NewValue.Col[2] * 255.f);
                ((FColor*)Base)->A = (BYTE)(W.NewValue.Col[3] * 255.f);
                break;
            case PWT_LinearColor:
                ((FLinearColor*)Base)->R = W.NewValue.Col[0];
                ((FLinearColor*)Base)->G = W.NewValue.Col[1];
                ((FLinearColor*)Base)->B = W.NewValue.Col[2];
                ((FLinearColor*)Base)->A = W.NewValue.Col[3];
                break;
            // Array element writes: Base points to TArray<T>; ArrayIdx selects element.
            case PWT_ArrayBool:
            {
                FScriptArray& Arr = *(FScriptArray*)Base;
                if (W.ArrayIdx >= 0 && W.ArrayIdx < Arr.Num()) {
                    BITFIELD* Elem = (BITFIELD*)((BYTE*)Arr.GetData() + W.ArrayIdx * sizeof(BITFIELD));
                    if (W.NewValue.Bool) *Elem |=  W.BitMask;
                    else                 *Elem &= ~W.BitMask;
                }
                break;
            }
            case PWT_ArrayInt:
            {
                FScriptArray& Arr = *(FScriptArray*)Base;
                if (W.ArrayIdx >= 0 && W.ArrayIdx < Arr.Num())
                    ((INT*)Arr.GetData())[W.ArrayIdx] = W.NewValue.Int;
                break;
            }
            case PWT_ArrayFloat:
            {
                FScriptArray& Arr = *(FScriptArray*)Base;
                if (W.ArrayIdx >= 0 && W.ArrayIdx < Arr.Num())
                    ((FLOAT*)Arr.GetData())[W.ArrayIdx] = W.NewValue.Float;
                break;
            }
            case PWT_ArrayByte:
            {
                FScriptArray& Arr = *(FScriptArray*)Base;
                if (W.ArrayIdx >= 0 && W.ArrayIdx < Arr.Num())
                    ((BYTE*)Arr.GetData())[W.ArrayIdx] = W.NewValue.Byte;
                break;
            }
            case PWT_ArrayStr:
            {
                FScriptArray& Arr = *(FScriptArray*)Base;
                if (W.ArrayIdx >= 0 && W.ArrayIdx < Arr.Num())
                    ((FString*)Arr.GetData())[W.ArrayIdx] = FString(ANSI_TO_TCHAR(W.StrValue));
                break;
            }
            case PWT_ArrayName:
            {
                FScriptArray& Arr = *(FScriptArray*)Base;
                if (W.ArrayIdx >= 0 && W.ArrayIdx < Arr.Num())
                    ((FName*)Arr.GetData())[W.ArrayIdx] = FName(ANSI_TO_TCHAR(W.StrValue));
                break;
            }
            }
        }

        // 2. Drain pending game-thread calls (enqueued by render thread via OLImGui_EnqueueCall).
        OLImGui_DrainCalls();

        // 3. Refresh MP HUD snapshot (always, not gated on overlay visibility).
        MpHud_UpdateSnapshot();

        // 3z. Refresh loading progress snapshot (always, not gated on overlay visibility).
        OLLoadingOverlay_UpdateSnapshot();

        // 3a. Checkpoint snapshot — always ticked so the tab visibility flag stays current.
        OLImGui_Checkpoints_Tick();

        // 3b. Placement confirm — must run every tick even when menu is closed.
        OLSpawns_TickPlacement();
        OLSpawns_TickLightPlacement();
        OLSpawns_TickLightConfirm();
        OLSpawns_TickLightList();
        OLSpawns_TickDoors();
        OLSpawns_ScanProps();
        OLSpawns_TickFog();
        OLSpawns_DispatchConfirm();

        // 3c. Console scrollback snapshot.
        OLImGui_Console_Tick();

        if (!GImGuiShowDemoWindow)
            return;

        // 4. Drive async object loads (IO already done in IO thread, UObject work here).
        Utils::TickAsyncObjectLoader();

        // 4. Re-render asset previews — only while menu is open.
        OLSpawns_TickProps();
        OLSpawns_TickPreviews();

        // 4a. Refresh player state snapshot for render thread.
        OLCmd_Player_UpdateSnapshot();

        // 4. Publish total object count — render thread uses it as Clipper row count
        //    and accesses objects via GetIndexedObject for visible rows only.
        GSceneObjCount = UObject::GetObjectArrayNum();

        // 5. Level-2: rebuild detail props for the actor the render thread has open.
        UObject* Requested = GDetailRequest;
        GDetailRequest = NULL;
        if (Requested && GDetailBack)
        {
            FDetailSnap& D = *GDetailBack;
            D.ActorPtr = Requested;
            D.Groups.Reset();

            for (UClass* Class = Requested->GetClass(); Class; Class = Class->GetSuperClass())
            {
                FPropGroup Group;
                TCHARToCharBuf(Group.ClassName, SNAP_NAME_LEN, *Class->GetName());

                for (UProperty* Prop = Class->PropertyLink; Prop; Prop = Prop->PropertyLinkNext)
                {
                    if (Prop->GetOwnerClass() != Class) continue;

                    FPropSnap PS;
                    TCHARToCharBuf(PS.Name, SNAP_NAME_LEN, *Prop->GetName());
                    PS.Offset      = Prop->Offset;
                    PS.ArrayIdx    = -1;
                    PS.BitMask     = 0;
                    PS.StrValue[0] = '\0';
                    PS.TexPtr      = NULL;

                    if (UBoolProperty* BP = Cast<UBoolProperty>(Prop)) {
                        PS.Type = PST_Bool; PS.BitMask = BP->BitMask;
                        PS.Value.Bool = !!(*(BITFIELD*)((BYTE*)Requested + BP->Offset) & BP->BitMask);
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UIntProperty* IP = Cast<UIntProperty>(Prop)) {
                        PS.Type = PST_Int;
                        PS.Value.Int = *(INT*)((BYTE*)Requested + IP->Offset);
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UFloatProperty* FP = Cast<UFloatProperty>(Prop)) {
                        PS.Type = PST_Float;
                        PS.Value.Float = *(FLOAT*)((BYTE*)Requested + FP->Offset);
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UByteProperty* YP = Cast<UByteProperty>(Prop)) {
                        PS.Type = PST_Byte;
                        PS.Value.Byte = *(BYTE*)((BYTE*)Requested + YP->Offset);
                        if (YP->Enum) {
                            int NV = YP->Enum->NumEnums();
                            for (int ei = 0; ei < NV; ++ei)
                                PS.EnumValues.AddItem(YP->Enum->GetEnum(ei).ToString());
                        }
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UNameProperty* NP = Cast<UNameProperty>(Prop)) {
                        PS.Type = PST_Name;
                        TCHARToCharBuf(PS.StrValue, SNAP_STR_LEN,
                            *(*(FName*)((BYTE*)Requested + NP->Offset)).ToString());
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UStrProperty* SP = Cast<UStrProperty>(Prop)) {
                        PS.Type = PST_Str;
                        TCHARToCharBuf(PS.StrValue, SNAP_STR_LEN,
                            *(*(FString*)((BYTE*)Requested + SP->Offset)));
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UObjectProperty* OP = Cast<UObjectProperty>(Prop)) {
                        UObject* Val = *(UObject**)((BYTE*)Requested + OP->Offset);
                        // Show textures as preview images; other objects as name strings.
                        if (Val && Val->IsA(UTexture2D::StaticClass())) {
                            PS.Type   = PST_Texture;
                            PS.TexPtr = Val;
                            TCHARToCharBuf(PS.StrValue, SNAP_STR_LEN, *Val->GetName());
                        } else {
                            PS.Type   = PST_Object;
                            PS.TexPtr = Val; // may be NULL; used for navigation on render thread
                            TCHARToCharBuf(PS.StrValue, SNAP_STR_LEN, Val ? *Val->GetName() : TEXT("None"));
                        }
                        Group.Props.AddItem(PS); continue;
                    }
                    if (UStructProperty* STP = Cast<UStructProperty>(Prop)) {
                        if (STP->Struct) {
                            FName SName = STP->Struct->GetFName();
                            if (SName == NAME_Vector) {
                                FVector& V = *(FVector*)((BYTE*)Requested + STP->Offset);
                                PS.Type = PST_Vector;
                                PS.Value.Vec[0] = V.X; PS.Value.Vec[1] = V.Y; PS.Value.Vec[2] = V.Z;
                                Group.Props.AddItem(PS); continue;
                            }
                            if (SName == NAME_Rotator) {
                                FRotator& R = *(FRotator*)((BYTE*)Requested + STP->Offset);
                                PS.Type = PST_Rotator;
                                PS.Value.Rot[0] = R.Pitch; PS.Value.Rot[1] = R.Yaw; PS.Value.Rot[2] = R.Roll;
                                Group.Props.AddItem(PS); continue;
                            }
                            if (SName == FName(TEXT("Color"))) {
                                FColor& C = *(FColor*)((BYTE*)Requested + STP->Offset);
                                PS.Type = PST_Color;
                                PS.Value.Col[0] = C.R / 255.f;
                                PS.Value.Col[1] = C.G / 255.f;
                                PS.Value.Col[2] = C.B / 255.f;
                                PS.Value.Col[3] = C.A / 255.f;
                                Group.Props.AddItem(PS); continue;
                            }
                            if (SName == FName(TEXT("LinearColor"))) {
                                FLinearColor& LC = *(FLinearColor*)((BYTE*)Requested + STP->Offset);
                                PS.Type = PST_LinearColor;
                                PS.Value.Col[0] = LC.R;
                                PS.Value.Col[1] = LC.G;
                                PS.Value.Col[2] = LC.B;
                                PS.Value.Col[3] = LC.A;
                                Group.Props.AddItem(PS); continue;
                            }
                        }
                    }
                    if (UArrayProperty* AP = Cast<UArrayProperty>(Prop)) {
                        UProperty* Inner = AP->Inner;
                        if (!Inner) continue;
                        FScriptArray& Arr = *(FScriptArray*)((BYTE*)Requested + AP->Offset);
                        const INT Count = Arr.Num();
                        if (Count < 0 || Count > 512) continue; // sanity guard

                        // Header sentinel: stores element count in Value.Int.
                        PS.Type      = PST_ArrayHeader;
                        PS.Value.Int = Count;
                        Group.Props.AddItem(PS);

                        // One FPropSnap per element.
                        BYTE* Data = (BYTE*)Arr.GetData();

                        for (INT ei = 0; ei < Count; ++ei)
                        {
                            FPropSnap EPS;
                            _snprintf(EPS.Name, SNAP_NAME_LEN, "[%d]", ei);
                            EPS.Offset      = AP->Offset; // TArray offset in object
                            EPS.ArrayIdx    = ei;
                            EPS.BitMask     = 0;
                            EPS.StrValue[0] = '\0';
                            EPS.TexPtr      = NULL;

                            BYTE* ElemPtr = Data + ei * Inner->ElementSize;

                            if (Cast<UBoolProperty>(Inner)) {
                                UBoolProperty* BP = Cast<UBoolProperty>(Inner);
                                EPS.Type      = PST_Bool;
                                EPS.BitMask   = BP->BitMask;
                                EPS.Value.Bool = !!(*(BITFIELD*)ElemPtr & BP->BitMask);
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (Cast<UIntProperty>(Inner)) {
                                EPS.Type      = PST_Int;
                                EPS.Value.Int = *(INT*)ElemPtr;
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (Cast<UFloatProperty>(Inner)) {
                                EPS.Type        = PST_Float;
                                EPS.Value.Float = *(FLOAT*)ElemPtr;
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (UByteProperty* YP = Cast<UByteProperty>(Inner)) {
                                EPS.Type       = PST_Byte;
                                EPS.Value.Byte = *(BYTE*)ElemPtr;
                                if (YP->Enum) {
                                    int NV = YP->Enum->NumEnums();
                                    for (int eei = 0; eei < NV; ++eei)
                                        EPS.EnumValues.AddItem(YP->Enum->GetEnum(eei).ToString());
                                }
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (Cast<UNameProperty>(Inner)) {
                                EPS.Type = PST_Name;
                                TCHARToCharBuf(EPS.StrValue, SNAP_STR_LEN,
                                    *(*(FName*)ElemPtr).ToString());
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (Cast<UStrProperty>(Inner)) {
                                EPS.Type = PST_Str;
                                TCHARToCharBuf(EPS.StrValue, SNAP_STR_LEN,
                                    *(*(FString*)ElemPtr));
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (UObjectProperty* OP = Cast<UObjectProperty>(Inner)) {
                                UObject* Val = *(UObject**)ElemPtr;
                                EPS.Type   = PST_Object;
                                EPS.TexPtr = Val;
                                TCHARToCharBuf(EPS.StrValue, SNAP_STR_LEN,
                                    Val ? *Val->GetName() : TEXT("None"));
                                Group.Props.AddItem(EPS); continue;
                            }
                            if (UStructProperty* STP2 = Cast<UStructProperty>(Inner)) {
                                if (STP2->Struct) {
                                    FName SN = STP2->Struct->GetFName();
                                    if (SN == NAME_Vector) {
                                        FVector& V = *(FVector*)ElemPtr;
                                        EPS.Type = PST_Vector;
                                        EPS.Value.Vec[0]=V.X; EPS.Value.Vec[1]=V.Y; EPS.Value.Vec[2]=V.Z;
                                        Group.Props.AddItem(EPS); continue;
                                    }
                                    if (SN == NAME_Rotator) {
                                        FRotator& R = *(FRotator*)ElemPtr;
                                        EPS.Type = PST_Rotator;
                                        EPS.Value.Rot[0]=R.Pitch; EPS.Value.Rot[1]=R.Yaw; EPS.Value.Rot[2]=R.Roll;
                                        Group.Props.AddItem(EPS); continue;
                                    }
                                }
                            }
                            // Unknown inner type — skip element but keep header visible.
                        }
                        continue;
                    }
                }

                if (Group.Props.Num() > 0)
                    D.Groups.AddItem(Group);
            }

            Exchange(GDetailFront, GDetailBack);
            GDetailReady = 1;
        }
    }

    virtual UBOOL IsTickable() const            { return TRUE; }
    virtual UBOOL IsTickableWhenPaused() const  { return TRUE; }
};

static FOLImGuiTicker* GOLImGuiTicker = NULL;

// ---------------------------------------------------------------------------
// OLImGui_InvalidateCaches — call before PrepareMapChange or CollectGarbage.
// Clears every raw UObject* cache in the menu that is NOT protected by
// AddToRoot or a UPROPERTY chain. Add new tab invalidation calls here.
// ---------------------------------------------------------------------------
void OLImGui_InvalidateCaches()
{
    // Spawns tab: GPreviewMeshCache / GWeaponMeshCache + enemy list snap.
    OLSpawns_InvalidateMeshCache();

    // Scene tab: clear selected object, nav stack, and detail request.
    OLImGui_Scene_Invalidate();
    GDetailRequest = NULL;
    // Leave GDetailFront/Back intact — render thread reads Front;
    // game thread will overwrite Back on next request.  Safe.
}

void OLImGui_EnsureTicker()
{
    if (!GOLImGuiTicker)
    {
        if (!GDetailFront) GDetailFront = new FDetailSnap();
        if (!GDetailBack)  GDetailBack  = new FDetailSnap();
        GOLImGuiTicker = new FOLImGuiTicker();
        GCallbackEvent->Register(CALLBACK_WorldTickFinished, &GPostTickObserver);
        OLImGui_Bindings_Load();
        //OLPreview_Init();
    }
}
