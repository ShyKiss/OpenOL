/**
 * Copyright 1998-2012 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the data used in reading/writing online profile settings.
 * Online profile settings are stored by an external service.
 */
class OnlineProfileSettings extends OnlinePlayerStorage
	native;

/**
 * Enum of profile setting IDs
 */
enum EProfileSettingID
{
	PSI_Unknown,
	// These are all read only
	PSI_ControllerVibration,
	PSI_YInversion,
	PSI_GamerCred,
	PSI_GamerRep,
	PSI_VoiceMuted,
	PSI_VoiceThruSpeakers,
	PSI_VoiceVolume,
	PSI_GamerPictureKey,
	PSI_GamerMotto,
	PSI_GamerTitlesPlayed,
	PSI_GamerAchievementsEarned,
	PSI_GameDifficulty,
	PSI_ControllerSensitivity,
	PSI_PreferredColor1,
	PSI_PreferredColor2,
	PSI_AutoAim,
	PSI_AutoCenter,
	PSI_MovementControl,
	PSI_RaceTransmission,
	PSI_RaceCameraLocation,
	PSI_RaceBrakeControl,
	PSI_RaceAcceleratorControl,
	PSI_GameCredEarned,
	PSI_GameAchievementsEarned,
	PSI_EndLiveIds,
	// Non-Live value that is used to invalidate a stored profile when the versions mismatch
	PSI_ProfileVersionNum,
	// Tracks how many times the profile has been saved
	PSI_ProfileSaveCount,
	
	//>> OUTLAST

	// Graphics options
	PSI_GammaSetting,
	PSI_TextureQuality,
	PSI_ShadowsQuality,
	PSI_EffectsQuality,
	PSI_VSync,
	PSI_Fullscreen,
	PSI_Resolution,
	
	// Input
	PSI_GamepadConfig,

	PSI_KB_MoveForward, // THIS MUST BE THE FIRST BINDING (or change accordingly in OLGame.cpp)
	PSI_KB_MoveBackward,
	PSI_KB_TurnLeft,
	PSI_KB_TurnRight,
	PSI_KB_StrafeLeft,
	PSI_KB_StrafeRight,
	PSI_KB_Crouch,
	PSI_KB_Use,
	PSI_KB_Run,
	PSI_KB_ToggleCamcorder,
	PSI_KB_ToggleNightVision,
	PSI_KB_LeanLeft,
	PSI_KB_LeanRight,
	PSI_KB_ZoomImpulseIn,
	PSI_KB_ZoomImpulseOut,
	PSI_KB_Reload,
	PSI_KB_Jump,
	PSI_KB_ShowMenu,
	PSI_KB_ShowTabMenu,
	PSI_KB_ShowRecordingMenu,
	PSI_KB_ShowEvidenceMenu, // THIS MUST BE THE LAST BINDING (or change accordingly in OLGame.cpp)

	PSI_Volume,
	PSI_Subtitles,
	PSI_Tutorials,
	PSI_ShowCrosshair,
	PSI_Language,

	PSI_GammaInitialized,

	PSI_ToggleCrouch,

	// VERSION 15: PS4
	PSI_ShowPrompts,
	PSI_FinishedGame,

	// VERSION 16: Post-ship PC & PS4, pre-DLC
	PSI_Southpaw,

	// VERSION 17: Post-ship PC & PS4, pre-DLC
	PSI_FinishedDLC,

	PSI_MaxFPS,

	//<<
};

/**
 * Holds the list of profile settings to read from the service.
 * NOTE: Only used for a read request and populated by the subclass
 */
var array<int> ProfileSettingIds;

/**
 * These are the settings to use when no setting has been specified yet for
 * a given id. These values should be used by subclasses to fill in per game
 * default settings
 */
var array<OnlineProfileSetting> DefaultSettings;

/** Mappings for owner information */
var const array<IdToStringMapping> OwnerMappings;

/**
 * Enum of difficulty profile values stored by the online service
 * Used with Profile ID PSI_GameDifficulty
 */
enum EProfileDifficultyOptions
{
    PDO_Normal,
    PDO_Easy,
    PDO_Hard,
	// Only add to this list
};

/**
 * Enum of controller sensitivity profile values stored by the online service
 * Used with Profile ID PSI_ControllerSensitivity
 */
enum EProfileControllerSensitivityOptions
{
    PCSO_Medium,
    PCSO_Low,
    PCSO_High,
	// Only add to this list
};

/**
 * Enum of team color preferences stored by the online service
 * Used with Profile ID PSI_PreferredColor1 & PSI_PreferredColor2
 */
