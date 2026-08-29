/*=============================================================================
	Launch.cpp: Game launcher.
	Copyright 1998-2012 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "LaunchPrivate.h"
#include "NvApexManager.h"

#if ((PLATFORM_DESKTOP || DINGO))

#include "SplashScreen.h"
#if WITH_OPEN_AUTOMATE
#include "OpenAutomate.h"
#endif

#if HAVE_WXWIDGETS
// use wxWidgets as a DLL
#include <wx/evtloop.h>  // has our base callback class
#endif

#if ((_WINDOWS || DINGO) && defined(_DEBUG))
#include <crtdbg.h>
#endif

FEngineLoop	GEngineLoop;
/** Whether to use wxWindows when running the game */
UBOOL		GUsewxWindows = FALSE;

/** Whether we should pause before exiting. used by UCC */
UBOOL		GShouldPauseBeforeExit;

/** Whether we should generate crash reports even if the debugger is attached. */
UBOOL		GAlwaysReportCrash = FALSE;

extern "C" int test_main(int argc, char ** argp)
{
	return 0;
}

/*-----------------------------------------------------------------------------
	WinMain.
-----------------------------------------------------------------------------*/

#if (_WINDOWS || DINGO)
extern TCHAR MiniDumpFilenameW[1024];
extern INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );

// use wxWidgets as a DLL
extern bool IsUnrealWindowHandle( HWND hWnd );
#endif



/**
 * Performs any required cleanup in the case of a fatal error.
 */
static void StaticShutdownAfterError()
{
	// Make sure Novodex is correctly torn down.
	DestroyGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is shutdown.
	UnShutdownFaceFX();
#endif // WITH_FACEFX

#if HAVE_WXWIDGETS
	// Unbind DLLs (e.g. SCC integration)
	WxLaunchApp* LaunchApp = (WxLaunchApp*) wxTheApp;
	if( LaunchApp )
	{
		LaunchApp->ShutdownAfterError();
	}
#endif
}



#if HAVE_WXWIDGETS
class WxUnrealCallbacks : public wxUnrealCallbacks
{
public:

	virtual bool IsUnrealWindowHandle(HWND hwnd) const
	{
		return ::IsUnrealWindowHandle(hwnd);
	}

	virtual bool IsRequestingExit() const 
	{ 
		return GIsRequestingExit ? true : false; 
	}

	virtual void SetRequestingExit( bool bRequestingExit ) 
	{ 
		GIsRequestingExit = bRequestingExit ? true : false; 
	}


	/** Called by WxWidgets if it catches an SEH exception within a WndProc */
	virtual INT WndProcExceptionFilter( LPEXCEPTION_POINTERS ExceptionInfo )
	{
		if( IsDebuggerPresent() )
		{
			// Break on exception thrown from WxWidgets as WndProc will sometimes swallow the
			// exception and we otherwise wouldn't get a chance to break.  Note that this is
			// likely a fatal error and if you opt to continue execution, state may be corrupt!
			appDebugBreak();
			return EXCEPTION_CONTINUE_SEARCH;
		}

		return CreateMiniDump( ExceptionInfo );
	}



	/** Called for unhandled exceptions from WxWidgets WndProc functions */
	virtual void WndProcUnhandledExceptionCallback()
	{
		GError->HandleError();
		StaticShutdownAfterError();
	}

};

static WxUnrealCallbacks s_UnrealCallbacks;
#endif //HAVE_WXWIDGETS

#if HAVE_WXWIDGETS && !SHIPPING_PC_GAME
#include <wx/hyperlink.h>

// Helper: routes any button click to EndModal(button_id), closes on X.
class WxLauncherHandler : public wxEvtHandler
{
	wxDialog*     Dlg;
	wxCheckBox*   ChkLog;
	wxCheckBox*   ChkWindowed;
	wxCheckBox*   ChkNoSteam;
	wxCheckBox*   ChkNoHomedir;
	wxStaticText* CmdLabel;
	int           PlayId;
	int           EditorId;
	wxString      BaseArgs;   // original args without the managed flags
public:
	WxLauncherHandler( wxDialog* InDlg, wxCheckBox* InLog, wxCheckBox* InWindowed,
		wxCheckBox* InNoSteam, wxCheckBox* InNoHomedir,
		wxStaticText* InLabel, int InPlayId, int InEditorId, const wxString& InBase )
		: Dlg(InDlg), ChkLog(InLog), ChkWindowed(InWindowed)
		, ChkNoSteam(InNoSteam), ChkNoHomedir(InNoHomedir), CmdLabel(InLabel)
		, PlayId(InPlayId), EditorId(InEditorId), BaseArgs(InBase) {}

