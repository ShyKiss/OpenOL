class OLHeatMarker extends OLGameplayMarker
	native
	placeable;

var DrawSphereComponent PreviewComp;
var() bool bNoDamage;

var float Radius;

cpptext
{
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
}

defaultproperties
{
	Radius=100.0

	Begin Object Class=DrawSphereComponent Name=SphereComp
		CollideActors=False
		BlockActors=False
		BlockZeroExtent=False
		BlockNonZeroExtent=False
		BlockRigidBody=False
		SphereRadius=100.0
	End Object
	PreviewComp=SphereComp
	Components.Add(SphereComp)

	Begin Object Name=Sprite
		Sprite=Texture2D'EditorResources.S_Thruster'
	End Object
}
