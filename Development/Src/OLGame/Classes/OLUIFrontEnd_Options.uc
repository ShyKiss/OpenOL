class OLUIFrontEnd_Options extends OLUIFrontEnd_Screen;

/** A cached reference to the profile of the player that owns this menu.  All affected settings will be changed here */
var transient OLProfileSettings   MyProfile;

/** Cached owner so that signin changes don't wipe a profile */
var UniqueNetId OwningId;

/** Ref to the online subsystem */
var OnlineSubsystem OnlineSub;

var localized string MouseSettingsText;
var localized string MouseInvertYText;
var localized string MouseSensitivityText;

var localized string ApplyText;
var localized string GammaText;
var localized string ResetText;

var localized string GameplayText;
var localized string GraphicsText;
var localized string ControlsText;

var localized string ConfirmResolutionTitleText;
var localized string ConfirmResolutionMessageText;
var localized string ConfirmChangesTitleText;
var localized string ConfirmChangesMessageText;
var localized string KeyBindingConflictTitleText;
var localized string KeyBindingConflictMessageText;
var localized string MustRestartTitleText;
var localized string MustRestartMessageText;
var localized string OKText;
var localized string CancelText;

var localized array<string> DifficultyOptions;

/** These CLIK Widgets are setup in the WidgetInitialized() callback and bound in default properties. */
var GFxClikWidget ApplyButton, BackButton, ResetButton;

var transient GFxClikWidget TabButtons;

var transient GFxObject GameplayList;
var transient GFxObject GraphicsList;
var transient GFxObject ControlsList;

var int PreviousResolutionSetting;
var int PreviousFullscreenSetting;
var bool bWaitingForPopup;
var transient bool bSwitchingFromJpn;

// SPECIAL CASE: The list of resolutions has unsupported values which should be hidden in the UI
var array<name> OriginalResolutionValueNames;
var array<name> DisplayedResolutionValueNames;

enum EOptionSelectorType
{
	OST_CheckBox,
	OST_Dropdown,
	OST_Slider,
	OST_KeyBinding,
	OST_ControllerConfigButton,
	OST_GammaButton,
};

enum ENonProfileOption
{
	NPO_None,
	NPO_DisableMotionBlur, // PC only, saved in config
	NPO_Difficulty, // Saved in savegame
	NPO_SmoothCamera, // PC only, saved in config
	NPO_GrainEffect, // PC only, runtime via OLFXManager
};

struct OptionInfo
{
	var bool				bInProfile;

	var int					ProfileSettingId;
	var ENonProfileOption	NonProfileId;

	var localized string	SettingDescription;
	var localized string	SettingTooltip;
	var EOptionSelectorType	Type;
	var bool                bUsesRawValue;
	var float               Slider_Minimum;
	var float               Slider_Maximum;

	var transient int		CurrentValueInt;
	var transient float		CurrentValueFloat;
	var transient string	CurrentValueString;

	structdefaultproperties
	{
		bInProfile=TRUE
	}
};

var array<OptionInfo> GeneralOptionsWithDifficulty;
var array<OptionInfo> GeneralOptionsNoDifficulty;

var array<OptionInfo> GraphicsOptions;
var array<OptionInfo> ControlsOptions;

// Keep this up to date with the tab order
var enum EOptionTabs
{
	OT_Gameplay,
	OT_Graphics,
	OT_Controls,
} CurrentTab;

/** Configures the view when it is first loaded. */
function OnViewLoaded()
{
	local OLEngine TheEngine;

	Super.OnViewLoaded();

	OwningId = GetOLPC().PlayerReplicationInfo.UniqueId;
	MyProfile = GetOLProfile();
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();

	TheEngine = OLEngine(class'Engine'.static.GetEngine());
	if (TheEngine != None && MyProfile != None)
	{
		TheEngine.UpdateProfileFromSystemSettings(MyProfile); // update properties such as fullscreen, vsync or resolution that don't really belong in profiles
	}

	SetFunction("OnConfirmChanges", self, nameof(OnConfirmChanges));
	SetFunction("OnConfirmResolution", self, nameof(OnConfirmResolution));
	SetFunction("OnDismissKeyBindingConflictDialog", self, nameof(OnDismissKeyBindingConflictDialog));
	SetFunction("Press_OptionItemButton", self, nameof(Press_OptionItemButton));
	SetFunction("OnSliderChanged", self, nameof(OnSliderChanged));
	SetFunction("OnMustRestartAccepted", self, nameof(OnMustRestartAccepted));

	// Filter out unsupported resolution names for resolution dropdown
	MyProfile.GetProfileSettingValues(PSI_Resolution, OriginalResolutionValueNames);
	DisplayedResolutionValueNames = RemoveUnsupportedResolutionsFromList(OriginalResolutionValueNames);
}

function OnViewActivated()
{
	FillOptionValuesFromProfile();

	if (TabButtons != None)
	{
		if (GameplayList != None)
		{
			PopulateGeneralOptions();
		}

		if (GraphicsList != None)
		{
			PopulateGraphicsOptions();
		}

		if (ControlsList != None)
		{
			PopulateControlsOptions();
		}

		if (CurrentTab != OT_Gameplay)
		{
			TabButtons.SetFloat("selectedIndex", 0);
		}
	}

	bSwitchingFromJpn = false;
}

function bool Back()
{
	local bool bAnyPropertyChanged;

	if (!bWaitingForPopup)
	{
		bAnyPropertyChanged = HasAnyPropertyChanged();
		if (bAnyPropertyChanged)
		{
			bWaitingForPopup = true;
			ShowChangeConfirmationDialog(
				ConfirmChangesTitleText,
				ConfirmChangesMessageText,
				OKText,
				CancelText,
				"OnConfirmChanges");
		}
		else
		{
			ExitOptionsScreen();
		}
	}
	return false;
}

function ExitOptionsScreen()
{
	local float Gamma, Volume;

	// Reset Gamma and Volume to actual settings (these can change while in the Options screen for previewing your changes)
	MyProfile.GetProfileSettingValueFloat(PSI_GammaSetting, Gamma);
	GetOLPC().SetGamma(Gamma);
	MyProfile.GetProfileSettingValueFloat(PSI_Volume, Volume);
	GetOLPC().SetVolume(Volume);

	// Reset menu keys in case they have changed
	MenuManager.OnKeyBindingsChanged();

	if (bSwitchingFromJpn)
	{
		bWaitingForPopup = true;
		ShowMessageDialog(
			MustRestartTitleText,
			MustRestartMessageText,
			OKText,
			"OnMustRestartAccepted");
	}
	else
	{
		super.Back();
	}
}

function OnMustRestartAccepted()
{
	super.Back();
}

