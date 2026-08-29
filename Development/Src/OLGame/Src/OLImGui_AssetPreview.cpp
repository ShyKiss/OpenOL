/*=============================================================================
    OLImGui_AssetPreview.cpp — Self-contained 3D asset preview instances.

    Each FOLPreviewInstance owns its own FPreviewScene + dual render targets +
    D3D9 output texture. Create as many as needed; call Tick() each game tick.
=============================================================================*/

#include "imgui_compat_msvc2012.h"
#include "imgui.h"

#include "OLGame.h"
#include "PreviewScene.h"
#include "OLImGui_AssetPreview.h"

// Forward-declared here to avoid circular include; defined in OLImGui_Spawns.cpp.
void OLSpawns_ShutdownPreviews();

#include "..\..\D3D9Drv\Inc\D3D9Resources.h"
#include "..\..\D3D9Drv\Inc\D3D9RenderTarget.h"
#include "..\..\..\Engine\Src\ScenePrivate.h"

#include <d3d9.h>
extern IDirect3DDevice9* GLegacyDirect3DDevice9;

// ---------------------------------------------------------------------------
// Global state (shared across all instances)
// ---------------------------------------------------------------------------

float GOLPreviewYaw = 0.f;
float GOLPreviewFOV = 75.f; // matches UE3 editor thumbnail FOV

void* volatile GOLPreviewPendingSkeletal     = NULL;
void* volatile GOLPreviewPendingStatic       = NULL;
void* volatile GOLPreviewPendingAttachMesh   = NULL;
int   volatile GOLPreviewPendingAttachSlot   = 0;
char           GOLPreviewPendingAttachSocket[256] = "";

void OLPreview_ApplySkeletal() { GPreviewMain.SetSkeletalMesh((USkeletalMesh*)GOLPreviewPendingSkeletal); }
void OLPreview_ApplyStatic()   { GPreviewMain.SetStaticMesh((UStaticMesh*)GOLPreviewPendingStatic); }

