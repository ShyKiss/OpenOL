class OLFXManager extends Object
	native
	config(Game);

//
// Components
// 
var PostProcessSettings NVPPSSettings;
var PostProcessSettings DeathPPSSettings;
var PostProcessChain DefaultPPSChain;
var PostProcessChain CamcorderPPSChain;
var PostProcessChain CamcorderPPSChainConsole;
var PostProcessChain NVPPSChain;
var PostProcessChain GammaCalibrationPPSChain;
var array<MaterialInstanceConstant> NVSensitiveMaterials;
var MaterialInstanceConstant CameraGlitchMat;
var OLUberPostProcessEffect CurrentUberPostEffect;
var bool bGrainDisabled;
var ParticleSystemComponent ElectricSparksParticles;
var OLFXHolder FXHolder;

var name UberPostEffectName;
var name CameraGlitchEffectName;

var name NVParamName;
var name NVLightParamName;
var name CameraGlitchParamName;

//
// Dynamic state
//
var float CamcorderPPSOpacity;
var float CurrentElectricEffect;
var float LastSetElectricEffect;
var float CurrentHurtEffect; // current (moving approach)
var float LastSetHurtEffect; // last set on the material (close to but not equal to current)

var bool bShowingHurtEffect;
var bool bShowingElectricityEffect;

enum EPPSMode
{
	PPS_Default,
	PPS_Camcorder,
	PPS_NightVision,
	PPS_GammaCalibration,
	PPS_Death
};

var EPPSMode CurrentPPSMode;

struct native BlurData
{
	var bool bActive;
	var float Amount;	
	var float Duration;
	var float BlendInTime;
	var float BlendOutTime;
	var float Desaturation;
	var float StartTime;
};

var BlurData CurrentBlur; 
var BlurData HeatBlur;

var bool bSwarmBlurActive;
var float SwarmBlurAmount;

var bool bShatteredGlassEffectActive;

enum CameraGlitchType
{
	CGT_OnOff,
	CGT_LinearDrop,
	CGT_Sine,
};

struct native CameraGlitchData
{
	var bool bActive;
	var float Duration;
	var float StartedTime;
	var float NextGlitchDelay;
	var float CurrentGlitchEffect;
	var CameraGlitchType GlitchType;
};

var CameraGlitchData CameraGlitch;

var const AkEvent SndCameraGlitch;
var const string CameraGlitchIntensity;

struct native MovieEffectData
{
	var bool bActive;
	var float StartedTime;
	var float Intensity;
	var TextureMovie Movie;
	var AkEvent SndEventStop;

	var init array<vector2d> Anim;
	var float Duration;	
};

var MovieEffectData MovieEffect;
var name MovieEffectRTPC;
	
cpptext
{	
	void Init();
	void Tick(FLOAT deltaTime);
	void GetPostProcessSettings(FPostProcessSettings& PPSettings) const;

	void SetPPS(EPPSMode newPPS);
	void UpdateHurtEffect(FLOAT deltaTime, FLOAT hurtEffect);
	void TriggerBlur(FLOAT amount, FLOAT duration, FLOAT desaturation=0.0f, FLOAT blendInTime = 0.1f, FLOAT blendOutTime = 0.25f);
	void KillBlur();
	void SetHeatBlur(FLOAT amount, FLOAT desaturation=0.0f);
	void BindToCurrentUberPostProcess();
	void UpdateSwarmBlur(FLOAT deltaTime);
	void UpdateCameraGlitch(FLOAT deltaTime);
	void UpdateShatteredGlass(FLOAT deltaTime);
	void TriggerElectricSparks(const FVector& location, const FVector& normal);
	void TriggerMovieEffect(const class UOLSeqAct_MovieEffect* movieEffect);
	void UpdateMovieEffect(FLOAT deltaTime);
	void ResetMovieEffect();
	UBOOL IsPlayingMovieEffect();

	void ResetEffects();

private:
	void SetEffectVisibility(FName effectName, UBOOL bVisible);
	void UpdateHurtAndShatteredGlassVisibility();

	UBOOL AreEffectsEnabled() const;
}

