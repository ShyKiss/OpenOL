// PlayerController for TASGame: drives movement/look axes from a scripted input buffer
// instead of (or blended with) real hardware input, and exposes frame-advance so a TAS
// tool can step the game exactly one tick at a time with deterministic input applied.
class TASPlayerController extends OLPlayerController
    native;

// TRUE while a scripted input buffer should override PlayerInput's axes this tick.
var bool bTASInputActive;

// Values applied to PlayerInput's axes for the current tick when bTASInputActive is set.
// Mirrors the axes NativePlayerMove/NativeUpdateRotation actually read (see OLPlayerController.cpp).
// NOTE: aBaseY/aBaseX are raw pre-scale input that PlayerInput.PlayerInput() folds into
// aForward/aTurn/aLookUp (aForward += aBaseY; aTurn *= TimeScale*LookRightScale; etc) - by
// the time our override runs (after that call, see PlayerTick below) those raw fields have
// already been consumed. We must therefore write the *post-scale* fields directly:
//   TASForward/TASStrafe: unit-ish direction, same convention as a held key (~[-1,1]),
//     consumed as a direction multiplied by Pawn.AccelRate in NativePlayerMove.
//   TASTurn/TASLookUp: rotator units to turn/look THIS TICK (65536 = full turn), consumed
//     directly by NativeUpdateRotation via FRotator((INT)lookUp,(INT)turn,0) - NOT a
//     per-second rate, and NOT in the small ~[-1,1] range aTurn/aLookUp start at before
//     PlayerInput() scales them by LookRightScale/LookUpScale.
var float TASForward;   // -> PlayerInput.aForward
var float TASStrafe;    // -> PlayerInput.aStrafe
var float TASTurn;      // -> PlayerInput.aTurn (rotator units, this tick)
var float TASLookUp;    // -> PlayerInput.aLookUp (rotator units, this tick)

// Called by the TAS driver (native, frame-advance loop) once per stepped frame, before
// unpausing for exactly one tick. Values persist until the next call or ClearTASInput().
// exec so it can be driven from the console for manual testing, e.g.:
//   settasinput 1.0 0.0 0.0 0.0     (walk forward)
//   settasinput 0.0 0.0 2000 0.0    (turn right ~11deg this tick - Turn/LookUp are rotator
//                                    units per tick, not [-1,1]; see field comments above)
exec native function SetTASInput(float Forward, float Strafe, float Turn, float LookUp);

// Stops overriding axes; PlayerInput reverts to reading real hardware input.
exec native function ClearTASInput();

// ---------------------------------------------------------------------------
// Convenience per-axis commands. Each only touches its own field (the others keep
// whatever they were last set to) and turns TAS override on. Handy for manual console
// testing and for hand-writing short move sequences without recomputing all four axes
// every call the way SetTASInput requires.
// ---------------------------------------------------------------------------

// Degrees relative to current facing (not rotator units) - converted internally so console
// use is intuitive: "turnby 45" turns 45 degrees this tick.
exec native function TurnBy(float Degrees);
exec native function LookUpBy(float Degrees);

// [-1,1]-ish direction, same convention as SetTASInput's Forward/Strafe.
exec native function MoveForward(float Amount);
exec native function Strafe(float Amount);

// Held modifiers: drives HeroPawn.bWantToRun (sprint, see OLHero.uc) and
// Pawn.bWantsToCrouch (crouch, inherited from base Pawn.uc) directly - the actual fields
// the movement/crouch systems check each tick, not a key-press simulation. Not sticky:
// call this every tick the modifier should be considered held, same as a real key.
exec native function SetTASModifiers(bool bShift, bool bCtrl);

// One-shot jump: sets Inputs.bPressedJump for this tick only (mirrors PressedJump(), see
// OLPlayerController.uc), not a held state - call it again on a later tick for another jump.
exec native function TASJump();