void OLPreview_ApplyAttach()
{
    FName SocketName = GOLPreviewPendingAttachSocket[0]
        ? FName(ANSI_TO_TCHAR(GOLPreviewPendingAttachSocket))
        : NAME_None;
    OLPreview_AttachToSocket(
        GOLPreviewPendingAttachSlot,
        (USkeletalMesh*)GOLPreviewPendingAttachMesh,
        SocketName);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Map change listener — shuts down all instances before level loads.
// ---------------------------------------------------------------------------

class FOLPreviewMapChangeListener : public FCallbackEventDevice
{
public:
    virtual void Send(ECallbackEventType InType)
    {
        if (InType == CALLBACK_PreLoadMap)
            OLPreview_ShutdownSystem();
    }
};
static FOLPreviewMapChangeListener* GPreviewMapListener = NULL;

void OLPreview_InitSystem()
{
    if (!GPreviewMapListener)
    {
        GPreviewMapListener = new FOLPreviewMapChangeListener();
        GCallbackEvent->Register(CALLBACK_PreLoadMap, GPreviewMapListener);
    }
}

void OLPreview_ShutdownSystem()
{
    // Shut down ALL preview instances and clear all mesh caches on any map change.
    // OLSpawns_ShutdownPreviews() handles GPreviewMain + GPreviewHover + GPreviewWeapon
    // in one place. Instances are lazy-Init()'d again on the next tick.
    OLSpawns_ShutdownPreviews();
}

// ---------------------------------------------------------------------------
// Shared D3D9 helpers
// ---------------------------------------------------------------------------

// Get the underlying IDirect3DTexture9 from a UTexture2D.
// Used by the Scene tab property inspector to display texture previews.
IDirect3DTexture9* OLImGui_GetD3DTexture(UTexture2D* Tex)
{
    if (!Tex || !Tex->Resource)
        return NULL;
    FTexture2DResource* Res = static_cast<FTexture2DResource*>(Tex->Resource);
    FTexture2DRHIRef RHI = Res->GetTexture2DRHI();
    if (!IsValidRef(RHI))
        return NULL;
    FD3D9Texture2D* D3DTex = static_cast<FD3D9Texture2D*>((TDynamicRHIResource<RRT_Texture2D>*)RHI);
    if (!D3DTex) return NULL;
    TRefCountPtr<IDirect3DTexture9>* Ptr = D3DTex;
    return Ptr->GetReference();
}

static IDirect3DTexture9* UnwrapRT(FTextureRenderTargetResource* Res)
{
    FTextureRenderTarget2DResource* RT2D = Res->GetTextureRenderTarget2DResource();
    if (!RT2D) return NULL;
    FTexture2DRHIRef RHI = RT2D->GetTextureRHI();
    FD3D9Texture2D* D3DTex = static_cast<FD3D9Texture2D*>((TDynamicRHIResource<RRT_Texture2D>*)RHI);
    if (!D3DTex) return NULL;
    TRefCountPtr<IDirect3DTexture9>* Ptr = D3DTex;
    return Ptr ? Ptr->GetReference() : NULL;
}

static bool ReadbackTexture(IDirect3DTexture9* Tex, UINT Size,
    IDirect3DSurface9*& StagingSurf, IDirect3DSurface9*& OffscreenSurf)
{
    if (!StagingSurf)
    {
        HRESULT hr = GLegacyDirect3DDevice9->CreateRenderTarget(
            Size, Size, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &StagingSurf, NULL);
        if (FAILED(hr)) return false;
    }
    if (!OffscreenSurf)
    {
        HRESULT hr = GLegacyDirect3DDevice9->CreateOffscreenPlainSurface(
            Size, Size, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &OffscreenSurf, NULL);
        if (FAILED(hr)) return false;
    }

    IDirect3DSurface9* TexSurf = NULL;
    if (FAILED(Tex->GetSurfaceLevel(0, &TexSurf))) return false;

    HRESULT hr = GLegacyDirect3DDevice9->StretchRect(TexSurf, NULL, StagingSurf, NULL, D3DTEXF_NONE);
    TexSurf->Release();
    if (FAILED(hr)) return false;

    hr = GLegacyDirect3DDevice9->GetRenderTargetData(StagingSurf, OffscreenSurf);
    return SUCCEEDED(hr);
}

// ---------------------------------------------------------------------------
// FImpl — per-instance data
// ---------------------------------------------------------------------------

enum EOLPreviewType { PT_None, PT_Skeletal, PT_Static };

struct FOLPreviewInstance::FImpl
{
    FPreviewScene*           Scene;
    USkeletalMeshComponent*  SkeletalComp;
    UStaticMeshComponent*    StaticComp;
    EOLPreviewType           PreviewType;

    // Dual RTs for alpha extraction (black bg + white bg).
    UTextureRenderTarget2D*  RTBlack;
    UTextureRenderTarget2D*  RTWhite;

    // D3D9 raw pointers (unwrapped from RHI).
    IDirect3DTexture9*       D3DBlack;
    IDirect3DTexture9*       D3DWhite;

    // Cached readback surfaces (allocated once).
    IDirect3DSurface9*       StagingB;
    IDirect3DSurface9*       StagingW;
    IDirect3DSurface9*       OffscreenB;
    IDirect3DSurface9*       OffscreenW;

    // Final alpha-extracted texture (written game thread, read render thread).
    IDirect3DTexture9* volatile ChromaTexture;

    // Attachment slots (only used on the main instance).
    struct FAttachment { USkeletalMeshComponent* Comp; bool bAttached; };
    FAttachment Attachments[OL_PREVIEW_MAX_ATTACHMENTS];

    // Weapon static mesh attached to a bone on SkeletalComp.
    UStaticMeshComponent*    WeaponComp;
    bool                     bWeaponAttached;

    // Animation playback node — recreated each time a mesh+animset is assigned.
    // Must be owned by SkeletalComp so InitAnimTree binds SkelComponent correctly.
    UAnimNodeSequence*       AnimNode;

    FImpl()
        : Scene(NULL), SkeletalComp(NULL), StaticComp(NULL)
        , PreviewType(PT_None)
        , RTBlack(NULL), RTWhite(NULL)
        , D3DBlack(NULL), D3DWhite(NULL)
        , StagingB(NULL), StagingW(NULL)
        , OffscreenB(NULL), OffscreenW(NULL)
        , ChromaTexture(NULL)
        , WeaponComp(NULL), bWeaponAttached(false)
        , AnimNode(NULL)
    {
        for (int i = 0; i < OL_PREVIEW_MAX_ATTACHMENTS; ++i)
        {
            Attachments[i].Comp      = NULL;
            Attachments[i].bAttached = false;
        }
    }

    void DetachAllSlots()
    {
        for (int i = 0; i < OL_PREVIEW_MAX_ATTACHMENTS; ++i)
        {
            FAttachment& A = Attachments[i];
            if (A.Comp && A.bAttached)
            {
                SkeletalComp->DetachComponent(A.Comp);
                A.bAttached = false;
            }
        }
    }

    void DetachWeapon()
    {
        if (WeaponComp && bWeaponAttached && SkeletalComp)
        {
            SkeletalComp->DetachComponent(WeaponComp);
            bWeaponAttached = false;
        }
    }

    void RemoveAll()
    {
        if (!Scene) return;
        DetachAllSlots();
        DetachWeapon();
        if (SkeletalComp && SkeletalComp->IsAttached()) Scene->RemoveComponent(SkeletalComp);
        if (StaticComp   && StaticComp->IsAttached())   Scene->RemoveComponent(StaticComp);
        PreviewType = PT_None;
    }

    FBoxSphereBounds GetBounds() const
    {
        if (PreviewType == PT_Skeletal && SkeletalComp)
            return SkeletalComp->Bounds; // already centered via FTranslationMatrix(-Mesh->Bounds.Origin)
        if (PreviewType == PT_Static && StaticComp && StaticComp->StaticMesh)
            return StaticComp->Bounds; // component bounds already reflect centering transform
        return FBoxSphereBounds(FVector(0,0,0), FVector(50,50,50), 50.f);
    }

    static FMatrix MakeViewMatrix(const FBoxSphereBounds& Bounds, float YawDeg, float FOVDeg, float DistScale)
    {
        const FLOAT TanHalfFOV = appTan(FOVDeg * (FLOAT)PI / 360.f);
        INT         ExtraYaw   = (INT)(YawDeg * (65536.f / 360.f) + 0.5f);

        FMatrix ViewMatrix;
        if (Bounds.BoxExtent.X > Bounds.BoxExtent.Y)
        {
            // Camera looks along +Y: horizontal extent = X, vertical = Z.
            const FLOAT DistH = Bounds.BoxExtent.X / TanHalfFOV;
            const FLOAT DistV = Bounds.BoxExtent.Z / TanHalfFOV;
            const FLOAT Dist  = Max(DistH, DistV) * DistScale;
            ViewMatrix = FTranslationMatrix(-(Bounds.Origin - FVector(0, Dist, 0)));
            ViewMatrix = ViewMatrix * FInverseRotationMatrix(FRotator(0, 16384 + ExtraYaw, 0));
        }
        else
        {
            // Camera looks along -X: horizontal extent = Y, vertical = Z.
            const FLOAT DistH = Bounds.BoxExtent.Y / TanHalfFOV;
            const FLOAT DistV = Bounds.BoxExtent.Z / TanHalfFOV;
            const FLOAT Dist  = Max(DistH, DistV) * DistScale;
            ViewMatrix = FTranslationMatrix(-(Bounds.Origin - FVector(-Dist, 0, 0)));
            ViewMatrix = ViewMatrix * FInverseRotationMatrix(FRotator(0, 32768 + ExtraYaw, 0));
        }
        ViewMatrix = ViewMatrix * FMatrix(
            FPlane(0, 0, 1, 0),
            FPlane(1, 0, 0, 0),
            FPlane(0, 1, 0, 0),
            FPlane(0, 0, 0, 1));
        return ViewMatrix;
    }
};

// ---------------------------------------------------------------------------
// FOLPreviewInstance
// ---------------------------------------------------------------------------

FOLPreviewInstance::FOLPreviewInstance() : Impl(NULL) {}

void FOLPreviewInstance::Init()
{
    if (Impl && Impl->Scene)
        return; // already inited

    OLPreview_InitSystem();

    if (!Impl) Impl = new FImpl();
    FImpl& I = *Impl;

    I.Scene = new FPreviewScene(FRotator(-8192, 8192, 0), 0.4f, 1.2f, FALSE, TRUE);
    if (I.Scene->DirectionalLight)      I.Scene->DirectionalLight->AddToRoot();
    if (I.Scene->SkyLight)             I.Scene->SkyLight->AddToRoot();
    if (I.Scene->GetLineBatcher())     I.Scene->GetLineBatcher()->AddToRoot();

    I.SkeletalComp = ConstructObject<USkeletalMeshComponent>(
        USkeletalMeshComponent::StaticClass(), UObject::GetTransientPackage());
    I.StaticComp = ConstructObject<UStaticMeshComponent>(
        UStaticMeshComponent::StaticClass(), UObject::GetTransientPackage());
    I.SkeletalComp->AddToRoot();
    I.StaticComp->AddToRoot();

    I.AnimNode = NULL;

    I.WeaponComp = ConstructObject<UStaticMeshComponent>(
        UStaticMeshComponent::StaticClass(), UObject::GetTransientPackage());
    I.WeaponComp->AddToRoot();
    I.bWeaponAttached = false;

    for (int i = 0; i < OL_PREVIEW_MAX_ATTACHMENTS; ++i)
    {
        I.Attachments[i].Comp = ConstructObject<USkeletalMeshComponent>(
            USkeletalMeshComponent::StaticClass(), UObject::GetTransientPackage());
        I.Attachments[i].Comp->AddToRoot();
        I.Attachments[i].bAttached = false;
    }

    I.RTBlack = ConstructObject<UTextureRenderTarget2D>(
        UTextureRenderTarget2D::StaticClass(), UObject::GetTransientPackage());
    I.RTBlack->AddToRoot();
    I.RTBlack->ClearColor = FLinearColor(0.f, 0.f, 0.f, 1.f);
    I.RTBlack->Init(OL_PREVIEW_SIZE, OL_PREVIEW_SIZE, PF_A8R8G8B8, FALSE);

    I.RTWhite = ConstructObject<UTextureRenderTarget2D>(
        UTextureRenderTarget2D::StaticClass(), UObject::GetTransientPackage());
    I.RTWhite->AddToRoot();
    I.RTWhite->ClearColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
    I.RTWhite->Init(OL_PREVIEW_SIZE, OL_PREVIEW_SIZE, PF_A8R8G8B8, FALSE);
}

void FOLPreviewInstance::ClearMesh()
{
    // Drop skeletal mesh + AnimSet references so GC-freed pointers don't
    // cause crashes on the next Tick(). We must call RemoveAll() first to
    // detach the component from the preview scene before calling SetSkeletalMesh(NULL)
    // — otherwise the subsequent SetSkeletalMeshWithAnim → SetSkeletalMesh(validMesh)
    // will call RemoveAll() again and double-remove the component, corrupting
    // the scene component list (TArray Count becomes -1).
    if (!Impl || !Impl->SkeletalComp) return;
    FImpl& I = *Impl;
    // 1. Drop anim refs before anything so SetSkeletalMesh(NULL) doesn't touch freed AnimNode.
    I.SkeletalComp->Animations = NULL;
    I.SkeletalComp->AnimSets.Empty();
    if (I.AnimNode) { I.AnimNode->RemoveFromRoot(); I.AnimNode = NULL; }
    // 2. Detach component from scene (sets PreviewType=PT_None internally).
    I.RemoveAll();
    // 3. Now it is safe to call UE3 SetSkeletalMesh(NULL) — component is detached.
    I.SkeletalComp->SetSkeletalMesh(NULL);
}

void FOLPreviewInstance::Shutdown()
{
    if (!Impl) return;
    FImpl& I = *Impl;

    I.RemoveAll();

    if (I.AnimNode)     { I.AnimNode->RemoveFromRoot();     I.AnimNode     = NULL; }
    if (I.WeaponComp)   { I.WeaponComp->RemoveFromRoot();   I.WeaponComp   = NULL; }
    if (I.SkeletalComp) { I.SkeletalComp->RemoveFromRoot(); I.SkeletalComp = NULL; }
    if (I.StaticComp)   { I.StaticComp->RemoveFromRoot();   I.StaticComp   = NULL; }
    for (int i = 0; i < OL_PREVIEW_MAX_ATTACHMENTS; ++i)
    {
        if (I.Attachments[i].Comp)
        {
            I.Attachments[i].Comp->RemoveFromRoot();
            I.Attachments[i].Comp = NULL;
        }
    }
    if (I.RTBlack) { I.RTBlack->RemoveFromRoot(); I.RTBlack = NULL; }
    if (I.RTWhite) { I.RTWhite->RemoveFromRoot(); I.RTWhite = NULL; }
    if (I.Scene)
    {
        if (I.Scene->GetLineBatcher())     I.Scene->GetLineBatcher()->RemoveFromRoot();
        if (I.Scene->DirectionalLight)     I.Scene->DirectionalLight->RemoveFromRoot();
        if (I.Scene->SkyLight)             I.Scene->SkyLight->RemoveFromRoot();
        I.Scene->RemoveAllComponents();
        delete I.Scene;
        I.Scene = NULL;
    }
    I.D3DBlack = NULL;
    I.D3DWhite = NULL;
    if (I.ChromaTexture) { I.ChromaTexture->Release(); I.ChromaTexture = NULL; }
    if (I.StagingB)   { I.StagingB->Release();   I.StagingB   = NULL; }
    if (I.StagingW)   { I.StagingW->Release();   I.StagingW   = NULL; }
    if (I.OffscreenB) { I.OffscreenB->Release(); I.OffscreenB = NULL; }
    if (I.OffscreenW) { I.OffscreenW->Release(); I.OffscreenW = NULL; }

    // Reset Impl to a clean default state so the next Init() starts fresh.
    // This clears all bool flags (bWeaponAttached, bAttached) and enum state
    // that Shutdown() does not explicitly reset.
    delete Impl;
    Impl = NULL;
}

void FOLPreviewInstance::ReleaseD3DObjects()
{
    // Release only D3DPOOL_DEFAULT surfaces so the device can be Reset.
    // The rest of the instance (scene, UObjects, mesh cache) stays intact
    // and ReadbackTexture() will recreate the surfaces on the next tick.
    if (!Impl) return;
    FImpl& I = *Impl;
    if (I.StagingB) { I.StagingB->Release(); I.StagingB = NULL; }
    if (I.StagingW) { I.StagingW->Release(); I.StagingW = NULL; }
}

bool FOLPreviewInstance::IsInited() const
{
    return Impl && Impl->Scene != NULL;
}

void FOLPreviewInstance::SetSkeletalMesh(USkeletalMesh* Mesh)
{
    if (!Impl || !Impl->Scene || !Impl->SkeletalComp) return;
    FImpl& I = *Impl;
    I.RemoveAll();
    if (!Mesh) return;

    I.SkeletalComp->SetSkeletalMesh(Mesh);
    // Center the mesh at origin so the camera distance formula works correctly.
    FMatrix LocalToWorld = FTranslationMatrix(-Mesh->Bounds.Origin);
    I.Scene->AddComponent(I.SkeletalComp, LocalToWorld);
    I.SkeletalComp->UpdateBounds();
    I.PreviewType = PT_Skeletal;
}

void FOLPreviewInstance::SetSkeletalMeshWithAnim(USkeletalMesh* Mesh, const char* AnimSetPkg, const char* AnimSetObj, const char* IdleAnimName)
{
    SetSkeletalMesh(Mesh);
    if (!Impl || !Impl->Scene || !Mesh || !AnimSetObj || !AnimSetObj[0]) return;
    FImpl& I = *Impl;

    // Load the AnimSet via the universal mod package loader.
    UObject* Loaded = Utils::LoadObjectFromModPackage(
        FString(ANSI_TO_TCHAR(AnimSetPkg)),
        FString(ANSI_TO_TCHAR(AnimSetObj)),
        UAnimSet::StaticClass());
    UAnimSet* AnimSet = Loaded ? Cast<UAnimSet>(Loaded) : NULL;
    if (!AnimSet) return;

    // Release previous AnimNode if any.
    if (I.AnimNode) { I.AnimNode->RemoveFromRoot(); I.AnimNode = NULL; }

    // Create a fresh AnimNodeSequence owned by this component.
    // Ownership by SkeletalComp ensures InitAnimTree sets SkelComponent correctly.
    I.AnimNode = ConstructObject<UAnimNodeSequence>(
        UAnimNodeSequence::StaticClass(), I.SkeletalComp);
    I.AnimNode->AddToRoot();
    I.AnimNode->bLooping                      = TRUE;
    I.AnimNode->bDisableWarningWhenAnimNotFound = TRUE;

    // Assign AnimSet and root node, then flush linkup cache and init the tree.
    I.SkeletalComp->AnimSets.Empty();
    I.SkeletalComp->AnimSets.AddItem(AnimSet);
    I.SkeletalComp->Animations = I.AnimNode;
    I.SkeletalComp->UpdateAnimations();
    I.SkeletalComp->InitAnimTree(TRUE);

    // Start looping idle.
    I.AnimNode->SetAnim(FName(ANSI_TO_TCHAR(IdleAnimName)));
    I.AnimNode->PlayAnim(TRUE, 1.0f, 0.0f);
}

void FOLPreviewInstance::AttachWeaponToBone(UStaticMesh* Mesh, const char* BoneName)
{
    if (!Impl || !Impl->Scene || !Impl->SkeletalComp || !Impl->WeaponComp) return;
    FImpl& I = *Impl;

    // Detach previous weapon first.
    I.DetachWeapon();

    if (!Mesh || !BoneName || !BoneName[0]) return;

    I.WeaponComp->SetStaticMesh(Mesh);
    I.WeaponComp->Materials.Empty();
    I.SkeletalComp->AttachComponent(I.WeaponComp, FName(ANSI_TO_TCHAR(BoneName)));
    I.bWeaponAttached = true;
}

void FOLPreviewInstance::SetStaticMesh(UStaticMesh* Mesh)
{
    if (!Impl || !Impl->Scene || !Impl->StaticComp) return;
    FImpl& I = *Impl;
    I.RemoveAll();
    if (!Mesh) return;

    I.StaticComp->StaticMesh = Mesh;
    I.StaticComp->Materials.Empty();
    FMatrix LocalToWorld = FTranslationMatrix(-Mesh->Bounds.Origin);
    I.Scene->AddComponent(I.StaticComp, LocalToWorld);
    I.PreviewType = PT_Static;
}

void FOLPreviewInstance::Tick(float YawDeg, float FOVDeg)
{
    if (!Impl) return;
    FImpl& I = *Impl;
    if (!I.Scene || !I.RTBlack || I.PreviewType == PT_None)
        return;

    // Advance skeletal animation by one game tick.
    if (I.PreviewType == PT_Skeletal && I.SkeletalComp && I.AnimNode)
    {
        // If the sequence wasn't found yet (AnimSet not ready), retry PlayAnim each tick.
        if (!I.AnimNode->bPlaying && I.AnimNode->AnimSeqName != NAME_None)
        {
            I.AnimNode->PlayAnim(TRUE, 1.0f, 0.0f);
        }
        I.SkeletalComp->TickAnimNodes(GDeltaTime);
        I.SkeletalComp->UpdateSkelPose(GDeltaTime);
        I.SkeletalComp->ConditionalUpdateTransform();
        // UpdateChildComponents() is protected — replicate its logic manually.
        for (INT ai = 0; ai < I.SkeletalComp->Attachments.Num(); ++ai)
        {
            FAttachment& A = I.SkeletalComp->Attachments(ai);
            if (!A.Component) continue;
            const INT BoneIdx = I.SkeletalComp->MatchRefBone(A.BoneName);
            if (BoneIdx == INDEX_NONE || BoneIdx >= I.SkeletalComp->SpaceBases.Num()) continue;
            FVector RelScale = (A.RelativeScale == FVector(0)) ? FVector(1) : A.RelativeScale;
            FMatrix AttachToWorld = FScaleRotationTranslationMatrix(RelScale, A.RelativeRotation, A.RelativeLocation)
                * I.SkeletalComp->SpaceBases(BoneIdx).ToMatrix()
                * I.SkeletalComp->LocalToWorld;
            A.Component->UpdateComponent(I.Scene->GetScene(), NULL, AttachToWorld);
        }
    }

    FTextureRenderTargetResource* RTResB = I.RTBlack->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* RTResW = I.RTWhite->GameThread_GetRenderTargetResource();
    if (!RTResB || !RTResW) return;

    FBoxSphereBounds Bounds = I.GetBounds();

    struct FRenderParams
    {
        FTextureRenderTargetResource* RT;
        FSceneInterface*              Scene;
        EShowFlags                    ShowFlags;
        FLOAT                         WorldTime;
        FLOAT                         DeltaTime;
        FMatrix                       ViewMatrix;
        FMatrix                       ProjMatrix;
        FLinearColor                  Background;
    };

    FRenderParams Base;
    Base.Scene       = I.Scene->GetScene();
    Base.ShowFlags   = (SHOW_DefaultEditor & ~SHOW_ViewMode_Mask) | SHOW_ViewMode_Lit;
    Base.WorldTime   = GCurrentTime - GStartTime;
    Base.DeltaTime   = GDeltaTime;
    // Skeletal meshes need a tighter framing than static meshes.
    const FLOAT DistScale = (I.PreviewType == PT_Skeletal) ? 0.6f : 1.05f;
    Base.ViewMatrix  = FImpl::MakeViewMatrix(Bounds, YawDeg, FOVDeg, DistScale);
    Base.ProjMatrix  = FPerspectiveMatrix(
        FOVDeg * (3.14159f / 360.f), 1.f, 1.f, GNearClippingPlane);

    FRenderParams ParamsB = Base;
    ParamsB.RT         = RTResB;
    ParamsB.Background = FLinearColor(0.f, 0.f, 0.f, 1.f);

    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(OLPreviewRenderBlack, FRenderParams, P, ParamsB,
    {
        if (P.RT)
        {
            FSceneViewFamilyContext VF(P.RT, P.Scene, P.ShowFlags,
                P.WorldTime, P.DeltaTime, P.WorldTime,
                FALSE, FALSE, FALSE, TRUE, TRUE, 1.0f, FALSE, TRUE);
            FSceneView* V = new FSceneView(&VF, NULL, -1, NULL, NULL, NULL, NULL, NULL, NULL,
                0.f, 0.f, (FLOAT)P.RT->GetSizeX(), (FLOAT)P.RT->GetSizeY(),
                P.ViewMatrix, P.ProjMatrix,
                P.Background, FLinearColor(0,0,0,0), FLinearColor::White,
                TSet<UPrimitiveComponent*>());
            VF.Views.AddItem(V);
            TArray<FPostProcessSceneProxy*> PP;
            FSceneRenderer* R = CreateSceneCaptureRenderer(V, &VF, PP, NULL, FMatrix::Identity, TRUE);
            R->Render(); delete R;
            RHICopyToResolveTarget(P.RT->GetRenderTargetSurface(), FALSE, FResolveParams());
        }
    });

    FRenderParams ParamsW = Base;
    ParamsW.RT         = RTResW;
    ParamsW.Background = FLinearColor(1.f, 1.f, 1.f, 1.f);

    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(OLPreviewRenderWhite, FRenderParams, P, ParamsW,
    {
        if (P.RT)
        {
            FSceneViewFamilyContext VF(P.RT, P.Scene, P.ShowFlags,
                P.WorldTime, P.DeltaTime, P.WorldTime,
                FALSE, FALSE, FALSE, TRUE, TRUE, 1.0f, FALSE, TRUE);
            FSceneView* V = new FSceneView(&VF, NULL, -1, NULL, NULL, NULL, NULL, NULL, NULL,
                0.f, 0.f, (FLOAT)P.RT->GetSizeX(), (FLOAT)P.RT->GetSizeY(),
                P.ViewMatrix, P.ProjMatrix,
                P.Background, FLinearColor(0,0,0,0), FLinearColor::White,
                TSet<UPrimitiveComponent*>());
            VF.Views.AddItem(V);
            TArray<FPostProcessSceneProxy*> PP;
            FSceneRenderer* R = CreateSceneCaptureRenderer(V, &VF, PP, NULL, FMatrix::Identity, TRUE);
            R->Render(); delete R;
            RHICopyToResolveTarget(P.RT->GetRenderTargetSurface(), FALSE, FResolveParams());
        }
    });

    FlushRenderingCommands();

    if (!GLegacyDirect3DDevice9) return;

    I.D3DBlack = UnwrapRT(RTResB);
    I.D3DWhite = UnwrapRT(RTResW);
    if (!I.D3DBlack || !I.D3DWhite) return;

    const UINT Size = OL_PREVIEW_SIZE;

    if (!ReadbackTexture(I.D3DBlack, Size, I.StagingB, I.OffscreenB) ||
        !ReadbackTexture(I.D3DWhite, Size, I.StagingW, I.OffscreenW))
        return;

    D3DLOCKED_RECT LB, LW;
    if (FAILED(I.OffscreenB->LockRect(&LB, NULL, D3DLOCK_READONLY)) ||
        FAILED(I.OffscreenW->LockRect(&LW, NULL, D3DLOCK_READONLY)))
        return;

    IDirect3DTexture9* Chroma = I.ChromaTexture;
    if (!Chroma)
    {
        HRESULT hr = GLegacyDirect3DDevice9->CreateTexture(
            Size, Size, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &Chroma, NULL);
        if (FAILED(hr)) { I.OffscreenB->UnlockRect(); I.OffscreenW->UnlockRect(); return; }
    }

    D3DLOCKED_RECT LD;
    if (FAILED(Chroma->LockRect(0, &LD, NULL, 0)))
    {
        I.OffscreenB->UnlockRect(); I.OffscreenW->UnlockRect();
        return;
    }

    for (UINT y = 0; y < Size; ++y)
    {
        const DWORD* SB  = (const DWORD*)((const BYTE*)LB.pBits + y * LB.Pitch);
        const DWORD* SW  = (const DWORD*)((const BYTE*)LW.pBits + y * LW.Pitch);
        DWORD*       Dst = (DWORD*)((BYTE*)LD.pBits + y * LD.Pitch);
        for (UINT x = 0; x < Size; ++x)
        {
            DWORD PB = SB[x], PW = SW[x];
            int BR = (PB>>16)&0xFF, WR = (PW>>16)&0xFF;
            int BG = (PB>> 8)&0xFF, WG = (PW>> 8)&0xFF;
            int BB =  PB    &0xFF,  WB =  PW    &0xFF;

            int AR = 255-(WR-BR); if (AR<0) AR=0;
            int AG = 255-(WG-BG); if (AG<0) AG=0;
            int AB = 255-(WB-BB); if (AB<0) AB=0;
            int A  = AR>AG ? (AR>AB?AR:AB) : (AG>AB?AG:AB);

            int OR = A>0 ? (BR*255)/A : 0; if (OR>255) OR=255;
            int OG = A>0 ? (BG*255)/A : 0; if (OG>255) OG=255;
            int OB = A>0 ? (BB*255)/A : 0; if (OB>255) OB=255;

            Dst[x] = ((DWORD)A<<24)|((DWORD)OR<<16)|((DWORD)OG<<8)|(DWORD)OB;
        }
    }

    Chroma->UnlockRect(0);
    I.OffscreenB->UnlockRect();
    I.OffscreenW->UnlockRect();

    I.ChromaTexture = Chroma;
}

void* FOLPreviewInstance::GetImTextureID() const
{
    if (!Impl) return NULL;
    return Impl->ChromaTexture ? (void*)Impl->ChromaTexture : (void*)Impl->D3DBlack;
}

// ---------------------------------------------------------------------------
// Attachment support (delegates to main instance's FImpl)
// ---------------------------------------------------------------------------

void OLPreview_AttachToSocket(int Slot, USkeletalMesh* Mesh, FName SocketName)
{
    if (!GPreviewMain.IsInited()) return;
    FOLPreviewInstance::FImpl& I = *GPreviewMain.Impl;
    if (!I.SkeletalComp || Slot < 0 || Slot >= OL_PREVIEW_MAX_ATTACHMENTS) return;

    FOLPreviewInstance::FImpl::FAttachment& A = I.Attachments[Slot];
    if (A.bAttached) { I.SkeletalComp->DetachComponent(A.Comp); A.bAttached = false; }
    if (!A.Comp) return;

    A.Comp->SetSkeletalMesh(Mesh);
    if (Mesh && SocketName != NAME_None)
    {
        I.SkeletalComp->AttachComponentToSocket(A.Comp, SocketName);
        A.bAttached = true;
    }
}

void OLPreview_DetachSlot(int Slot)
{
    OLPreview_AttachToSocket(Slot, NULL, NAME_None);
}

// ---------------------------------------------------------------------------
// Global instance
// ---------------------------------------------------------------------------

FOLPreviewInstance GPreviewMain;

// ---------------------------------------------------------------------------
// OLWorldCapture — live world-space actor preview
// ---------------------------------------------------------------------------

// How far back and up from the actor to place the capture camera (UU).
static const float OL_WCAP_DIST_BACK = 300.f;
static const float OL_WCAP_DIST_UP   = 150.f;

// No actor needed — component is attached directly to WorldInfo.
static USceneCapture2DComponent* GWCapComp    = NULL;
static UTextureRenderTarget2D*   GWCapRT      = NULL;
// Cached RT resource pointer — updated on game thread, read by render thread.
static FTextureRenderTargetResource* volatile GWCapRTRes = NULL;

// We register a separate map-change listener for world capture.
class FOLWorldCaptureMapListener : public FCallbackEventDevice
{
public:
    virtual void Send(ECallbackEventType InType)
    {
        if (InType == CALLBACK_PreLoadMap)
            OLWorldCapture_Shutdown();
    }
};
static FOLWorldCaptureMapListener* GWCapMapChangeListener = NULL;

void OLWorldCapture_Ensure()
{
    // Register map-change listener once.
    if (!GWCapMapChangeListener)
    {
        GWCapMapChangeListener = new FOLWorldCaptureMapListener();
        GCallbackEvent->Register(CALLBACK_PreLoadMap, GWCapMapChangeListener);
    }

    if (GWCapComp && GWCapRT)
        return; // already valid

    if (!GWorld) return;

    // Create the render target.
    if (!GWCapRT)
    {
        GWCapRT = ConstructObject<UTextureRenderTarget2D>(
            UTextureRenderTarget2D::StaticClass(), UObject::GetTransientPackage());
        if (!GWCapRT) return;
        GWCapRT->AddToRoot();
        GWCapRT->ClearColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.f);
        GWCapRT->Init(OL_WORLD_CAPTURE_SIZE, OL_WORLD_CAPTURE_SIZE, PF_A8R8G8B8, FALSE);
    }

    // Attach USceneCapture2DComponent directly to WorldInfo — no actor spawn needed.
    // SceneCaptureActor has bNoDelete=true which blocks SpawnActor at runtime.
    AWorldInfo* WI = GWorld->GetWorldInfo();
    if (!WI) return;

    GWCapComp = ConstructObject<USceneCapture2DComponent>(
        USceneCapture2DComponent::StaticClass(), WI);
    if (!GWCapComp) return;
    GWCapComp->AddToRoot();

    GWCapComp->TextureTarget   = GWCapRT;
    GWCapComp->FieldOfView     = 70.f;
    GWCapComp->NearPlane       = 10.f;
    // bUpdateMatrices=FALSE: SetParentToWorld won't overwrite ViewMatrix set by SetView().
    // We must build ProjMatrix manually since UpdateProjMatrix() also skips when FALSE.
    GWCapComp->bUpdateMatrices = FALSE;
    GWCapComp->ProjMatrix = FPerspectiveMatrix(
        70.f * (FLOAT)PI / 360.f,
        (FLOAT)OL_WORLD_CAPTURE_SIZE,
        (FLOAT)OL_WORLD_CAPTURE_SIZE,
        10.f);

    // Attach to WorldInfo so it participates in scene rendering.
    WI->AttachComponent(GWCapComp);
    // GWCapRTRes is updated lazily in OLWorldCapture_SetTarget once resource is ready.
}