function Press_Apply(GFxClikWidget.EventData ev)
{
	local bool bHasResolutionChanged;
	local array<string> KeyBindingConflicts;
	local array<string> KeyBindingConflictsLocalized;
	local string KeyBindingConflictsString;
	local int i;

	KeyBindingConflicts = GetKeyBindingConflicts();
	if (KeyBindingConflicts.length > 0)
	{
		for (i = 0; i < KeyBindingConflicts.length; ++i)
		{
			KeyBindingConflictsLocalized.AddItem(Localize("InputKeys", KeyBindingConflicts[i], "OLGame"));
		}
		JoinArray(KeyBindingConflictsLocalized, KeyBindingConflictsString, ", ");

		bWaitingForPopup = true;
		ShowKeyBindingConflictDialog(
			KeyBindingConflictTitleText,
			KeyBindingConflictMessageText @ KeyBindingConflictsString,
			OKText,
			CancelText,
			"OnDismissKeyBindingConflictDialog");
	}
	else
	{
		bHasResolutionChanged = SaveSettingsToProfile();
		if (bHasResolutionChanged)
		{
			bWaitingForPopup = true;
			ShowResolutionConfirmationDialogAfterDelay(
				ConfirmResolutionTitleText,
				ConfirmResolutionMessageText,
				OKText,
				CancelText,
				"OnConfirmResolution");
		}
		else
		{
			ExitOptionsScreen();
		}
	}
}

function OnDismissKeyBindingConflictDialog(bool bOK)
{
	bWaitingForPopup = false;
}

function OnConfirmResolution(bool bOK)
{
	bWaitingForPopup = false;

	if (!bOK)
	{
		RevertToPreviousResolution();
		GetCurrentGFxList().SetBool("focused", true);
	}
	else
	{
		ExitOptionsScreen();
	}
}

function RevertToPreviousResolution()
{
	// Reset options in UI
	GraphicsOptions[GetResolutionOptionIndex()].CurrentValueInt = GetDisplayedResolutionIndexFromOriginalIndex(PreviousResolutionSetting);
	GraphicsOptions[GetFullscreenOptionIndex()].CurrentValueInt = PreviousFullscreenSetting;
	PopulateGraphicsOptions();

	// Re-save to profile
	SaveSettingsToProfile();
}

function OnConfirmChanges(bool bOK)
{
	local GFxClikWidget.EventData ev;

	bWaitingForPopup = false;

	if (!bOK)
	{
		ExitOptionsScreen();
	}
	else
	{
		Press_Apply(ev);
	}
}

function Press_Back(GFxClikWidget.EventData ev)
{
	Back();
}

function Press_Gamma(GFxClikWidget.EventData ev)
{
	MenuManager.PushViewByName('GammaScreen');
}

function Press_Reset(GFxClikWidget.EventData ev)
{
	local OLEngine TheEngine;
	local int GammaIndex;
	local float GammaValue;

	if (CurrentTab == OT_Gameplay)
	{
		if (UseGeneralOptionsWithDifficulty())
		{
			SetDefaultOptionValuesForList(GeneralOptionsWithDifficulty);
			MyProfile.SetLanguageFromSteam();

			FillOptionValuesForList(GeneralOptionsWithDifficulty); 
		}
		else
		{
			SetDefaultOptionValuesForList(GeneralOptionsNoDifficulty);
			MyProfile.SetLanguageFromSteam();

			FillOptionValuesForList(GeneralOptionsNoDifficulty); 
		}	
		
		PopulateGeneralOptions();
	}
	else if (CurrentTab == OT_Graphics)
	{
		SetDefaultOptionValuesForList(GraphicsOptions); // get all the defaults
		MyProfile.AutoDetectPerformanceSettings(); // check the performance settings
		
		// Reset Gamma
		GammaIndex = GraphicsOptions.Find('ProfileSettingId', PSI_GammaSetting);
		GammaValue = GraphicsOptions[GammaIndex].CurrentValueFloat;
		GetOLPC().SetGamma(GammaValue);
		MyProfile.SetProfileSettingValueFloat(PSI_GammaSetting, GammaValue); // rcharpentier - not sure why it's needed (the data binding doesn't work for gamma?)

		FillOptionValuesForList(GraphicsOptions); // update with the new profile
		PopulateGraphicsOptions();
	}
	else if (CurrentTab == OT_Controls)
	{
		MyProfile.ResetKeysToDefault();

		TheEngine = OLEngine(class'Engine'.static.GetEngine());
		if (TheEngine != None && MyProfile != None)
		{
			TheEngine.UpdateProfileKeyBindingsFromSystem(MyProfile);
		}

		FillOptionValuesForList(ControlsOptions);
		PopulateControlsOptions();
	}

	SaveProfile();
}

function Press_OptionItemButton(int PSID)
{
	if (PSID == PSI_GamepadConfig)
	{
		MenuManager.PushViewByName('GamepadScreen');
	}
	else if (PSID == PSI_GammaSetting)
	{
		MenuManager.PushViewByName('GammaScreen');
	}
}

function bool UseGeneralOptionsWithDifficulty()
{
	local OLGame TheGame;
	TheGame = OLGame(class'WorldInfo'.static.GetWorldInfo().Game);

	if (TheGame != None && (GetOLPC().HUD.IsMainMenuOpen() || TheGame.DifficultyMode == EDM_Insane))
	{
		// don't show the difficulty option
		return false;
	}

	// show the difficulty option
	return true;
}

function PopulateTabButtons()
{
	local GFxObject Obj, DataProvider;

	DataProvider = CreateArray();

	Obj = CreateObject("Object");
	Obj.SetString("label", GameplayText);
	Obj.SetString("data", "GameplayView");
	DataProvider.SetElementObject(0, Obj);

	Obj = CreateObject("Object");
	Obj.SetString("label", GraphicsText);
	Obj.SetString("data", "GraphicsView");
	DataProvider.SetElementObject(1, Obj);

	Obj = CreateObject("Object");
	Obj.SetString("label", ControlsText);
	Obj.SetString("data", "ControlsView");
	DataProvider.SetElementObject(2, Obj);

	TabButtons.SetObject("dataProvider", DataProvider);
	TabButtons.SetFloat("selectedIndex", 0);

	CurrentTab = OT_Gameplay;
}

