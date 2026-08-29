class OLProfileSettings extends UDKProfileSettings
	native
	config(Game);

enum ProfileQualityLevel
{
	PQL_VeryLow,
	PQL_Low,
	PQL_Medium,
	PQL_High,
	PQL_VeryHigh,
	PQL_Custom // (for OverallQuality only)
};

enum EScreenResolution
{
	SR_800x600,		// 4:3
	SR_1024x768,	// 4:3
	SR_1280x720,	// 16:9
	SR_1280x800,	// 16:10
	SR_1280x960,	// 4:3
	SR_1366x768,	// 16:9
	SR_1600x900,	// 16:9
	SR_1600x1200,	// 4:3
	SR_1680x1050,	// 16:10
	SR_1920x1080,	// 16:9	
	SR_1920x1200,	// 16:10
	SR_1920x1440,	// 4:3
	SR_2560x1440,	// 16:9
	SR_2560x1600,	// 16:10

	SR_Other
};

struct native ScreenResolutionInfo
{
	var int Width;
	var int Height;
};

var const array<ScreenResolutionInfo> ScreenResolutions;

enum EGamepadConfigId
{
	GC_TypeA,
	GC_TypeB,
	GC_TypeC,
};

enum ELanguage
{
	EL_English,
	EL_French,
	EL_Spanish,
	EL_Italian,
	EL_German,
	EL_Russian,
	EL_Polish,
	EL_Brazilian,
	EL_Japanese,
};

enum EFPSCap
{
	FPSC_30,
	FPSC_62,
	FPSC_120,
	FPSC_144,
	FPSC_240,
	FPSC_Unlimited,
};


/**
 * Gets the users gamma setting from the profile
 */
function float GetGammaSetting()
{
	local float Value;

	GetProfileSettingValueFloat(PSI_GammaSetting,Value);

	return Value;
}

native function bool GetProfileSettingValues(int ProfileSettingId, out array<name> Values);

native event SetToDefaults();
native function SetLanguageFromSteam();
native function MatchMonitorResolution(bool bLimitForPerformance);
native function AutoDetectPerformanceSettings();

