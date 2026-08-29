class OLUIFrontEnd_MainMenu extends OLUIFrontEnd_Screen;

var localized string TitleText;
var localized string ContinueText;
var localized string StartText;
var localized string PlayDemoText;
var localized string LoadText;
var localized string OptionsText;
var localized string ModMapsText;
var localized string MultiplayerText;
var localized string CreditsText;
var localized string ExitText;
var localized string NewGameIntroText;
var localized string ChaptersText;
var localized string StartDLCText;
var localized string PressStartText;
var localized string GameSelectTitleText;
var localized string GameSelectMainText;
var localized string GameSelectDLCText;
var localized string GameSelectBodyText;

var transient GFxClikWidget ButtonBar;
var transient GFxClikWidget IntroLabel;
var transient GFxClikWidget OKButton;
var transient GFxObject GamertagLabel;
var transient GFxObject GamertagButton;
var transient GFxClikWidget GamertagContainer;

var int ContinueButtonIndex;
var int StartButtonIndex;
var int NewGameButtonIndex;
var int StartDLCButtonIndex;
var int LoadButtonIndex;
var int OptionsButtonIndex;
var int MultiplayerButtonIndex;
var int CreditsButtonIndex;
var int ExitButtonIndex;
var int ChaptersButtonIndex;
var int ModMapsButtonIndex;

var bool bShowIntroMsg;
var string StartCPName;

var transient bool bShowingBadCheckpointMsg;
var transient bool bOnPressStartScreen;
var transient bool bPendingProfileSaveOnNewGame;

/** Configures the view when it is first loaded. */
function OnViewLoaded()
{
	Super.OnViewLoaded();
	bShowingBadCheckpointMsg = false;
	bOnPressStartScreen = false;

	SetFunction("OnBadCheckpointMsgAcknowledged", self, nameof(OnBadCheckpointMsgAcknowledged));
	SetFunction("OnMainGameSelected", self, nameof(OnMainGameSelected));
	SetFunction("OnDLCSelected", self, nameof(OnDLCSelected));
}

/** 
 *  Update the view.  
 *  This method is called whenever the view is pushed or popped from the view stakc.
 */
function OnTopMostView(optional bool bPlayOpenAnimation = false)
{   		
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());

	Super.OnTopMostView(bPlayOpenAnimation);

	if (Engine.ShouldOpenPressStartScreen())
	{
		ShowPressStartScreen();		
	}
	else
	{		
		bOnPressStartScreen = false;

		PopulateButtons();
		GotoAndStop("MainMenu");
	}
}

private final function OnContinueButtonPress(GFxClikWidget.EventData ev)
{
	local OLEngine Engine;
	local bool bSuccess;
	local Dingo_LoadGameResult LoadResult;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None && Engine.HasValidSaveGame())
	{
		if (class'OLUtils'.static.IsDingo())
		{
			LoadResult = Engine.Dingo_LoadMostRecentSaveFile();

			if (LoadResult == LGR_Success)
			{
				bSuccess = true;
			}
			else if (LoadResult == LGR_BadCheckpoint)
			{
				ShowBadCheckpointMsg();
			}
		}
		else if (class'OLUtils'.static.IsPS4())
		{
			bSuccess = Engine.PS4_LoadMostRecentSaveFile();
		}
		else
		{
			bSuccess = Engine.LoadSaveFile(Engine.SaveFiles[0].FileName);
		}

		if (bSuccess)
		{
			GetOLPC().StopAllSounds();
			Close(true);
		}
	}
}

function ShowBadCheckpointMsg()
{
	ShowMessageDialog("", Localize("Generic", "DLCUninstalled", "OLGame"), Localize("OLUIFrontEnd_Options", "OKText", "OLGame"), "OnBadCheckpointMsgAcknowledged");
	bShowingBadCheckpointMsg = true;
}

function OnBadCheckpointMsgAcknowledged()
{
	bShowingBadCheckpointMsg = false;
}

function SaveLocationSelected(bool bSuccess)
{
	if (bSuccess)
	{
		// User selected save location, proceed to start game
		StartNewGame();
	}
}

function HideGametag()
{
	if (GamertagLabel != None)
	{
		GamertagLabel.SetText("");
	}

	if (GamertagButton != None)
	{
		GamertagButton.SetVisible(false);
	}

	if (GamertagContainer != None)
	{
		GamertagContainer.SetVisible(false);
	}
}