function GFxObject GetObjectFromOption(OptionInfo CurrentOptionInfo)
{
	local int j, CurrentOptionPSID;
	local GFxObject Obj, OptionListProvider;

	local array<name> OptionValueNames;
	local array<string> OptionValueStrings;
	local name OptVal;

	CurrentOptionPSID = CurrentOptionInfo.ProfileSettingId;

	Obj = CreateObject("Object");

	Obj.SetString("label", CurrentOptionInfo.SettingDescription);
	Obj.SetFloat("ProfileSettingID", CurrentOptionInfo.ProfileSettingId);
	Obj.SetFloat("OptionType", CurrentOptionInfo.Type);

	switch(CurrentOptionInfo.Type)
	{
	case OST_CheckBox:
		//MyProfile.GetProfileSettingValueId(CurrentOptionPSID, Option_ValueId);
		Obj.SetBool("CheckboxSelected", CurrentOptionInfo.CurrentValueInt == 1 ? true : false);
		break;
	case OST_Dropdown:
		OptionValueNames.length = 0;
		OptionValueStrings.length = 0;

		if (!CurrentOptionInfo.bInProfile && CurrentOptionInfo.NonProfileId == NPO_Difficulty)
		{
			OptionValueStrings = DifficultyOptions;
		}
		else
		{
			if( !CurrentOptionInfo.bUsesRawValue )
			{
				// Normal Case:  Pull the potential values from the profile
				OptionValueNames.Length = 0;
				MyProfile.GetProfileSettingValues(CurrentOptionPSID, OptionValueNames);
			}
			else
			{
				// Not Implemented Yet
			}

			// SPECIAL CASE: The list of resolutions has unsupported values which should be hidden in the UI
			if (CurrentOptionPSID == PSI_Resolution)
			{
				OptionValueNames = DisplayedResolutionValueNames;
			}

			foreach OptionValueNames(OptVal)
			{
				OptionValueStrings.AddItem(string(OptVal));
			}
		}

		// Create the data provider that feeds the list of choices for this option
		OptionListProvider = CreateArray();
		for( j = 0; j < OptionValueStrings.length; j++ )
		{
			OptionListProvider.SetElementString(j, class'UIRoot'.static.SafeCaps(OptionValueStrings[j]));
		}
		Obj.SetObject("DropdownList", OptionListProvider);
		Obj.SetFloat("DropdownIndex", CurrentOptionInfo.CurrentValueInt);
		break;
	case OST_Slider:
		//MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, Slider_Value);

		Obj.SetFloat("SliderMin",  CurrentOptionInfo.Slider_Minimum);
		Obj.SetFloat("SliderMax",  CurrentOptionInfo.Slider_Maximum);
		//Obj.SetFloat("SliderIncrement",CurrentOptionInfo.Slider_Increment);
		Obj.SetFloat("SliderValue",    CurrentOptionInfo.CurrentValueFloat);
		break;
	case OST_KeyBinding:
		Obj.SetString("KeyBindingValue", CurrentOptionInfo.CurrentValueString);
		break;
	case OST_ControllerConfigButton:
		OptionValueNames.Length = 0;

		if( !CurrentOptionInfo.bUsesRawValue )
		{
			// Normal Case:  Pull the potential values from the profile
			OptionValueNames.Length = 0;
			MyProfile.GetProfileSettingValues(CurrentOptionPSID, OptionValueNames);
		}
		else
		{
			// Not Implemented Yet
		}

		// Create the data provider that feeds the list of choices for this option
		OptionListProvider = CreateArray();
		for( j = 0; j < OptionValueNames.length; j++ )
		{
			OptionListProvider.SetElementString(j, class'UIRoot'.static.SafeCaps(string(OptionValueNames[j])));
		}
		Obj.SetObject("ButtonLabelsList", OptionListProvider);
		Obj.SetFloat("ButtonLabelIndex", CurrentOptionInfo.CurrentValueInt);
		break;
	case OST_GammaButton:
		Obj.SetFloat("ButtonValue", CurrentOptionInfo.CurrentValueFloat);
		break;
	}

	return Obj;
}

function PopulateGeneralOptions()
{
	local int i;
	local GFxObject Obj, DataProvider;
	
	DataProvider = CreateArray();

	if (UseGeneralOptionsWithDifficulty())
	{
		for( i = 0; i < GeneralOptionsWithDifficulty.length; i++ )
		{
			Obj = GetObjectFromOption(GeneralOptionsWithDifficulty[i]);
			DataProvider.SetElementObject(i, Obj);
		}
	}
	else
	{
		for( i = 0; i < GeneralOptionsNoDifficulty.length; i++ )
		{
			Obj = GetObjectFromOption(GeneralOptionsNoDifficulty[i]);
			DataProvider.SetElementObject(i, Obj);
		}
	}

	GameplayList.SetObject("dataProvider", DataProvider);
	GameplayList.SetFloat("selectedIndex", 0);
}

function PopulateGraphicsOptions()
{
	local int i;
	local GFxObject Obj, DataProvider;

	DataProvider = CreateArray();
	for( i = 0; i < GraphicsOptions.length; i++ )
	{
		Obj = GetObjectFromOption(GraphicsOptions[i]);
		DataProvider.SetElementObject(i, Obj);
	}

	if (GraphicsList != None)
	{
		GraphicsList.SetObject("dataProvider", DataProvider);
		GraphicsList.SetFloat("selectedIndex", 0);
	}
}

function PopulateControlsOptions()
{
	local int i;
	local GFxObject Obj, DataProvider;

	DataProvider = CreateArray();
	for( i = 0; i < ControlsOptions.length; i++ )
	{
		Obj = GetObjectFromOption(ControlsOptions[i]);
		DataProvider.SetElementObject(i, Obj);
	}

	ControlsList.SetObject("dataProvider", DataProvider);
	ControlsList.SetFloat("selectedIndex", 0);
}

function FillOptionValuesFromProfile()
{
	if (UseGeneralOptionsWithDifficulty())
	{		
		FillOptionValuesForList(GeneralOptionsWithDifficulty);
	}
	else
	{
		FillOptionValuesForList(GeneralOptionsNoDifficulty);
	}

	FillOptionValuesForList(GraphicsOptions);
	FillOptionValuesForList(ControlsOptions);
}

function FillOptionValuesForList(out array<OptionInfo> OptionInfos)
{
	local OLEngine TheEngine;
	local OLFXManager FXMgr;
	local int i, CurrentOptionPSID, CurrentOptionValueInt;
	local float CurrentOptionValueFloat;
	local string CurrentOptionValueString;

	for ( i = 0; i < OptionInfos.length; i++)
	{
		if (OptionInfos[i].bInProfile)
		{
			CurrentOptionPSID = OptionInfos[i].ProfileSettingId;

			switch(OptionInfos[i].Type)
			{
			case OST_CheckBox:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, CurrentOptionValueInt);
				OptionInfos[i].CurrentValueInt = CurrentOptionValueInt;
				break;
			case OST_Dropdown:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, CurrentOptionValueInt);

				// SPECIAL CASE: The list of resolutions has unsupported values which should be hidden in the UI
				if (CurrentOptionPSID == PSI_Resolution) {
					CurrentOptionValueInt = GetDisplayedResolutionIndexFromOriginalIndex(CurrentOptionValueInt);
				}

				OptionInfos[i].CurrentValueInt = CurrentOptionValueInt;
				break;
			case OST_Slider:
				MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, CurrentOptionValueFloat);
				OptionInfos[i].CurrentValueFloat = CurrentOptionValueFloat;
				break;
			case OST_KeyBinding:
				MyProfile.GetProfileSettingValueString(CurrentOptionPSID, CurrentOptionValueString);
				OptionInfos[i].CurrentValueString = CurrentOptionValueString;
				break;
			case OST_ControllerConfigButton:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, CurrentOptionValueInt);
				OptionInfos[i].CurrentValueInt = CurrentOptionValueInt;
				break;
			case OST_GammaButton:
				MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, CurrentOptionValueFloat);
				OptionInfos[i].CurrentValueFloat = CurrentOptionValueFloat;
				break;
			}
		}
		else
		{
			switch(OptionInfos[i].NonProfileId)
			{
			case NPO_DisableMotionBlur:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					OptionInfos[i].CurrentValueInt = TheEngine.bDisableMotionBlur ? 1 : 0;
				}
				break;
			case NPO_SmoothCamera:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					OptionInfos[i].CurrentValueInt = TheEngine.bSmoothCamera ? 1 : 0;
				}
				break;
			case NPO_GrainEffect:
				FXMgr = class'OLFXManager'.static.GetFXManager();
				OptionInfos[i].CurrentValueInt = (FXMgr != None && !FXMgr.bGrainDisabled) ? 1 : 0;
				break;
			case NPO_Difficulty:
				if (GetGame() != None)
				{
					OptionInfos[i].CurrentValueInt = GetGame().DifficultyMode;
				}
				else
				{
					OptionInfos[i].CurrentValueInt = 0;
				}
				break;
			}
		}
	}
}