// Pick a camera position around TargetLoc by tracing 8 rays outward from the
// actor's location and choosing the direction with the longest clear path.
// SourceActor is excluded from collision (its own components won't block the trace).
// Volumes are excluded. Hard limit: MaxTraceDist UU.
static FVector WCap_ChooseCamPos(FVector TargetLoc, float Radius, AActor* SourceActor)
{
    const float R            = Clamp(Radius, 30.f, 2000.f);
    const float MaxTraceDist = 3000.f;
    const float MinCamDist   = R * 1.2f;

    // Trace up and down from TargetLoc; camera goes toward whichever side has more space.
    const float VertProbe = R * 6.f;
    float DistUp   = VertProbe;
    float DistDown = VertProbe;
    {
        FCheckResult HitUp;
        if (!GWorld->SingleLineCheck(HitUp, SourceActor,
            TargetLoc + FVector(0,0, VertProbe), TargetLoc, TRACE_AllColliding | TRACE_ComplexCollision))
            DistUp = (HitUp.Location - TargetLoc).Size();

        FCheckResult HitDown;
        if (!GWorld->SingleLineCheck(HitDown, SourceActor,
            TargetLoc + FVector(0,0,-VertProbe), TargetLoc, TRACE_AllColliding | TRACE_ComplexCollision))
            DistDown = (HitDown.Location - TargetLoc).Size();
    }
    const float PitchSign = (DistUp >= DistDown) ? 1.f : -1.f;

    // 8 yaw directions × 3 pitch levels, sign chosen by floor detection.
    const int   NumYaw        = 8;
    const float PitchAngles[] = { 20.f, 40.f, 60.f };
    const int   NumPitch      = 3;

    float   BestDist = -1.f;
    FVector BestDir(0.f, -1.f, 0.f);

    for (int pi = 0; pi < NumPitch; ++pi)
    {
        float PitchRad   = PitchAngles[pi] * (PI / 180.f);
        float HorizScale = appCos(PitchRad);
        float VertScale  = appSin(PitchRad) * PitchSign;

        for (int yi = 0; yi < NumYaw; ++yi)
        {
            float YawRad = yi * (2.f * PI / NumYaw);
            FVector Dir(
                appCos(YawRad) * HorizScale,
                appSin(YawRad) * HorizScale,
                VertScale);

            FVector TraceEnd = TargetLoc + Dir * MaxTraceDist;

            FCheckResult Hit;
            float HitDist;
            if (GWorld->SingleLineCheck(Hit, SourceActor, TraceEnd, TargetLoc, TRACE_AllColliding | TRACE_ComplexCollision))
            {
                HitDist = MaxTraceDist;
            }
            else
            {
                HitDist = (Hit.Location - TargetLoc).Size();
            }

            if (HitDist > BestDist)
            {
                BestDist = HitDist;
                BestDir  = Dir;
            }
        }
    }

    // Ideal distance so the object's sphere fills the frame at FOV=70°.
    // tan(FOV/2) = R / IdealDist  →  IdealDist = R / tan(35°)
    const float IdealDist = R / appTan(35.f * (PI / 180.f));

    // Camera sits at ideal distance, but never farther than what the trace found clear.
    float CamDist = Clamp(IdealDist, MinCamDist, BestDist * 0.9f);
    FVector CamPos = TargetLoc + BestDir * CamDist;

    // Safety trace from chosen camera position back to target —
    // pull camera to hit point if the path turned out to be blocked.
    FCheckResult Hit2;
    if (!GWorld->SingleLineCheck(Hit2, SourceActor, TargetLoc, CamPos, TRACE_AllColliding | TRACE_ComplexCollision))
    {
        // Move slightly away from the hit surface toward the camera direction.
        CamPos = Hit2.Location + BestDir * (-20.f);
    }

    return CamPos;
}

