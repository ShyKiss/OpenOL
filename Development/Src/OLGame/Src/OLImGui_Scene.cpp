/*=============================================================================
    OLImGui_Scene.cpp — "Scene" tab: actor list + property inspector.
=============================================================================*/
#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void TCHARToCharBuf_Scene(char* Dst, int Len, const TCHAR* Src)
{
    int i = 0;
    while (i < Len - 1 && Src[i]) { Dst[i] = (char)Src[i]; ++i; }
    Dst[i] = '\0';
}

#define ENQUEUE_WRITE(ActorPtr_, Offset_, Type_, Field_, Val_) \
    { int _s = GWriteCount; if (_s < WRITE_QUEUE_SIZE) { \
        GWriteQueue[_s].Actor = ActorPtr_; GWriteQueue[_s].Offset = (Offset_); \
        GWriteQueue[_s].Type  = Type_;     GWriteQueue[_s].NewValue.Field_ = Val_; \
        GWriteCount = _s + 1; } }

// ---------------------------------------------------------------------------
// World capture hover preview
// ---------------------------------------------------------------------------

// Pending target — either from an Actor or a component world position.
static AActor*  volatile GWCapPendingActor  = NULL;
static FVector           GWCapPendingPos;
static float             GWCapPendingRadius = 50.f;
static bool              GWCapUsePosOnly    = false;

static void WCapApplyTarget()
{
    OLWorldCapture_Ensure();
    if (!GWCapUsePosOnly && GWCapPendingActor)
    {
        // The actor may have been killed between enqueue and execution
        // (e.g. player dies, then menu is opened with stale pointer).
        AActor* A = GWCapPendingActor;
        GWCapPendingActor = NULL; // clear first — prevents double-use
        if (!A->IsPendingKill() && A->IsValid())
            OLWorldCapture_SetTarget(A);
    }
    else
        OLWorldCapture_SetPos(GWCapPendingPos, GWCapPendingRadius);
}

static void WCapEnqueue(AActor* Actor)
{
    GWCapPendingActor = Actor;
    GWCapUsePosOnly   = false;
    OLImGui_EnqueueCall(WCapApplyTarget);
}

static void WCapEnqueuePos(FVector Pos, float Radius)
{
    GWCapPendingPos    = Pos;
    GWCapPendingRadius = Radius;
    GWCapUsePosOnly    = true;
    OLImGui_EnqueueCall(WCapApplyTarget);
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static UObject* GSelectedObj    = NULL; // currently inspected object
static char     GSceneFilter[128] = "";

// Filtered index list — only used when GSceneFilter is non-empty.
// Maps visible row index → GetIndexedObject index.
static TArray<int> GFilteredIdx;
static char        GLastFilter[128] = ""; // filter string at last RebuildFilter call

// Rebuild filtered index. Only called when filter is non-empty and text changed.
static void RebuildFilter()
{
    GFilteredIdx.Reset();
    appStrncpyANSI(GLastFilter, GSceneFilter, sizeof(GLastFilter));

    const int Total = UObject::GetObjectArrayNum();
    for (int i = 0; i < Total; ++i)
    {
        UObject* Obj = UObject::GetIndexedObject(i);
        if (!Obj || Obj->IsTemplate() || Obj->IsPendingKill())
            continue;
        if (Obj->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject |
                             RF_DisregardForGC     | RF_ZombieComponent))
            continue;
        if (Obj->IsA(UField::StaticClass())   ||
            Obj->IsA(UPackage::StaticClass())  ||
            Obj->IsA(UTextBuffer::StaticClass()))
            continue;

        // Build "Level:ObjName" for matching (same format as the list label).
        char Name[SNAP_NAME_LEN * 2 + 2];
        {
            char LvlBuf[SNAP_NAME_LEN] = "";
            char ObjBuf[SNAP_NAME_LEN];
            if (Obj->GetOuter())
                TCHARToCharBuf_Scene(LvlBuf, SNAP_NAME_LEN, *Obj->GetOuter()->GetFName().ToString());
            TCHARToCharBuf_Scene(ObjBuf, SNAP_NAME_LEN, *Obj->GetFName().ToString());
            if (LvlBuf[0])
                _snprintf(Name, sizeof(Name), "%s:%s", LvlBuf, ObjBuf);
            else
                appStrncpyANSI(Name, ObjBuf, sizeof(Name));
        }
        bool Found = false;
        for (int h = 0; Name[h] && !Found; ++h)
        {
            int hi = h, ni = 0;
            while (Name[hi] && GSceneFilter[ni] &&
                   (Name[hi]|0x20) == (GSceneFilter[ni]|0x20))
            { ++hi; ++ni; }
            if (!GSceneFilter[ni]) Found = true;
        }
        if (Found) GFilteredIdx.AddItem(i);
    }
}

