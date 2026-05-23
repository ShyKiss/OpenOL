class OLUIFrontEnd extends OLGFxMoviePlayer
    config(UI)
	dependson(OLHUD);

var name EscapeMenuKey;
var name TabMenuKey;
var name RecordingMenuKey;
var name EvidenceMenuKey;

/** Reference to _root of the movie's (udk_manager.swf) stage. */
var GFxObject RootMC;

/** Reference to the manager MovieClip (_root.manager) where views will be attached. */
var GFxObject ManagerMC;

var bool bInitialized;

var EMenuType MenuType;

/** View declarations. */
var OLUIFrontEnd_MainMenu MainMenuView;
var OLUIFrontEnd_PauseMenu PauseMenuView;
var OLUIFrontEnd_LoadGame LoadGameView;
var OLUIFrontEnd_Options OptionsView;
var OLUIFrontEnd_Screen GammaScreenView;
var OLUIFrontEnd_Screen GamepadScreenView;
var OLUIFrontEnd_Credits CreditsView;
var OLUIFrontEnd_DifficultySelectionScreen DifficultySelectionView;
var OLUIFrontEnd_ChapterSelection ChapterSelection;

// Dingo-specific
var OLUIFrontEnd_SaveLocationList SaveLocationList;

var OLUIFrontEnd_TabMenu TabMenuView;
var OLUIFrontEnd_RecordingList RecordingListView;
var OLUIFrontEnd_Screen RecordingView;
var OLUIFrontEnd_EvidenceList EvidenceListView;
var OLUIFrontEnd_Screen EvidenceView;

var bool bCapturingKeyBinding;

/** Structure which defines a unique menu view to be loaded. */
struct ViewInfo
{
	/** Unique string. */
	var name ViewName;

    /** SWF content to be loaded. */
    var string SWFName;

    /** Dependant views that should be loaded if this view is displayed. */
    var array<name> DependantViews;
};

/** Array of all menu views to be loaded, defined in DefaultUI.ini. */
var config array<ViewInfo>			ViewData;

/** 
 *  Shadow of the AS view stack. Necessary to update ( View.OnTopMostView(false) ) views that
 *  alreday exist on the stack. 
 */
var array<OLUIFrontEnd_View>		ViewStack;

/**
 * An array of names of views which have been attachMovie()'d and loadMovie()'d. Views
 * are loaded based on their DependantViews array, defined in Default.ini.
 */
var array<name>						LoadedViews;

function bool Start(optional bool StartPaused = false)
{
	super.Start();
	Advance(0);

	SetViewScaleMode(SM_ShowAll);

	if (!bInitialized)
	{
		ConfigFrontEnd();
	}

	LoadViews();

	AddCaptureKeys();
	AddFocusIgnoreKey('F8');

	return true;
}

event OnClose()
{
	while (ViewStack.Length > 1)
	{
		PopView();
	}
}

function AddCaptureKeys()
{
	local OLProfileSettings MyProfile;
	local string EscapeMenuKeyString, TabMenuKeyString, RecordingMenuKeyString, EvidenceMenuKeyString;

	AddCaptureKey('Escape');
   AddCaptureKey('XboxTypeS_B');
	AddCaptureKey('XboxTypeS_X');
	AddCaptureKey('XboxTypeS_Start');
	AddCaptureKey('XboxTypeS_DPad_Left');
	AddCaptureKey('XboxTypeS_DPad_Right');
	AddCaptureKey('Gamepad_LeftStick_Left');
	AddCaptureKey('Gamepad_LeftStick_Right');

	// Look up key bindings so we capture the right keys
	MyProfile = GetOLPC().ProfileSettings;
	MyProfile.GetProfileSettingValueString(PSI_KB_ShowMenu, EscapeMenuKeyString);
	MyProfile.GetProfileSettingValueString(PSI_KB_ShowTabMenu, TabMenuKeyString);
	MyProfile.GetProfileSettingValueString(PSI_KB_ShowRecordingMenu, RecordingMenuKeyString);
	MyProfile.GetProfileSettingValueString(PSI_KB_ShowEvidenceMenu, EvidenceMenuKeyString);
	EscapeMenuKey = name(EscapeMenuKeyString);
	TabMenuKey = name(TabMenuKeyString);
	RecordingMenuKey = name(RecordingMenuKeyString);
	EvidenceMenuKey = name(EvidenceMenuKeyString);

	AddCaptureKey(EscapeMenuKey);
	AddCaptureKey(TabMenuKey);
	AddCaptureKey(RecordingMenuKey);
	AddCaptureKey(EvidenceMenuKey);
}