function SetDefaultOptionValuesForList(out array<OptionInfo> OptionInfos)
{
	local OLEngine TheEngine;
	local OLFXManager FXMgr;
	local int i, CurrentOptionPSID, CurrentOptionValueInt, ListIndex;
	local float CurrentOptionValueFloat;

	for ( i = 0; i < OptionInfos.length; i++)
	{
		if (OptionInfos[i].bInProfile)
		{
			CurrentOptionPSID = OptionInfos[i].ProfileSettingId;

			switch(OptionInfos[i].Type)
			{
			case OST_CheckBox:
				MyProfile.GetProfileSettingDefaultId(CurrentOptionPSID, CurrentOptionValueInt, ListIndex);
				MyProfile.SetProfileSettingValueId(CurrentOptionPSID, CurrentOptionValueInt);
				break;
			case OST_Dropdown:
				MyProfile.GetProfileSettingDefaultId(CurrentOptionPSID, CurrentOptionValueInt, ListIndex);
				MyProfile.SetProfileSettingValueId(CurrentOptionPSID, CurrentOptionValueInt);

				break;
			case OST_Slider:
				MyProfile.GetProfileSettingDefaultFloat(CurrentOptionPSID, CurrentOptionValueFloat);
				MyProfile.SetProfileSettingValueFloat(CurrentOptionPSID, CurrentOptionValueFloat);
				break;
			case OST_KeyBinding:
				// Keys Bindings are handled differently
				break;
			case OST_ControllerConfigButton:
				MyProfile.GetProfileSettingDefaultId(CurrentOptionPSID, CurrentOptionValueInt, ListIndex);
				MyProfile.SetProfileSettingValueId(CurrentOptionPSID, CurrentOptionValueInt);
				break;
			case OST_GammaButton:
				MyProfile.GetProfileSettingDefaultFloat(CurrentOptionPSID, CurrentOptionValueFloat);
				MyProfile.SetProfileSettingValueFloat(CurrentOptionPSID, CurrentOptionValueFloat);
				break;
			}
		}
		else
		{
			switch(OptionInfos[i].NonProfileId)
			{
			case NPO_DisableMotionBlur:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					TheEngine.bDisableMotionBlur = false;
					TheEngine.SaveConfig();
				}
				break;
			case NPO_SmoothCamera:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					TheEngine.SetSmoothCamera(false);
				}
				break;
			case NPO_GrainEffect:
				FXMgr = class'OLFXManager'.static.GetFXManager();
				if (FXMgr != None)
					FXMgr.bGrainDisabled = false;
				break;
			case NPO_Difficulty:
				if (GetGame().DifficultyMode != EDM_Insane)
				{
					GetGame().DifficultyMode = EDM_Normal;
				}
				break;
			}
		}
	}
}

function float GetOptionValueAt(GFxObject OptionsList, int Index)
{
	local array<ASValue> args;
	local ASValue RetVal;

	args.length = 1;

	args[0].type = AS_Number;
	args[0].n = Index;

	RetVal = OptionsList.Invoke("GetSelectionValueAt", args);

	return RetVal.n;
}

function string GetOptionValueStringAt(GFxObject OptionsList, int Index)
{
	local array<ASValue> args;
	local ASValue RetVal;

	args.length = 1;

	args[0].type = AS_Number;
	args[0].n = Index;

	RetVal = OptionsList.Invoke("GetSelectionValueStringAt", args);

	return RetVal.s;
}

function StoreOptionValuesForList(GFxObject OptionsList, out array<OptionInfo> OptionInfos)
{
	local int i;
	local float FloatValue;
	local string StringValue;

	for (i = 0; i < OptionInfos.length; i++)
	{
		switch(OptionInfos[i].Type)
		{
		case OST_CheckBox:
		case OST_Dropdown:
			FloatValue = GetOptionValueAt(OptionsList, i);
			OptionInfos[i].CurrentValueInt = FloatValue;
			break;
		case OST_Slider:
			FloatValue = GetOptionValueAt(OptionsList, i);
			OptionInfos[i].CurrentValueFloat = FloatValue;
			break;
		case OST_KeyBinding:
			StringValue = GetOptionValueStringAt(OptionsList, i);
			OptionInfos[i].CurrentValueString = StringValue;
			break;
		case OST_ControllerConfigButton:
			FloatValue = GetOptionValueAt(OptionsList, i);
			OptionInfos[i].CurrentValueInt = FloatValue;
			break;
		case OST_GammaButton:
			FloatValue = GetOptionValueAt(OptionsList, i);
			OptionInfos[i].CurrentValueFloat = FloatValue;
			break;
		}
	}
}

function TabChanged(GFxClikWidget.EventData ev)
{	
	if (CurrentTab == OT_Gameplay)
	{
		if (UseGeneralOptionsWithDifficulty())
		{
			StoreOptionValuesForList(GameplayList, GeneralOptionsWithDifficulty);
		}
		else
		{
			StoreOptionValuesForList(GameplayList, GeneralOptionsNoDifficulty);
		}
	}
	else if (CurrentTab == OT_Graphics)
	{
		StoreOptionValuesForList(GraphicsList, GraphicsOptions);
	}
	else if (CurrentTab == OT_Controls)
	{
		StoreOptionValuesForList(ControlsList, ControlsOptions);
	}

	CurrentTab = EOptionTabs(ev.index);

	//if (CurrentTab == OT_Gameplay && GameplayList != None)
	//{
	//	PopulateGeneralOptions();
	//}
	//else if (CurrentTab == OT_Graphics && GraphicsList != None)
	//{
	//	PopulateGraphicsOptions();
	//}
}