static void WCap_ApplyView(FVector TargetLoc, float Radius, AActor* SourceActor)
{
    // Update cached RT resource lazily — ready a few frames after Init().
    if (!GWCapRTRes && GWCapRT)
        GWCapRTRes = GWCapRT->GameThread_GetRenderTargetResource();

    FVector CamLoc = WCap_ChooseCamPos(TargetLoc, Radius, SourceActor);
    FVector Dir    = TargetLoc - CamLoc;
    FRotator Rot   = Dir.Rotation();
    GWCapComp->SetView(CamLoc, Rot);
}

// Build aggregate bounds from all primitive components of an actor.
// Returns false if no components found (fallback to actor Location).
static bool WCap_GetAggrBounds(AActor* Target, FBoxSphereBounds& OutBounds)
{
    bool bHasAny = false;
    for (INT ci = 0; ci < Target->Components.Num(); ++ci)
    {
        UPrimitiveComponent* PC = Cast<UPrimitiveComponent>(Target->Components(ci));
        if (!PC || PC->Bounds.SphereRadius < 1.f) continue;
        OutBounds = bHasAny ? (OutBounds + PC->Bounds) : PC->Bounds;
        bHasAny = true;
    }
    return bHasAny;
}

void OLWorldCapture_SetTarget(AActor* Target)
{
    if (!Target || !GWCapComp) return;
    if (Target->IsPendingKill() || !Target->IsValid()) return;

    FBoxSphereBounds Aggr;
    FVector Center;
    float   Radius;
    if (WCap_GetAggrBounds(Target, Aggr))
    {
        Center = Aggr.Origin;       // real geometric center, not actor pivot
        Radius = Max(Aggr.SphereRadius, 30.f);
    }
    else
    {
        Center = Target->Location;
        Radius = 50.f;
    }

    WCap_ApplyView(Center, Radius, Target);
}