function OnKeyBindingsChanged()
{
	ClearCaptureKeys();
	AddCaptureKeys();
}

/** 
 *  Configuration method which stores references to _root and
 *  _root.manager for use in attaching views.
 */
final function ConfigFrontEnd()
{ 
	//`log("ConfigFrontEnd() executed.");
	RootMC = GetVariableObject("_root");
	ManagerMC = RootMC.GetObject("manager");

	bInitialized = true;
}

/** 
 *  Creates MovieClips in ManagerMC into which the views are loaded. These MovieClips
 *  are then stored in MenuViews for later manipulation.
 */
final function LoadViews()
{
	local byte i;
	for (i = 0; i < ViewData.Length; i++) 
	{
		//`log("Loading View: "@ViewData[i].ViewName);
		LoadView( ViewData[i] );
	}
}

/** 
 *  Create a view using existing ViewInfo. 
 *
 *  @param InViewInfo, the data for the view which includes the SWFName and the name for the view.
 */
final function LoadView(ViewInfo InViewInfo)
{
	local ASValue asval;
	local array<ASValue> args;
	local GFxObject ViewContainer, ViewLoader;

	ViewContainer = ManagerMC.CreateEmptyMovieClip( String(InViewInfo.ViewName) $ "Container" );
	ViewLoader = ViewContainer.CreateEmptyMovieClip( String(InViewInfo.ViewName) );

	asval.Type = AS_String;
	asval.s = InViewInfo.SWFName;
	args[0] = asval;

	ViewContainer.SetVisible( false );
	ViewLoader.Invoke( "loadMovie", args );
	LoadedViews.AddItem( InViewInfo.ViewName );
	//`log("Loaded View: "@InViewInfo.ViewName);
}

/** Pushes a view onto MenuManager.as view stack. */
function PushView(coerce OLUIFrontEnd_View targetView) 
{   
	//`log("PushView() executed");
    ActionScriptVoid("pushStandardView"); 
}

final function PushViewByName(name TargetViewName, optional OLUIFrontEnd_Screen ParentView)
{
	//`log( "GFxUDKFrontEnd::PushViewByName(" @ string(TargetViewName) @ ")",,'DevUI');    
	switch (TargetViewName)
	{
		case ( 'Options' ): 
			ConfigureTargetView( OptionsView ); 
			break;
		case ( 'GammaScreen' ):
			ConfigureTargetView( GammaScreenView );
			break;
		case ( 'GamepadScreen' ):
			ConfigureTargetView( GamepadScreenView );
			break;
		case ( 'LoadGame' ):
			ConfigureTargetView( LoadGameView );
			break;
		case ( 'RecordingList' ):
			ConfigureTargetView( RecordingListView );
			break;
		case ( 'RecordingView' ):
			ConfigureTargetView( RecordingView );
			break;
		case ( 'EvidenceList' ):
			ConfigureTargetView( EvidenceListView );
			break;
		case ( 'EvidenceView' ):
			ConfigureTargetView( EvidenceView );
			break;
		case ( 'Credits' ):
			ConfigureTargetView( CreditsView );
			break;
		case ( 'DifficultySelectionScreen' ):
			ConfigureTargetView( DifficultySelectionView );
			break;
		case ( 'ChapterSelection' ):
			ConfigureTargetView( ChapterSelection );
			break;
		case ( 'SaveLocationList' ):
			ConfigureTargetView( SaveLocationList );
			break;
		default:
			//`log( "View ["$TargetViewName$"] not found." ,,'DevUI');  
			break;
	}
}