	void OnButton( wxCommandEvent& Event )
	{
		Dlg->EndModal( Event.GetId() );
	}
	void OnClose( wxCloseEvent& )
	{
		Dlg->EndModal( wxID_NONE );
	}
	void OnCheckbox( wxCommandEvent& )
	{
		UpdateLabel();
	}
	void UpdateLabel()
	{
		wxString Cmd = wxT("OLGame.exe");
		if( !BaseArgs.IsEmpty() ) { Cmd += wxT(" "); Cmd += BaseArgs; }
		if( ChkLog->GetValue()       ) Cmd += wxT(" -log");
		if( ChkWindowed->GetValue()  ) Cmd += wxT(" -WINDOWED");
		Cmd += wxT(" -nosteam");
		if( ChkNoHomedir->GetValue() ) Cmd += wxT(" -nohomedir");
		CmdLabel->SetLabel( Cmd );
		CmdLabel->Wrap( CmdLabel->GetParent()->GetClientSize().GetWidth() - 20 );
		CmdLabel->GetParent()->Layout();
		CmdLabel->GetParent()->Fit();
	}
	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE( WxLauncherHandler, wxEvtHandler )
	EVT_BUTTON(    wxID_ANY, WxLauncherHandler::OnButton  )
	EVT_CLOSE(              WxLauncherHandler::OnClose   )
	EVT_CHECKBOX(  wxID_ANY, WxLauncherHandler::OnCheckbox )
END_EVENT_TABLE()

// Returns updated CmdLine (static buffer), or NULL if user cancelled.
struct FLauncherPrefs
{
	bool bLog;
	bool bWindowed;
	bool bNoSteam;
	bool bNoHomedir;
};

static void GetLauncherPrefsPath( TCHAR* OutPath, int MaxLen )
{
	GetModuleFileNameW( NULL, OutPath, MaxLen );
	// Replace filename after last backslash with "launcher.ini"
	TCHAR* LastSlash = OutPath;
	for( TCHAR* C = OutPath; *C; C++ )
		if( *C == TEXT('\\') || *C == TEXT('/') ) LastSlash = C;
	wcscpy( LastSlash + 1, TEXT("launcher.ini") );
}

static FLauncherPrefs LoadLauncherPrefs()
{
	TCHAR Path[1024];
	GetLauncherPrefsPath( Path, ARRAY_COUNT(Path) );
	FLauncherPrefs P;
	P.bLog      = !!GetPrivateProfileIntW( L"Launcher", L"log",      0, Path );
	P.bWindowed = !!GetPrivateProfileIntW( L"Launcher", L"windowed", 0, Path );
	P.bNoSteam  = !!GetPrivateProfileIntW( L"Launcher", L"nosteam",  0, Path );
	P.bNoHomedir= !!GetPrivateProfileIntW( L"Launcher", L"nohomedir",0, Path );
	return P;
}

static void SaveLauncherPrefs( const FLauncherPrefs& P )
{
	TCHAR Path[1024];
	GetLauncherPrefsPath( Path, ARRAY_COUNT(Path) );
	WritePrivateProfileStringW( L"Launcher", L"log",      P.bLog       ? L"1" : L"0", Path );
	WritePrivateProfileStringW( L"Launcher", L"windowed", P.bWindowed  ? L"1" : L"0", Path );
	WritePrivateProfileStringW( L"Launcher", L"nosteam",  P.bNoSteam   ? L"1" : L"0", Path );
	WritePrivateProfileStringW( L"Launcher", L"nohomedir",P.bNoHomedir ? L"1" : L"0", Path );
}

static const TCHAR* ShowLaunchModeDialog( const TCHAR* CmdLine )
{
	const TCHAR* Args = RemoveExeName( CmdLine );
	while( *Args == ' ' || *Args == '\t' ) Args++;
	// A commandlet is a bare word (no leading '-'). Map URLs contain '?' or end with a known
	// extension, so exclude them — they are not commandlets.
	UBOOL bIsCommandlet = FALSE;
	if( *Args && *Args != TEXT('-') && *Args != TEXT('/') && *Args != TEXT('"') )
	{
		const TCHAR* End = Args;
		while( *End && *End != TEXT(' ') && *End != TEXT('\t') ) End++;
		bIsCommandlet = TRUE;
		for( const TCHAR* C = Args; C < End; C++ )
		{
			if( *C == TEXT('?') || *C == TEXT('.') || *C == TEXT('\\') )
			{
				bIsCommandlet = FALSE;
				break;
			}
		}
	}

	if( bIsCommandlet
		|| ParseParam(CmdLine, TEXT("SEEKFREELOADINGPCCONSOLE"))
		|| ParseParam(CmdLine, TEXT("EDITOR"))
		|| ParseParam(CmdLine, TEXT("SERVER"))
		|| ParseParam(CmdLine, TEXT("SILENT"))
		|| ParseParam(CmdLine, TEXT("INSTALLED")) )
	{
		return CmdLine;
	}

	// Create a minimal wxApp just to host the dialog.
	// Do NOT call wxEntryCleanup — wx will be re-entered properly by wxEntry() later.
	wxApp* TempApp = new wxApp();
	wxApp::SetInstance( TempApp );
	int ArgC = 0;
	wxEntryStart( ArgC, (wxChar**)NULL );
	TempApp->OnInit();

	enum { ID_PLAY = wxID_HIGHEST + 1, ID_EDITOR };

	// Launcher dialog
	wxDialog Dlg( NULL, wxID_ANY, wxT("OpenOL"), wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP );
	Dlg.SetBackgroundColour( wxColour(30, 30, 30) );

	wxBoxSizer* Root = new wxBoxSizer( wxVERTICAL );

	// Title
	wxStaticText* Title = new wxStaticText( &Dlg, wxID_ANY, wxT("OpenOL"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL );
	wxFont TitleFont = Title->GetFont();
	TitleFont.SetPointSize( 28 );
	TitleFont.SetWeight( wxFONTWEIGHT_BOLD );
	Title->SetFont( TitleFont );
	Title->SetForegroundColour( wxColour(255, 255, 255) );
	Root->Add( Title, 0, wxALIGN_CENTRE | wxTOP | wxLEFT | wxRIGHT, 20 );

	// Links row (same width as title text)
	wxBoxSizer* LinkRow = new wxBoxSizer( wxHORIZONTAL );
	wxHyperlinkCtrl* LinkGithub = new wxHyperlinkCtrl( &Dlg, wxID_ANY,
		wxT("Github"), wxT("https://github.com/ShyKiss/OpenOL") );
	wxHyperlinkCtrl* LinkSteam  = new wxHyperlinkCtrl( &Dlg, wxID_ANY,
		wxT("Steam"),  wxT("https://steamcommunity.com/id/beerymaid") );
	LinkGithub->SetNormalColour( wxColour(100, 160, 255) );
	LinkGithub->SetVisitedColour( wxColour(100, 160, 255) );
	LinkSteam->SetNormalColour( wxColour(100, 160, 255) );
	LinkSteam->SetVisitedColour( wxColour(100, 160, 255) );
	// Size the link row to match the title label width, measured after font is set
	int TitleW = 0, TitleH = 0;
	Title->GetTextExtent( wxT("OpenOL"), &TitleW, &TitleH );
	LinkRow->SetMinSize( TitleW, -1 );
	LinkRow->Add( LinkGithub, 1, wxEXPAND | wxLEFT | wxRIGHT, 4 );
	LinkRow->Add( LinkSteam,  1, wxEXPAND | wxLEFT | wxRIGHT, 4 );
	Root->Add( LinkRow, 0, wxALIGN_CENTRE | wxTOP, 4 );

	// Parse args after exe, respecting quoted tokens (e.g. "map?opt -flag").
	// Each quoted token is expanded: outer quotes stripped, inner flags detected individually.
	// BaseArgs = all args with the four managed flags removed; bHas* = whether they were present.
	bool bHasLog      = false;
	bool bHasWindowed = false;
	bool bHasNoSteam  = false;
	bool bHasNoHomedir= false;
	wxString BaseArgs;
	{
		const TCHAR* P = RemoveExeName( CmdLine );
		while( *P )
		{
			while( *P == ' ' || *P == '\t' ) P++;
			if( !*P ) break;

			// Collect one shell token (quoted or unquoted)
			wxString Tok;
			if( *P == TEXT('"') )
			{
				P++; // skip opening quote
				const TCHAR* TokStart = P;
				while( *P && *P != TEXT('"') ) P++;
				Tok = wxString( TokStart, (size_t)(P - TokStart) );
				if( *P == TEXT('"') ) P++; // skip closing quote
			}
			else
			{
				const TCHAR* TokStart = P;
				while( *P && *P != ' ' && *P != '\t' ) P++;
				Tok = wxString( TokStart, (size_t)(P - TokStart) );
			}

			// A quoted token may itself contain space-separated sub-tokens (e.g. "map?x -log").
			// Split it and classify each piece.
			wxString Remaining = Tok;
			wxString NonFlagPieces;
			while( !Remaining.IsEmpty() )
			{
				wxString Piece;
				int SpacePos = Remaining.Find( wxT(' ') );
				if( SpacePos == wxNOT_FOUND )
				{
					Piece = Remaining;
					Remaining.Clear();
				}
				else
				{
					Piece = Remaining.Left( SpacePos );
					Remaining = Remaining.Mid( SpacePos + 1 );
				}
				if( Piece.IsEmpty() ) continue;

				// Check if this piece is one of our managed flags
				wxString PieceLower = Piece.Lower();
				if( PieceLower.StartsWith(wxT("-")) || PieceLower.StartsWith(wxT("/")) )
					PieceLower = PieceLower.Mid(1);
				if( PieceLower == wxT("log") )           { bHasLog       = true; continue; }
				if( PieceLower == wxT("windowed") )      { bHasWindowed  = true; continue; }
				if( PieceLower == wxT("nosteam") )       { bHasNoSteam   = true; continue; }
				if( PieceLower == wxT("nohomedir") )     { bHasNoHomedir = true; continue; }

				if( !NonFlagPieces.IsEmpty() ) NonFlagPieces += wxT(" ");
				NonFlagPieces += Piece;
			}

			if( !NonFlagPieces.IsEmpty() )
			{
				if( !BaseArgs.IsEmpty() ) BaseArgs += wxT(" ");
				// Re-quote if the original token was quoted (contained spaces or special chars)
				if( NonFlagPieces.Find(wxT(' ')) != wxNOT_FOUND )
				{
					BaseArgs += wxT("\"");
					BaseArgs += NonFlagPieces;
					BaseArgs += wxT("\"");
				}
				else
				{
					BaseArgs += NonFlagPieces;
				}
			}
		}
	}

	// Load saved prefs; use them only for flags not already present in CmdLine
	{
		const FLauncherPrefs Saved = LoadLauncherPrefs();
		if( !bHasLog       ) bHasLog       = Saved.bLog;
		if( !bHasWindowed  ) bHasWindowed  = Saved.bWindowed;
		if( !bHasNoSteam   ) bHasNoSteam   = Saved.bNoSteam;
		if( !bHasNoHomedir ) bHasNoHomedir = Saved.bNoHomedir;
	}

	// Checkboxes row (pre-checked from CmdLine flags or saved prefs)
	wxBoxSizer* CheckRow = new wxBoxSizer( wxHORIZONTAL );
	wxCheckBox* ChkLog      = new wxCheckBox( &Dlg, wxID_ANY, wxT("-log") );
	wxCheckBox* ChkWindowed = new wxCheckBox( &Dlg, wxID_ANY, wxT("-WINDOWED") );
	wxCheckBox* ChkNoHomedir= new wxCheckBox( &Dlg, wxID_ANY, wxT("-nohomedir") );
	ChkLog->SetForegroundColour( wxColour(200, 200, 200) );
	ChkWindowed->SetForegroundColour( wxColour(200, 200, 200) );
	ChkNoHomedir->SetForegroundColour( wxColour(200, 200, 200) );
	CheckRow->Add( ChkLog,       0, wxALL, 6 );
	CheckRow->Add( ChkWindowed,  0, wxALL, 6 );
	CheckRow->Add( ChkNoHomedir, 0, wxALL, 6 );
	Root->Add( CheckRow, 0, wxALIGN_CENTRE | wxTOP, 14 );

	// Buttons row
	wxBoxSizer* BtnRow = new wxBoxSizer( wxHORIZONTAL );
	wxButton* BtnPlay   = new wxButton( &Dlg, ID_PLAY,   wxT("Play with Seekfree"),  wxDefaultPosition, wxSize(200, 40) );
	wxButton* BtnEditor = new wxButton( &Dlg, ID_EDITOR, wxT("Play with Content"), wxDefaultPosition, wxSize(200, 40) );
	BtnRow->Add( BtnPlay,   0, wxALL, 8 );
	BtnRow->Add( BtnEditor, 0, wxALL, 8 );
	Root->Add( BtnRow, 0, wxALIGN_CENTRE | wxTOP, 6 );

	// Command line preview label
	wxStaticText* CmdLabel = new wxStaticText( &Dlg, wxID_ANY, wxT("OLGame.exe"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL );
	CmdLabel->SetForegroundColour( wxColour(140, 140, 140) );
	Root->Add( CmdLabel, 0, wxALIGN_CENTRE | wxALL, 10 );

	Dlg.SetSizer( Root );
	Root->Fit( &Dlg );
	Dlg.Centre();
	Dlg.SetEscapeId( wxID_NONE );

	// Apply initial checkbox state after layout so native Win32 controls are realized
	ChkLog->SetValue( bHasLog );
	ChkWindowed->SetValue( bHasWindowed );
	ChkNoHomedir->SetValue( bHasNoHomedir );

	// Push event handler: buttons → EndModal(id), X → EndModal(wxID_NONE), checkboxes → update label.
	WxLauncherHandler Handler( &Dlg, ChkLog, ChkWindowed, nullptr, ChkNoHomedir,
		CmdLabel, ID_PLAY, ID_EDITOR, BaseArgs );
	Dlg.PushEventHandler( &Handler );
	Handler.UpdateLabel();

	const int ModalResult = Dlg.ShowModal();
	Dlg.PopEventHandler( false );
	const bool bLog       = ChkLog->GetValue();
	const bool bWindowed  = ChkWindowed->GetValue();
	const bool bNoSteam   = true;
	const bool bNoHomedir = ChkNoHomedir->GetValue();

	// Save checkbox state for next launch (always, even on cancel)
	{
		FLauncherPrefs P;
		P.bLog      = bLog;
		P.bWindowed = bWindowed;
		P.bNoSteam  = bNoSteam;
		P.bNoHomedir= bNoHomedir;
		SaveLauncherPrefs( P );
	}

	wxApp::SetInstance( NULL );
	delete TempApp;

	// X or unknown = exit
	if( ModalResult != ID_PLAY && ModalResult != ID_EDITOR )
		return NULL;

	const int Choice = ( ModalResult == ID_PLAY ) ? 1 : 2;

	// Rebuild command line from scratch using BaseArgs (managed flags stripped),
	// then append only the flags the user selected via checkboxes.
	static TCHAR NewCmdLine[4096];
	// Copy exe token (quoted or unquoted) from the original CmdLine
	{
		const TCHAR* ExeEnd = CmdLine;
		if( *ExeEnd == TEXT('"') )
		{
			ExeEnd++;
			while( *ExeEnd && *ExeEnd != TEXT('"') ) ExeEnd++;
			if( *ExeEnd ) ExeEnd++; // include closing quote
		}
		else
		{
			while( *ExeEnd && *ExeEnd != ' ' && *ExeEnd != '\t' ) ExeEnd++;
		}
		const int ExeLen = (int)(ExeEnd - CmdLine);
		appStrncpy( NewCmdLine, CmdLine, ExeLen + 1 );
		NewCmdLine[ExeLen] = 0;
	}
	// Append base args (already stripped of managed flags)
	if( !BaseArgs.IsEmpty() )
	{
		appStrncat( NewCmdLine, TEXT(" "), ARRAY_COUNT(NewCmdLine) );
		appStrncat( NewCmdLine, BaseArgs.c_str(), ARRAY_COUNT(NewCmdLine) );
	}
	if( Choice == 1 )
		appStrncat( NewCmdLine, TEXT(" -seekfreeloadingpcconsole"), ARRAY_COUNT(NewCmdLine) );
	if( bLog )
		appStrncat( NewCmdLine, TEXT(" -log"), ARRAY_COUNT(NewCmdLine) );
	if( bWindowed )
		appStrncat( NewCmdLine, TEXT(" -WINDOWED"), ARRAY_COUNT(NewCmdLine) );
	if( bNoSteam )
		appStrncat( NewCmdLine, TEXT(" -nosteam"), ARRAY_COUNT(NewCmdLine) );
	if( bNoHomedir )
		appStrncat( NewCmdLine, TEXT(" -nohomedir"), ARRAY_COUNT(NewCmdLine) );
	return NewCmdLine;
}
#endif // HAVE_WXWIDGETS && !SHIPPING_PC_GAME

/**
 * Sets global to TRUE if the app should pause infinitely before exit.
 * Currently used by UCC.
 */
void SetShouldPauseBeforeExit(INT ErrorLevel)
{
	// If we are UCC, determine 
	if( GIsUCC )
	{
		// UCC.
		UBOOL bInheritConsole = FALSE;

#if !CONSOLE
		if(NULL != GLogConsole)
		{
			// if we're running from a console we inherited, do not sleep indefinitely
			bInheritConsole = GLogConsole->IsInherited();
		}
#endif

		// Either close log window manually or press CTRL-C to exit if not in "silent" or "nopause" mode.
		GShouldPauseBeforeExit = !bInheritConsole && !GIsSilent && !ParseParam(appCmdLine(),TEXT("NOPAUSE"));
		// if it was specified to not pause if successful, then check that here
		if (ParseParam(appCmdLine(),TEXT("NOPAUSEONSUCCESS")) && ErrorLevel == 0)
		{
			// we succeeded, so don't pause 
			GShouldPauseBeforeExit = FALSE;
		}
	}
}

/** 
 * PreInits the engine loop 
 */
INT EnginePreInit( const TCHAR* CmdLine )
{
	INT ErrorLevel = GEngineLoop.PreInit( CmdLine );

	SetShouldPauseBeforeExit( ErrorLevel );

	return( ErrorLevel );
}

/** 
 * Inits the engine loop 
 */
INT EngineInit( const TCHAR* SplashName )
{
#if !DINGO
	appShowSplash( SplashName );
#endif //!DINGO

	INT ErrorLevel = GEngineLoop.Init();

#if !DINGO
	if ( !GIsGame )
	{
		appHideSplash();
	}
#endif //!DINGO

	return( ErrorLevel );
}

/** 
 * Ticks the engine loop 
 */
void EngineTick( void )
{
	GEngineLoop.Tick();
}

/**
 * Shuts down the engine
 */
void EngineExit( void )
{
	// Make sure this is set
	GIsRequestingExit = TRUE;

	GEngineLoop.Exit();
}

/**
 * Static guarded main function. Rolled into own function so we can have error handling for debug/ release builds depending
 * on whether a debugger is attached or not.
 */
INT GuardedMain( const TCHAR* CmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstance, INT nCmdShow )
{
	// For unix based OS's, it is >essential< that this is called as early on in the process as possible;
	//	it determines the base directory by caching the current working directory (and the working directory is changed regularly later).
	appBaseDir();

	// make sure GEngineLoop::Exit() is always called.
	struct EngineLoopCleanupGuard 
	{ 
		~EngineLoopCleanupGuard()
		{
			EngineExit();
		}
	} CleanupGuard;

	// Set up minidump filename. We cannot do this directly inside main as we use an FString that requires 
	// destruction and main uses SEH.
	// These names will be updated as soon as the Filemanager is set up so we can write to the log file.
	// That will also use the user folder for installed builds so we don't write into program files or whatever.
#if _WINDOWS
	appStrcpy( MiniDumpFilenameW, *FString::Printf( TEXT("unreal-v%i-%s.dmp"), GEngineVersion, *appSystemTimeString() ) );

	CmdLine = RemoveExeName(CmdLine);
#endif // _WINDOWS

#if DEDICATED_SERVER
	//Made to match size of GCmdLine in UnMisc.cpp
	TCHAR ServerCmdLine[16384];
	ServerCmdLine[0] = '\0';
	//Inject server commandlet onto commandline
	appStrncat( ServerCmdLine, TEXT("SERVER "), ARRAY_COUNT(ServerCmdLine) );
	appStrncat( ServerCmdLine, CmdLine, ARRAY_COUNT(ServerCmdLine) );
	CmdLine = ServerCmdLine;
#endif

#if WITH_STEAMWORKS
	extern void appSteamHandleCmdLine(const TCHAR** CmdLine);
	appSteamHandleCmdLine(&CmdLine);
#endif

#if WITH_OPEN_AUTOMATE
	if( ParseParam( CmdLine, TEXT( "OPENAUTOMATE"), TRUE ) )
	{
		GOpenAutomate = new FOpenAutomate();
	}
#endif // WITH_OPEN_AUTOMATE

	INT ErrorLevel = EnginePreInit( CmdLine );

	GUsewxWindows = 0;
#if HAVE_WXWIDGETS
	GUsewxWindows	= GIsEditor || ParseParam(appCmdLine(),TEXT("WXWINDOWS")) || ParseParam(appCmdLine(),TEXT("REMOTECONTROL"));
#endif

	// exit if PreInit failed.
	if ( ErrorLevel != 0 || GIsRequestingExit )
	{
		return ErrorLevel;
	}

	if( GUsewxWindows )
	{
#if HAVE_WXWIDGETS
		// use wxWidgets as a DLL
		// set the call back class here
		SetUnrealCallbacks( &s_UnrealCallbacks );

		// UnrealEd of game with wxWindows.
		ErrorLevel = wxEntry( hInInstance, hPrevInstance, "", nCmdShow);
#endif
	}
#if WITH_OPEN_AUTOMATE
	else if( ( GOpenAutomate != NULL ) && !GIsEditor )
	{
		ErrorLevel = EngineInit( TEXT( "PC\\Splash.bmp" ) );

		if( GOpenAutomate->Init( CmdLine ) )
		{
			GIsRequestingExit = GOpenAutomate->ProcessLoop();

			while( !GIsRequestingExit )
			{
				EngineTick();
			}
		}

		delete GOpenAutomate;
		GOpenAutomate = NULL;
	}
#endif // WITH_OPEN_AUTOMATE
	else
	{
#if PLATFORM_MACOSX
		ErrorLevel = EngineInit( TEXT("Mac\\Splash.bmp") );
#else
		// Game without wxWindows.
		ErrorLevel = EngineInit( GIsEditor ? TEXT("PC\\EdSplash.bmp") : TEXT("PC\\Splash.bmp") );
#endif

		while( !GIsRequestingExit )
		{
			EngineTick();
		}
	}
	return ErrorLevel;
}

#if (_WINDOWS || DINGO)
/**
 * Maintain a named mutex to detect whether we are the first instance of this game
 */
HANDLE GNamedMutex = NULL;

void ReleaseNamedMutex( void )
{
	if( GNamedMutex )
	{
		ReleaseMutex( GNamedMutex );
		GNamedMutex = NULL;
	}
}

UBOOL MakeNamedMutex( const TCHAR* CmdLine )
{
	UBOOL bIsFirstInstance = FALSE;

	TCHAR MutexName[MAX_SPRINTF] = TEXT( "" );
	appSprintf( MutexName, TEXT( "UnrealEngine3_%d" ), GAMENAME );

	GNamedMutex = CreateMutex( NULL, TRUE, MutexName );

	if( GNamedMutex	&& GetLastError() != ERROR_ALREADY_EXISTS && !ParseParam( CmdLine, TEXT( "NEVERFIRST" ) ) )
	{
		// We're the first instance!
		bIsFirstInstance = TRUE;
	}
	else
	{
		// Still need to release it in this case, because it gave us a valid copy
		ReleaseNamedMutex();
		// There is already another instance of the game running.
		bIsFirstInstance = FALSE;
	}

	return( bIsFirstInstance );
}

/**
 * Handler for CRT parameter validation. Triggers error
 *
 * @param Expression - the expression that failed crt validation
 * @param Function - function which failed crt validation
 * @param File - file where failure occured
 * @param Line - line number of failure
 * @param Reserved - not used
 */
void InvalidParameterHandler(const TCHAR* Expression,
							 const TCHAR* Function, 
							 const TCHAR* File, 
							 UINT Line, 
							 uintptr_t Reserved)
{
	appErrorf(TEXT("SECURE CRT: Invalid parameter detected.\nExpression: %s Function: %s. File: %s Line: %d\n"), 
		Expression ? Expression : TEXT("Unknown"), 
		Function ? Function : TEXT("Unknown"), 
		File ? File : TEXT("Unknown"), 
		Line );
}

/**
 * Setup the common debug settings 
 */
void SetupWindowsEnvironment( void )
{
	// all crt validation should trigger the callback
	_set_invalid_parameter_handler(InvalidParameterHandler);

#ifdef _DEBUG
	// Disable the message box for assertions and just write to debugout instead
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
	// don't fill buffers with 0xfd as we make assumptions for FNames st we only use a fraction of the entire buffer
	_CrtSetDebugFillThreshold( 0 );
#endif
}

#if WITH_MANAGED_CODE
// Implemented in ManagedCodeSupportCLR.cpp
extern INT ManagedGuardedMain( const TCHAR* CmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstance, INT nCmdShow );
#endif

/**
 * The inner exception handler catches crashes/asserts in native C++ code and is the only way to get the correct callstack
 * when running a 64-bit executable. However, XAudio2 doesn't always like this and it may result in no sound.
 */
#if _WIN64
	UBOOL GEnableInnerException = TRUE;
#else
	UBOOL GEnableInnerException = FALSE;
#endif

/**
 * Called from Managed code.
 * The inner exception handler catches crashes/asserts in native C++ code and is the only way to get the correct callstack
 * when running a 64-bit executable. However, XAudio2 doesn't like this and it may result in no sound.
 */
INT GuardedMainWrapper( const TCHAR* CmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstance, INT nCmdShow )
{
	INT ErrorLevel = 0;
	if ( GEnableInnerException )
	{
	 	__try
		{
			// Run the guarded code.
			ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
		}
		__except( CreateMiniDump( GetExceptionInformation() ), EXCEPTION_CONTINUE_SEARCH )
		{
		}
	}
	else
	{
		// Run the guarded code.
		ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
	}
	return ErrorLevel;
}

#if (WITH_WINRT || DINGO)

#if USE_WINRT_MAIN

#pragma warning(disable : 4946)	// reinterpret_cast used between related classes: 'Platform::Object' and ...

using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::ApplicationModel::Activation;

ref class ViewProvider : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
    ViewProvider();

    void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    void SetWindow(Windows::UI::Core::CoreWindow^ window);
    void Load(Platform::String^ entryPoint);
    void Run();
    void Uninitialize();

	void appWinPumpMessages();

private:
    Windows::UI::Core::CoreWindow^ m_window;
    Windows::ApplicationModel::Core::CoreApplicationView^ m_applicationView;
};

ViewProvider^ GViewProvider = nullptr;

ref class ViewProviderFactory : Windows::ApplicationModel::Core::IFrameworkViewSource 
{
public:
    ViewProviderFactory() {}
    Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        GViewProvider = ref new ViewProvider();
		return GViewProvider;
    }
};

ViewProvider::ViewProvider()
{
}

void ViewProvider::appWinPumpMessages()
{
	CoreDispatcher^ disp = m_window->Dispatcher;
	if (disp != nullptr)
	{
		disp->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
	}
}

void appWinPumpMessages()
{
	GViewProvider->appWinPumpMessages();
}

void ViewProvider::Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
{
    m_window = applicationView->CoreWindow;
	
	// Setup common Windows settings
	SetupWindowsEnvironment();

	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	INT ErrorLevel			= 0;
	GIsStarted				= 1;
	// how can we get this?
	hInstance				= NULL; // hInInstance
	const TCHAR* CmdLine	= GetCommandLine();
	
#if !SHIPPING_PC_GAME && !CONSOLE
	// Named mutex we use to figure out whether we are the first instance of the game running. This is needed to e.g.
	// make sure there is no contention when trying to save the shader cache.
	GIsFirstInstance = MakeNamedMutex( CmdLine );

	if ( ParseParam( CmdLine,TEXT("crashreports") ) )
	{
		GAlwaysReportCrash = TRUE;
	}
#endif

	// Using the -noinnerexception parameter will disable the exception handler within native C++, which is call from managed code,
	// which is called from this function.
	// The default case is to have three wrapped exception handlers (note that wxWidgets also use an exception handler, which would be a 4th one):
	// Native: WinMain() -> Managed: ManagedGuardedMain() -> Native: GuardedMainWrapper().
	// The inner exception handler in GuardedMainWrapper() catches crashes/asserts in native C++ code and is the only way to get the
	// correct callstack when running a 64-bit executable. However, XAudio2 sometimes (?) don't like this and it may result in no sound.
#if _WIN64
	if ( ParseParam(CmdLine,TEXT("noinnerexception")) || GIsBenchmarking )
	{
		GEnableInnerException = FALSE;
	}
#endif
}

void ViewProvider::SetWindow(Windows::UI::Core::CoreWindow^ window)
{
	m_window = window;
}

// this method is called after Initialize
void ViewProvider::Load(Platform::String^ entryPoint)
{
}

// this method is called after Load
void ViewProvider::Run()
{
	// call the usual main, but with no hInstance - note this will not work with wxWidgets most likely
	GuardedMain( TEXT(""), NULL, NULL, 0 );
}

void ViewProvider::Uninitialize()
{
	// Final shut down.
	appExit();

#if !SHIPPING_PC_GAME && !CONSOLE
	// Release the named mutex again now that we are done.
	ReleaseNamedMutex();
#endif

	// pause if we should
	if (GShouldPauseBeforeExit)
	{
		Sleep(INFINITE);
	}

	GIsStarted = 0;
}



[Platform::MTAThread]
INT main(array<Platform::String^>^ args)
{
    auto viewProviderFactory = ref new ViewProviderFactory();
    Windows::ApplicationModel::Core::CoreApplication::Run(viewProviderFactory);
}
#else

void appDingoEarlyInit()
{
#if DINGO
	GLog->SetCurrentThreadAsMasterThread();
	//set the main thread affinity. I'm not a big fan of calling GetCurrentThread for this but the main thread doesnt 
	// get created with CreateThread so we dont have proper thread handle to set the affinity with.
	appSetThreadAffinity(GetCurrentThread(), GAME_HWTHREAD);
#endif 
	// Setup common Windows settings
	SetupWindowsEnvironment();

	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	INT ErrorLevel			= 0;
	GIsStarted				= 1;
	// how can we get this?
	DINGO_CONSOLE_TODO
	//hInstance				= NULL; // hInInstance
	const TCHAR* CmdLine	= GetCommandLine();
	
#if !SHIPPING_PC_GAME && !CONSOLE
	// Named mutex we use to figure out whether we are the first instance of the game running. This is needed to e.g.
	// make sure there is no contention when trying to save the shader cache.
	GIsFirstInstance = MakeNamedMutex( CmdLine );

	if ( ParseParam( CmdLine,TEXT("crashreports") ) )
	{
		GAlwaysReportCrash = TRUE;
	}
#endif

	// Using the -noinnerexception parameter will disable the exception handler within native C++, which is call from managed code,
	// which is called from this function.
	// The default case is to have three wrapped exception handlers (note that wxWidgets also use an exception handler, which would be a 4th one):
	// Native: WinMain() -> Managed: ManagedGuardedMain() -> Native: GuardedMainWrapper().
	// The inner exception handler in GuardedMainWrapper() catches crashes/asserts in native C++ code and is the only way to get the
	// correct callstack when running a 64-bit executable. However, XAudio2 sometimes (?) don't like this and it may result in no sound.
#if _WIN64
	if ( ParseParam(CmdLine,TEXT("noinnerexception")) || GIsBenchmarking )
	{
		GEnableInnerException = FALSE;
	}
#endif
}
#endif

#else

INT WINAPI WinMain( HINSTANCE hInInstance, HINSTANCE hPrevInstance, char*, INT nCmdShow )
{
	// Setup common Windows settings
	SetupWindowsEnvironment();

	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	INT ErrorLevel			= 0;
	GIsStarted				= 1;
	hInstance				= hInInstance;
	const TCHAR* CmdLine	= GetCommandLine();

#if HAVE_WXWIDGETS && !SHIPPING_PC_GAME
	CmdLine = ShowLaunchModeDialog( CmdLine );
	if( !CmdLine )
		return 0;
#elif !WITH_EDITOR
	static TCHAR ShippingCmdLine[4096];
	if( appStrstr(CmdLine, TEXT("-seekfreeloadingpcconsole")) )
		appSprintf( ShippingCmdLine, TEXT("%s"), CmdLine );
	else
		appSprintf( ShippingCmdLine, TEXT("%s -seekfreeloadingpcconsole"), CmdLine );
	CmdLine = ShippingCmdLine;
#endif
	
#if !SHIPPING_PC_GAME && !CONSOLE
	// Named mutex we use to figure out whether we are the first instance of the game running. This is needed to e.g.
	// make sure there is no contention when trying to save the shader cache.
	GIsFirstInstance = MakeNamedMutex( CmdLine );

	if ( ParseParam( CmdLine,TEXT("crashreports") ) )
	{
		GAlwaysReportCrash = TRUE;
	}
#endif

	// Using the -noinnerexception parameter will disable the exception handler within native C++, which is call from managed code,
	// which is called from this function.
	// The default case is to have three wrapped exception handlers (note that wxWidgets also use an exception handler, which would be a 4th one):
	// Native: WinMain() -> Managed: ManagedGuardedMain() -> Native: GuardedMainWrapper().
	// The inner exception handler in GuardedMainWrapper() catches crashes/asserts in native C++ code and is the only way to get the
	// correct callstack when running a 64-bit executable. However, XAudio2 sometimes (?) don't like this and it may result in no sound.
#if _WIN64
	if ( ParseParam(CmdLine,TEXT("noinnerexception")) || GIsBenchmarking )
	{
		GEnableInnerException = FALSE;
	}
#endif

#if defined( _DEBUG )
	if( TRUE && !GAlwaysReportCrash )
#else
	if( appIsDebuggerPresent() && !GAlwaysReportCrash )
#endif
	{
		// Don't use exception handling when a debugger is attached to exactly trap the crash. This does NOT check
		// whether we are the first instance or not!
		ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
	}
	else
	{
		// Use structured exception handling to trap any crashes, walk the the stack and display a crash dialog box.
 		__try
 		{
			GIsGuarded = 1;
			// Run the guarded code.
#if WITH_MANAGED_CODE
			ErrorLevel = ManagedGuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
#else
			ErrorLevel = GuardedMainWrapper( CmdLine, hInInstance, hPrevInstance, nCmdShow );
#endif
			GIsGuarded = 0;
		}
		__except( GEnableInnerException ? EXCEPTION_EXECUTE_HANDLER : CreateMiniDump( GetExceptionInformation() ) )
		{
#if !SHIPPING_PC_GAME && !CONSOLE
			// Release the mutex in the error case to ensure subsequent runs don't find it.
			ReleaseNamedMutex();
#endif
			// Crashed.
			ErrorLevel = 1;
			GError->HandleError();
			StaticShutdownAfterError();
			appRequestExit( TRUE );
		}
	}

	// Final shut down.
	appExit();

#if !SHIPPING_PC_GAME && !CONSOLE
	// Release the named mutex again now that we are done.
	ReleaseNamedMutex();
#endif

	// pause if we should
	if (GShouldPauseBeforeExit)
	{
		Sleep(INFINITE);
	}
	
	GIsStarted = 0;
	return ErrorLevel;
}
#endif

#ifdef _WINDLL

INT PIBGuardedMainInit( const TCHAR* CmdLine )
{
	INT ErrorLevel = EnginePreInit( CmdLine );

	// exit if PreInit failed.
	if ( ErrorLevel != 0 || GIsRequestingExit )
	{
		return( 1 );
	}

	ErrorLevel = EngineInit( TEXT("PCConsole\\Splash.bmp") );

	return( ErrorLevel );
}

/** 
 * Inits the loaded DLL version of UE3
 */
INT PIBInit( HINSTANCE hInInstance, const TCHAR* CmdLine )
{
	SetupWindowsEnvironment();

	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	INT ErrorLevel = 0;
	GIsStarted = 1;
	hInstance = hInInstance;

#ifdef _DEBUG
	if( TRUE )
#else
	if( IsDebuggerPresent() )
#endif
	{
		// Don't use exception handling when a debugger is attached to exactly trap the crash. This does NOT check
		// whether we are the first instance or not!
		ErrorLevel = PIBGuardedMainInit( CmdLine );
	}
	else
	{
		// Use structured exception handling to trap any crashes, walk the the stack and display a crash dialog box.
		__try
		{
			GIsGuarded = 1;
			ErrorLevel = PIBGuardedMainInit( CmdLine );
			GIsGuarded = 0;
		}
		__except( CreateMiniDump( GetExceptionInformation() ) )
		{
			// Crashed.
			ReleaseNamedMutex();

			ErrorLevel = 1;
			GError->HandleError();
			StaticShutdownAfterError();
			appRequestExit( TRUE );
		}
	}

	return( ErrorLevel );
}

/**
 * Shuts the loaded DLL version of UE3 down
 */
void PIBShutdown( void )
{
	EngineExit();

	appExit();

	GIsStarted = 0;
}

#endif // _WINDLL

#endif
#endif