enum EProfilePreferredColorOptions
{
    PPCO_None,
    PPCO_Black,
    PPCO_White,
    PPCO_Yellow,
    PPCO_Orange,
    PPCO_Pink,
    PPCO_Red,
    PPCO_Purple,
    PPCO_Blue,
    PPCO_Green,
    PPCO_Brown,
    PPCO_Silver,
	// Only add to this list
};

/**
 * Enum of auto aim preferences stored by the online service
 * Used with Profile ID PSI_AutoAim
 */
enum EProfileAutoAimOptions
{
    PAAO_Off,
    PAAO_On
};

/**
 * Enum of auto center preferences stored by the online service
 * Used with Profile ID PSI_AutoCenter
 */
enum EProfileAutoCenterOptions
{
    PACO_Off,
    PACO_On
};

/**
 * Enum of movement stick preferences stored by the online service
 * Used with Profile ID PSI_MovementControl
 */
enum EProfileMovementControlOptions
{
    PMCO_L_Thumbstick,
    PMCO_R_Thumbstick
};

/**
 * Enum of player's car transmission preferences stored by the online service
 * Used with Profile ID PSI_RaceTransmission
 */
enum EProfileRaceTransmissionOptions
{
    PRTO_Auto,
    PRTO_Manual
};

/**
 * Enum of player's race camera preferences stored by the online service
 * Used with Profile ID PSI_RaceCameraLocation
 */
enum EProfileRaceCameraLocationOptions
{
    PRCLO_Behind,
    PRCLO_Front,
    PRCLO_Inside
};

/**
 * Enum of player's race brake control preferences stored by the online service
 * Used with Profile ID PSI_RaceCameraLocation
 */
enum EProfileRaceBrakeControlOptions
{
    PRBCO_Trigger,
    PRBCO_Button
};

/**
 * Enum of player's race gas control preferences stored by the online service
 * Used with Profile ID PSI_RaceAcceleratorControl
 */
enum EProfileRaceAcceleratorControlOptions
{
    PRACO_Trigger,
    PRACO_Button
};

/**
 * Enum of player's Y axis invert preferences stored by the online service
 * Used with Profile ID PSI_YInversion
 */
enum EProfileYInversionOptions
{
    PYIO_Off,
    PYIO_On
};


/**
* Enum of player's X axis invert preferences stored by the online service
* Used with Profile ID PSI_YInversion
*/
enum EProfileXInversionOptions
{
	PXIO_Off,
	PXIO_On
};

/**
* Enum of player's omnidirectional evade preferences stored by the online service
* Used with Profile ID OmniDirEvade
*/
enum EProfileOmniDirEvadeOptions
{
	PODI_Off,
	PODI_On
};


/**
 * Enum of player's vibration preferences stored by the online service
 * Used with Profile ID PSI_ControllerVibration
 */
enum EProfileControllerVibrationToggleOptions
{
    PCVTO_Off,
	PCVTO_IgnoreThis,
	PCVTO_IgnoreThis2,
    PCVTO_On
};

/**
 * Enum of player's voice through speakers preference stored by the online service
 * Used with Profile ID PSI_VoiceThruSpeakers
 */
enum EProfileVoiceThruSpeakersOptions
{
    PVTSO_Off,
    PVTSO_On,
    PVTSO_Both
};

//>> OUTLAST

enum EProfileSettingBoolean
{
	PSB_Off,
	PSB_On,
};

//<<

/**
 * Searches for the profile setting by id and gets the default value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param DefaultId the out value of the default id
 * @param ListIndex the out value of the index where that value lies in the ValueMappings list
 *
 * @return true if the profile setting was found and retrieved the default id, false otherwise
 */
native function bool GetProfileSettingDefaultId(int ProfileSettingId,out int DefaultId, out int ListIndex);

/**
 * Searches for the profile setting by id and gets the default value int
 *
 * @param ProfileSettingId the id of the profile setting to return the default of
 * @param Value the out value of the default setting
 *
 * @return true if the profile setting was found and retrieved the default int, false otherwise
 */
native function bool GetProfileSettingDefaultInt(int ProfileSettingId,out int DefaultInt);

/**
 * Searches for the profile setting by id and gets the default value float
 *
 * @param ProfileSettingId the id of the profile setting to return the default of
 * @param Value the out value of the default setting
 *
 * @return true if the profile setting was found and retrieved the default float, false otherwise
 */
native function bool GetProfileSettingDefaultFloat(int ProfileSettingId,out float DefaultFloat);