// ---------------------------------------------------------------------------
// Property inspector screen
// ---------------------------------------------------------------------------

static char GPropFilter[128] = ""; // property name search filter

static bool PropMatchesFilter(const char* PropName, const char* Filter)
{
    if (!Filter[0]) return true;
    // Case-insensitive substring search.
    for (int h = 0; PropName[h]; ++h)
    {
        int hi = h, ni = 0;
        while (PropName[hi] && Filter[ni] &&
               (PropName[hi] | 0x20) == (Filter[ni] | 0x20))
        { ++hi; ++ni; }
        if (!Filter[ni]) return true;
    }
    return false;
}

static void DrawPropertyInspector(UObject* Obj)
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        GSelectedObj   = NULL;
        GPropFilter[0] = '\0';
        return;
    }

    ImGui::SameLine();
    OL_SEARCH("##propfilter", "scene.search.properties", GPropFilter, sizeof(GPropFilter));

    char ObjName[SNAP_NAME_LEN];
    TCHARToCharBuf_Scene(ObjName, SNAP_NAME_LEN, *Obj->GetFName().ToString());
    ImGui::TextUnformatted(ObjName);
    ImGui::Separator();

    bool DetailReady = GDetailFront &&
                       GDetailFront->ActorPtr == Obj &&
                       GDetailFront->Groups.Num() > 0;
    GDetailRequest = Obj;

    if (!DetailReady)
    {
        ImGui::TextDisabled("Loading properties...");
        return;
    }

    ImGui::BeginChild("##props", ImVec2(0, 0), false);

    const FDetailSnap& D = *GDetailFront;
    const bool bPropFiltered = (GPropFilter[0] != '\0');
    for (int g = 0; g < D.Groups.Num(); ++g)
    {
        const FPropGroup& G = D.Groups(g);

        // Skip group entirely if no property matches the filter.
        if (bPropFiltered)
        {
            bool bAny = false;
            for (int p = 0; p < G.Props.Num(); ++p)
                if (PropMatchesFilter(G.Props(p).Name, GPropFilter)) { bAny = true; break; }
            if (!bAny) continue;
        }

        ImGui::PushID(g);
        ImGuiTreeNodeFlags HdrFlags = ImGuiTreeNodeFlags_DefaultOpen |
                                      ImGuiTreeNodeFlags_SpanAvailWidth;
        bool GroupOpen = ImGui::CollapsingHeader(G.ClassName, HdrFlags);
        if (GroupOpen)
        {
            if (ImGui::BeginTable("##props", 2,
                ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit |
                ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn(OLLocale_T("scene.col.property"), ImGuiTableColumnFlags_WidthFixed, 160.0f);
                ImGui::TableSetupColumn(OLLocale_T("scene.col.value"),    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (int p = 0; p < G.Props.Num(); ++p)
                {
                    const FPropSnap& PS = G.Props(p);
                    if (bPropFiltered && !PropMatchesFilter(PS.Name, GPropFilter))
                        continue;
                    ImGui::PushID(p);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(PS.Name);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);

                    switch (PS.Type)
                    {
                    case PST_Bool:
                    {
                        bool Val = PS.Value.Bool;
                        if (ImGui::Checkbox("##v", &Val))
                        {
                            int Slot = GWriteCount;
                            if (Slot < WRITE_QUEUE_SIZE)
                            {
                                GWriteQueue[Slot].Actor         = Obj;
                                GWriteQueue[Slot].Offset        = PS.Offset;
                                GWriteQueue[Slot].Type          = PWT_Bool;
                                GWriteQueue[Slot].BitMask       = PS.BitMask;
                                GWriteQueue[Slot].NewValue.Bool = Val;
                                GWriteCount = Slot + 1;
                            }
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_Int:
                    {
                        int Val = (int)PS.Value.Int;
                        if (ImGui::DragInt("##v", &Val))
                        {
                            ENQUEUE_WRITE(Obj, PS.Offset, PWT_Int, Int, (INT)Val);
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_Float:
                    {
                        float Val = PS.Value.Float;
                        if (ImGui::DragFloat("##v", &Val, 0.01f))
                        {
                            ENQUEUE_WRITE(Obj, PS.Offset, PWT_Float, Float, Val);
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_Byte:
                    {
                        if (PS.EnumValues.Num() > 0)
                        {
                            // Enum combo.
                            int Val = (int)PS.Value.Byte;
                            static char B[SNAP_NAME_LEN];
                            const FString& Cur = PS.EnumValues(Val < PS.EnumValues.Num() ? Val : 0);
                            int i = 0;
                            const TCHAR* S = *Cur;
                            while (i < SNAP_NAME_LEN - 1 && S[i]) { B[i] = (char)S[i]; ++i; }
                            B[i] = '\0';
                            if (ImGui::BeginCombo("##v", B))
                            {
                                for (int ei = 0; ei < PS.EnumValues.Num(); ++ei)
                                {
                                    const FString& EV = PS.EnumValues(ei);
                                    int j = 0; const TCHAR* ES = *EV;
                                    while (j < SNAP_NAME_LEN - 1 && ES[j]) { B[j] = (char)ES[j]; ++j; }
                                    B[j] = '\0';
                                    bool Sel = (ei == Val);
                                    if (ImGui::Selectable(B, Sel))
                                    {
                                        ENQUEUE_WRITE(Obj, PS.Offset, PWT_Byte, Byte, (BYTE)ei);
                                        GDetailRequest = Obj;
                                    }
                                    if (Sel) ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else
                        {
                            int Val = (int)PS.Value.Byte;
                            if (ImGui::DragInt("##v", &Val, 1.0f, 0, 255))
                            {
                                ENQUEUE_WRITE(Obj, PS.Offset, PWT_Byte, Byte, (BYTE)Val);
                                GDetailRequest = Obj;
                            }
                        }
                        break;
                    }
                    case PST_Name:
                    case PST_Str:
                    {
                        // One shared edit buffer per object snapshot.
                        // Keyed by (ObjPtr, g, p) — reset when the object changes.
                        static char    SEditBuf[SNAP_STR_LEN] = "";
                        static UObject* SEditObj = NULL;
                        static int     SEditG = -1, SEditP = -1;

                        // Re-seed from snapshot whenever object or property changes.
                        if (SEditObj != Obj || SEditG != g || SEditP != p)
                        {
                            appStrncpyANSI(SEditBuf, PS.StrValue, SNAP_STR_LEN);
                            SEditObj = Obj; SEditG = g; SEditP = p;
                        }

                        bool Done = ImGui::InputText("##v", SEditBuf, SNAP_STR_LEN,
                            ImGuiInputTextFlags_EnterReturnsTrue);
                        if (Done)
                        {
                            int Slot = GWriteCount;
                            if (Slot < WRITE_QUEUE_SIZE)
                            {
                                GWriteQueue[Slot].Actor  = Obj;
                                GWriteQueue[Slot].Offset = PS.Offset;
                                GWriteQueue[Slot].Type   = (PS.Type == PST_Name)
                                                            ? PWT_Name : PWT_Str;
                                appStrncpyANSI(GWriteQueue[Slot].StrValue,
                                               SEditBuf, SNAP_STR_LEN);
                                GWriteCount = Slot + 1;
                            }
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_Object:
                        ImGui::TextUnformatted(PS.StrValue);
                        break;
                    case PST_Texture:
                    {
                        // Name label
                        ImGui::TextUnformatted(PS.StrValue);
                        if (PS.TexPtr && !PS.TexPtr->IsPendingKill())
                        {
                            IDirect3DTexture9* D3DTex = OLImGui_GetD3DTexture((UTexture2D*)PS.TexPtr);
                            if (D3DTex)
                            {
                                // Preview: square thumbnail fitting column width, max 128px.
                                float AvailW = ImGui::GetContentRegionAvail().x;
                                float ThumbW = AvailW < 128.f ? AvailW : 128.f;
                                ImGui::Image((ImTextureID)D3DTex, ImVec2(ThumbW, ThumbW));
                            }
                        }
                        break;
                    }
                    case PST_Vector:
                    {
                        float V[3] = { PS.Value.Vec[0], PS.Value.Vec[1], PS.Value.Vec[2] };
                        if (ImGui::DragFloat3("##v", V, 1.0f))
                        {
                            int Slot = GWriteCount;
                            if (Slot < WRITE_QUEUE_SIZE)
                            {
                                GWriteQueue[Slot].Actor          = Obj;
                                GWriteQueue[Slot].Offset         = PS.Offset;
                                GWriteQueue[Slot].Type           = PWT_Vector;
                                GWriteQueue[Slot].NewValue.Vec[0]= V[0];
                                GWriteQueue[Slot].NewValue.Vec[1]= V[1];
                                GWriteQueue[Slot].NewValue.Vec[2]= V[2];
                                GWriteCount = Slot + 1;
                            }
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_Rotator:
                    {
                        float V[3] = {
                            PS.Value.Rot[0] * (360.f / 65536.f),
                            PS.Value.Rot[1] * (360.f / 65536.f),
                            PS.Value.Rot[2] * (360.f / 65536.f)
                        };
                        if (ImGui::DragFloat3("##v", V, 0.5f, 0.f, 0.f, "%.1f\xc2\xb0"))
                        {
                            int Slot = GWriteCount;
                            if (Slot < WRITE_QUEUE_SIZE)
                            {
                                GWriteQueue[Slot].Actor          = Obj;
                                GWriteQueue[Slot].Offset         = PS.Offset;
                                GWriteQueue[Slot].Type           = PWT_Rotator;
                                GWriteQueue[Slot].NewValue.Rot[0]= (INT)(V[0] * (65536.f / 360.f));
                                GWriteQueue[Slot].NewValue.Rot[1]= (INT)(V[1] * (65536.f / 360.f));
                                GWriteQueue[Slot].NewValue.Rot[2]= (INT)(V[2] * (65536.f / 360.f));
                                GWriteCount = Slot + 1;
                            }
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_Color:
                    {
                        float C[4] = { PS.Value.Col[0], PS.Value.Col[1], PS.Value.Col[2], PS.Value.Col[3] };
                        if (ImGui::ColorEdit4("##v", C, ImGuiColorEditFlags_AlphaBar))
                        {
                            int Slot = GWriteCount;
                            if (Slot < WRITE_QUEUE_SIZE)
                            {
                                GWriteQueue[Slot].Actor          = Obj;
                                GWriteQueue[Slot].Offset         = PS.Offset;
                                GWriteQueue[Slot].Type           = PWT_Color;
                                GWriteQueue[Slot].NewValue.Col[0] = C[0];
                                GWriteQueue[Slot].NewValue.Col[1] = C[1];
                                GWriteQueue[Slot].NewValue.Col[2] = C[2];
                                GWriteQueue[Slot].NewValue.Col[3] = C[3];
                                GWriteCount = Slot + 1;
                            }
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    case PST_LinearColor:
                    {
                        float C[4] = { PS.Value.Col[0], PS.Value.Col[1], PS.Value.Col[2], PS.Value.Col[3] };
                        if (ImGui::ColorEdit4("##v", C, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Float))
                        {
                            int Slot = GWriteCount;
                            if (Slot < WRITE_QUEUE_SIZE)
                            {
                                GWriteQueue[Slot].Actor          = Obj;
                                GWriteQueue[Slot].Offset         = PS.Offset;
                                GWriteQueue[Slot].Type           = PWT_LinearColor;
                                GWriteQueue[Slot].NewValue.Col[0] = C[0];
                                GWriteQueue[Slot].NewValue.Col[1] = C[1];
                                GWriteQueue[Slot].NewValue.Col[2] = C[2];
                                GWriteQueue[Slot].NewValue.Col[3] = C[3];
                                GWriteCount = Slot + 1;
                            }
                            GDetailRequest = Obj;
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::PopID();
    }

    ImGui::EndChild();
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------

void OLImGui_TabScene()
{
    // --- Property inspector screen ---
    if (GSelectedObj)
    {
        if (GSelectedObj->IsPendingKill() || !GSelectedObj->IsValid())
            GSelectedObj = NULL;
        else
        {
            DrawPropertyInspector(GSelectedObj);
            return;
        }
    }

    // --- Object list screen ---
    const int TotalObjs = GSceneObjCount; // published each tick by game thread

    bool FilterChanged = OL_SEARCH("##filter", "scene.search.actors", GSceneFilter, sizeof(GSceneFilter));

    const bool bFiltered = (GSceneFilter[0] != '\0');

    // Rebuild only when filter is active and text changed.
    if (bFiltered && (FilterChanged || appStrcmpANSI(GSceneFilter, GLastFilter) != 0))
        RebuildFilter();
    // Clear index list when filter is cleared so it doesn't linger.
    if (!bFiltered && GLastFilter[0] != '\0')
    {
        GFilteredIdx.Reset();
        GLastFilter[0] = '\0';
    }

    const int DisplayCount = bFiltered ? GFilteredIdx.Num() : TotalObjs;
    ImGui::Text(OLLocale_T("scene.objects_count"), DisplayCount, TotalObjs);
    ImGui::Separator();

    ImGui::BeginChild("##actors", ImVec2(0, 0), false);

    ImGuiListClipper Clipper;
    Clipper.Begin(DisplayCount);
    while (Clipper.Step())
    {
        for (int ci = Clipper.DisplayStart; ci < Clipper.DisplayEnd; ++ci)
        {
            const int ObjIdx = bFiltered ? GFilteredIdx(ci) : ci;
            if (ObjIdx >= TotalObjs) continue;
            UObject* Obj = UObject::GetIndexedObject(ObjIdx);
            if (!Obj || Obj->IsPendingKill() || !Obj->IsValid()) continue;

            // Build label: "Level:ObjName" or "Level:ObjName [MeshName]"
            char Buf[SNAP_NAME_LEN * 2 + 8];
            {
                char LvlBuf[SNAP_NAME_LEN] = "";
                if (Obj->GetOuter())
                    TCHARToCharBuf_Scene(LvlBuf, SNAP_NAME_LEN, *Obj->GetOuter()->GetFName().ToString());
                char ObjBuf[SNAP_NAME_LEN];
                TCHARToCharBuf_Scene(ObjBuf, SNAP_NAME_LEN, *Obj->GetFName().ToString());
                if (LvlBuf[0])
                    _snprintf(Buf, sizeof(Buf), "%s:%s", LvlBuf, ObjBuf);
                else
                    appStrncpyANSI(Buf, ObjBuf, sizeof(Buf));
            }

            char AssetBuf[SNAP_NAME_LEN] = "";
            if (USkeletalMeshComponent* SKC = Cast<USkeletalMeshComponent>(Obj))
            {
                if (SKC->SkeletalMesh)
                    TCHARToCharBuf_Scene(AssetBuf, SNAP_NAME_LEN, *SKC->SkeletalMesh->GetFName().ToString());
            }
            else if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Obj))
            {
                if (SMC->StaticMesh)
                    TCHARToCharBuf_Scene(AssetBuf, SNAP_NAME_LEN, *SMC->StaticMesh->GetFName().ToString());
            }

            if (AssetBuf[0])
            {
                // Append " [MeshName]" in place.
                int i = 0;
                while (Buf[i]) ++i;
                Buf[i++] = ' '; Buf[i++] = '[';
                int j = 0;
                while (i < (int)sizeof(Buf)-2 && AssetBuf[j]) Buf[i++] = AssetBuf[j++];
                Buf[i++] = ']'; Buf[i] = '\0';
            }

            ImGui::PushID(Obj);
            if (ImGui::Selectable(Buf, false))
            {
                GSelectedObj   = Obj;
                GDetailRequest = Obj;
                GPropFilter[0] = '\0'; // reset property filter for new selection
            }

            // Hover: show live world-capture preview for Actors and world components.
            if (ImGui::IsItemHovered())
            {
                AActor*              HovActor = Cast<AActor>(Obj);
                UStaticMeshComponent* HovSMC  = HovActor ? NULL : Cast<UStaticMeshComponent>(Obj);
                // Skip components with no mesh or zero bounds (not placed in world).
                if (HovSMC && (!HovSMC->StaticMesh ||
                    (HovSMC->Bounds.Origin.X == 0.f &&
                     HovSMC->Bounds.Origin.Y == 0.f &&
                     HovSMC->Bounds.Origin.Z == 0.f)))
                    HovSMC = NULL;

                FVector HovLoc(0,0,0);
                bool    bShowCapture = false;

                if (HovActor)
                {
                    HovLoc = HovActor->Location;
                    WCapEnqueue(HovActor);
                    bShowCapture = true;
                }
                else if (HovSMC)
                {
                    HovLoc = HovSMC->Bounds.Origin;
                    WCapEnqueuePos(HovLoc, HovSMC->Bounds.SphereRadius);
                    bShowCapture = true;
                }

                if (bShowCapture)
                {
                    ImGui::BeginTooltip();
                    // Header: class name + object name
                    char ClsBuf[SNAP_NAME_LEN];
                    TCHARToCharBuf_Scene(ClsBuf, SNAP_NAME_LEN,
                        *Obj->GetClass()->GetName());
                    ImGui::TextDisabled("%s", ClsBuf);
                    ImGui::TextUnformatted(Buf); // already built above

                    // Location
                    ImGui::Text("Loc: %.0f  %.0f  %.0f",
                        HovLoc.X, HovLoc.Y, HovLoc.Z);

                    // RT preview (available after first capture tick)
                    IDirect3DTexture9* WCapTex = OLWorldCapture_GetTexture();
                    if (WCapTex)
                    {
                        // Switch to opaque blend (ONE, ZERO) before drawing the RT,
                        // then restore standard alpha blend after.
                        // This ensures RT alpha channel (which the scene doesn't write)
                        // doesn't make the image transparent.
                        ImDrawList* DL = ImGui::GetWindowDrawList();
                        DL->AddCallback(OLWorldCapture_SetOpaqueBlend, NULL);
                        ImGui::Image((ImTextureID)WCapTex,
                            ImVec2(OL_WORLD_CAPTURE_SIZE, OL_WORLD_CAPTURE_SIZE));
                        DL->AddCallback(OLWorldCapture_RestoreBlend, NULL);
                    }
                    else
                    {
                        ImGui::Dummy(ImVec2(OL_WORLD_CAPTURE_SIZE, 20.f));
                        ImGui::SameLine(8.f);
                        ImGui::TextDisabled("Capturing...");
                    }
                    ImGui::EndTooltip();
                }
            }

            ImGui::PopID();
        }
    }
    Clipper.End();
    ImGui::EndChild();
}

#undef ENQUEUE_WRITE
