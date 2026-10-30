#include "TASGame.h"

IMPLEMENT_CLASS(ATASPlayerController);

extern UBOOL GDelayedSingleStep;
extern UBOOL GDelayedPause;

void ATASPlayerController::SetTASInput(FLOAT Forward, FLOAT Strafe, FLOAT Turn, FLOAT LookUp)
{
	bTASInputActive = TRUE;
	TASForward = Forward;
	TASStrafe  = Strafe;
	TASTurn    = Turn;
	TASLookUp  = LookUp;
}

void ATASPlayerController::ClearTASInput()
{
	bTASInputActive = FALSE;
}

void ATASPlayerController::TurnBy(FLOAT Degrees)
{
	SetTASInput(TASForward, TASStrafe, Degrees * 65536.f / 360.f, TASLookUp);
}

void ATASPlayerController::LookUpBy(FLOAT Degrees)
{
	SetTASInput(TASForward, TASStrafe, TASTurn, Degrees * 65536.f / 360.f);
}

void ATASPlayerController::MoveForward(FLOAT Amount)
{
	SetTASInput(Amount, TASStrafe, TASTurn, TASLookUp);
}

void ATASPlayerController::Strafe(FLOAT Amount)
{
	SetTASInput(TASForward, Amount, TASTurn, TASLookUp);
}

// Sets the *input* flags real held keys populate - PlayerController::bRunInput and
// ::bDuck (both "var input", auto-cleared by the engine before PlayerInput() each tick,
// same category as aBaseY/aTurn) - NOT HeroPawn::bWantToRun/bWantsToCrouch directly.
// Writing those directly doesn't stick: AOLPlayerController::ProcessPlayerActions runs
// every tick as part of PlayerMove() (i.e. AFTER our override point - see
// OLPlayerController.cpp) and unconditionally calls HeroPawn->Walk() (which resets
// bWantToRun to FALSE) whenever bRunInput is FALSE, and similarly resets crouch via
// bDuck. Setting bRunInput/bDuck here - in the same spot as the axis override, after
// PlayerInput.PlayerInput() cleared them and before PlayerMove() reads them - lets that
// same real logic drive bWantToRun/bWantsToCrouch correctly, instead of fighting it.
// Must be called every tick the modifier should be considered "held" - it is not sticky.
void ATASPlayerController::SetTASModifiers(UBOOL bShift, UBOOL bCtrl)
{
	bRunInput = bShift ? 1 : 0;
	bDuck     = bCtrl;
}

// One-shot jump: mirrors PressedJump() (OLPlayerController.uc) without going through a
// real key event. Inputs.bPressedJump is cleared at the end of TASPlayerController's
// PlayerTick (see the .uc) so this only lasts the tick it was called on.
void ATASPlayerController::TASJump()
{
	Inputs.bPressedJump = TRUE;
}

// ---------------------------------------------------------------------------
// Scripted input playback.
//
// Each non-comment line is either:
//   - a movement frame: "forward strafe turn lookup" (4 whitespace-separated floats,
//     same units as SetTASInput), consuming exactly one game tick, or
//   - a command: "wait N" (hold the last-applied axes for N more ticks) or
//     "maxfps N" (set the engine's FPS cap via the existing MAXFPS console command;
//     does not consume a tick - takes effect immediately and moves on to the next line).
// Blank lines and lines starting with ';' are comments and are skipped without
// consuming a tick. The script is auto-prefixed with "maxfps 62" and, on completion,
// automatically clears TAS input and unpauses - see TASPlay/TickTASScript.
//
// Driven entirely from TickTASScript(), called once per PlayerTick. Per the frame-advance
// timing in UWorld::Tick (UnLevTic.cpp): setting GDelayedSingleStep unpauses for exactly
// the tick in which PlayerTick next runs, then GDelayedPause re-pauses before the tick
// after that. So PlayerTick (and therefore TickTASScript) is guaranteed to run exactly
// once per unpaused frame - each call here corresponds 1:1 with one movement-frame line
// (wait/maxfps lines are folded into surrounding frames and don't get their own tick).
// ---------------------------------------------------------------------------

enum ETASLineType
{
	TASLINE_Move,
	TASLINE_MaxFPS,
};

struct FTASFrame
{
	ETASLineType Type;
	FLOAT Forward, Strafe, Turn, LookUp; // TASLINE_Move
	UBOOL bShift, bCtrl;                  // TASLINE_Move: held modifiers this tick
	UBOOL bJump;                          // TASLINE_Move: one-shot jump this tick
	FLOAT FPSValue;                      // TASLINE_MaxFPS
};