function bool SaveSettingsForList(array<OptionInfo> OptionInfos)
{
	local OLEngine TheEngine;
	local OLFXManager FXMgr;
	local int i, CurrentOptionPSID;
	local OptionInfo CurrentOptionInfo;
	local float OptionProfileValue_Float;
	local int OptionProfileValue_Int, OptionProfileValue_Idx, ConvertedIdx;
	local string OptionProfileValue_String;
	local bool bPropertyChanged, OptionOldValue_Bool;
	local int CurrentDropdownValue;
	local EDifficultyMode OptionOldValue_Difficulty;

	bPropertyChanged = false;

	for (i = 0; i < OptionInfos.length; i++)
	{
		CurrentOptionInfo = OptionInfos[i];

		if (CurrentOptionInfo.bInProfile)
		{
			CurrentOptionPSID = CurrentOptionInfo.ProfileSettingId;

			switch(CurrentOptionInfo.Type)
			{
			case OST_CheckBox:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, OptionProfileValue_Int);
				if( OptionProfileValue_Int != CurrentOptionInfo.CurrentValueInt)
				{
					MyProfile.SetProfileSettingValueId(CurrentOptionPSID, CurrentOptionInfo.CurrentValueInt);
					bPropertyChanged = true;
				}
				break;
			case OST_Dropdown:
				// SPECIAL CASE: The list of resolutions has unsupported values which should be hidden in the UI
				if (CurrentOptionPSID == PSI_Resolution) 
				{
					CurrentDropdownValue = GetOriginalResolutionIndexFromDisplayedIndex(CurrentOptionInfo.CurrentValueInt);
				} 
				else 
				{
					CurrentDropdownValue = CurrentOptionInfo.CurrentValueInt;
				}
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, OptionProfileValue_Int, OptionProfileValue_Idx);
				MyProfile.GetProfileSettingValueFromListIndex(CurrentOptionPSID, CurrentDropdownValue, ConvertedIdx);

				if(  OptionProfileValue_Int != ConvertedIdx )
				{
					MyProfile.SetProfileSettingValueId(CurrentOptionPSID, ConvertedIdx);
					bPropertyChanged = true;
				}
				break;
			case OST_Slider:
				MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, OptionProfileValue_Float);
				if( OptionProfileValue_Float != CurrentOptionInfo.CurrentValueFloat)
				{
					MyProfile.SetProfileSettingValueFloat(CurrentOptionPSID, CurrentOptionInfo.CurrentValueFloat);
					bPropertyChanged = true;
				}
				break;
			case OST_KeyBinding:
				MyProfile.GetProfileSettingValueString(CurrentOptionPSID, OptionProfileValue_String);
				if( OptionProfileValue_String != CurrentOptionInfo.CurrentValueString)
				{
					MyProfile.SetProfileSettingValueString(CurrentOptionPSID, CurrentOptionInfo.CurrentValueString);
					bPropertyChanged = true;
				}
				break;
			case OST_ControllerConfigButton:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, OptionProfileValue_Int, OptionProfileValue_Idx);
				MyProfile.GetProfileSettingValueFromListIndex(CurrentOptionPSID, CurrentOptionInfo.CurrentValueInt, ConvertedIdx);
				if(  OptionProfileValue_Int != ConvertedIdx )
				{
					MyProfile.SetProfileSettingValueId(CurrentOptionPSID, ConvertedIdx);
					bPropertyChanged = true;
				}
				break;
			case OST_GammaButton:
				MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, OptionProfileValue_Float);
				if( OptionProfileValue_Float != CurrentOptionInfo.CurrentValueFloat)
				{
					MyProfile.SetProfileSettingValueFloat(CurrentOptionPSID, CurrentOptionInfo.CurrentValueFloat);
					bPropertyChanged = true;
				}
				break;
			}
		}
		else
		{
			switch(CurrentOptionInfo.NonProfileId)
			{
			case NPO_DisableMotionBlur:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					OptionOldValue_Bool = TheEngine.bDisableMotionBlur;
					TheEngine.bDisableMotionBlur = CurrentOptionInfo.CurrentValueInt != 0;
					TheEngine.SaveConfig();

					if ((OptionOldValue_Bool && !TheEngine.bDisableMotionBlur)
						||(!OptionOldValue_Bool && TheEngine.bDisableMotionBlur))
					{
						bPropertyChanged = true;
					}
				}
				break;
			case NPO_SmoothCamera:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					OptionOldValue_Bool = TheEngine.bSmoothCamera;
					TheEngine.SetSmoothCamera(CurrentOptionInfo.CurrentValueInt != 0);

					if (OptionOldValue_Bool != TheEngine.bSmoothCamera)
					{
						bPropertyChanged = true;
					}
				}
				break;
			case NPO_GrainEffect:
				FXMgr = class'OLFXManager'.static.GetFXManager();
				if (FXMgr != None)
				{
					OptionOldValue_Bool = !FXMgr.bGrainDisabled;
					FXMgr.bGrainDisabled = CurrentOptionInfo.CurrentValueInt == 0;
					// Apply immediately to the current post effect
					if (FXMgr.CurrentUberPostEffect != None)
					{
						if (FXMgr.bGrainDisabled)
							FXMgr.CurrentUberPostEffect.GrainOpacity = 0.0;
						else
							FXMgr.CurrentUberPostEffect.GrainOpacity = FXMgr.CurrentUberPostEffect.Default.GrainOpacity;
					}
					FXMgr.SaveConfig();
					if (OptionOldValue_Bool != (CurrentOptionInfo.CurrentValueInt != 0))
						bPropertyChanged = true;
				}
				break;
			case NPO_Difficulty:
				OptionOldValue_Difficulty = GetGame().DifficultyMode;
				GetGame().DifficultyMode = EDifficultyMode(CurrentOptionInfo.CurrentValueInt);

				if (OptionOldValue_Difficulty != GetGame().DifficultyMode)
				{
					GetOLPC().NotifyDifficultyChanged();					
					bPropertyChanged = true;
				}
				break;
			}
		}
	}

	return bPropertyChanged;
}

function bool HasPropertyChangedInList(GFxObject OptionsList, array<OptionInfo> OptionInfos)
{
	local OLEngine TheEngine;
	local OLFXManager FXMgr;
	local int i, CurrentOptionPSID;
	local OptionInfo CurrentOptionInfo;
	local float OptionProfileValue_Float;
	local int OptionProfileValue_Int, OptionProfileValue_Idx, ConvertedIdx;
	local string OptionProfileValue_String;
	local bool bPropertyChanged;
	local int CurrentDropdownValue;

	if (OptionsList == None) {
		return false;
	}

	bPropertyChanged = false;

	for (i = 0; i < OptionInfos.length; i++)
	{
		CurrentOptionInfo = OptionInfos[i];
		if (CurrentOptionInfo.bInProfile)
		{
			CurrentOptionPSID = CurrentOptionInfo.ProfileSettingId;

			switch(CurrentOptionInfo.Type)
			{
			case OST_CheckBox:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, OptionProfileValue_Int);
				if( OptionProfileValue_Int != CurrentOptionInfo.CurrentValueInt)
				{
					bPropertyChanged = true;
				}
				break;
			case OST_Dropdown:
				// SPECIAL CASE: The list of resolutions has unsupported values which should be hidden in the UI
				if (CurrentOptionPSID == PSI_Resolution) {
					CurrentDropdownValue = GetOriginalResolutionIndexFromDisplayedIndex(CurrentOptionInfo.CurrentValueInt);
				} else {
					CurrentDropdownValue = CurrentOptionInfo.CurrentValueInt;
				}
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, OptionProfileValue_Int, OptionProfileValue_Idx);
				MyProfile.GetProfileSettingValueFromListIndex(CurrentOptionPSID, CurrentDropdownValue, ConvertedIdx);
				if(  OptionProfileValue_Int != ConvertedIdx )
				{
					bPropertyChanged = true;					
				}
				break;
			case OST_Slider:
				MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, OptionProfileValue_Float);
				if( OptionProfileValue_Float != CurrentOptionInfo.CurrentValueFloat)
				{
					bPropertyChanged = true;
				}
				break;
			case OST_KeyBinding:
				MyProfile.GetProfileSettingValueString(CurrentOptionPSID, OptionProfileValue_String);
				if( OptionProfileValue_String != CurrentOptionInfo.CurrentValueString)
				{
					bPropertyChanged = true;
				}
				break;
			case OST_ControllerConfigButton:
				MyProfile.GetProfileSettingValueId(CurrentOptionPSID, OptionProfileValue_Int, OptionProfileValue_Idx);
				MyProfile.GetProfileSettingValueFromListIndex(CurrentOptionPSID, CurrentOptionInfo.CurrentValueInt, ConvertedIdx);
				if(  OptionProfileValue_Int != ConvertedIdx )
				{
					bPropertyChanged = true;
				}
				break;
			case OST_GammaButton:
				MyProfile.GetProfileSettingValueFloat(CurrentOptionPSID, OptionProfileValue_Float);
				if( OptionProfileValue_Float != CurrentOptionInfo.CurrentValueFloat)
				{
					bPropertyChanged = true;
				}
				break;
			}
		}
		else
		{
			switch(CurrentOptionInfo.NonProfileId)
			{
			case NPO_DisableMotionBlur:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					if ((CurrentOptionInfo.CurrentValueInt != 0 && !TheEngine.bDisableMotionBlur)
						||(CurrentOptionInfo.CurrentValueInt == 0 && TheEngine.bDisableMotionBlur))
					{
						bPropertyChanged = true;
					}
				}
				break;
			case NPO_SmoothCamera:
				TheEngine = OLEngine(class'Engine'.static.GetEngine());
				if (TheEngine != None)
				{
					if ((CurrentOptionInfo.CurrentValueInt != 0) != TheEngine.bSmoothCamera)
					{
						bPropertyChanged = true;
					}
				}
				break;
			case NPO_GrainEffect:
				FXMgr = class'OLFXManager'.static.GetFXManager();
				if (FXMgr != None)
				{
					if ((CurrentOptionInfo.CurrentValueInt != 0) == FXMgr.bGrainDisabled)
						bPropertyChanged = true;
				}
				break;
			case NPO_Difficulty:
				if (CurrentOptionInfo.CurrentValueInt != GetGame().DifficultyMode)
				{
					bPropertyChanged = true;
				}
				break;
			}
		}
	}

	return bPropertyChanged;
}

