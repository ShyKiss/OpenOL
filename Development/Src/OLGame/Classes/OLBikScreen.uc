/**
 * OLBikScreen
 * A vertical plane that plays a .bik file from OLGame/Movies/.
 * Spawned at runtime via the "playbik <filename>" exec command.
 */
class OLBikScreen extends Actor
    placeable;

/** The dynamic TextureMovie used for playback. */
var TextureMovie MovieTex;

/** The material instance applied to the plane mesh. */
var MaterialInstanceConstant MovieMaterial;

/** Static mesh component displaying the plane. */
var StaticMeshComponent ScreenMesh;

/**
 * Initialize the screen: load the .bik and apply it to the mesh material.
 *
 * @param Filename  Name of the .bik file (without extension) inside Movies\.
 */
/** Load mesh and create MIC. If Filename is empty, skips Bink (used by OLVideoPlayer). */
function Init(string Filename)
{
    local MaterialInstanceConstant MIC;
    local MaterialInstanceConstant BaseMIC;
    local StaticMesh PlaneMesh;

    // Load the security screen mesh — material slot 1 is the video screen
    PlaneMesh = StaticMesh(class'OLUtils'.static.LoadObjectFromModPackage(
        "Center_block_01b", "Security.Screen.security_screen", class'StaticMesh'));
    if (PlaneMesh != None)
        ScreenMesh.SetStaticMesh(PlaneMesh);

    // Load SecurityVideoBink MIC as parent — it already has Emissive wired to a TextureMovie
    BaseMIC = MaterialInstanceConstant(class'OLUtils'.static.LoadObjectFromModPackage(
        "Center_block_01b", "Security.Screen.SecurityVideoBink", class'MaterialInstanceConstant'));
    // Create our own MIC on top so we can override Emissive2
    MIC = new(self) class'MaterialInstanceConstant';
    if (BaseMIC != None)
        MIC.SetParent(BaseMIC);

    MovieMaterial = MIC;
    ScreenMesh.SetMaterial(1, MIC);

    // Only load Bink if a filename was given (playbik); playvideo leaves this empty
    if (Filename != "")
    {
        MovieTex = new(self) class'OLBikMovieTexture';
        MovieTex.OpenFromFile(Filename);
        MIC.SetTextureParameterValue('Emissive2', MovieTex);
    }
}

defaultproperties
{
    Begin Object Class=StaticMeshComponent Name=ScreenMeshComp
        Scale3D=(X=4.0,Y=4.0,Z=4.0)
        Rotation=(Pitch=0,Yaw=-16384,Roll=0)
        CastShadow=false
        bAcceptsLights=false
        CollideActors=false
    End Object
    ScreenMesh=ScreenMeshComp
    Components.Add(ScreenMeshComp)

    bCollideActors=false
    bBlockActors=false
    bCollideWorld=false
    Physics=PHYS_None
    bStatic=false
    bNoDelete=false
}