struct FTASScriptState
{
	TArray<FTASFrame> Frames;
	INT                FrameIndex;
	UBOOL              bPlaying;

	FTASScriptState() : FrameIndex(0), bPlaying(FALSE) {}
};

// One playback state per controller instance (keyed by 'this') rather than a plain member,
// so FTASFrame/TArray don't need to be threaded through the UnrealScript-visible class
// layout in TASPlayerController.uc.
static TMap<ATASPlayerController*, FTASScriptState> GTASScripts;

static void ApplyMaxFPS(FLOAT FPS)
{
	if (GEngine)
		GEngine->Exec(*FString::Printf(TEXT("MAXFPS %g"), FPS));
}

void ATASPlayerController::TASPlay(const FString& Filename)
{
	// Resolve relative to <exe dir>/OpenOL/TAS/, matching the OpenOL/... convention used
	// for other tooling paths (see OLImGui_Locale.cpp's OpenOL\settings.ini).
	FString FullPath = FString(appBaseDir()) + TEXT("OpenOL\\TAS\\") + Filename;

	FString FileContents;
	if (!appLoadFileToString(FileContents, *FullPath))
	{
		debugf(TEXT("TASPlay: failed to load '%s'"), *FullPath);
		return;
	}

	TArray<FString> Lines;
	FileContents.ParseIntoArray(&Lines, TEXT("\n"), TRUE);

	FTASScriptState NewState;
	NewState.Frames.Empty(Lines.Num() + 1);

	// Force a known, deterministic tick rate before any scripted input is applied -
	// requested so replays don't silently depend on whatever FPS cap happened to be
	// active (e.g. the 120 that was live during earlier testing here).
	{
		FTASFrame FpsFrame;
		FpsFrame.Type     = TASLINE_MaxFPS;
		FpsFrame.FPSValue = 62.f;
		NewState.Frames.AddItem(FpsFrame);
	}

	FLOAT LastForward = 0.f, LastStrafe = 0.f, LastTurn = 0.f, LastLookUp = 0.f;
	UBOOL LastShift = FALSE, LastCtrl = FALSE;

	for (INT i = 0; i < Lines.Num(); ++i)
	{
		FString Line = Lines(i).Trim().TrimTrailing();
		if (Line.Len() == 0 || Line.StartsWith(TEXT(";")))
			continue;

		TArray<FString> Parts;
		Line.ParseIntoArrayWS(&Parts);
		if (Parts.Num() == 0)
			continue;

		if (appStricmp(*Parts(0), TEXT("maxfps")) == 0)
		{
			if (Parts.Num() < 2)
			{
				debugf(TEXT("TASPlay: 'maxfps' needs a value, skipping line %d: '%s'"), i + 1, *Line);
				continue;
			}
			FTASFrame Frame;
			Frame.Type     = TASLINE_MaxFPS;
			Frame.FPSValue = appAtof(*Parts(1));
			NewState.Frames.AddItem(Frame);
			continue;
		}

		if (appStricmp(*Parts(0), TEXT("wait")) == 0)
		{
			if (Parts.Num() < 2)
			{
				debugf(TEXT("TASPlay: 'wait' needs a tick count, skipping line %d: '%s'"), i + 1, *Line);
				continue;
			}
			INT WaitTicks = appAtoi(*Parts(1));
			for (INT w = 0; w < WaitTicks; ++w)
			{
				FTASFrame Frame;
				Frame.Type    = TASLINE_Move;
				Frame.Forward = LastForward;
				Frame.Strafe  = LastStrafe;
				Frame.Turn    = LastTurn;
				Frame.LookUp  = LastLookUp;
				Frame.bShift  = LastShift;
				Frame.bCtrl   = LastCtrl;
				Frame.bJump   = FALSE; // jump never repeats via wait - see .uc doc comment
				NewState.Frames.AddItem(Frame);
			}
			continue;
		}

		if (Parts.Num() < 4)
		{
			debugf(TEXT("TASPlay: skipping malformed line %d: '%s'"), i + 1, *Line);
			continue;
		}

		FTASFrame Frame;
		Frame.Type    = TASLINE_Move;
		Frame.Forward = LastForward = appAtof(*Parts(0));
		Frame.Strafe  = LastStrafe  = appAtof(*Parts(1));
		Frame.Turn    = LastTurn    = appAtof(*Parts(2));
		Frame.LookUp  = LastLookUp  = appAtof(*Parts(3));

		// Optional trailing tokens, any subset, any order: "shift", "ctrl", "jump".
		UBOOL bShift = FALSE, bCtrl = FALSE, bJump = FALSE;
		for (INT p = 4; p < Parts.Num(); ++p)
		{
			if (appStricmp(*Parts(p), TEXT("shift")) == 0) bShift = TRUE;
			else if (appStricmp(*Parts(p), TEXT("ctrl")) == 0) bCtrl = TRUE;
			else if (appStricmp(*Parts(p), TEXT("jump")) == 0) bJump = TRUE;
			else debugf(TEXT("TASPlay: unrecognized token '%s' on line %d: '%s'"), *Parts(p), i + 1, *Line);
		}
		Frame.bShift = LastShift = bShift;
		Frame.bCtrl  = LastCtrl  = bCtrl;
		Frame.bJump  = bJump;
		NewState.Frames.AddItem(Frame);
	}

	if (NewState.Frames.Num() <= 1) // just the auto maxfps frame, no real content
	{
		debugf(TEXT("TASPlay: '%s' had no usable frames"), *Filename);
		return;
	}

	NewState.FrameIndex = 0;
	NewState.bPlaying   = TRUE;
	GTASScripts.Set(this, NewState);

	debugf(TEXT("TASPlay: loaded '%s', %d frames"), *FullPath, NewState.Frames.Num());
}