// ---------------------------------------------------------------------------
// Scripted input playback: a text file where each non-comment line is either:
//   - a movement frame, "forward strafe turn lookup [shift] [ctrl] [jump]" (the first 4
//     fields are required floats, same units as SetTASInput - Turn/LookUp in rotator
//     units for that tick, not degrees; the optional trailing tokens are literal words,
//     any subset, any order - "shift" and "ctrl" set held modifiers per SetTASModifiers,
//     "jump" fires a one-shot TASJump for that tick only). Consumes exactly one game tick.
//   - "wait N" - hold the last movement frame's axes AND modifiers for N more ticks
//     (jump is never repeated by wait - it only ever fires on the line that named it), or
//   - "maxfps N" - set the engine's FPS cap immediately (does not consume a tick).
// Blank lines and lines starting with ';' are comments, skipped without consuming a tick.
// The script is auto-prefixed with "maxfps 62" for deterministic timing, and on
// completion automatically calls ClearTASInput and unpauses so control returns to the
// player immediately. The driver steps exactly one game tick per movement-frame line via
// the engine's existing frame-advance (see UOLCheatManager::SingleFrame /
// GDelayedSingleStep in UnLevTic.cpp).
// ---------------------------------------------------------------------------

// Loads Filename from <exe dir>/OpenOL/TAS/Filename (e.g. "tasplay run1.txt" reads
// Binaries/Win64/OpenOL/TAS/run1.txt) and starts stepping it one tick at a time starting
// next frame. Replaces any script already running. Logs and does nothing if the file
// can't be read.
exec native function TASPlay(string Filename);

// Pauses playback without discarding position in the script (TASResume continues from
// where it left off). No-op if nothing is loaded.
exec native function TASPause();

// Resumes a paused script from where TASPause left it.
exec native function TASResume();

// Stops playback, discards the loaded script, and clears TAS input override.
exec native function TASStop();

// Internal: called once per PlayerTick, before the TAS override block below. Advances
// script playback state (see FTASScriptPlayer in TASPlayerController.cpp) and, if a
// script is actively playing, applies its next line's axes via SetTASInput and arms the
// engine's frame-advance (GDelayedSingleStep) so exactly one tick elapses per line. No-op
// if nothing is playing.
native function TickTASScript();

// Mirrors PlayerController::PlayerTick (see Engine/Classes/PlayerController.uc), with the
// TAS override injected between PlayerInput.PlayerInput() - which is where the engine
// finishes populating the raw axes from hardware/network input - and PlayerMove(), which is
// what actually consumes them via NativePlayerMove/NativeUpdateRotation. This is the latest
// point at which overwriting the axes still reaches movement for this tick, and the earliest
// point after which nothing has read them yet.
event PlayerTick(float DeltaTime)
{
    if (!bShortConnectTimeOut)
    {
        bShortConnectTimeOut = true;
        ServerShortTimeout();
    }

    if (Pawn != AcknowledgedPawn)
    {
        if (Role < ROLE_Authority)
        {
            if ((AcknowledgedPawn != None) && (AcknowledgedPawn.Controller == self))
                AcknowledgedPawn.Controller = None;
        }
        AcknowledgePossession(Pawn);
    }

    PlayerInput.PlayerInput(DeltaTime);

    TickTASScript();

    if (bTASInputActive)
    {
        PlayerInput.aForward = TASForward;
        PlayerInput.aStrafe  = TASStrafe;
        PlayerInput.aTurn    = TASTurn;
        PlayerInput.aLookUp  = TASLookUp;
    }

    if (bUpdatePosition)
    {
        ClientUpdatePosition();
    }
    PlayerMove(DeltaTime);

    // TASJump() sets Inputs.bPressedJump for exactly this tick (see TASJump in
    // TASPlayerController.cpp) - clear it now that PlayerMove has had a chance to see it,
    // so it doesn't stay "held" into ticks that didn't ask for a jump.
    Inputs.bPressedJump = false;
}

DefaultProperties
{
}