/** Pops a view from the view stack and handles update/close of existing views. */
function GFxObject PopView() 
{       
    if ( ViewStack.Length <= 1 ) 
    {
        return none;
    }

	ViewStack[ViewStack.Length-1].OnViewDeactivated();

    // Remove the view from the stack in US so we know what's still on top.   
    ViewStack.Remove(ViewStack.Length-1, 1);     

    // Update the new top most view.    
    ViewStack[ViewStack.Length-1].OnTopMostView( false ); 

    return PopViewStub();
}

/** Pops a view from the MenuManager.as view stack. */
final function GFxObject PopViewStub() { return ActionScriptObject("popView"); }

final function ConfigureView(OLUIFrontEnd_View InView, name WidgetName, name WidgetPath)
{	
	//`log("ConfigureView() executed");
	
	// All widget's found in the view's path will send their initCallBack events to the view's WidgetInitialized() handler.
    SetWidgetPathBinding(InView, WidgetPath);
	
    InView.MenuManager = self;
    InView.ViewName = WidgetName;
    InView.OnViewLoaded();
}

/** 
 * Activates, updates, and pushes a view on the stack if it is allowed.
 * This method is called when a view is created by name using PushViewByName().
 */
function ConfigureTargetView(OLUIFrontEnd_View TargetView)
{
	//`log("ConfigureTargetView() executed");
    TargetView.OnViewActivated();
    TargetView.OnTopMostView( true );

    ViewStack.AddItem( TargetView );
    PushView( TargetView );      
}

