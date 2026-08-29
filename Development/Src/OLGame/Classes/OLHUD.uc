/**
 * Copyright 2012 Red Barrels, Inc. All Rights Reserved.
 */
class OLHUD extends UDKHUD
	native;

var OLBot CurrentDebugBot;

var bool bGammaCalibrationOpen;
var EPPSMode PreGammaPPSMode;

var bool bSplashScreenOpen;
var bool bSplashScreenReady;
var float SplashScreenActivatedTimestamp;

var bool bGameOver;

var config bool bShowCrosshair;
var bool bCrosshairDesired;
var float CrosshairOpacity;
var float LastCrosshairUpdateRealTime;
var float InteractionCrosshairPct; // 0.0 - default crosshair, 1.0 - interaction crosshair

var bool bAlwaysShowPrompts;

var float NewObjectiveTimestamp;
var vector NewObjectiveHeroPos; // location of the hero when the objective was set (keep alive for a certain radius)
var config float NewObjectiveZoneRadius;
var float ShowInventoryTimestamp;

var float LastCamCycleTime;
var name LatestRecordingName;
var float LatestRecordingTimer;
var name LatestDocumentName;
var float LatestDocumentTimer;
var float NewObjectiveTimer;
var float LatestCheckpointTimer;

var const float SubtitleMaxDistance;
var const float SubtitleMaxDistanceOccluded;
var const float SubtitleOcclusionThreshold;
var const float SubtitleObstructionThreshold;

struct native SubtitleData
{
	var string Text;
	var Actor Speaker;
	var AkEvent SoundEvent;
	var float EffectiveDistance;
	var float TimeFired;
	var bool bUseAsImportant;
	var bool bOccluded;
};

var array<SubtitleData> SubtitleQueue;

var string CurrentSubtitle;

var config bool bShowSubtitles;
var config bool bForcePS4UI;

// Assets
var Texture2D InventoryBackgoundImg;
var Texture2D NormalCrosshairImage;
var Texture2D InteractionCrosshairImage;
var Texture2D SplashScreenImage;
var Texture2D GameOverImage;
var Texture2D GameOverImageDemo;
var Font SmallFont;
var Font MediumFont;
var Font LargeFont;
var Font HugeFont;
var OLCamcorderHud CamcorderHUD;
var OLMainHud MainHUD;

var AkEvent SoundEventSelect;
var AkEvent SoundEventEnter;

enum EMenuType
{
	EMT_MainMenu,
	EMT_PauseMenu,
	EMT_TabMenu,
	EMT_RecordingMenu,
	EMT_EvidenceMenu,
	EMT_Credits
};

var OLUIFrontEnd MenuManager;
var OLUIMessage ObjectiveScreen;
var OLUIMessage MessageScreen;

var OLUIMessage SubtitleScreen;

enum EHUDMessageType
{
	EHMT_None,
	EHMT_Objective,
	EHMT_Tutorial,
	EHMT_Generic,
	EHMT_Interaction,
	EHMT_Subtitle,
	EHMT_ShakeMouse,
	EHMT_ShakeStick
};

var EHUDMessageType CurrentMessageType;

var string CurrentObjectiveText;
var string CurrentMessageText;

var string LastUntranslatedMessageText;
var string CachedTranslatedMessageText;

struct native GenericMessage
{
	var init string MessageText;
	var float RemainingTime;
	var EHUDMessageType MessageType;
};

var array<GenericMessage> GenericMessages;

var bool bLostFocus;

native function Draw();
native function bool ShowingFullScreenOverlay();
native function SetGammaCalibrationActive(bool bActive);

cpptext
{
	virtual void TickSpecial(FLOAT deltaSeconds);
	virtual void PostBeginPlay();
	void Reset();

	void ShowRecordingCompleteMessage();
	void ShowNewObjective();
	void ShowSplashScreen();
	void ShowGameOver();
	void ShowCamcorderHUD();
	void HideCamcorderHUD();
	void ShowMainHUD();
	void HideMainHUD();

	void AddMessage(EHUDMessageType messageType, const FString& messageText, FLOAT duration = 3.0f);

	void SetLatestDocument(FName documentName);
	void SetLatestRecording(FName recordingName);
	
	void PostSubtitle(FString Subtitle, AActor* Speaker, UAkEvent* SoundEvent);
	void PopSubtitle(AActor* Speaker, UAkEvent* SoundEvent);

private:
	void DrawCrosshair();
	void DrawSplashScreen();
	void DrawGameOver();

	void UpdateObjective(FLOAT deltaSeconds);
	void UpdateMessages(FLOAT deltaSeconds);
	void UpdateCrosshair(FLOAT deltaSeconds);
	void UpdateNote(FLOAT DeltaSeconds);
	void UpdateSubtitles(FLOAT DeltaSeconds);

	void SortAndUpdateCurrentSubtitle();
	void GetDistanceAndOccludedForSpeaker(AActor* Speaker, FLOAT& out_Distance, UBOOL& out_Occluded);
}