function bool HasAnyPropertyChanged()
{
	local bool bPropertyChanged;

	// Make sure cached option values are up-to-date.
	if (CurrentTab == OT_Gameplay)
	{
		if (UseGeneralOptionsWithDifficulty())
		{
			StoreOptionValuesForList(GameplayList, GeneralOptionsWithDifficulty);
		}
		else
		{
			StoreOptionValuesForList(GameplayList, GeneralOptionsNoDifficulty);
		}
	}
	else if (CurrentTab == OT_Graphics)
	{
		StoreOptionValuesForList(GraphicsList, GraphicsOptions);
	}
	else if (CurrentTab == OT_Controls)
	{
		StoreOptionValuesForList(ControlsList, ControlsOptions);
	}

	if (UseGeneralOptionsWithDifficulty())
	{
		bPropertyChanged = HasPropertyChangedInList(GameplayList, GeneralOptionsWithDifficulty);
	}
	else
	{
		bPropertyChanged = HasPropertyChangedInList(GameplayList, GeneralOptionsNoDifficulty);
	}

	bPropertyChanged = bPropertyChanged ||
						HasPropertyChangedInList(GraphicsList, GraphicsOptions) ||
						HasPropertyChangedInList(ControlsList, ControlsOptions);

	return bPropertyChanged;
}

function bool SaveSettingsToProfile()
{
	local float OptionValue, OptionProfileValue_Float;
	local bool bHasResolutionChanged;
	local bool bProfileIsDirty;
	local bool bGeneralOptionsDirty;
	local bool bGraphicsOptionsDirty;
	local bool bControlsOptionsDirty;
	local bool bWasJPN;
	local bool bIsJPN;
	local int CurrentLanguage;

	// Store settings into info arrays.
	if (CurrentTab == OT_Gameplay)
	{
		if (UseGeneralOptionsWithDifficulty())
		{
			StoreOptionValuesForList(GameplayList, GeneralOptionsWithDifficulty);
		}
		else
		{
			StoreOptionValuesForList(GameplayList, GeneralOptionsNoDifficulty);
		}
	}
	else if (CurrentTab == OT_Graphics)
	{
		StoreOptionValuesForList(GraphicsList, GraphicsOptions);
	}
	else if (CurrentTab == OT_Controls)
	{
		StoreOptionValuesForList(ControlsList, ControlsOptions);
	}

	// Remember previous resolution setting
	bHasResolutionChanged = HasResolutionChanged();
	MyProfile.GetProfileSettingValueId(PSI_Resolution, PreviousResolutionSetting);
	MyProfile.GetProfileSettingValueId(PSI_Fullscreen, PreviousFullscreenSetting);
	MyProfile.GetProfileSettingValueId(PSI_Language, CurrentLanguage);
	bWasJPN = (CurrentLanguage == EL_Japanese);

	if (UseGeneralOptionsWithDifficulty())
	{
		bGeneralOptionsDirty = SaveSettingsForList(GeneralOptionsWithDifficulty);
	}
	else
	{
		bGeneralOptionsDirty = SaveSettingsForList(GeneralOptionsNoDifficulty);
	}
	
	bGraphicsOptionsDirty = SaveSettingsForList(GraphicsOptions);
	bControlsOptionsDirty = SaveSettingsForList(ControlsOptions);
	
	bProfileIsDirty = bGeneralOptionsDirty || bGraphicsOptionsDirty || bControlsOptionsDirty;
	
	// Gamma
	OptionValue = GetOLPC().GetGamma();
	MyProfile.GetProfileSettingValueFloat(PSI_GammaSetting, OptionProfileValue_Float);
	if (OptionProfileValue_Float != OptionValue)
	{
		MyProfile.SetProfileSettingValueFloat(PSI_GammaSetting, OptionValue);
		bProfileIsDirty = TRUE;
	}

	MyProfile.GetProfileSettingValueId(PSI_Language, CurrentLanguage);
	bIsJPN = (CurrentLanguage == EL_Japanese);
	bSwitchingFromJpn = (bWasJPN && !bIsJPN);

	if( bProfileIsDirty )
	{
		// Save the profile changes so the settings persist
		SaveProfile();
	}

	return bHasResolutionChanged;
}

/** 
 *  Grabs a reference to the current player's profile
 *  
 *  @return     The current player's profile settings
 */
function OLProfileSettings GetOLProfile()
{
	return GetOLPC().ProfileSettings;
}

/** Callback when a CLIK widget with enableInitCallback set to TRUE is initialized.  Returns TRUE if the widget was handled, FALSE if not. */
event bool WidgetInitialized(name WidgetName, name WidgetPath, GFxObject Widget)
{    
	local bool bWasHandled;
	bWasHandled = TRUE;
	
    switch(WidgetName)
    {                 
        case ('applyBtn'):
			ApplyButton = GFxClikWidget(Widget);
			ApplyButton.AddEventListener('CLIK_press', Press_Apply);
			ApplyButton.SetString("label", ApplyText);
			break;
		case ('backBtn'):
			BackButton = GFxClikWidget(Widget);
			BackButton.AddEventListener('CLIK_press', Press_Back);
			BackButton.SetString("label", BackText);
			break;
		case ('resetBtn'):
			ResetButton = GFxClikWidget(Widget);
			ResetButton.AddEventListener('CLIK_press', Press_Reset);
			ResetButton.SetString("label", ResetText);
			break;
		case ('tabButtons'):
			TabButtons = GFxClikWidget(Widget);
			PopulateTabButtons();
			TabButtons.AddEventListener('CLIK_change', TabChanged);
			break;
		case ('gameplayList'):
			GameplayList = Widget;
			PopulateGeneralOptions();
			break;
		case ('graphicsList'):
			GraphicsList = Widget;
			PopulateGraphicsOptions();
			break;
		case ('controlsList'):
			ControlsList = Widget;
			PopulateControlsOptions();
			break;
		default:
			bWasHandled = FALSE;
	}

	if (!bWasHandled)
	{
		bWasHandled = Super.WidgetInitialized(WidgetName, WidgetPath, Widget);  
	}
	return bWasHandled;
}