function ShowGamertag(string Gamertag)
{
	if (GamertagLabel != None)
	{
		GamertagLabel.SetText(Gamertag);
	}

	if (GamertagButton != None)
	{
		GamertagButton.SetVisible(true);
	}

	if (GamertagContainer != None)
	{
		GamertagContainer.SetVisible(true);
	}
}

function DifficultySelected()
{
	local OLEngine Engine;

	GetOLPC().NotifyDifficultyChanged();

	if (bShowIntroMsg)
	{
		HideGametag();
		ASShowNewGameIntroText();
	}
	else
	{
		Engine = OLEngine(class'Engine'.static.GetEngine());
		if (class'OLUtils'.static.IsDingo() && GetGame().DifficultyMode != EDM_Insane)
		{
			if (!MenuManager.SaveLocationList.TrySkipScreen(StartCPName))
			{
				MenuManager.SaveLocationList.bSaving = true;
				MenuManager.SaveLocationList.bFromPauseMenu = false;
				MenuManager.SaveLocationList.StartCP = StartCPName;
				MenuManager.PushViewByName('SaveLocationList'); 
				MenuManager.PushViewByName('SaveLocationList'); 
				MenuManager.PopView();
			}
		}
		else if (Engine != None && class'OLUtils'.static.IsPS4())
		{
			Engine.SelectSaveLocation(StartCPName, SaveLocationSelected);
		}
		else
		{
			StartNewGame();
		}
	}
}

private function StartNewGame()
{
	`log("### Starting new game at cp: " $ StartCPName);

	if (bPendingProfileSaveOnNewGame)
	{
		`log("### Saving pending profile");
		bPendingProfileSaveOnNewGame = false;
		SaveProfile();
	}

	GetOLPC().StartNewGameAtCheckpoint(StartCPName, true);	
	Close(true);
}

function GameLoadedCallback(bool bSuccess)
{	
	if (bSuccess)
	{
		Close(true);
	}
}

private final function OnNewGameButtonPress(GFxClikWidget.EventData ev)
{
	ASShowGameSelectDialog(GameSelectTitleText, GameSelectBodyText, GameSelectMainText, GameSelectDLCText);
}

function OnMainGameSelected()
{
	GetOLPC().ClearAllProgress();
	StartCPName = "StartGame";
	bShowIntroMsg = true;
	DifficultySelectionView.bSpecificChapter = false;
	MenuManager.PushViewByName('DifficultySelectionScreen');
}

function OnDLCSelected()
{
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None && Engine.TryStartDLCGame())
	{
		GetOLPC().ClearAllProgress();
		StartCPName = "DLC_Start";
		bShowIntroMsg = false;
		DifficultySelectionView.bSpecificChapter = false;
		MenuManager.PushViewByName('DifficultySelectionScreen');
	}
}

private final function OnStartButtonPress(GFxClikWidget.EventData ev)
{
	GetOLPC().ClearAllProgress(); // shouldn't be necessary, but just to make sure
	StartCPName = "StartGame";
	bShowIntroMsg = true;
	DifficultySelectionView.bSpecificChapter = false;
	MenuManager.PushViewByName('DifficultySelectionScreen');
}

private final function OnStartDLCButtonPress(GFxClikWidget.EventData ev)
{
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None && Engine.TryStartDLCGame())
	{
		GetOLPC().ClearAllProgress(); // shouldn't be necessary, but just to make sure
		StartCPName = "DLC_Start";
		bShowIntroMsg = false;
		DifficultySelectionView.bSpecificChapter = false;
		MenuManager.PushViewByName('DifficultySelectionScreen');
	}
}

private final function OnChaptersButtonPress(GFxClikWidget.EventData ev)
{
	GetOLPC().ClearAllProgress(); // shouldn't be necessary, but just to make sure
	StartCPName = "StartGame"; // failsafe, will be overwritten
	bShowIntroMsg = false;
	MenuManager.PushViewByName('ChapterSelection');
}

private final function OnModMapsButtonPress(GFxClikWidget.EventData ev)
{
	MenuManager.PushViewByName('ModMaps');
}

private final function OnOKButtonClick(GFxClikWidget.EventData ev)
{
	// The user accepted the intro text

	local OLEngine Engine;

	bShowIntroMsg = false;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (class'OLUtils'.static.IsDingo() && GetGame().DifficultyMode != EDM_Insane)
	{
		if (!MenuManager.SaveLocationList.TrySkipScreen(StartCPName))
		{
			MenuManager.SaveLocationList.bSaving = true;
			MenuManager.SaveLocationList.bFromPauseMenu = false;
			MenuManager.SaveLocationList.StartCP = StartCPName;
			MenuManager.PushViewByName('SaveLocationList'); 
		}
	}
	else if (Engine != None && class'OLUtils'.static.IsPS4() && GetGame().DifficultyMode != EDM_Insane)
	{
		Engine.SelectSaveLocation(StartCPName, SaveLocationSelected);
	}
	else
	{
		StartNewGame();
	}
}