native function NotifyGameSaved();

exec function DebugNextAI()
{
	local OLBot Bot;
	local bool bFound;
	local OLBot FirstBot;
	
	bFound = false;
	foreach WorldInfo.AllControllers(class'OLBot', Bot)
	{
		if (Bot.bDeleteMe)
		{
			continue;
		}

		if (FirstBot == None)
		{
			FirstBot = Bot;
		}

		if (bFound)
		{
			CurrentDebugBot = Bot;
			bFound = false;
			break;
		}
		else if (CurrentDebugBot == None)
		{
			CurrentDebugBot = Bot;
		}
		else if(CurrentDebugBot == Bot)
		{
			bFound = true;
		}
	}

	// For wrapping the list.
	if (bFound)
	{
		CurrentDebugBot = FirstBot;
	}
}

function ShowDebugInfo(out float out_YL, out float out_YPos)
{
	local OLGame TheGame;
	local float SecondYPos;
	local float OldOrgX;

	if (ShouldDisplayDebug('voicemanager'))
	{
		// Don't call super so we can use the full screen.
		TheGame = OLGame(WorldInfo.Game);

		TheGame.VoiceManager.DisplayDebug(self, out_YL, out_YPos);
	}
	else
	{
		Super.ShowDebugInfo(out_YL, out_YPos);

		if (ShouldDisplayDebug('OLAI'))
		{
			if (CurrentDebugBot != None && CurrentDebugBot.bDeleteMe)
			{
				CurrentDebugBot = None;
			}

			if(CurrentDebugBot == None)
			{
				// Try to find an AI.
				DebugNextAI();
			}

			if(CurrentDebugBot != None)
			{
				Canvas.SetDrawColor(200,88,237);

				OldOrgX = Canvas.OrgX;
				Canvas.SetOrigin(Canvas.OrgX + Canvas.SizeX - 500, Canvas.OrgY);

				SecondYPos = 12;
				Canvas.SetPos(4,SecondYPos);
				Canvas.DrawText("" $ CurrentDebugBot $ " - " $ CurrentDebugBot.EnemyPawn);

				SecondYPos += out_YL;
				Canvas.SetPos(4,SecondYPos);
				CurrentDebugBot.DisplayDebug(self, out_YL, SecondYPos);

				Canvas.SetOrigin(OldOrgX, Canvas.OrgY);
			}
		}
	}
}

delegate bool CanUnpauseInPauseMenu()
{
	return MenuManager == None || !MenuManager.bMovieIsOpen;
}

event bool IsInPauseMenu()
{
	return MenuManager != None && MenuManager.MenuType != EMT_MainMenu;
}

event bool IsMainMenuOpen()
{
	return MenuManager != None && MenuManager.MenuType == EMT_MainMenu;
}

event bool IsOnMainMenuScreen()
{
	return MenuManager != None && MenuManager.MenuType == EMT_MainMenu && MenuManager.ViewStack.Length >= 1 && OLUIFrontEnd_MainMenu(MenuManager.ViewStack[MenuManager.ViewStack.Length-1]) != None;
}

event bool IsInCreditsMenu() 
{
	return MenuManager != None && MenuManager.ViewStack.Length > 1 && OLUIFrontEnd_Credits(MenuManager.ViewStack[MenuManager.ViewStack.Length-1]) != None;
}

event bool IsAMenuOpen()
{
	return MenuManager != None && MenuManager.bMovieIsOpen;
}

exec function ReloadMenu()
{
	if (MenuManager != None && MenuManager.bMovieIsOpen)
	{
		if (MenuManager.MenuType == EMT_MainMenu)
		{
			HideMenu();
			ShowMainMenu();
		}
		else
		{
			HideMenu();
			ShowMenu();
		}
	}
}

event SimulateBackInput()
{
	if (MenuManager != None && MenuManager.bMovieIsOpen)
	{
		MenuManager.FilterButtonInput(-1, 'XboxTypeS_B', IE_Pressed);
	}
}

event ClosePauseMenu()
{
	if (IsInPauseMenu())
	{
		HideMenu();
	}
}

