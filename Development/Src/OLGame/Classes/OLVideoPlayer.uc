/**
 * OLVideoPlayer
 * Streams video from a URL (YouTube etc.) via yt-dlp + ffmpeg into a Texture2DDynamic.
 * yt-dlp.exe and ffmpeg.exe must be in the same directory as OLGame.exe.
 *
 * Usage (UC):
 *   local OLVideoPlayer VP;
 *   VP = Spawn(class'OLVideoPlayer');
 *   VP.Play("https://www.youtube.com/watch?v=...");
 *   // VP.VideoTexture is updated each tick with the current frame
 */
class OLVideoPlayer extends Actor
    native
    placeable;

/** Dynamic texture updated each frame with decoded video. */
var Texture2DDynamic VideoTexture;

/** Video width in pixels (set after stream is opened). */
var int VideoWidth;
/** Video height in pixels (set after stream is opened). */
var int VideoHeight;

/** TRUE while video is actively streaming. */
var bool bPlaying;

/** Screen to assign VideoTexture to once ready. */
var OLBikScreen TargetScreen;

/** TRUE once VideoTexture has been assigned to TargetScreen. */
var bool bTextureAssigned;

/** Internal native handle to the ffmpeg pipe thread. */
var native const pointer NativeHandle;

/**
 * Start streaming video from the given URL.
 * Spawns yt-dlp to resolve the direct stream URL, then ffmpeg to decode frames.
 */
native function Play(string URL);

/** Stop playback and clean up the pipe/thread. */
native function Stop();

// Assign texture to screen once VideoTexture is ready
event Tick(float DeltaTime)
{
    if (!bTextureAssigned && VideoTexture != None && TargetScreen != None)
    {
        TargetScreen.MovieMaterial.SetTextureParameterValue('Emissive2', VideoTexture);
        TargetScreen.ScreenMesh.SetMaterial(1, TargetScreen.MovieMaterial);
        bTextureAssigned = true;
    }
}

cpptext
{
    virtual void BeginDestroy();
    virtual UBOOL Tick(FLOAT DeltaTime, ELevelTick TickType);
}

defaultproperties
{
    bPlaying=false
    bCollideActors=false
    bBlockActors=false
    Physics=PHYS_None
    bStatic=false
    bNoDelete=false
}