private final function OnLoadButtonPress(GFxClikWidget.EventData ev)
{
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (class'OLUtils'.static.IsDingo())
	{
		MenuManager.SaveLocationList.bSaving = false;
		MenuManager.PushViewByName('SaveLocationList'); 
	}
	else if (Engine != None && class'OLUtils'.static.IsPS4())
	{
		Engine.SelectAndLoadGame(GameLoadedCallback);
	}
	else
	{
		MenuManager.PushViewByName('LoadGame'); 
	}
}

function Select_Options()
{    
	MenuManager.PushViewByName('Options');
}

private final function OnOptionsButtonPress(GFxClikWidget.EventData ev)
{
	Select_Options();
}

private final function OnMultiplayerButtonPress(GFxClikWidget.EventData ev)
{
	MenuManager.PushViewByName('Multiplayer');
}

private final function OnCreditsButtonPress(GFxClikWidget.EventData ev)
{
	MenuManager.CreditsView.bGameOver = false;
	MenuManager.PushViewByName('Credits');
}

private final function OnExitButtonPress(GFxClikWidget.EventData ev)
{
	ConsoleCommand("quit");
}

function bool Back()
{
	if (class'OLUtils'.static.IsDingo() && !bShowIntroMsg)
	{
		ShowPressStartScreen();
		return true;
	}

	return false;
}

function PopulateButtons()
{
	local int i;
	local GFxObject Obj, DataProvider;
	local OLEngine Engine;
	local WorldInfo WI;
	local OLGame CurrentGame;

	local int bHasFinishedGame;
	local int bHasFinishedDLC;
	local bool bHasValidSaveGame;

	DataProvider = CreateArray();

	i = 0;

	bHasValidSaveGame = false;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None && !IsDemo() && GetOLPC().HUD.IsMainMenuOpen())
	{
		Engine.RefreshSaveFiles();
		bHasValidSaveGame = Engine.HasValidSaveGame();

		if (Engine.RefreshDLC())
		{
			// dlc just got installed - reload to dlc_intro_persistent
			WI = Class'WorldInfo'.static.GetWorldInfo();
			CurrentGame = OLGame(WI.Game);
			CurrentGame.TravelToStartupMap();

			return;
		}
	}

	if (bHasValidSaveGame)
	{
		Obj = CreateObject("Object");
		Obj.SetString("label", ContinueText);
		DataProvider.SetElementObject(i, Obj);
		ContinueButtonIndex = i;

		++i;
	}
	else
	{
		ContinueButtonIndex = -1;
	}

	Obj = CreateObject("Object");
	if (IsDemo())
	{
		Obj.SetString("label", PlayDemoText);
		DataProvider.SetElementObject(i, Obj);
		StartButtonIndex = i;
		NewGameButtonIndex = -1;
		StartDLCButtonIndex = -1;
	}
	else if (Engine != None && Engine.ShouldShowNewDLCGame())
	{
		Obj.SetString("label", StartText);
		DataProvider.SetElementObject(i, Obj);
		NewGameButtonIndex = i;
		StartButtonIndex = -1;
		StartDLCButtonIndex = -1;
	}
	else
	{
		Obj.SetString("label", StartText);
		DataProvider.SetElementObject(i, Obj);
		StartButtonIndex = i;
		NewGameButtonIndex = -1;
		StartDLCButtonIndex = -1;
	}
		
	GetOLPC().ProfileSettings.GetProfileSettingValueId(PSI_FinishedGame, bHasFinishedGame);
	GetOLPC().ProfileSettings.GetProfileSettingValueId(PSI_FinishedDLC, bHasFinishedDLC);
	if (bHasFinishedGame == 1 || (class'OLUtils'.static.IsDLCInstalled() && bHasFinishedDLC == 1))
	{
		++i;
		Obj = CreateObject("Object");
		Obj.SetString("label", ChaptersText);
		DataProvider.SetElementObject(i, Obj);
		ChaptersButtonIndex = i;
	}
	else
	{
		ChaptersButtonIndex = -1;
	}

	if (bHasValidSaveGame)
	{
		++i;
		Obj = CreateObject("Object");
		Obj.SetString("label", LoadText);
		DataProvider.SetElementObject(i, Obj);
		LoadButtonIndex = i;
	}
	else
	{
		LoadButtonIndex = -1;
	}
		
	++i;
	Obj = CreateObject("Object");
	Obj.SetString("label", OptionsText);
	DataProvider.SetElementObject(i, Obj);
	OptionsButtonIndex = i;

	++i;
	Obj = CreateObject("Object");
	Obj.SetString("label", MultiplayerText);
	DataProvider.SetElementObject(i, Obj);
	MultiplayerButtonIndex = i;

	++i;
	Obj = CreateObject("Object");
	Obj.SetString("label", ModMapsText);
	DataProvider.SetElementObject(i, Obj);
	ModMapsButtonIndex = i;

	++i;
	Obj = CreateObject("Object");
	Obj.SetString("label", CreditsText);
	DataProvider.SetElementObject(i, Obj);
	CreditsButtonIndex = i;

	if (class'OLUtils'.static.IsConsole())
	{
		// can't exit on console
		ExitButtonIndex = -1;	
	}
	else
	{
		++i;
		Obj = CreateObject("Object");
		Obj.SetString("label", ExitText);
		DataProvider.SetElementObject(i, Obj);
		ExitButtonIndex = i;
	}

	if (ButtonBar != None)
	{
		ButtonBar.SetObject("dataProvider", DataProvider);
		ButtonBar.SetFloat("selectedIndex", 0);
	}

	if (bOnPressStartScreen)
	{
		HideGametag();
	}
	else
	{	
		ShowGamertag(Engine.Dingo_GetActiveGamertag());		
	}

	if (IsTopMostView())
	{
		ASInitButtonFocus();
	}
}