event bool WidgetInitialized(name WidgetName, name WidgetPath, GFxObject Widget)
{    
	local bool bResult;

	bResult = false;

    switch(WidgetName)
    {           
        case ('MainMenu'):
            if (MainMenuView == none)
            {
                MainMenuView = OLUIFrontEnd_MainMenu(Widget);
                ConfigureView(MainMenuView, WidgetName, WidgetPath);

				if (MenuType == EMT_MainMenu)
				{
					ConfigureTargetView(MainMenuView);
				}
                bResult = true;
            }            
            break;
		case ('Options'):
            if (OptionsView == none)
            {
					OptionsView = OLUIFrontEnd_Options(Widget);
					ConfigureView(OptionsView, WidgetName, WidgetPath);
					bResult = true;
            }
			break;
		case ('GammaScreen'):
			if (GammaScreenView == none)
			{
				GammaScreenView = OLUIFrontEnd_Screen(Widget);
				ConfigureView(GammaScreenView, WidgetName, WidgetPath);

				if (MenuType == EMT_MainMenu && !HasInitializedGamma())
				{
					ConfigureTargetView(GammaScreenView);
				}
				bResult = true;
			}
			break;
		case ('DifficultySelectionScreen'):
			if (DifficultySelectionView == none)
			{
				DifficultySelectionView = OLUIFrontEnd_DifficultySelectionScreen(Widget);
				ConfigureView(DifficultySelectionView, WidgetName, WidgetPath);
				bResult = true;
			}
			break;
		case ('ChapterSelection'):
			if (ChapterSelection == none)
			{
				ChapterSelection = OLUIFrontEnd_ChapterSelection(Widget);
				ConfigureView(ChapterSelection, WidgetName, WidgetPath);
				bResult = true;
			}
			break;
		case ('GamepadScreen'):
			if (GamepadScreenView == none)
			{
				GamepadScreenView = OLUIFrontEnd_Screen(Widget);
				ConfigureView(GamepadScreenView, WidgetName, WidgetPath);
				bResult = true;
			}
			break;
		case ('PauseMenu'):
			if (PauseMenuView == none)
			{
				PauseMenuView = OLUIFrontEnd_PauseMenu(Widget);
				ConfigureView(PauseMenuView, WidgetName, WidgetPath);

				if (MenuType == EMT_PauseMenu)
				{
					ConfigureTargetView(PauseMenuView);                 
				}
				bResult = true;
			}
			break;		
		case ('TabMenu'):
			if (TabMenuView == none)
			{
				TabMenuView = OLUIFrontEnd_TabMenu(Widget);
				ConfigureView(TabMenuView, WidgetName, WidgetPath);

				if (MenuType == EMT_TabMenu || MenuType == EMT_EvidenceMenu || MenuType == EMT_RecordingMenu)
				{
					ConfigureTargetView(TabMenuView);
				}
				bResult = true;
			}
			break;
		case ('LoadGame'):
			if (LoadGameView == none)
			{
				LoadGameView = OLUIFrontEnd_LoadGame(Widget);
				ConfigureView(LoadGameView, WidgetName, WidgetPath);
				bResult = true;
			}
			break;
		case ('RecordingList'):
			if (RecordingListView == none)
			{
				RecordingListView = OLUIFrontEnd_RecordingList(Widget);
				ConfigureView(RecordingListView, WidgetName, WidgetPath);

				if (MenuType == EMT_RecordingMenu)
				{
					ConfigureTargetView(RecordingListView);
				}
				bResult = true;
			}
			break;
		case ('RecordingView'):
			if (RecordingView == none)
			{
				RecordingView = OLUIFrontEnd_Screen(Widget);
				ConfigureView(RecordingView, WidgetName, WidgetPath);

				if (MenuType == EMT_RecordingMenu && GetOLPC().HUD.LatestRecordingTimer > 0.f)
				{
					OLUIFrontEnd_RecordingView(RecordingView).Recording = GetOLPC().HUD.LatestRecordingName;
					ConfigureTargetView(RecordingView);
				}

				bResult = true;
			}
			break;
		case ('EvidenceList'):
			if (EvidenceListView == none)
			{
				EvidenceListView = OLUIFrontEnd_EvidenceList(Widget);
				ConfigureView(EvidenceListView, WidgetName, WidgetPath);
				
				if (MenuType == EMT_EvidenceMenu)
				{
					ConfigureTargetView(EvidenceListView);
				}
				bResult = true;
			}
			break;
		case ('EvidenceView'):
			if (EvidenceView == none)
			{
				EvidenceView = OLUIFrontEnd_Screen(Widget);
				ConfigureView(EvidenceView, WidgetName, WidgetPath);

				if (MenuType == EMT_EvidenceMenu && GetOLPC().HUD.LatestDocumentTimer > 0.f)
				{
					OLUIFrontEnd_EvidenceView(EvidenceView).Collectible = GetOLPC().HUD.LatestDocumentName;
					ConfigureTargetView(EvidenceView);
				}
				bResult = true;
			}
			break;
		case ('Credits'):
			if (CreditsView == none)
			{
				CreditsView = OLUIFrontEnd_Credits(Widget);
				ConfigureView(CreditsView, WidgetName, WidgetPath);

				if (MenuType == EMT_Credits)
				{
					CreditsView.bGameOver = true;
					ConfigureTargetView(CreditsView);
				}
				bResult = true;
			}
			break;
        default:
            break;
    }
    return bResult;
}

event bool FilterButtonInput(int ControllerId, name ButtonName, EInputEvent InputEvent)
{
	if (bCapturingKeyBinding)
	{
		if (class'OLUtils'.static.IsBindableKey(ButtonName))
		{
			if (InputEvent == IE_Pressed)
			{
				ViewStack[ViewStack.Length-1].OnKeyBindingCaptured(ButtonName);
			}
			else if (InputEvent == IE_Released)
			{
				bCapturingKeyBinding = false;
			}
		}
		return true;
	}

	if (ViewStack.length > 0 && ViewStack[ViewStack.length -1].FilterButtonInput(ControllerId, ButtonName, InputEvent))
	{
		return true;
	}

	if (InputEvent == IE_Pressed)
	{
		if (ButtonName == 'Escape' || ButtonName == 'XboxTypeS_B')
		{		
			return ViewStack[ViewStack.length -1].Back();
		}
	}

	return false;
}

function StartKeyBindingCapture()
{
	bCapturingKeyBinding = true;
}

function string GetFriendlyKeyBindingName(string KeyName)
{
	if (Len(KeyName) > 0)
	{
		return Localize("InputKeys", KeyName, "OLGame");
	}
	else
	{
		return "";
	}
}

