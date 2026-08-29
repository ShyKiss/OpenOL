/**
 * OLBrowser
 * Runs openol_browser.exe as a child process, reads its BGRA frames from shared memory,
 * and uploads them to a Texture2DDynamic applied to a target OLBikScreen mesh.
 *
 * The player's crosshair position is projected onto the screen mesh and forwarded
 * to the browser as a mouse-move event. When the player aims at the screen,
 * mouse clicks are also forwarded.
 */
class OLBrowser extends Actor
    native
    placeable;

/** Dynamic texture updated each frame with the browser's rendered output. */
var Texture2DDynamic BrowserTexture;

/** Browser viewport width in pixels. */
var int BrowserWidth;
/** Browser viewport height in pixels. */
var int BrowserHeight;

/** Screen mesh actor to display the browser texture on. */
var OLBikScreen TargetScreen;

/** Cached reference to TargetScreen's mesh component for ray intersection (set by SetTargetScreen). */
var native const pointer ScreenMeshComponent;

/** TRUE once BrowserTexture has been assigned to TargetScreen. */
var bool bTextureAssigned;

/** TRUE while the crosshair is on the screen (set by C++ Tick); used by OLPlayerInput to route input. */
var bool bFocused;

/** TRUE while the browser process is running. */
var bool bRunning;

/** Initial URL to open (can be changed at runtime via Navigate). */
var string StartURL;

/** Internal native handle. */
var native const pointer NativeHandle;

/** Start the browser process and open StartURL. */
native function Open();

/** Stop the browser process and clean up. */
native function Close();

/** Navigate to a new URL. */
native function Navigate(string URL);

/** Send a left mouse click at the current cursor position. */
native function SendClick(bool bDown);

/** Send a right mouse click at the current cursor position. */
native function SendRightClick(bool bDown);

/** Send a mouse wheel scroll (positive = up, negative = down). */
native function SendScroll(int DeltaY);

/** Send a keyboard key down/up event (Windows virtual key code). */
native function SendKey(int WinKeyCode, bool bDown);

/** Send a Unicode character input event. */
native function SendChar(int CharCode);

/** Cache TargetScreen's mesh component pointer so C++ can access it without AOLBikScreen definition. */
native function SetScreenMesh(StaticMeshComponent Mesh);

// Assign browser texture to screen once ready; cache mesh component for C++ ray intersection
event Tick(float DeltaTime)
{
    if (!bTextureAssigned && BrowserTexture != None && TargetScreen != None)
    {
        TargetScreen.MovieMaterial.SetTextureParameterValue('Emissive2', BrowserTexture);
        TargetScreen.ScreenMesh.SetMaterial(1, TargetScreen.MovieMaterial);
        SetScreenMesh(TargetScreen.ScreenMesh);
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
    BrowserWidth=1280
    BrowserHeight=720
    bRunning=false
    StartURL="about:blank"

    bCollideActors=false
    bBlockActors=false
    Physics=PHYS_None
    bStatic=false
    bNoDelete=false
}