private final function OnButtonClick(GFxClikWidget.EventData ev)
{
	if (ContinueButtonIndex != -1 && ev.index == ContinueButtonIndex)
	{
		OnContinueButtonPress(ev);	
	}
	else if (StartButtonIndex != -1 && ev.index == StartButtonIndex)
	{
		OnStartButtonPress(ev);
	}
	else if (NewGameButtonIndex != -1 && ev.index == NewGameButtonIndex)
	{
		OnNewGameButtonPress(ev);
	}
	else if (StartDLCButtonIndex != -1 && ev.index == StartDLCButtonIndex)
	{
		OnStartDLCButtonPress(ev);
	}
	else if (LoadButtonIndex != -1 && ev.index == LoadButtonIndex)
	{
		OnLoadButtonPress(ev);
	}
	else if (ChaptersButtonIndex != -1 && ev.index == ChaptersButtonIndex)
	{
		OnChaptersButtonPress(ev);
	}
	else if (OptionsButtonIndex != -1 && ev.index == OptionsButtonIndex)
	{
		OnOptionsButtonPress(ev);
	}
	else if (MultiplayerButtonIndex != -1 && ev.index == MultiplayerButtonIndex)
	{
		OnMultiplayerButtonPress(ev);
	}
	else if (ModMapsButtonIndex != -1 && ev.index == ModMapsButtonIndex)
	{
		OnModMapsButtonPress(ev);
	}
	else if (CreditsButtonIndex != -1 && ev.index ==  CreditsButtonIndex)
	{
		OnCreditsButtonPress(ev);
	}
	else if (ExitButtonIndex != -1 && ev.index ==  ExitButtonIndex)
	{
		OnExitButtonPress(ev);
	}
}


function OnDingoUserInitialized(bool bSuccess)
{
	if (bSuccess)
	{
		bOnPressStartScreen = false;

		PopulateButtons();
		GotoAndStop("MainMenu");

		if (!MenuManager.HasInitializedGamma())
		{
			bPendingProfileSaveOnNewGame = true;
			MenuManager.ConfigureTargetView(MenuManager.GammaScreenView);
		}
	}
	else
	{
		ShowPressStartScreen();
	}
}

function OnPressStart(int ControllerId)
{
	local OLEngine Engine;
	local int NewControllerIdx;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None)
	{
		GotoAndPlay("Init");

		NewControllerIdx = Engine.Dingo_OnInitialPressStart(ControllerId);
		if (Engine.Dingo_ShouldShowLoginUI(NewControllerIdx))
		{
			Engine.Dingo_ShowLoginUIAndInitializeUser(OnDingoUserInitialized);
		}
		else
		{
			Engine.Dingo_InitializeUser(OnDingoUserInitialized);
		}
	}	
}

