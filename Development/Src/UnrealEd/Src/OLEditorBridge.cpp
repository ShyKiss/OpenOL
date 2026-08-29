/*=============================================================================
    OLEditorBridge.cpp — bridge between OLEditorServer and UnrealEd internals.
=============================================================================*/
#include "UnrealEd.h"
#include "OLEditorProto.h"

// Shared buffer: OLEditorBridge fills it, OLEditorServer reads and sends UDP
#define OL_SCENE_BUF_SIZE (sizeof(OLProto_Header) + OL_PROTO_MAX_ACTORS * sizeof(OLProto_Actor))
static BYTE          g_scene_buf[OL_SCENE_BUF_SIZE];
static int           g_scene_len  = 0;
static unsigned int  g_frame_id   = 0;
static CRITICAL_SECTION g_scene_cs;
static bool          g_cs_init    = false;

static void EnsureCS()
{
    if (!g_cs_init) { InitializeCriticalSection(&g_scene_cs); g_cs_init = true; }
}

// Called by OLEditorServer to read the latest scene packet
int OLEditor_GetScenePacket(void* out_buf, int max_len)
{
    EnsureCS();
    EnterCriticalSection(&g_scene_cs);
    int len = (g_scene_len <= max_len) ? g_scene_len : 0;
    if (len > 0) appMemcpy(out_buf, g_scene_buf, len);
    LeaveCriticalSection(&g_scene_cs);
    return len;
}

static void CopyStr(char* dst, int dstlen, const TCHAR* src)
{
    if (!src) { dst[0] = '\0'; return; }
    int i = 0;
    for (; i < dstlen - 1 && src[i]; i++)
        dst[i] = (char)src[i];
    dst[i] = '\0';
}

// Called from OLEditorServer when editor clicks viewport.
// NormX, NormY are 0..1 relative to our display viewport (matching backbuffer resolution).
void OLEditor_ClickAt(FLOAT NormX, FLOAT NormY)
{
    if (!GUnrealEd) return;
    for (INT i = 0; i < GUnrealEd->ViewportClients.Num(); i++)
    {
        FEditorLevelViewportClient* VC = GUnrealEd->ViewportClients(i);
        if (!VC->IsPerspective() || !VC->Viewport) continue;

        INT VW = (INT)VC->Viewport->GetSizeX();
        INT VH = (INT)VC->Viewport->GetSizeY();
        if (VW <= 0 || VH <= 0) continue;

        INT HitX = (INT)(NormX * VW);
        INT HitY = (INT)(NormY * VH);
        HitX = Clamp(HitX, 0, VW-1);
        HitY = Clamp(HitY, 0, VH-1);

        // Force a redraw so hit proxy buffer is up to date
        VC->Viewport->Draw();

        HHitProxy* HitProxy = VC->Viewport->GetHitProxy(HitX, HitY);

        // Always select (never toggle/deselect) the hit actor.
        // ProcessClick() has toggle semantics — bypass it and call SelectActor directly.
        if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
        {
            HActor* HA = (HActor*)HitProxy;
            if (HA->Actor)
            {
                GEditor->SelectNone(FALSE, TRUE);
                GEditor->SelectActor(HA->Actor, TRUE, NULL, TRUE);
                GEditor->RedrawLevelEditingViewports();
            }
        }
        else if (!HitProxy)
        {
            // Clicked empty space — deselect all
            GEditor->SelectNone(TRUE, TRUE);
            GEditor->RedrawLevelEditingViewports();
        }
        break;
    }
}

static AActor* FindActorByName(const char* name)
{
    if (!GWorld) return NULL;
    for (FActorIterator It; It; ++It)
    {
        AActor* A = *It;
        if (A && !A->bDeleteMe)
        {
            FString ActorName = A->GetName();
            if (appStrcmpANSI(TCHAR_TO_ANSI(*ActorName), name) == 0)
                return A;
        }
    }
    return NULL;
}

void OLEditor_MoveActor(const char* name, FLOAT x, FLOAT y, FLOAT z)
{
    AActor* A = FindActorByName(name);
    if (!A) return;
    GEditor->BeginTransaction(TEXT("Move Actor"));
    A->Modify();
    A->SetLocation(FVector(x, y, z));
    A->PostEditMove(TRUE);
    GEditor->EndTransaction();
    GEditor->RedrawLevelEditingViewports();
}

void OLEditor_RotateActor(const char* name, INT pitch, INT yaw, INT roll)
{
    AActor* A = FindActorByName(name);
    if (!A) return;
    GEditor->BeginTransaction(TEXT("Rotate Actor"));
    A->Modify();
    A->Rotation.Pitch = pitch;
    A->Rotation.Yaw   = yaw;
    A->Rotation.Roll  = roll;
    A->PostEditMove(TRUE);
    GEditor->EndTransaction();
    GEditor->RedrawLevelEditingViewports();
}

// Rotate actor by angle (radians) around an arbitrary world-space axis.
// Mirrors EditorApplyRotation: ResultQ = DeltaQ * ActorQ, then convert back to FRotator.
void OLEditor_RotateActorAxis(const char* name, FLOAT ax, FLOAT ay, FLOAT az, FLOAT angle_rad)
{
    AActor* A = FindActorByName(name);
    if (!A) return;

    FVector Axis(ax, ay, az);
    Axis = Axis.SafeNormal();

    const FQuat DeltaQ(Axis, angle_rad);

    FRotator ActorRotWind, ActorRotRem;
    A->Rotation.GetWindingAndRemainder(ActorRotWind, ActorRotRem);
    const FQuat ActorQ = ActorRotRem.Quaternion();
    const FQuat ResultQ = DeltaQ * ActorQ;
    const FRotator NewActorRotRem = FRotator(ResultQ);
    FRotator DeltaRot = NewActorRotRem - ActorRotRem;
    DeltaRot.MakeShortestRoute();

    GEditor->BeginTransaction(TEXT("Rotate Actor"));
    A->Modify();
    A->Rotation += DeltaRot;
    A->PostEditMove(TRUE);
    GEditor->EndTransaction();
    GEditor->RedrawLevelEditingViewports();
}

