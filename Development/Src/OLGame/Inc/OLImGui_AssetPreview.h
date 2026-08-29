#pragma once
/*=============================================================================
    OLImGui_AssetPreview.h — Self-contained 3D asset preview instance.

    Each FOLPreviewInstance owns its own FPreviewScene + render targets + D3D9
    textures. Create as many as needed; call Tick() each game tick; destroy with
    Shutdown() before deleting. Thread-safe: Tick runs on game thread, GetTexture
    is read on render thread (atomic pointer).
=============================================================================*/

class USkeletalMesh;
class UStaticMesh;

// Preview render size (pixels). All instances share the same size for now.
static const int OL_PREVIEW_SIZE = 256;

// ---------------------------------------------------------------------------
// FOLPreviewInstance — one self-contained preview scene + texture.
// ---------------------------------------------------------------------------

struct FOLPreviewInstance
{
    // Call once to allocate scene + RTs. Safe to call multiple times (no-op).
    // Automatically registers this instance for ShutdownAll().
    void Init();

    // Release all resources. Must be called before engine shutdown or map change.
    void Shutdown();

    // Clear the skeletal mesh + AnimSet from the component without full shutdown.
    // Call after CollectGarbage to drop potentially freed AnimSet references.
    void ClearMesh();

    bool IsInited() const;

    // Set the mesh to display. Game thread only.
    void SetSkeletalMesh(USkeletalMesh* Mesh);
    // Set skeletal mesh and play a looping animation from the given AnimSet.
    // AnimSetPkg/"" + AnimSetObj — passed to LoadObjectFromModPackage; IdleAnimName — e.g. "Idle".
    void SetSkeletalMeshWithAnim(USkeletalMesh* Mesh, const char* AnimSetPkg, const char* AnimSetObj, const char* IdleAnimName);

    // Attach a static mesh weapon to a bone on the skeletal mesh. Pass NULL to detach.
    void AttachWeaponToBone(UStaticMesh* Mesh, const char* BoneName);
    void SetStaticMesh(UStaticMesh* Mesh);

    // Re-render and update the texture. Call each game tick.
    void Tick(float YawDeg, float FOVDeg);

    // Returns D3D9 texture ready for ImGui::Image, or NULL. Safe from render thread.
    void* GetImTextureID() const;

    // Zero-init constructor.
    FOLPreviewInstance();

    // Release only D3DPOOL_DEFAULT surfaces (StagingB/W) before a device Reset.
    // Safe to call even if the instance is not yet Init()'d.
    void ReleaseD3DObjects();

    // Implementation pointer — public so OLPreview_AttachToSocket (same TU) can access it.
    struct FImpl;
    FImpl* Impl;
};

// ---------------------------------------------------------------------------
// Global system init/shutdown (map-change listener, etc.)
// Call once; individual instances manage their own scenes.
// ---------------------------------------------------------------------------
void OLPreview_InitSystem();
void OLPreview_ShutdownSystem();

// ---------------------------------------------------------------------------
// Backwards-compat single global instance (used by existing callers).
// ---------------------------------------------------------------------------
extern FOLPreviewInstance GPreviewMain;

// Backwards-compat helpers (map to GPreviewMain).
inline void  OLPreview_Init()                            { GPreviewMain.Init(); }
inline void  OLPreview_Shutdown()                        { GPreviewMain.Shutdown(); }
inline bool  OLPreview_IsInited()                        { return GPreviewMain.IsInited(); }
inline void  OLPreview_SetSkeletalMesh(USkeletalMesh* M) { GPreviewMain.SetSkeletalMesh(M); }
inline void  OLPreview_SetStaticMesh(UStaticMesh* M)     { GPreviewMain.SetStaticMesh(M); }
inline void* OLPreview_GetImTextureID()                  { return GPreviewMain.GetImTextureID(); }
inline void  OLPreview_SetDirty()                        { /* no-op: renders every tick */ }
// OLPreview_Tick() — legacy shim; actual ticking is done by OLSpawns_TickPreviews() each game tick.

// Yaw and FOV for the main preview (read by OLImGui_Tabs.h macro).
extern float GOLPreviewYaw;
extern float GOLPreviewFOV;

// Pending mesh slots — written on render thread, applied by game-thread callbacks.
extern void* volatile GOLPreviewPendingSkeletal;
extern void* volatile GOLPreviewPendingStatic;

// Attachment support on the main instance.
static const int OL_PREVIEW_MAX_ATTACHMENTS = 4;
void OLPreview_AttachToSocket(int Slot, USkeletalMesh* Mesh, FName SocketName);
void OLPreview_DetachSlot(int Slot);

// Game-thread callbacks — pass to OLImGui_EnqueueCall.
void OLPreview_ApplySkeletal();
void OLPreview_ApplyStatic();

// ---------------------------------------------------------------------------
// World capture — renders a live view of a world-space actor for hover preview.
//
// One permanent ASceneCapture2DActor lives in GWorld for the lifetime of the
// game session. On hover, OLWorldCapture_SetTarget() repositions the camera
// above+behind the target actor. The RT texture can be read immediately on the
// next render frame.
//
// All functions are game-thread only.
// ---------------------------------------------------------------------------

class AActor;
struct IDirect3DTexture9;

// Size of the world-capture render target (pixels).
static const int OL_WORLD_CAPTURE_SIZE = 256;

// Ensure the capture actor and RT exist; safe to call every frame.
void OLWorldCapture_Ensure();

// Point the capture camera at the given actor.
void OLWorldCapture_SetTarget(AActor* Target);

// Point the capture camera at an arbitrary world position (for components without an actor).
// Radius hints the camera distance so the object fills the frame.
void OLWorldCapture_SetPos(FVector WorldPos, float Radius = 50.f);

// Shut down and release all resources (call on map change).
void OLWorldCapture_Shutdown();

// Returns the D3D9 texture with the most recent captured frame, or NULL.
// Safe to call from the render thread.
IDirect3DTexture9* OLWorldCapture_GetTexture();

// ImDrawCallbacks to bracket the world-capture Image() call.
// Switch to opaque blend so RT alpha (unwritten by scene) doesn't make the image transparent.
struct ImDrawList;
struct ImDrawCmd;
void OLWorldCapture_SetOpaqueBlend(const ImDrawList* dl, const ImDrawCmd* cmd);
void OLWorldCapture_RestoreBlend(const ImDrawList* dl, const ImDrawCmd* cmd);