native static function OLFXManager GetFXManager();
native function SetPPSFromScript(EPPSMode newPPS);
native function ActivateNightVisionEffect(bool bPowered);
native function ActivateCamcorderEffect();
native function DeactivateNightVisionEffect();
native function SetFXForEnemyPawn(OLEnemyPawn enemyPawn);

defaultproperties
{
	CurrentPPSMode=PPS_Default

	NVPPSChain=PostProcessChain'Asylum_post_process.asylum_NV_PostProcess'
	DefaultPPSChain=PostProcessChain'Asylum_post_process.asylum_PostProcess'
	CamcorderPPSChain=PostProcessChain'Asylum_post_process.asylum_PostProcess_camera'
	CamcorderPPSChainConsole=PostProcessChain'Asylum_post_process.asylum_PostProcess_camera_no_motionblur'
	GammaCalibrationPPSChain=PostProcessChain'Asylum_post_process.Gamma.GammaPPC'
	NVPPSSettings=(bOverride_MotionBlur_InterpolationDuration=True,bOverride_Scene_ImageGrainScale=True,bOverride_Scene_ColorGradingLUT=True,bOverride_RimShader_Color=False,bEnableDOF=True,Bloom_InterpolationDuration=0.000000,DOF_BlurBloomKernelSize=64.000000,DOF_FalloffExponent=3.000000,DOF_BlurKernelSize=1.000000,DOF_FocusInnerRadius=200.000000,DOF_InterpolationDuration=0.000000,MotionBlur_InterpolationDuration=0.000000,Scene_Desaturation=0.500000,Scene_ImageGrainScale=0.020000,Scene_HighLights=(X=0.200000,Y=0.200000,Z=0.200000),Scene_Shadows=(X=0.050000,Y=0.050000,Z=0.050000),Scene_InterpolationDuration=0.000000,RimShader_InterpolationDuration=0.000000,ColorGrading_LookupTable=Texture2D'Asylum_post_process.lut_night_vision_desatured')
	DeathPPSSettings=(bEnableDOF=True,DOF_FocusInnerRadius=0.000000,DOF_MaxNearBlurAmount=0.000000,DOF_InterpolationDuration=0.250000)
	NVSensitiveMaterials[0]=MaterialInstanceConstant'02_Generic_Material.Material.Eye_Mat_INST'
	NVSensitiveMaterials[1]=MaterialInstanceConstant'02_NanoCloud.Material.EyeCloud_INST'
	NVSensitiveMaterials[2]=MaterialInstanceConstant'02_NanoCloud.Material.NANOSWARM_MAT_INST'
	NVSensitiveMaterials[3]=MaterialInstanceConstant'VFX_Nano.Materials.NanoSwarmParticles_INST'
	NVSensitiveMaterials[4]=MaterialInstanceConstant'VFX_Nano.Materials.NanoSwarmParticlesMask_INST'
	CameraGlitchMat=MaterialInstanceConstant'Asylum_post_process.Camera.CameraGlitch_INST'

	Begin Object Class=ParticleSystemComponent Name=ElectricSparksComp
		Template=ParticleSystem'VFX_Lightning.FenceLightning'
		bAutoActivate=false
	End Object
	ElectricSparksParticles=ElectricSparksComp
		
	NVParamName=nvOn
	NVLightParamName=NvHigh
	UberPostEffectName=Uber
	CameraGlitchEffectName=CameraGlitch
	CameraGlitchParamName=GlitchAmount
	MovieEffectRTPC=RTPC_TortureFlashBack

	CameraGlitchIntensity="Cam_Glitch"
	SndCameraGlitch=AkEvent'Player_Sound.CAM_Broken_Noise'
}