void OLEditor_ScaleActor(const char* name, FLOAT sx, FLOAT sy, FLOAT sz)
{
    AActor* A = FindActorByName(name);
    if (!A) return;
    GEditor->BeginTransaction(TEXT("Scale Actor"));
    A->Modify();
    A->DrawScale3D.X = sx;
    A->DrawScale3D.Y = sy;
    A->DrawScale3D.Z = sz;
    A->PostEditMove(TRUE);
    GEditor->EndTransaction();
    GEditor->RedrawLevelEditingViewports();
}

void OLEditor_SetView(FLOAT X, FLOAT Y, FLOAT Z, INT Pitch, INT Yaw, INT Roll)
{
    if (!GUnrealEd) return;
    FVector  Loc(X, Y, Z);
    FRotator Rot(Pitch, Yaw, Roll);
    for (INT i = 0; i < GUnrealEd->ViewportClients.Num(); i++)
    {
        GUnrealEd->ViewportClients(i)->ViewLocation = Loc;
        GUnrealEd->ViewportClients(i)->ViewRotation = Rot;
    }
}

void OLEditor_BroadcastScene()
{
    if (!GUnrealEd || !GWorld) return;
    EnsureCS();

    static OLProto_Actor actor_buf[OL_PROTO_MAX_ACTORS];
    unsigned int num = 0;

    for (FActorIterator It; It && num < OL_PROTO_MAX_ACTORS; ++It)
    {
        AActor* A = *It;
        if (!A || A->bDeleteMe || A->IsA(AWorldInfo::StaticClass())) continue;

        OLProto_Actor& out = actor_buf[num++];
        appMemzero(&out, sizeof(out));

        CopyStr(out.name, sizeof(out.name), *A->GetName());
        CopyStr(out.cls,  sizeof(out.cls),  *A->GetClass()->GetName());
        CopyStr(out.tag,  sizeof(out.tag),  *A->Tag.ToString());

        AStaticMeshActor* SMA = Cast<AStaticMeshActor>(A);
        if (SMA && SMA->StaticMeshComponent && SMA->StaticMeshComponent->StaticMesh)
            CopyStr(out.mesh, sizeof(out.mesh), *SMA->StaticMeshComponent->StaticMesh->GetName());

        out.location.x = A->Location.X;    out.location.y = A->Location.Y;    out.location.z = A->Location.Z;
        out.rotation.pitch = A->Rotation.Pitch; out.rotation.yaw = A->Rotation.Yaw; out.rotation.roll = A->Rotation.Roll;
        out.scale3d.x  = A->DrawScale3D.X; out.scale3d.y  = A->DrawScale3D.Y; out.scale3d.z  = A->DrawScale3D.Z;
        out.draw_scale = A->DrawScale;

        FBox box = A->GetComponentsBoundingBox(TRUE);
        out.bounds.min.x = box.Min.X; out.bounds.min.y = box.Min.Y; out.bounds.min.z = box.Min.Z;
        out.bounds.max.x = box.Max.X; out.bounds.max.y = box.Max.Y; out.bounds.max.z = box.Max.Z;

        if (A->IsSelected())    out.flags |= OL_ACTOR_SELECTED;
        if (A->bHidden)         out.flags |= OL_ACTOR_HIDDEN;
        if (A->bHiddenEdLayer)  out.flags |= OL_ACTOR_HIDDEN_ED;
    }

    OLProto_Header hdr;
    appMemzero(&hdr, sizeof(hdr));
    hdr.magic      = OL_PROTO_MAGIC;
    hdr.version    = OL_PROTO_VERSION;
    hdr.frame_id   = ++g_frame_id;
    hdr.num_actors = num;

    for (INT i = 0; i < GUnrealEd->ViewportClients.Num(); i++)
    {
        FEditorLevelViewportClient* VC = GUnrealEd->ViewportClients(i);
        if (VC->IsPerspective())
        {
            hdr.camera.location.x = VC->ViewLocation.X; hdr.camera.location.y = VC->ViewLocation.Y; hdr.camera.location.z = VC->ViewLocation.Z;
            hdr.camera.rotation.pitch = VC->ViewRotation.Pitch; hdr.camera.rotation.yaw = VC->ViewRotation.Yaw; hdr.camera.rotation.roll = VC->ViewRotation.Roll;
            hdr.camera.fov      = VC->ViewFOV;
            hdr.camera.aspect   = (VC->Viewport && VC->Viewport->GetSizeY() > 0)
                ? (FLOAT)VC->Viewport->GetSizeX() / VC->Viewport->GetSizeY() : 1.777f;
            break;
        }
    }

    int total = (int)(sizeof(OLProto_Header) + num * sizeof(OLProto_Actor));

    EnterCriticalSection(&g_scene_cs);
    if (total <= (int)sizeof(g_scene_buf))
    {
        appMemcpy(g_scene_buf, &hdr, sizeof(hdr));
        appMemcpy(g_scene_buf + sizeof(hdr), actor_buf, num * sizeof(OLProto_Actor));
        g_scene_len = total;
    }
    LeaveCriticalSection(&g_scene_cs);
}