function bool HasInitializedGamma()
{
	local int GammaInitialized;
	local OLProfileSettings MyProfile;

	MyProfile = GetOLPC().ProfileSettings;
	MyProfile.GetProfileSettingValueId(PSI_GammaInitialized, GammaInitialized);

	return (GammaInitialized == PSB_On);
}

function string GetGamepadActionBoundToKey(string KeyNameString, int ConfigType)
{
	local name KeyName;
	local OLPlayerInput PlayerInput;
	local int BindIndex;
	local string ActionString;
	local array<KeyBind> GPBindings;

	KeyName = name(KeyNameString);
	PlayerInput = OLPlayerInput(GetOLPC().PlayerInput);
	switch (ConfigType)
	{
		case 0:
			GPBindings = PlayerInput.GPBindingsA;
			break;
		case 1:
			GPBindings = PlayerInput.GPBindingsB;
			break;
		case 2:
			GPBindings = PlayerInput.GPBindingsC;
			break;
	}

	BindIndex = GPBindings.Find('Name', KeyName);
	if (BindIndex >= 0)
	{
		if (Len(GPBindings[BindIndex].Command) > 0)
		{
			ActionString = Localize("InputActions", GPBindings[BindIndex].Command, "OLGame");
		}
	}
	return ActionString;
}

function string GetLocalizedString(string Category, string KeyName, string File)
{
	return Localize(Category, KeyName, File);
}

defaultproperties
{    
	//Views
	WidgetBindings.Add((WidgetName="MainMenu",WidgetClass=class'OLUIFrontEnd_MainMenu'))
	WidgetBindings.Add((WidgetName="PauseMenu",WidgetClass=class'OLUIFrontEnd_PauseMenu'))
	WidgetBindings.Add((WidgetName="Options",WidgetClass=class'OLUIFrontEnd_Options'))
	WidgetBindings.Add((WidgetName="LoadGame",WidgetClass=class'OLUIFrontEnd_LoadGame'))
	WidgetBindings.Add((WidgetName="GammaScreen",WidgetClass=class'OLUIFrontEnd_GammaScreen'))
	WidgetBindings.Add((WidgetName="DifficultySelectionScreen",WidgetClass=class'OLUIFrontEnd_DifficultySelectionScreen'))
	WidgetBindings.Add((WidgetName="ChapterSelection",WidgetClass=class'OLUIFrontEnd_ChapterSelection'))
	WidgetBindings.Add((WidgetName="GamepadScreen",WidgetClass=class'OLUIFrontEnd_GamepadScreen'))
	WidgetBindings.Add((WidgetName="Credits",WidgetClass=class'OLUIFrontEnd_Credits'))
	WidgetBindings.Add((WidgetName="TabMenu",WidgetClass=class'OLUIFrontEnd_TabMenu'))
	WidgetBindings.Add((WidgetName="RecordingList",WidgetClass=class'OLUIFrontEnd_RecordingList'))
	WidgetBindings.Add((WidgetName="RecordingView",WidgetClass=class'OLUIFrontEnd_RecordingView'))
	WidgetBindings.Add((WidgetName="EvidenceList",WidgetClass=class'OLUIFrontEnd_EvidenceList'))
	WidgetBindings.Add((WidgetName="EvidenceView",WidgetClass=class'OLUIFrontEnd_EvidenceView'))

    //bDisplayWithHudOff = true   
	MenuType=EMT_MainMenu
   TimingMode = TM_Real
   bInitialized = false
	MovieInfo=SwfMovie'OLFrontEnd.Manager'
	bPauseGameWhileActive=false
	bCaptureInput=true
	bIgnoreMouseInput=false

	Priority=4

	SoundThemes(0)=(ThemeName=default,Theme=UISoundTheme'Menu.MenuUISoundTheme')
	SoundThemes(1)=(ThemeName=start,Theme=UISoundTheme'Menu.StartUISoundTheme')
	SoundThemes(2)=(ThemeName=major,Theme=UISoundTheme'Menu.MajorUISoundTheme')
}