defaultproperties
{
	VersionNumber=19

	ProfileSettingIds.Empty
	ProfileSettingIds(0)=PSI_ControllerVibration
	ProfileSettingIds(1)=PSI_YInversion
	ProfileSettingIds(2)=PSI_ControllerSensitivity
	ProfileSettingIds(3)=PSI_GammaSetting
	ProfileSettingIds(4)=PSI_TextureQuality
	ProfileSettingIds(5)=PSI_ShadowsQuality
	ProfileSettingIds(6)=PSI_EffectsQuality
	ProfileSettingIds(7)=PSI_VSync
	ProfileSettingIds(8)=PSI_Fullscreen
	ProfileSettingIds(9)=PSI_Resolution
	ProfileSettingIds(10)=PSI_GamepadConfig
	ProfileSettingIds(11)=PSI_KB_MoveForward,
	ProfileSettingIds(12)=PSI_KB_MoveBackward,
	ProfileSettingIds(13)=PSI_KB_TurnLeft,
	ProfileSettingIds(14)=PSI_KB_TurnRight,
	ProfileSettingIds(15)=PSI_KB_StrafeLeft,
	ProfileSettingIds(16)=PSI_KB_StrafeRight,
	ProfileSettingIds(17)=PSI_KB_Crouch
	ProfileSettingIds(18)=PSI_KB_Use
	ProfileSettingIds(19)=PSI_KB_Run
	ProfileSettingIds(20)=PSI_KB_ToggleCamcorder
	ProfileSettingIds(21)=PSI_KB_ToggleNightVision
	ProfileSettingIds(22)=PSI_KB_LeanLeft
	ProfileSettingIds(23)=PSI_KB_LeanRight
	ProfileSettingIds(24)=PSI_KB_ZoomImpulseIn
	ProfileSettingIds(25)=PSI_KB_ZoomImpulseOut
	ProfileSettingIds(26)=PSI_KB_Reload
	ProfileSettingIds(27)=PSI_KB_Jump
	ProfileSettingIds(28)=PSI_KB_ShowMenu
	ProfileSettingIds(29)=PSI_KB_ShowTabMenu
	ProfileSettingIds(30)=PSI_KB_ShowRecordingMenu
	ProfileSettingIds(31)=PSI_KB_ShowEvidenceMenu
	ProfileSettingIds(32)=PSI_Volume
	ProfileSettingIds(33)=PSI_Subtitles
	ProfileSettingIds(34)=PSI_Tutorials
	ProfileSettingIds(35)=PSI_ShowCrosshair
	ProfileSettingIds(36)=PSI_Language
	ProfileSettingIds(37)=PSI_GammaInitialized
	ProfileSettingIds(38)=PSI_ToggleCrouch
	ProfileSettingIds(39)=PSI_ShowPrompts
	ProfileSettingIds(40)=PSI_FinishedGame
	ProfileSettingIds(41)=PSI_Southpaw
	ProfileSettingIds(42)=PSI_FinishedDLC
	ProfileSettingIds(43)=PSI_MaxFPS

	DefaultSettings.Empty
	DefaultSettings(0)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_ControllerVibration,Data=(Type=SDT_Int32,Value1=PSB_On)))
	DefaultSettings(1)=(Owner=OPPO_OnlineService,ProfileSetting=(PropertyId=PSI_YInversion,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(2)=(Owner=OPPO_OnlineService,ProfileSetting=(PropertyId=PSI_ControllerSensitivity,Data=(Type=SDT_Float,Value1=1109393408))) // 40.0
	DefaultSettings(3)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_GammaSetting,Data=(Type=SDT_Float,Value1=1074580685))) // 2.2
	DefaultSettings(4)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_TextureQuality,Data=(Type=SDT_Int32,Value1=PQL_VeryHigh)))
	DefaultSettings(5)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_ShadowsQuality,Data=(Type=SDT_Int32,Value1=PQL_VeryHigh)))
	DefaultSettings(6)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_EffectsQuality,Data=(Type=SDT_Int32,Value1=PQL_VeryHigh)))
	DefaultSettings(7)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_VSync,Data=(Type=SDT_Int32,Value1=1)))
	DefaultSettings(8)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Fullscreen,Data=(Type=SDT_Int32,Value1=1)))
	DefaultSettings(9)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Resolution,Data=(Type=SDT_Int32,Value1=SR_1280x800)))
	DefaultSettings(10)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_GamepadConfig,Data=(Type=SDT_Int32,Value1=GC_TypeA)))
	DefaultSettings(11)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_MoveForward,Data=(Type=SDT_String)))
	DefaultSettings(12)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_MoveBackward,Data=(Type=SDT_String)))
	DefaultSettings(13)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_TurnLeft,Data=(Type=SDT_String)))
	DefaultSettings(14)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_TurnRight,Data=(Type=SDT_String)))
	DefaultSettings(15)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_StrafeLeft,Data=(Type=SDT_String)))
	DefaultSettings(16)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_StrafeRight,Data=(Type=SDT_String)))
	DefaultSettings(17)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_Crouch,Data=(Type=SDT_String)))
	DefaultSettings(18)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_Use,Data=(Type=SDT_String)))
	DefaultSettings(19)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_Run,Data=(Type=SDT_String)))
	DefaultSettings(20)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ToggleCamcorder,Data=(Type=SDT_String)))
	DefaultSettings(21)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ToggleNightVision,Data=(Type=SDT_String)))
	DefaultSettings(22)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_LeanLeft,Data=(Type=SDT_String)))
	DefaultSettings(23)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_LeanRight,Data=(Type=SDT_String)))
	DefaultSettings(24)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ZoomImpulseIn,Data=(Type=SDT_String)))
	DefaultSettings(25)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ZoomImpulseOut,Data=(Type=SDT_String)))
	DefaultSettings(26)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_Reload,Data=(Type=SDT_String)))
	DefaultSettings(27)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_Jump,Data=(Type=SDT_String)))
	DefaultSettings(28)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ShowMenu,Data=(Type=SDT_String)))
	DefaultSettings(29)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ShowTabMenu,Data=(Type=SDT_String)))
	DefaultSettings(30)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ShowRecordingMenu,Data=(Type=SDT_String)))
	DefaultSettings(31)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_KB_ShowEvidenceMenu,Data=(Type=SDT_String)))
	DefaultSettings(32)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Volume,Data=(Type=SDT_Float,Value1=1065353216))) // 1.0
	DefaultSettings(33)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Subtitles,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(34)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Tutorials,Data=(Type=SDT_Int32,Value1=PSB_On)))
	DefaultSettings(35)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_ShowCrosshair,Data=(Type=SDT_Int32,Value1=PSB_On)))
	DefaultSettings(36)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Language,Data=(Type=SDT_Int32,Value1=EL_English)))
	DefaultSettings(37)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_GammaInitialized,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(38)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_ToggleCrouch,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(39)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_ShowPrompts,Data=(Type=SDT_Int32,Value1=PSB_On)))
	DefaultSettings(40)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_FinishedGame,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(41)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_Southpaw,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(42)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_FinishedDLC,Data=(Type=SDT_Int32,Value1=PSB_Off)))
	DefaultSettings(43)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PSI_MaxFPS,Data=(Type=SDT_Int32,Value1=FPSC_62)))
						 
	ProfileMappings.Empty
	ProfileMappings(0)=(Id=PSI_ControllerVibration,Name="Controller Vibration",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_On),(Id=PSB_Off)))
	ProfileMappings(1)=(Id=PSI_YInversion,Name="Invert Y",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(2)=(Id=PSI_ControllerSensitivity,Name="Mouse Sensitivity",MappingType=PVMT_RawValue)
	ProfileMappings(3)=(Id=PSI_GammaSetting,Name="Gamma Setting",MappingType=PVMT_RawValue)
	ProfileMappings(4)=(Id=PSI_TextureQuality,Name="Textures",MappingType=PVMT_IdMapped,ValueMappings=((Id=PQL_VeryLow),(Id=PQL_Low),(Id=PQL_Medium),(Id=PQL_High),(Id=PQL_VeryHigh)))
	ProfileMappings(5)=(Id=PSI_ShadowsQuality,Name="Shadows",MappingType=PVMT_IdMapped,ValueMappings=((Id=PQL_VeryLow),(Id=PQL_Low),(Id=PQL_Medium),(Id=PQL_High),(Id=PQL_VeryHigh)))
	ProfileMappings(6)=(Id=PSI_EffectsQuality,Name="Effects",MappingType=PVMT_IdMapped,ValueMappings=((Id=PQL_VeryLow),(Id=PQL_Low),(Id=PQL_Medium),(Id=PQL_High),(Id=PQL_VeryHigh)))	
	ProfileMappings(7)=(Id=PSI_VSync,Name="VSync",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(8)=(Id=PSI_Fullscreen,Name="Fullscreen",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(9)=(Id=PSI_Resolution,Name="Resolution",MappingType=PVMT_IdMapped,ValueMappings=((Id=SR_800x600),(Id=SR_1024x768),(Id=SR_1280x720),(Id=SR_1280x800),(Id=SR_1280x960),(Id=SR_1366x768),(Id=SR_1600x900),(Id=SR_1600x1200),(Id=SR_1680x1050),(Id=SR_1920x1080),(Id=SR_1920x1200),(Id=SR_1920x1440),(Id=SR_2560x1440),(Id=SR_2560x1600),(Id=SR_Other)))
	ProfileMappings(10)=(Id=PSI_GamepadConfig,Name="Gamepad Config Id",MappingType=PVMT_IdMapped,ValueMappings=((Id=GC_TypeA),(Id=GC_TypeB),(Id=GC_TypeC)))
	ProfileMappings(11)=(Id=PSI_KB_MoveForward,Name="KB MoveForward",MappingType=PVMT_RawValue)
	ProfileMappings(12)=(Id=PSI_KB_MoveBackward,Name="KB MoveBackward",MappingType=PVMT_RawValue)
	ProfileMappings(13)=(Id=PSI_KB_TurnLeft,Name="KB TurnLeft",MappingType=PVMT_RawValue)
	ProfileMappings(14)=(Id=PSI_KB_TurnRight,Name="KB TurnRight",MappingType=PVMT_RawValue)
	ProfileMappings(15)=(Id=PSI_KB_StrafeLeft,Name="KB StrafeLeft",MappingType=PVMT_RawValue)
	ProfileMappings(16)=(Id=PSI_KB_StrafeRight,Name="KB StrafeRight",MappingType=PVMT_RawValue)
	ProfileMappings(17)=(Id=PSI_KB_Crouch,Name="KB Crouch",MappingType=PVMT_RawValue)
	ProfileMappings(18)=(Id=PSI_KB_Use,Name="KB Use",MappingType=PVMT_RawValue)
	ProfileMappings(19)=(Id=PSI_KB_Run,Name="KB Run",MappingType=PVMT_RawValue)
	ProfileMappings(20)=(Id=PSI_KB_ToggleCamcorder,Name="KB ToggleCamcorder",MappingType=PVMT_RawValue)
	ProfileMappings(21)=(Id=PSI_KB_ToggleNightVision,Name="KB ToggleNightVision",MappingType=PVMT_RawValue)
	ProfileMappings(22)=(Id=PSI_KB_LeanLeft,Name="KB LeanLeft",MappingType=PVMT_RawValue)
	ProfileMappings(23)=(Id=PSI_KB_LeanRight,Name="KB LeanRight",MappingType=PVMT_RawValue)
	ProfileMappings(24)=(Id=PSI_KB_ZoomImpulseIn,Name="KB ZoomImpulseIn",MappingType=PVMT_RawValue)
	ProfileMappings(25)=(Id=PSI_KB_ZoomImpulseOut,Name="KB ZoomImpulseOut",MappingType=PVMT_RawValue)
	ProfileMappings(26)=(Id=PSI_KB_Reload,Name="KB Reload",MappingType=PVMT_RawValue)
	ProfileMappings(27)=(Id=PSI_KB_Jump,Name="KB Jump",MappingType=PVMT_RawValue)
	ProfileMappings(28)=(Id=PSI_KB_ShowMenu,Name="KB ShowMenu",MappingType=PVMT_RawValue)
	ProfileMappings(29)=(Id=PSI_KB_ShowTabMenu,Name="KB ShowTabMenu",MappingType=PVMT_RawValue)
	ProfileMappings(30)=(Id=PSI_KB_ShowRecordingMenu,Name="KB ShowRecordingMenu",MappingType=PVMT_RawValue)
	ProfileMappings(31)=(Id=PSI_KB_ShowEvidenceMenu,Name="KB ShowEvidenceMenu",MappingType=PVMT_RawValue)
	ProfileMappings(32)=(Id=PSI_Volume,Name="Volume",MappingType=PVMT_RawValue)
	ProfileMappings(33)=(Id=PSI_Subtitles,Name="Show Subtitles",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(34)=(Id=PSI_Tutorials,Name="Show Tutorials",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(35)=(Id=PSI_ShowCrosshair,Name="Show Crosshair",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(36)=(Id=PSI_Language,Name="Language",MappingType=PVMT_IdMapped,ValueMappings=((Id=EL_English),(Id=EL_French),(Id=EL_Spanish),(Id=EL_Italian),(Id=EL_German),(Id=EL_Russian),(Id=EL_Polish),(Id=EL_Brazilian),(Id=EL_Japanese)))
	ProfileMappings(37)=(Id=PSI_GammaInitialized,Name="Gamma Initialized",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(38)=(Id=PSI_ToggleCrouch,Name="Toggle Crouch",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(39)=(Id=PSI_ShowPrompts,Name="Show Prompts",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(40)=(Id=PSI_FinishedGame,Name="Finished Game",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(41)=(Id=PSI_Southpaw,Name="Southpaw",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(42)=(Id=PSI_FinishedDLC,Name="Finished DLC",MappingType=PVMT_IdMapped,ValueMappings=((Id=PSB_Off),(Id=PSB_On)))
	ProfileMappings(43)=(Id=PSI_MaxFPS,Name="Max FPS",MappingType=PVMT_IdMapped,ValueMappings=((Id=FPSC_30),(Id=FPSC_62),(Id=FPSC_120),(Id=FPSC_144),(Id=FPSC_240),(Id=FPSC_Unlimited)))

	ScreenResolutions(SR_800x600)=(Width=800,Height=600)
	ScreenResolutions(SR_1024x768)=(Width=1024,Height=768)
	ScreenResolutions(SR_1280x720)=(Width=1280,Height=720)
	ScreenResolutions(SR_1280x800)=(Width=1280,Height=800)
	ScreenResolutions(SR_1280x960)=(Width=1280,Height=960)
	ScreenResolutions(SR_1366x768)=(Width=1366,Height=768)
	ScreenResolutions(SR_1600x900)=(Width=1600,Height=900)
	ScreenResolutions(SR_1600x1200)=(Width=1600,Height=1200)
	ScreenResolutions(SR_1680x1050)=(Width=1680,Height=1050)
	ScreenResolutions(SR_1920x1080)=(Width=1920,Height=1080)
	ScreenResolutions(SR_1920x1200)=(Width=1920,Height=1200)
	ScreenResolutions(SR_1920x1440)=(Width=1920,Height=1440)
	ScreenResolutions(SR_2560x1440)=(Width=2560,Height=1440)
	ScreenResolutions(SR_2560x1600)=(Width=2560,Height=1600)
	ScreenResolutions(SR_Other)=(Width=1280,Height=800)
}