void ATASPlayerController::TASPause()
{
	FTASScriptState* State = GTASScripts.Find(this);
	if (State) State->bPlaying = FALSE;
}

void ATASPlayerController::TASResume()
{
	FTASScriptState* State = GTASScripts.Find(this);
	if (State && State->FrameIndex < State->Frames.Num()) State->bPlaying = TRUE;
}

void ATASPlayerController::TASStop()
{
	GTASScripts.Remove(this);
	ClearTASInput();
	SetTASModifiers(FALSE, FALSE);
}

void ATASPlayerController::TickTASScript()
{
	FTASScriptState* State = GTASScripts.Find(this);
	if (!State || !State->bPlaying)
		return;

	// Non-consuming command lines (maxfps) may appear back-to-back or right before a
	// movement line; drain them all within this single tick before deciding whether a
	// tick was actually consumed.
	while (State->FrameIndex < State->Frames.Num() && State->Frames(State->FrameIndex).Type == TASLINE_MaxFPS)
	{
		ApplyMaxFPS(State->Frames(State->FrameIndex).FPSValue);
		++State->FrameIndex;
	}

	if (State->FrameIndex >= State->Frames.Num())
	{
		debugf(TEXT("TASPlay: script finished"));
		State->bPlaying = FALSE;
		GTASScripts.Remove(this);
		ClearTASInput();
		// Release the frame-advance pause so control returns to the player immediately.
		// Both are needed: Pauser directly unpauses this frame, but UWorld::Tick already
		// set GDelayedPause TRUE earlier THIS SAME frame (the single-step that got us
		// here always arms the following re-pause - see UnLevTic.cpp) - if left set it
		// would silently re-pause the game on the very next frame regardless of Pauser.
		if (GWorld && GWorld->GetWorldInfo())
			GWorld->GetWorldInfo()->Pauser = NULL;
		GDelayedPause = FALSE;
		return;
	}

	const FTASFrame& Frame = State->Frames(State->FrameIndex);
	SetTASInput(Frame.Forward, Frame.Strafe, Frame.Turn, Frame.LookUp);
	SetTASModifiers(Frame.bShift, Frame.bCtrl);
	if (Frame.bJump) TASJump();
	++State->FrameIndex;

	// Arm frame-advance so exactly one more tick elapses before the game re-pauses,
	// mirroring UOLCheatManager::SingleFrame() (see UnLevTic.cpp for the pause/unpause
	// sequencing this relies on). Not gated on bCheatsEnabled: TAS mode is itself an
	// explicit debug entry point via a separate GameType, not a cheat toggle within
	// normal play.
	GDelayedSingleStep = TRUE;
}

// ---------------------------------------------------------------------------
// TASGame package registration.
// Every native UnrealScript package needs this boilerplate (see Multiplayer.cpp for the
// same pattern) so its classes/native functions actually get registered - without it,
// ATASPlayerController::StaticClass() is never forced and the engine fails to bind the
// native class at startup ("Can't bind to native class").
// ---------------------------------------------------------------------------

#define STATIC_LINKING_MOJO 1

// Register names and functions.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName TASGAME_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "TASGameClasses.h"
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#undef NAMES_ONLY

// Import natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "TASGameClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef NATIVES_ONLY
#undef NAMES_ONLY

void AutoInitializeRegistrantsTASGame( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_TASGAME;
}