void OLWorldCapture_SetPos(FVector WorldPos, float Radius)
{
    if (!GWCapComp) return;
    // No SourceActor to exclude for free-standing component positions.
    WCap_ApplyView(WorldPos, Radius, NULL);
}

void OLWorldCapture_Shutdown()
{
    GWCapRTRes = NULL;
    if (GWCapComp)
    {
        AWorldInfo* WI = GWorld ? GWorld->GetWorldInfo() : NULL;
        if (WI) WI->DetachComponent(GWCapComp);
        GWCapComp->RemoveFromRoot();
        GWCapComp = NULL;
    }
    if (GWCapRT)
    {
        GWCapRT->RemoveFromRoot();
        GWCapRT = NULL;
    }
}

IDirect3DTexture9* OLWorldCapture_GetTexture()
{
    FTextureRenderTargetResource* Res = GWCapRTRes;
    if (!Res) return NULL;
    return UnwrapRT(Res);
}

// ImDrawCallback: switch to opaque blend (ONE, ZERO) so RT alpha is ignored.
void OLWorldCapture_SetOpaqueBlend(const ImDrawList*, const ImDrawCmd*)
{
    if (!GLegacyDirect3DDevice9) return;
    GLegacyDirect3DDevice9->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    GLegacyDirect3DDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}

// ImDrawCallback: restore standard premultiplied alpha blend used by ImGui.
void OLWorldCapture_RestoreBlend(const ImDrawList*, const ImDrawCmd*)
{
    if (!GLegacyDirect3DDevice9) return;
    GLegacyDirect3DDevice9->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    GLegacyDirect3DDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}