/**
 * Sets all of the profile settings to their default values
 */
native event SetToDefaults();

/**
 * Adds the version id to the read ids if it is not present
 */
native function AppendVersionToReadIds();

/**
 * Hooks to allow child classes to dynamically adjust available profile settings or mappings based on e.g. ini values.
 */
event ModifyAvailableProfileSettings();

defaultproperties
{
	// This must be set by subclasses
	VersionNumber=-1
	VersionSettingsId=PSI_ProfileVersionNum;
	// UI readable versions of the owners
	OwnerMappings(0)=(Id=OPPO_None)
	OwnerMappings(1)=(Id=OPPO_OnlineService)
	OwnerMappings(2)=(Id=OPPO_Game)
	// Meta data for displaying in the UI
	ProfileMappings(0)=(Id=PSI_ControllerVibration,Name="Controller Vibration",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCVTO_On),(Id=PCVTO_Off)))
	ProfileMappings(1)=(Id=PSI_YInversion,Name="Invert Y",MappingType=PVMT_IdMapped,ValueMappings=((Id=PYIO_Off),(Id=PYIO_On)))
	ProfileMappings(2)=(Id=PSI_VoiceMuted,Name="Mute Voice",MappingType=PVMT_IdMapped,ValueMappings=((Id=0),(Id=1)))
	ProfileMappings(3)=(Id=PSI_VoiceThruSpeakers,Name="Voice Via Speakers",MappingType=PVMT_IdMapped,ValueMappings=((Id=PVTSO_Off),(Id=PVTSO_On),(Id=PVTSO_Both)))
	ProfileMappings(4)=(Id=PSI_VoiceVolume,Name="Voice Volume",MappingType=PVMT_RawValue)
	ProfileMappings(5)=(Id=PSI_GameDifficulty,Name="Difficulty Level",MappingType=PVMT_IdMapped,ValueMappings=((Id=PDO_Normal),(Id=PDO_Easy),(Id=PDO_Hard)))
	ProfileMappings(6)=(Id=PSI_ControllerSensitivity,Name="Controller Sensitivity",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCSO_Medium),(Id=PCSO_Low),(Id=PCSO_High)))
	ProfileMappings(7)=(Id=PSI_PreferredColor1,Name="First Preferred Color",MappingType=PVMT_IdMapped,ValueMappings=((Id=PPCO_None),(Id=PPCO_Black),(Id=PPCO_White),(Id=PPCO_Yellow),(Id=PPCO_Orange),(Id=PPCO_Pink),(Id=PPCO_Red),(Id=PPCO_Purple),(Id=PPCO_Blue),(Id=PPCO_Green),(Id=PPCO_Brown),(Id=PPCO_Silver)))
	ProfileMappings(8)=(Id=PSI_PreferredColor2,Name="Second Preferred Color",MappingType=PVMT_IdMapped,ValueMappings=((Id=PPCO_None),(Id=PPCO_Black),(Id=PPCO_White),(Id=PPCO_Yellow),(Id=PPCO_Orange),(Id=PPCO_Pink),(Id=PPCO_Red),(Id=PPCO_Purple),(Id=PPCO_Blue),(Id=PPCO_Green),(Id=PPCO_Brown),(Id=PPCO_Silver)))
	ProfileMappings(9)=(Id=PSI_AutoAim,Name="Auto Aim",MappingType=PVMT_IdMapped,ValueMappings=((Id=PAAO_Off),(Id=PAAO_On)))
	ProfileMappings(10)=(Id=PSI_AutoCenter,Name="Auto Center",MappingType=PVMT_IdMapped,ValueMappings=((Id=PACO_Off),(Id=PACO_On)))
	ProfileMappings(11)=(Id=PSI_MovementControl,Name="Movement Control",MappingType=PVMT_IdMapped,ValueMappings=((Id=PMCO_L_Thumbstick),(Id=PMCO_R_Thumbstick)))
	ProfileMappings(12)=(Id=PSI_RaceTransmission,Name="Transmission Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRTO_Auto),(Id=PRTO_Manual)))
	ProfileMappings(13)=(Id=PSI_RaceCameraLocation,Name="Race Camera Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRCLO_Behind),(Id=PRCLO_Front),(Id=PRCLO_Inside)))
	ProfileMappings(14)=(Id=PSI_RaceBrakeControl,Name="Brake Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRBCO_Trigger),(Id=PRBCO_Button)))
	ProfileMappings(15)=(Id=PSI_RaceAcceleratorControl,Name="Accelerator Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRACO_Trigger),(Id=PRACO_Button)))
}