event ReturnToPressStartMenu()
{
	`log("## ReturnToPressStartMenu");

	if (MenuManager != None && MenuManager.bMovieIsOpen)
	{
		MenuManager.Close(true);
		MenuManager = None;
	}
	ShowMenuType(EMT_MainMenu);
}

exec function ShowMainMenu()
{
	ShowMenuType(EMT_MainMenu);
}

exec function ShowMenu()
{
	// if using GFx HUD, use GFx pause menu
	ShowMenuType(EMT_PauseMenu);
}

exec function ShowTabMenu()
{
	// if using GFx HUD, use GFx pause menu
	ShowMenuType(EMT_TabMenu);
}

exec function ShowRecordingMenu()
{
	ShowMenuType(EMT_RecordingMenu);
}

exec function ShowEvidenceMenu()
{
	ShowMenuType(EMT_EvidenceMenu);
}

native function bool CanShowSubMenu();
native function MuteSelectSound(optional float MuteTime = 1.5);
native function PostSoundEventSelect();
native function PostSoundEventEnter();

event ShowMenuType(EMenuType MenuType)
{
	local OLGame TheGame;

	if (MenuType != EMT_MainMenu && !CanShowSubMenu())
	{
		return;
	}

	TheGame = OLGame(WorldInfo.Game);
	if (MenuManager == None || !MenuManager.bMovieIsOpen)
	{
		if (PlayerOwner != None)
		{
			PlayerOwner.PlayerInput.ResetInput();
		}

		if (MenuType != EMT_MainMenu)
		{
			if (MenuType == EMT_Credits)
			{
				TheGame.bSoundOnPause = FALSE;
			}
			else
			{
				TheGame.bSoundOnPause = TRUE;
			}
			TheGame.SetPause(PlayerOwner, CanUnpauseInPauseMenu);
		}		

		if (MenuManager == None)
		{
			if (class'OLUtils'.static.IsConsole())
			{
				if (class'OLUtils'.static.IsDingo())
				{
					MenuManager = new(self) class'OLUIFrontEnd_ConsoleXbox';
				}
				else
				{
					MenuManager = new(self) class'OLUIFrontEnd_Console';
				}
			}
			else
			{
				MenuManager = new(self) class'OLUIFrontEnd';
			}

			MenuManager.MenuType = MenuType;
		}

		if (MenuManager != None)
		{
			if (MenuType == EMT_MainMenu)
			{
				MuteSelectSound(2.5); // mute for some extra time, it takes a bit to load
			}
			else
			{
				MuteSelectSound();
			}

			MenuManager.Start(false);

			if (CamcorderHUD != None)
			{
				CamcorderHUD.SetVisible(false);
			}
			HideHUDMessages();
		}
	}
}

exec function HideMenu()
{
	local OLGame TheGame;
	local bool bPaused;

	if (MenuManager != None && MenuManager.bMovieIsOpen)
	{
		TheGame = OLGame(WorldInfo.Game);
		bPaused = (MenuManager.MenuType != EMT_MainMenu);
		MenuManager.Close(true);
		MenuManager = None;

		if (bPaused)
		{
			TheGame.bSoundOnPause = TRUE;
			TheGame.ClearPause();
		}
		
		if (CamcorderHUD != None)
		{
			CamcorderHUD.SetVisible(true);
		}
		ShowHUDMessages();
	}
}

event ShowMessage(EHUDMessageType MessageType, string MessageText)
{	
	if (MessageScreen == None)
	{
		if (class'OLUtils'.static.IsConsole())
		{
			if (class'OLUtils'.static.IsDingo())
			{
				MessageScreen = new(self) class'OLUIMessage_ConsoleXbox';
			}
			else
			{
				MessageScreen = new(self) class'OLUIMessage_Console';
			}
		}
		else
		{
			MessageScreen = new(self) class'OLUIMessage';
		}
	}

	if (MessageScreen != None)
	{
		CurrentMessageType = MessageType;
		CurrentMessageText = MessageText;
		MessageScreen.Start(false);
		MessageScreen.SetMessage(MessageType, MessageText);
	}
}

event HideMessage()
{
	if (MessageScreen != None && MessageScreen.bMovieIsOpen)
	{
		MessageScreen.Close(false);
		CurrentMessageText = "";
	}
}

event ShowObjective(string ObjectiveText)
{
	if (ObjectiveScreen == None)
	{
		if (class'OLUtils'.static.IsConsole())
		{
			if (class'OLUtils'.static.IsDingo())
			{
				ObjectiveScreen = new(self) class'OLUIMessage_ConsoleXbox';
			}
			else
			{
				ObjectiveScreen = new(self) class'OLUIMessage_Console';
			}
		}
		else
		{
			ObjectiveScreen = new(self) class'OLUIMessage';
		}
	}

	if (ObjectiveScreen != None)
	{
		CurrentObjectiveText = ObjectiveText;
		ObjectiveScreen.Start(false);
		ObjectiveScreen.SetMessage(EHMT_Objective, ObjectiveText);
	}
}

event HideObjective()
{
	if (ObjectiveScreen != None && ObjectiveScreen.bMovieIsOpen)
	{
		ObjectiveScreen.Close(false);
		CurrentObjectiveText = "";
	}
}

event ShowSubtitle(string MessageText)
{	
	if (SubtitleScreen == None)
	{
		if (class'OLUtils'.static.IsConsole())
		{
			if (class'OLUtils'.static.IsDingo())
			{
				SubtitleScreen = new(self) class'OLUIMessage_ConsoleXbox';
			}
			else
			{
				SubtitleScreen = new(self) class'OLUIMessage_Console';
			}
		}
		else
		{
			SubtitleScreen = new(self) class'OLUIMessage';
		}
	}

	if (SubtitleScreen != None)
	{
		//CurrentMessageText = MessageText;
		if (!SubtitleScreen.bMovieIsOpen)
		{
			SubtitleScreen.Start(false);
			CurrentSubtitle = MessageText;
			SubtitleScreen.SetMessage(EHMT_Subtitle, MessageText);
		}
		else
		{
			CurrentSubtitle = MessageText;
			SubtitleScreen.SetMessage(EHMT_Subtitle, MessageText);
			SubtitleScreen.SetVisible(true);
		}

	}
}

event HideSubtitle()
{
	CurrentSubtitle = "";
		
	if (SubtitleScreen != None && SubtitleScreen.bMovieIsOpen)
	{
		SubtitleScreen.SetVisible(false);
	}
}

event bool ShowingSubtitle()
{
	return SubtitleScreen != None && SubtitleScreen.bMovieIsOpen && CurrentSubtitle != "";
}

// Hides HUD messages like Objectives, Tutorials, and Pickup text
function HideHUDMessages()
{
	if (ObjectiveScreen != None)
	{
		ObjectiveScreen.SetVisible(false);
	}
	if (MessageScreen != None)
	{
		MessageScreen.SetVisible(false);
	}
	if (SubtitleScreen != None)
	{
		SubtitleScreen.SetVisible(false);
	}
}

// Shows HUD messages like Objectives, Tutorials, and Pickup text
function ShowHUDMessages()
{
	if (ObjectiveScreen != None)
	{
		ObjectiveScreen.SetVisible(true);
	}
	if (MessageScreen != None)
	{
		MessageScreen.SetVisible(true);
	}
	if (SubtitleScreen != None && CurrentSubtitle != "")
	{
		SubtitleScreen.SetVisible(true);
	}
}

event OnLostFocusPause(bool bEnable)
{
	bLostFocus = bEnable;
}

function GamepadConfigChanged()
{
	// invalidate the cached translated string
	CachedTranslatedMessageText = "";
	LastUntranslatedMessageText = "";
}

defaultproperties
{
	InventoryBackgoundImg=Texture2D'EditorResources.Cloudcast'
	SmallFont=MultiFont'UI_Fonts_Final.HUD.MF_Small'
	MediumFont=MultiFont'UI_Fonts_Final.HUD.MF_Medium'
	LargeFont=MultiFont'UI_Fonts_Final.HUD.MF_Large'
	HugeFont=MultiFont'UI_Fonts_Final.HUD.MF_Huge'
	NormalCrosshairImage=Texture2D'Engine_MI_Shaders.Textures.Bokeh'
	InteractionCrosshairImage=Texture2D'OLFrontEnd.InteractiveCrosshair-01'
	SplashScreenImage=Texture2D'Engine_MI_Shaders.T_Base_Tile_Variation'
	GameOverImage=Texture2D'OLFrontEnd.GameOver'
	GameOverImageDemo=Texture2D'OLFrontEnd.OutlastLogoDemo'
	
	LastCamCycleTime=-1.0
	NewObjectiveTimestamp=-1.0

	SubtitleMaxDistance=1000.0f
	SubtitleMaxDistanceOccluded=500.0f
	SubtitleOcclusionThreshold=0.1f
	SubtitleObstructionThreshold=0.1f

	SoundEventEnter=AkEvent'Menu.PARAMETER_Valid'
	SoundEventSelect=AkEvent'Menu.PARAMETER_Move'
}