function ForceShowLoginUI(int ControllerId)
{
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None)
	{
		GotoAndPlay("Init");		
		HideGametag();

		Engine.Dingo_OnInitialPressStart(ControllerId);
		Engine.Dingo_ShowLoginUIAndInitializeUser(OnDingoUserInitialized);
	}	
}

function ShowPressStartScreen()
{
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None)
	{
		Engine.Dingo_AllowAllControllersInput();
	}

	bOnPressStartScreen = true;
	HideGametag();

	GotoAndPlayI(1);
}

event bool FilterButtonInput(int ControllerId, name ButtonName, EInputEvent InputEvent)
{
	if (InputEvent != IE_Pressed)
	{
		return false;
	}

	if (bShowingBadCheckpointMsg)
	{
		if ((ButtonName == 'XboxTypeS_A' || ButtonName == 'XboxTypeS_B'))
		{
			ASHideDialogs();
			bShowingBadCheckpointMsg = false;
			return true;
		}
	}

	if (bOnPressStartScreen)
	{
		if (ButtonName == 'XboxTypeS_B')
		{
			// ignore		
			return true;
		}
		else if (ButtonName == 'XboxTypeS_A' || ButtonName == 'XboxTypeS_Start')
		{
			OnPressStart(ControllerId);
			return true;
		}
	}
	else if (!bShowingBadCheckpointMsg && !bOnPressStartScreen && !bShowIntroMsg && ButtonName == 'XboxTypeS_Y')
	{
		ForceShowLoginUI(ControllerId);
	}

	return super.FilterButtonInput(ControllerId, ButtonName, InputEvent);
}

/** Callback when a CLIK widget with enableInitCallback set to TRUE is initialized.  Returns TRUE if the widget was handled, FALSE if not. */
event bool WidgetInitialized(name WidgetName, name WidgetPath, GFxObject Widget)
{    
	local bool bWasHandled;
	bWasHandled = false;
	
    switch(WidgetName)
    {
		case ('buttonBar'):
			ButtonBar = GFxClikWidget(Widget);
			PopulateButtons();
			ButtonBar.AddEventListener('CLIK_itemClick', OnButtonClick);
			bWasHandled = true;
			break;
		case ('introText'):
			IntroLabel = GFxClikWidget(Widget);
			IntroLabel.GetObject("textField").SetString("verticalAutoSize", "center");
			IntroLabel.SetString("text", NewGameIntroText);
			bWasHandled = true;
			break;
		case ('okButton'):
			OKButton = GFxClikWidget(Widget);
			OKButton.SetString("label", ContinueText);
			OKButton.SetBool("focused", true);
			OKButton.AddEventListener('CLIK_press', OnOKButtonClick);
			bWasHandled = true;
			break;
		case ('CrossLabel'):
			Widget.SetText(PressStartText);
			bWasHandled = true;
			break;
		case ('TriangleLabel'):
			GamertagButton = Widget;
			GamertagButton.SetText("");
			GamertagButton.SetVisible(false);
			bWasHandled = true;
			break;
		case ('Tag'):
			GamertagLabel = Widget;
			GamertagLabel.SetText("");
			bWasHandled = true;
			break;
		case ('Ybutton'):
			GamertagContainer = GFxClikWidget(Widget);
			GamertagContainer.SetVisible(false);
			bWasHandled = true;
			break;
		default:
			bWasHandled = false;
	}

	if (!bWasHandled)
	{
		bWasHandled = Super.WidgetInitialized(WidgetName, WidgetPath, Widget);  
	}
	return bWasHandled;
}

private function ShowMessageDialog(string title, string message, string okButtonLabel, string callbackName) { ActionScriptVoid("ShowMessageDialog"); }
function ASShowNewGameIntroText() { ActionScriptVoid("ShowNewGameIntroText"); }
function ASShowGameSelectDialog(string titleStr, string bodyStr, string mainGameStr, string dlcStr) { ActionScriptVoid("ShowGameSelectDialog"); }

function ASInitButtonFocus() { ActionScriptVoid("InitFocus"); }

defaultproperties
{
	SubWidgetBindings.Add((WidgetName="buttonBar",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="introText",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="okButton",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="Ybutton",WidgetClass=class'GFxClikWidget'))

	ContinueButtonIndex=-1
	StartButtonIndex=-1
	NewGameButtonIndex=-1
	StartDLCButtonIndex=-1
	LoadButtonIndex=-1
	OptionsButtonIndex=-1
	MultiplayerButtonIndex=-1
	CreditsButtonIndex=-1
	ExitButtonIndex=-1
	ModMapsButtonIndex=-1

}