function bool HasResolutionChanged()
{
	local int OptionProfileValue_Int, OptionProfileValue_Idx, ConvertedIdx;
	local bool bResolutionChanged, bFullscreenChanged;
	local int OriginalResolutionListIndex;

	// The displayed list of resolutions is different from the actual list in the profile
	OriginalResolutionListIndex = GetOriginalResolutionIndexFromDisplayedIndex(GraphicsOptions[GetResolutionOptionIndex()].CurrentValueInt);

	MyProfile.GetProfileSettingValueId(PSI_Resolution, OptionProfileValue_Int, OptionProfileValue_Idx);
	MyProfile.GetProfileSettingValueFromListIndex(PSI_Resolution, OriginalResolutionListIndex, ConvertedIdx);
	bResolutionChanged = (OptionProfileValue_Int != ConvertedIdx);

	MyProfile.GetProfileSettingValueId(PSI_Fullscreen, OptionProfileValue_Int, OptionProfileValue_Idx);
	MyProfile.GetProfileSettingValueFromListIndex(PSI_Fullscreen, GraphicsOptions[GetFullscreenOptionIndex()].CurrentValueInt, ConvertedIdx);
	bFullscreenChanged = (OptionProfileValue_Int != ConvertedIdx);

	return bResolutionChanged || bFullscreenChanged;
}

function int GetResolutionOptionIndex()
{
	return GraphicsOptions.Find('ProfileSettingId', PSI_Resolution);
}

function int GetFullscreenOptionIndex()
{
	return GraphicsOptions.Find('ProfileSettingId', PSI_Fullscreen);
}

function array<name> RemoveUnsupportedResolutionsFromList(array<name> OriginalList)
{
	local int i;
	local array<name> newList;

	for (i = 0; i < originalList.length; ++i)
	{
		if (Left(originalList[i], 1) != "(") {
			newList.AddItem(originalList[i]);
		}
	}
	return newList;
}

function int GetOriginalResolutionIndexFromDisplayedIndex(int DisplayedIndex)
{
	local name ResolutionName;
	local int OriginalIdx;
	ResolutionName = DisplayedResolutionValueNames[DisplayedIndex];

	OriginalIdx = OriginalResolutionValueNames.Find(ResolutionName);

	return OriginalIdx;
}

function int GetDisplayedResolutionIndexFromOriginalIndex(int OriginalIndex)
{
	local name ResolutionName;
	local int DisplayedIdx;
	ResolutionName = OriginalResolutionValueNames[OriginalIndex];
	DisplayedIdx = DisplayedResolutionValueNames.Find(ResolutionName);

	return DisplayedIdx;
}

function OnKeyBindingCaptured(name KeyName)
{
	ASOnKeyBindingCaptured(string(KeyName));
}

function int GetGamepadConfig()
{
	local int GamepadConfigOption;

	if (UseGeneralOptionsWithDifficulty())
	{
		GamepadConfigOption = GeneralOptionsWithDifficulty.Find('ProfileSettingId', PSI_GamepadConfig);
		return GeneralOptionsWithDifficulty[GamepadConfigOption].CurrentValueInt;
	}
	else
	{
		GamepadConfigOption = GeneralOptionsNoDifficulty.Find('ProfileSettingId', PSI_GamepadConfig);
		return GeneralOptionsNoDifficulty[GamepadConfigOption].CurrentValueInt;
	}	
}

function SetGamepadConfigExternally(int ConfigIndex)
{
	local int GamepadConfigOption;
	
	if (UseGeneralOptionsWithDifficulty())
	{
		GamepadConfigOption = GeneralOptionsWithDifficulty.Find('ProfileSettingId', PSI_GamepadConfig);
		GeneralOptionsWithDifficulty[GamepadConfigOption].CurrentValueInt = ConfigIndex;
	}
	else
	{
		GamepadConfigOption = GeneralOptionsNoDifficulty.Find('ProfileSettingId', PSI_GamepadConfig);
		GeneralOptionsNoDifficulty[GamepadConfigOption].CurrentValueInt = ConfigIndex;
	}
	
	PopulateGeneralOptions();
}

function SetGammaExternally(float Gamma)
{
	local int GammaOption;
	GammaOption = GraphicsOptions.Find('ProfileSettingId', PSI_GammaSetting);
	GraphicsOptions[GammaOption].CurrentValueFloat = Gamma;
	PopulateGraphicsOptions();
}

function GFxObject GetCurrentGFxList()
{
	if (CurrentTab == OT_Gameplay)
	{
		return GameplayList;
	}
	else if (CurrentTab == OT_Graphics)
	{
		return GraphicsList;
	}
	else if (CurrentTab == OT_Controls)
	{
		return ControlsList;
	}
	return None;
}

function array<string> GetKeyBindingConflicts()
{
	local array<string> KeysBound;
	local array<string> Conflicts;
	local string Key;
	local int i;

	if (CurrentTab == OT_Controls)
	{
		StoreOptionValuesForList(ControlsList, ControlsOptions);
	}

	for (i = 0; i < ControlsOptions.length; ++i)
	{
		Key = ControlsOptions[i].CurrentValueString;
		if (KeysBound.Find(Key) != -1)
		{
			if (Len(Key) > 0 && Conflicts.Find(Key) == -1)
			{
				Conflicts.AddItem(Key);
			}
		}
		else
		{
			KeysBound.AddItem(ControlsOptions[i].CurrentValueString);
		}
	}
	return Conflicts;
}

function OnSliderChanged(int ProfileSettingID, float SliderValue)
{
	if (ProfileSettingID == PSI_Volume)
	{
		// Temporarily set game volume (will be reset when you leave the menu if the settings are not applied)
		GetOLPC().SetVolume(SliderValue);

		PlaySoundFromTheme('slider');
	}
}

function float GetCurrentGammaSetting()
{
	local int GammaIndex;
	GammaIndex = GraphicsOptions.Find('ProfileSettingId', PSI_GammaSetting);
	return GraphicsOptions[GammaIndex].CurrentValueFloat;
}

// ActionScript functions
private function ShowResolutionConfirmationDialogAfterDelay(string title, string message, string okButtonLabel, string cancelButtonLabel, string callbackName) { ActionScriptVoid("showResolutionConfirmationDialogAfterDelay"); }
private function ShowChangeConfirmationDialog(string title, string message, string okButtonLabel, string cancelButtonLabel, string callbackName) { ActionScriptVoid("showChangeConfirmationDialog"); }
private function ShowKeyBindingConflictDialog(string title, string message, string okButtonLabel, string cancelButtonLabel, string callbackName) { ActionScriptVoid("showKeyBindingConflictDialog"); }
private function ShowMessageDialog(string title, string message, string okButtonLabel, string callbackName) { ActionScriptVoid("ShowMessageDialog"); }
private function ASOnKeyBindingCaptured(string keyName) { ActionScriptVoid("onKeyBindingCaptured"); }

defaultproperties
{
	SubWidgetBindings.Add((WidgetName="applyBtn",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="backBtn",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="resetBtn",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="tabButtons",WidgetClass=class'GFxClikWidget'))

	GeneralOptionsWithDifficulty(0) = (ProfileSettingId=PSI_Language,Type=OST_Dropdown)
	GeneralOptionsWithDifficulty(1) = (bInProfile=FALSE,ProfileSettingId=PSI_Unknown,NonProfileId=NPO_Difficulty,Type=OST_Dropdown)
	GeneralOptionsWithDifficulty(2) = (ProfileSettingId=PSI_Tutorials,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(3) = (ProfileSettingId=PSI_ShowCrosshair,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(4) = (ProfileSettingId=PSI_ShowPrompts,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(5) = (ProfileSettingId=PSI_YInversion,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(6) = (ProfileSettingId=PSI_Southpaw,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(7) = (ProfileSettingId=PSI_ToggleCrouch,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(8) = (bInProfile=FALSE,ProfileSettingId=PSI_Unknown,NonProfileId=NPO_SmoothCamera,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(9) = (ProfileSettingId=PSI_ControllerSensitivity,Type=OST_Slider,Slider_Minimum=1.f,Slider_Maximum=100.f)
	GeneralOptionsWithDifficulty(10) = (ProfileSettingId=PSI_GamepadConfig,Type=OST_ControllerConfigButton)
	GeneralOptionsWithDifficulty(11) = (ProfileSettingId=PSI_ControllerVibration,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(12) = (ProfileSettingId=PSI_Subtitles,Type=OST_CheckBox)
	GeneralOptionsWithDifficulty(13) = (ProfileSettingId=PSI_Volume,Type=OST_Slider,Slider_Minimum=0.f,Slider_Maximum=1.f)

	GeneralOptionsNoDifficulty(0) = (ProfileSettingId=PSI_Language,Type=OST_Dropdown)
	GeneralOptionsNoDifficulty(1) = (ProfileSettingId=PSI_Tutorials,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(2) = (ProfileSettingId=PSI_ShowCrosshair,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(3) = (ProfileSettingId=PSI_ShowPrompts,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(4) = (ProfileSettingId=PSI_YInversion,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(5) = (ProfileSettingId=PSI_Southpaw,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(6) = (ProfileSettingId=PSI_ToggleCrouch,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(7) = (bInProfile=FALSE,ProfileSettingId=PSI_Unknown,NonProfileId=NPO_SmoothCamera,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(8) = (ProfileSettingId=PSI_ControllerSensitivity,Type=OST_Slider,Slider_Minimum=1.f,Slider_Maximum=100.f)
	GeneralOptionsNoDifficulty(9) = (ProfileSettingId=PSI_GamepadConfig,Type=OST_ControllerConfigButton)
	GeneralOptionsNoDifficulty(10) = (ProfileSettingId=PSI_ControllerVibration,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(11) = (ProfileSettingId=PSI_Subtitles,Type=OST_CheckBox)
	GeneralOptionsNoDifficulty(12) = (ProfileSettingId=PSI_Volume,Type=OST_Slider,Slider_Minimum=0.f,Slider_Maximum=1.f)

	GraphicsOptions(0) = (ProfileSettingId=PSI_TextureQuality,Type=OST_Dropdown)
	GraphicsOptions(1) = (ProfileSettingId=PSI_ShadowsQuality,Type=OST_Dropdown)
	GraphicsOptions(2) = (ProfileSettingId=PSI_EffectsQuality,Type=OST_Dropdown)
	GraphicsOptions(3) = (bInProfile=FALSE,ProfileSettingId=PSI_Unknown,NonProfileId=NPO_GrainEffect,Type=OST_CheckBox)
	GraphicsOptions(4) = (ProfileSettingId=PSI_MaxFPS,Type=OST_Dropdown)
	GraphicsOptions(5) = (bInProfile=FALSE,ProfileSettingId=PSI_Unknown,NonProfileId=NPO_DisableMotionBlur,Type=OST_CheckBox)
	GraphicsOptions(6) = (ProfileSettingId=PSI_VSync,Type=OST_CheckBox)
	GraphicsOptions(7) = (ProfileSettingId=PSI_Fullscreen,Type=OST_CheckBox)
	GraphicsOptions(8) = (ProfileSettingId=PSI_Resolution,Type=OST_Dropdown)
	GraphicsOptions(9) = (ProfileSettingId=PSI_GammaSetting,Type=OST_GammaButton)

	ControlsOptions(0) = (ProfileSettingId=PSI_KB_MoveForward,Type=OST_KeyBinding)
	ControlsOptions(1) = (ProfileSettingId=PSI_KB_MoveBackward,Type=OST_KeyBinding)
	ControlsOptions(2) = (ProfileSettingId=PSI_KB_StrafeLeft,Type=OST_KeyBinding)
	ControlsOptions(3) = (ProfileSettingId=PSI_KB_StrafeRight,Type=OST_KeyBinding)
	ControlsOptions(4) = (ProfileSettingId=PSI_KB_TurnLeft,Type=OST_KeyBinding)
	ControlsOptions(5) = (ProfileSettingId=PSI_KB_TurnRight,Type=OST_KeyBinding)
	ControlsOptions(6) = (ProfileSettingId=PSI_KB_Crouch,Type=OST_KeyBinding)
	ControlsOptions(7) = (ProfileSettingId=PSI_KB_Use,Type=OST_KeyBinding)
	ControlsOptions(8) = (ProfileSettingId=PSI_KB_Run,Type=OST_KeyBinding)
	ControlsOptions(9) = (ProfileSettingId=PSI_KB_ToggleCamcorder,Type=OST_KeyBinding)
	ControlsOptions(10) = (ProfileSettingId=PSI_KB_ToggleNightVision,Type=OST_KeyBinding)
	ControlsOptions(11) = (ProfileSettingId=PSI_KB_LeanLeft,Type=OST_KeyBinding)
	ControlsOptions(12) = (ProfileSettingId=PSI_KB_LeanRight,Type=OST_KeyBinding)
	ControlsOptions(13) = (ProfileSettingId=PSI_KB_ZoomImpulseIn,Type=OST_KeyBinding)
	ControlsOptions(14) = (ProfileSettingId=PSI_KB_ZoomImpulseOut,Type=OST_KeyBinding)
	ControlsOptions(15) = (ProfileSettingId=PSI_KB_Reload,Type=OST_KeyBinding)
	ControlsOptions(16) = (ProfileSettingId=PSI_KB_Jump,Type=OST_KeyBinding)
	ControlsOptions(17) = (ProfileSettingId=PSI_KB_ShowMenu,Type=OST_KeyBinding)
	ControlsOptions(18) = (ProfileSettingId=PSI_KB_ShowTabMenu,Type=OST_KeyBinding)
	ControlsOptions(19) = (ProfileSettingId=PSI_KB_ShowRecordingMenu,Type=OST_KeyBinding)
	ControlsOptions(20) = (ProfileSettingId=PSI_KB_ShowEvidenceMenu,Type=OST_KeyBinding)
}