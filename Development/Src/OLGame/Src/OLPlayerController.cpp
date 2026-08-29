// winsock2.h before any UE3 headers (required by P2PBridge/RelayThread includes).
#ifdef _WIN32
#  ifndef _WINDOWS_
#    define WIN32_LEAN_AND_MEAN
#    ifndef NOMINMAX
#      define NOMINMAX
#    endif
#    include <winsock2.h>
#  endif
#endif
#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif

#include "OLGame.h"
#include "OLUtilities.h"
#include "..\..\OnlineSubsystemSteamworks\Inc\OnlineSubsystemSteamworks.h"
#include "OpenOLGlobals.h"
#include "EngineAnimClasses.h"
#include "UDKBaseAnimationClasses.h"
#include "GameFrameworkAnimClasses.h"
#include "OLGameAnimClasses.h"

#if WITH_ORBISCONTROLLEREMULATION
#include "PCOrbisController.h"
#endif

IMPLEMENT_CLASS(AOLPlayerController);
IMPLEMENT_CLASS(UOLPlayerInput);

TArray<BYTE> GWhistleTitleMovieBuffer;

////////////////////////////////////////////////////////////////////////////////////////////
// Base class overrides
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::PostBeginPlay()
{
	Super::PostBeginPlay();

	GOLPC = this;

	InventoryManager = ConstructObject<UOLInventoryManager>(UOLInventoryManager::StaticClass(), this);
	InventoryManager->OwnerPC = this;
	TutorialManager = ConstructObject<UOLTutorialManager>(UOLTutorialManager::StaticClass(), this);
	SoundEnvManager = ConstructObject<UOLSoundEnvironmentManager>(UOLSoundEnvironmentManager::StaticClass(), this);
	FXManager = ConstructObject<UOLFXManager>(UOLFXManager::StaticClass(), this);
	FXManager->Init();

	if (MusicAIStateGroup != NAME_None)
	{
		SetState(MusicAIStateGroup, MusicAIStateNone);
	}

	InventoryManager->ClearAll();

	if (!GIsPlayInEditorWorld)
	{
		eventClientSetCameraFade(TRUE, FColor(0, 0, 0), FVector2D(1.0f, 0.0f), 2.0f, TRUE);
	}

	// Hide all possessed documents
	for (FActorIterator It; It; ++It)
	{
		AOLCollectiblePickup* collectible = Cast<AOLCollectiblePickup>(*It);
		if (collectible && !collectible->ShouldShowCollectible())
		{
			collectible->SetHidden(TRUE);
		}
	}
}

void AOLPlayerController::BeginDestroy()
{
	GOLPC = NULL;

	Super::BeginDestroy();
}

UBOOL AOLPlayerController::Tick( FLOAT deltaSeconds, ELevelTick TickType )
{
	// Check for pending actions
	eventCheckForProfileUpdate();

	AOLGame* currentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	if (currentGame && currentGame->PendingCheckpointName != NAME_None)
	{
		AOLCheckpoint* cp = Utils::GetCheckpointFromName(currentGame->PendingCheckpointName); // This is not to get the object, but to validate that it has been loaded
		if (cp)
		{
			// restart the player - we've just loaded the checkpoint map and so can finish the desired travel
			eventStartNewGameAtCheckpoint(currentGame->PendingCheckpointName.ToString(), FALSE);
			currentGame->PendingCheckpointName = NAME_None;
			return TRUE;
		}
	}

	if (PendingRecordingMarker && HeroPawn)
	{
		if (GWorld->GetTimeSeconds() > (RecordingCompletedTime + PendingRecordingMarker->NotificationDelay))
		{
			ProcessCompletedRecording(PendingRecordingMarker);
		}
	}
	
#if WITH_ORBISCONTROLLEREMULATION || ORBIS
	UpdateOrbisController(deltaSeconds);
#endif

	// Update stuff
	if (HeroPawn)
	{
		SetLocation(HeroPawn->Location);
		SetRotation(HeroPawn->GetViewRotation());

		UpdateOverlay(deltaSeconds);
		UpdateFade(deltaSeconds);
		UpdateMusic(deltaSeconds);
		
		TutorialManager->Tick(deltaSeconds);
		FXManager->Tick(deltaSeconds);

		UpdateLightingOptimisation(deltaSeconds);

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
		DrawDebug();

		UBOOL bLongFrame = bSlowDownFPS;

		UOLCheatManager* cheatMgr = Utils::GetCheatManager();
		if (cheatMgr && cheatMgr->NextSpikeTime > 0.0f && GWorld->GetTimeSeconds() > cheatMgr->NextSpikeTime)
		{
			bLongFrame = TRUE;

			if (cheatMgr->AutoSpikeDelay > 0.0f)
			{
				cheatMgr->NextSpikeTime = GWorld->GetTimeSeconds() + cheatMgr->AutoSpikeDelay;
			}
			else
			{
				cheatMgr->NextSpikeTime = -1.0f;
			}
		}

		if (bLongFrame)
		{
			TWEAKABLE INT BaseCount = 10000000;
			INT maxCount = (INT)(SlowDownFactor * BaseCount);
			volatile INT dummyCnt = 0;
			for (INT i = 0; i < maxCount; i++)
			{
				dummyCnt += i;
			}
		}

#endif
	}

	if (bPlacementMode)
		TickPlacementMode();

	return Super::Tick(deltaSeconds, TickType);
}

void AOLPlayerController::TickPlacementMode()
{
	if (!PlacementGhost || PlacementGhost->bDeleteMe)
	{
		bPlacementMode = FALSE;
		PlacementGhost = NULL;
		return;
	}

	FVector  CamLoc;
	FRotator CamRot;
	eventGetPlayerViewPoint(CamLoc, CamRot);

	const FLOAT TraceLen = 2048.f;
	FVector TraceEnd = CamLoc + FRotationMatrix(CamRot).GetAxis(0) * TraceLen;

	FCheckResult Hit;
	if (!GWorld->SingleLineCheck(Hit, this, TraceEnd, CamLoc, TRACE_World | TRACE_StopAtAnyHit))
		PlacementGhost->SetLocation(Hit.Location);
	else
		PlacementGhost->SetLocation(TraceEnd);

	FRotator GhostRot(0, appRound(PlacementYaw), 0);
	PlacementGhost->SetRotation(GhostRot);
}

void AOLPlayerController::UpdateTouchZoom(FLOAT deltaSeconds)
{
#if ORBIS

	TWEAKABLE FLOAT MaxTouchZoomingDelay = 0.2f;
	TWEAKABLE FLOAT InputApproachCoeff = 0.999f;
	TWEAKABLE FLOAT VelThreshold = 0.1f;
	
	UBOOL bCurrentlyTouchZooming = HeroPawn->CanZoom() && (TouchZoom.LastInputTime > 0.0f) && (GWorld->GetTimeSeconds() - TouchZoom.LastInputTime) < MaxTouchZoomingDelay;

	if (bCurrentlyTouchZooming)
	{
		if (!TouchZoom.bActive)
		{
			// started touch
			TouchZoom.SmoothedPosition = TouchZoom.LastPosition;
			TouchZoom.CurrentDirection = Zoom_Undefined;
		}
		else
		{
			TouchZoom.SmoothedPosition = Utils::Approach(TouchZoom.SmoothedPosition, TouchZoom.LastPosition, InputApproachCoeff, deltaSeconds);
		}

		FLOAT currentInputDelta = TouchZoom.LastPosition - TouchZoom.SmoothedPosition; // is the player making a zoom in or out motion
		FLOAT currentZoomDelta = TouchZoom.LastPosition - HeroPawn->TargetCamcorderZoomFactor; // would we zoom in or out

		ZoomMovementType currentDirection = Zoom_NotMoving;

		if (currentInputDelta > VelThreshold)
		{
			currentDirection = Zoom_MovingRight;
		}
		else if (currentInputDelta < -VelThreshold)
		{
			currentDirection = Zoom_MovingLeft;
		}

		UBOOL bInvalidTouch = (((currentDirection == Zoom_MovingRight) && (currentZoomDelta < 0)) ||	 // motion opposite the effective zoom direction
										((currentDirection == Zoom_MovingLeft) && (currentZoomDelta > 0)) ||  // motion opposite the effective zoom direction
										(currentDirection == Zoom_NotMoving && TouchZoom.CurrentDirection == Zoom_Undefined)); // first touch - must noticeably move before it takes effect
		if (!bInvalidTouch)
		{
			if (currentDirection != Zoom_NotMoving && currentDirection != TouchZoom.CurrentDirection)
			{
				// Started zooming in or out
				HeroPawn->StartedActiveZoom(currentDirection == Zoom_MovingRight);
			}		

			TouchZoom.CurrentDirection = currentDirection;

			HeroPawn->SetTargetZoom(TouchZoom.LastPosition);
		}
	}

	TouchZoom.bActive = bCurrentlyTouchZooming;

#endif
}

void AOLPlayerController::UpdateOrbisController(FLOAT deltaSeconds)
{
#if WITH_ORBISCONTROLLEREMULATION || ORBIS
#if WITH_ORBISCONTROLLEREMULATION
	if (GOrbisController)
#endif
	{
		TWEAKABLE FLinearColor whiteColor(0.41f, 0.63f, 0.26f);
		TWEAKABLE FLinearColor fullHealthWithNV(0.15f, 1.0f, 0.2f);
		TWEAKABLE FLinearColor fullHealthWithNVNoBattery(0.01f, 0.06f, 0.015f);
		TWEAKABLE FLinearColor medHealth(0.65f, 0.9f, 0.0f);
		TWEAKABLE FLinearColor medHealthWithNV(0.65f, 0.9f, 0.0f);
		TWEAKABLE FLinearColor lowHealthPulseHigh(1.0f, 0.0f, 0.0f);
		TWEAKABLE FLinearColor lowHealthPulseLow(0.3f, 0.0f, 0.0f);
		TWEAKABLE FLOAT DeadHeroApproachCoeff = 0.65f;
		TWEAKABLE FLOAT DeadFullRedDelay = 5.0;
		
		if (!HeroPawn || GWorld->IsPaused() || bTravellingToCheckpoint)
		{
			LightBarColor = whiteColor;
		}
		else if (HeroPawn->Health == 0)
		{
			if (GWorld->GetTimeSeconds() < HeroPawn->TimeOfDeath + DeadFullRedDelay)
			{
				// dead, keep red for a little while
				LightBarColor = lowHealthPulseHigh;
			}
			else
			{
				// fade back to white
				LightBarColor.R = Utils::Approach(LightBarColor.R, whiteColor.R, DeadHeroApproachCoeff, deltaSeconds);
				LightBarColor.G = Utils::Approach(LightBarColor.G, whiteColor.G, DeadHeroApproachCoeff, deltaSeconds);
				LightBarColor.B = Utils::Approach(LightBarColor.B, whiteColor.B, DeadHeroApproachCoeff, deltaSeconds);
			}
		}
		else
		{
			TWEAKABLE FLOAT highThresh = 70.0f;
			TWEAKABLE FLOAT lowThresh = 30.0f;
			TWEAKABLE FLOAT PulseFreqLow = 3.0f;
			TWEAKABLE FLOAT PulseFreqHigh = 10.0f;
			TWEAKABLE FLOAT PulseFreqHealthThresh = 50.0f;
			
			FLOAT instantFreq = MapClamped(HeroPawn->PreciseHealth, 0.0f, PulseFreqHealthThresh, PulseFreqHigh, PulseFreqLow);
			LightBarPulsePhase += deltaSeconds * 2.0f * PI * instantFreq;

			FLOAT pulseAlpha = (0.5f + 0.5f * appSin(LightBarPulsePhase)); // 0 - 1
			FLinearColor lowHealth = lowHealthPulseLow + pulseAlpha * (lowHealthPulseHigh - lowHealthPulseLow);

			FLinearColor effectiveFullHealth = whiteColor; 
			FLinearColor effectiveMidHealth = medHealth;

			if (HeroPawn->IsInNightVision())
			{
				if (HeroPawn->CamcorderMode == CCM_NightVision)
				{
					effectiveFullHealth = fullHealthWithNVNoBattery;
				}
				else if (HeroPawn->NVGlitch.bGlitching)
				{
					FLOAT effectiveLevel = HeroPawn->NVGlitch.CurrentLevel * HeroPawn->NVGlitch.CurrentLevel;
					effectiveFullHealth.R = LerpClamped(effectiveLevel, fullHealthWithNVNoBattery.R, fullHealthWithNV.R);
					effectiveFullHealth.G = LerpClamped(effectiveLevel, fullHealthWithNVNoBattery.G, fullHealthWithNV.G);
					effectiveFullHealth.B = LerpClamped(effectiveLevel, fullHealthWithNVNoBattery.B, fullHealthWithNV.B);
				}
				else
				{
					effectiveFullHealth = fullHealthWithNV;
				}

				effectiveMidHealth = medHealthWithNV;
			}

			if (HeroPawn->PreciseHealth < KINDA_SMALL_NUMBERF)
			{
				LightBarColor = lowHealthPulseHigh;
			}
			else if (HeroPawn->PreciseHealth < lowThresh)
			{
				LightBarColor = lowHealth;
			}
			else if (HeroPawn->PreciseHealth < highThresh)
			{
				LightBarColor = (lowHealth + ( (HeroPawn->PreciseHealth - lowThresh) / (highThresh - lowThresh) ) * (effectiveMidHealth - lowHealth));
			}
			else
			{
				LightBarColor = (effectiveMidHealth + ((HeroPawn->PreciseHealth - highThresh) / (100.0f - highThresh) ) * (effectiveFullHealth - effectiveMidHealth));
			}
		}
		
#if WITH_ORBISCONTROLLEREMULATION
		GOrbisController->SetLightBarColor(LightBarColor.ToFColor(FALSE));		
#elif ORBIS
		ULocalPlayer* LP = Cast<ULocalPlayer>(Player);
		if (LP && GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			GEngine->GameViewport->Viewport->SetControllerLightBarColor(LP->ControllerId, LightBarColor.ToFColor(FALSE));
		}
#endif
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLPlayerInput
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void UOLPlayerInput::ResetInput()
{
	Super::ResetInput();

	AOLPlayerController* OLPC = GetOuterAOLPlayerController();

	OLPC->bLeanInputLeft = 0;
	OLPC->bLeanInputRight = 0;
	OLPC->AnalogLeanInputLeft = 0.0f;
	OLPC->AnalogLeanInputRight = 0.0f;
	OLPC->bRunInput = 0;
	OLPC->bUseButtonDown = 0;
	OLPC->PureMouseX = 0.0f;	
	OLPC->ZoomInput = 0.0f;
	appMemZero(OLPC->TouchZoom);
}

UBOOL UOLPlayerInput::InputTouch(INT ControllerId, UINT Handle, ETouchType Type, FVector2D TouchLocation, DOUBLE DeviceTimestamp, UINT TouchpadIndex)
{
#if ORBIS

	TWEAKABLE UBOOL ZoomUpDown = TRUE;

	if (Type == Touch_Moved)
	{
		FLOAT fullPosition = ZoomUpDown ? (1.0f - TouchLocation.Y) : TouchLocation.X;

		AOLPlayerController* OLPC = GetOuterAOLPlayerController();

		TWEAKABLE FLOAT EdgeZone = 0.15f;
		OLPC->TouchZoom.LastPosition = MapClamped(fullPosition, EdgeZone, 1.0f - EdgeZone, 0.0f, 1.0f); // give a buffer so that the user doesn't have to reach to the very end to full zoom in/out
		OLPC->TouchZoom.LastInputTime = GWorld->GetTimeSeconds();
	}

#endif

	return TRUE;
}

// Maps a UE3 key FName to a Windows virtual key code for forwarding to the CEF browser.
// Returns 0 if the key has no VK equivalent (mouse buttons, axes, etc.).
static INT KeyNameToWindowsVK(FName Key)
{
    static TMap<FName,INT> sMap;
    if (sMap.Num() == 0)
    {
        // Control keys
        sMap.Set(KEY_BackSpace,  VK_BACK);
        sMap.Set(KEY_Tab,        VK_TAB);
        sMap.Set(KEY_Enter,      VK_RETURN);
        sMap.Set(KEY_Pause,      VK_PAUSE);
        sMap.Set(KEY_CapsLock,   VK_CAPITAL);
        sMap.Set(KEY_Escape,     VK_ESCAPE);
        sMap.Set(KEY_SpaceBar,   VK_SPACE);
        sMap.Set(KEY_PageUp,     VK_PRIOR);
        sMap.Set(KEY_PageDown,   VK_NEXT);
        sMap.Set(KEY_End,        VK_END);
        sMap.Set(KEY_Home,       VK_HOME);
        sMap.Set(KEY_Left,       VK_LEFT);
        sMap.Set(KEY_Up,         VK_UP);
        sMap.Set(KEY_Right,      VK_RIGHT);
        sMap.Set(KEY_Down,       VK_DOWN);
        sMap.Set(KEY_Insert,     VK_INSERT);
        sMap.Set(KEY_Delete,     VK_DELETE);
        // Digits
        sMap.Set(KEY_Zero,  0x30); sMap.Set(KEY_One,   0x31); sMap.Set(KEY_Two,   0x32);
        sMap.Set(KEY_Three, 0x33); sMap.Set(KEY_Four,  0x34); sMap.Set(KEY_Five,  0x35);
        sMap.Set(KEY_Six,   0x36); sMap.Set(KEY_Seven, 0x37); sMap.Set(KEY_Eight, 0x38);
        sMap.Set(KEY_Nine,  0x39);
        // Letters
        sMap.Set(KEY_A,0x41); sMap.Set(KEY_B,0x42); sMap.Set(KEY_C,0x43); sMap.Set(KEY_D,0x44);
        sMap.Set(KEY_E,0x45); sMap.Set(KEY_F,0x46); sMap.Set(KEY_G,0x47); sMap.Set(KEY_H,0x48);
        sMap.Set(KEY_I,0x49); sMap.Set(KEY_J,0x4A); sMap.Set(KEY_K,0x4B); sMap.Set(KEY_L,0x4C);
        sMap.Set(KEY_M,0x4D); sMap.Set(KEY_N,0x4E); sMap.Set(KEY_O,0x4F); sMap.Set(KEY_P,0x50);
        sMap.Set(KEY_Q,0x51); sMap.Set(KEY_R,0x52); sMap.Set(KEY_S,0x53); sMap.Set(KEY_T,0x54);
        sMap.Set(KEY_U,0x55); sMap.Set(KEY_V,0x56); sMap.Set(KEY_W,0x57); sMap.Set(KEY_X,0x58);
        sMap.Set(KEY_Y,0x59); sMap.Set(KEY_Z,0x5A);
        // Numpad
        sMap.Set(KEY_NumPadZero,  VK_NUMPAD0); sMap.Set(KEY_NumPadOne,   VK_NUMPAD1);
        sMap.Set(KEY_NumPadTwo,   VK_NUMPAD2); sMap.Set(KEY_NumPadThree, VK_NUMPAD3);
        sMap.Set(KEY_NumPadFour,  VK_NUMPAD4); sMap.Set(KEY_NumPadFive,  VK_NUMPAD5);
        sMap.Set(KEY_NumPadSix,   VK_NUMPAD6); sMap.Set(KEY_NumPadSeven, VK_NUMPAD7);
        sMap.Set(KEY_NumPadEight, VK_NUMPAD8); sMap.Set(KEY_NumPadNine,  VK_NUMPAD9);
        sMap.Set(KEY_Multiply, VK_MULTIPLY); sMap.Set(KEY_Add,      VK_ADD);
        sMap.Set(KEY_Subtract, VK_SUBTRACT); sMap.Set(KEY_Decimal,  VK_DECIMAL);
        sMap.Set(KEY_Divide,   VK_DIVIDE);
        // F-keys
        sMap.Set(KEY_F1,VK_F1);  sMap.Set(KEY_F2,VK_F2);  sMap.Set(KEY_F3,VK_F3);
        sMap.Set(KEY_F4,VK_F4);  sMap.Set(KEY_F5,VK_F5);  sMap.Set(KEY_F6,VK_F6);
        sMap.Set(KEY_F7,VK_F7);  sMap.Set(KEY_F8,VK_F8);  sMap.Set(KEY_F9,VK_F9);
        sMap.Set(KEY_F10,VK_F10); sMap.Set(KEY_F11,VK_F11); sMap.Set(KEY_F12,VK_F12);
        // Punctuation
        sMap.Set(KEY_Semicolon,    VK_OEM_1);
        sMap.Set(KEY_Equals,       VK_OEM_PLUS);
        sMap.Set(KEY_Comma,        VK_OEM_COMMA);
        sMap.Set(KEY_Underscore,   VK_OEM_MINUS);
        sMap.Set(KEY_Period,       VK_OEM_PERIOD);
        sMap.Set(KEY_Slash,        VK_OEM_2);
        sMap.Set(KEY_Tilde,        VK_OEM_3);
        sMap.Set(KEY_LeftBracket,  VK_OEM_4);
        sMap.Set(KEY_Backslash,    VK_OEM_5);
        sMap.Set(KEY_RightBracket, VK_OEM_6);
        sMap.Set(KEY_Quote,        VK_OEM_7);
        // Modifiers
        sMap.Set(KEY_LeftShift,    VK_LSHIFT);
        sMap.Set(KEY_RightShift,   VK_RSHIFT);
        sMap.Set(KEY_LeftControl,  VK_LCONTROL);
        sMap.Set(KEY_RightControl, VK_RCONTROL);
        sMap.Set(KEY_LeftAlt,      VK_LMENU);
        sMap.Set(KEY_RightAlt,     VK_RMENU);
    }
    INT* pVK = sMap.Find(Key);
    return pVK ? *pVK : 0;
}

// Returns the focused OLBrowser actor (crosshair on screen), or NULL.
static AOLBrowser* FindFocusedBrowser(AOLPlayerController* PC)
{
    if (!PC) return NULL;
    for (FActorIterator It; It; ++It)
    {
        AOLBrowser* B = Cast<AOLBrowser>(*It);
        if (B && B->bFocused && B->bRunning)
            return B;
    }
    return NULL;
}

UBOOL UOLPlayerInput::InputKey(INT ControllerId, FName Key, enum EInputEvent Event, FLOAT AmountDepressed, UBOOL bGamepad)
{
	if (bUsingGamepad && Event == IE_Pressed && Utils::GetCheatManager())
	{
		if (Utils::GetCheatManager()->eventProcessCheatInput(this, Key))
		{
			return TRUE;
		}
	}

    // Forward input to focused browser (crosshair on screen)
    AOLPlayerController* PC = Cast<AOLPlayerController>(GetOuter());

    // Placement mode input — intercept before browser and game
    if (PC && PC->bPlacementMode && Event == IE_Pressed)
    {
        if (Key == KEY_MouseScrollUp)
        {
            PC->PlacementYaw += 3640.f; // ~20 degrees
            return TRUE;
        }
        if (Key == KEY_MouseScrollDown)
        {
            PC->PlacementYaw -= 3640.f;
            return TRUE;
        }
        if (Key == KEY_LeftMouseButton)
        {
            // Caller (OLEnemyPlacer etc.) polls bPlacementMode + Ghost each frame;
            // signal a confirm by setting the pending flag.
            PC->bPlacementConfirmPending = TRUE;
            return TRUE;
        }
        if (Key == KEY_RightMouseButton)
        {
            PC->eventEndPlacementMode();
            return TRUE;
        }
    }

    AOLBrowser* Browser = FindFocusedBrowser(PC);
    if (Browser)
    {
        // Mouse scroll
        if (Key == KEY_MouseScrollUp)
        {
            Browser->SendScroll(120);
            return TRUE;
        }
        if (Key == KEY_MouseScrollDown)
        {
            Browser->SendScroll(-120);
            return TRUE;
        }

        // Right mouse button
        if (Key == KEY_RightMouseButton)
        {
            Browser->SendRightClick(Event == IE_Pressed || Event == IE_Repeat);
            // Don't swallow — RMB may also be used for game functions, but here we return TRUE
            // so the game doesn't react while looking at the screen
            return TRUE;
        }

        // Left mouse button is already handled via bUseButtonDown in OLBrowser::Tick.
        // Middle mouse button — send as scroll click (optional, pass through for now)
        if (Key == KEY_MiddleMouseButton)
            return TRUE; // swallow to avoid accidental game actions

        // Keyboard: swallow and forward on press/repeat/release
        if (Key != KEY_LeftMouseButton && Key != KEY_MouseX && Key != KEY_MouseY)
        {
            INT VK = KeyNameToWindowsVK(Key);
            if (VK != 0)
            {
                UBOOL bDown = (Event == IE_Pressed || Event == IE_Repeat);
                Browser->SendKey(VK, bDown);

                // For printable characters on press, also send a CHAR event so the browser
                // receives text input (key down alone is not enough for <input> fields).
                if (bDown && Event == IE_Pressed && VK >= 0x20)
                {
                    // MapVirtualKey translates VK → character taking shift state into account
                    UINT Char = MapVirtualKey((UINT)VK, MAPVK_VK_TO_CHAR);
                    if (Char != 0)
                        Browser->SendChar((INT)Char);
                }
                return TRUE;
            }
        }
    }

	return Super::InputKey(ControllerId, Key, Event, AmountDepressed, bGamepad);
}

UBOOL UOLPlayerInput::IsKeyPressed(FName Key)
{
	return IsPressed(Key);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Hero control
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::NativeUpdateRotation(FLOAT deltaTime)
{
	FRotator deltaRot((INT)PlayerInput->aLookUp, (INT)PlayerInput->aTurn, 0);
	HeroPawn->Camera->LatchInput(deltaRot.GetNormalized());
}

void AOLPlayerController::NativePlayerMove(FLOAT deltaTime)
{
	if (bDebugFreeCam)
	{
		// Fix MoveActor spam
		Pawn->Acceleration = FVector(0.0f);
		HeroPawn->InputMovementScaling = 0.0f;

		if (Inputs.bPressedToggleCamcorder)
			HeroPawn->TryToggleCamcorder();
		if (Inputs.bPressedToggleNightVision)
			HeroPawn->TryToggleNightVision();
		appMemZero(Inputs);

		ProcessFreeCam(deltaTime);
		return;
	}

	if (bIgnoreMoveInput)
	{
		Pawn->Acceleration = FVector(0.0f);
		HeroPawn->InputMovementScaling = 0.0f;
	}
	else
	{
		FRotationMatrix pawnRotMtx(Pawn->Rotation);
		FVector X = pawnRotMtx.GetAxis(0).SafeNormal2D();
		FVector Y = pawnRotMtx.GetAxis(1).SafeNormal2D();
		
		// Update acceleration.
		FVector NewAccel = FVector(0.f);

		UBOOL cameraRelative = (HeroPawn->LocomotionMode == LM_LedgeHang || HeroPawn->LocomotionMode == LM_LedgeWalk || HeroPawn->LocomotionMode == LM_Squeeze);

		if (cameraRelative)
		{
			FRotationMatrix camRotMtx(HeroPawn->EyeRotation);
			X = camRotMtx.GetAxis(0);
			Y = camRotMtx.GetAxis(1);
			NewAccel = PlayerInput->aForward*X + PlayerInput->aStrafe*Y;
			NewAccel.Z	= 0;
		}	
		else if (HeroPawn->LocomotionMode == LM_Ladder)
		{
			NewAccel = FVector(0, 0, PlayerInput->aForward);
		}
		else
		{		
			NewAccel = PlayerInput->aForward*X + PlayerInput->aStrafe*Y;
			NewAccel.Z	= 0;
		}

		NewAccel = Pawn->AccelRate * NewAccel.SafeNormal();
		Pawn->Acceleration = NewAccel;
				
		if (PlayerInput->bUsingGamepad)
		{
			HeroPawn->InputMovementScaling = Saturate(FVector2D(PlayerInput->RawJoyRight, PlayerInput->RawJoyUp).Size());
		}
		else
		{
			HeroPawn->InputMovementScaling = 1.0f; // keyboard always at full strength
		}
	}

	NativeUpdateRotation(deltaTime);

	AvailableInteractions.Empty();

	if (HeroPawn->Health > 0)
	{
		if (!bIgnoreMoveInput)
		{
			ProcessPlayerActions(deltaTime);
		}

		UpdateStruggle(deltaTime);

		if (Struggle.bActiveStrugging)
		{
			AvailableInteractions.AddItem(PIT_Struggle);
		}

		if (HeroPawn->IsCamcorderActive() && appIsNearlyZero(HeroPawn->CurrentBatterySetEnergy, KINDA_SMALL_NUMBERF) && HeroPawn->CanReloadBatteries())
		{
			AvailableInteractions.AddItem(PIT_ReloadBatteries);
		}

		if (!TutorialManager->bClimbUpTutorialComplete && AvailableInteractions.ContainsItem(PIT_ClimbUpLedge))
		{
			TutorialManager->ClimbUpTutorial();
		}
	}

	if (!bIgnoreMoveInput && bUseButtonDown)
	{
		UsePressedTime += deltaTime;
	}
	else
	{
		UsePressedTime = 0.0f;
	}

	appMemZero(Inputs);	

	// I don't get Unreal's input axis; we need to reset those manually
	AnalogLeanInputLeft = 0.0f;
	AnalogLeanInputRight = 0.0f;
}

UBOOL AOLPlayerController::ProcessPlayerActions(FLOAT deltaSeconds)
{
	TWEAKABLE FLOAT MinPlayerInputIntentForSpecialMove = 500.0f;

	FRotationMatrix camRotMtx(HeroPawn->EyeRotation);
	FVector camX = camRotMtx.GetAxis(0).SafeNormal2D();
	FVector camY = camRotMtx.GetAxis(1).SafeNormal2D();		
	FVector playerInputIntent = PlayerInput->aForward*camX + PlayerInput->aStrafe*camY;
	LastPlayerInputIntent = playerInputIntent;

	UBOOL bEffectiveLeanLeft =	(bLeanInputLeft || AnalogLeanInputLeft > 0.0f);
	UBOOL bEffectiveLeanRight = (bLeanInputRight || AnalogLeanInputRight > 0.0f);
	FLOAT analogLeanInput = Clamp(-AnalogLeanInputLeft + AnalogLeanInputRight + (bLeanInputLeft ? -1.0f : 0.0f) + (bLeanInputRight ? 1.0f : 0.0f), -1.0f, 1.0f);
	
	UBOOL bLeanInput = (bEffectiveLeanLeft != bEffectiveLeanRight);

	if (HeroPawn->LocomotionMode == LM_Walk)
	{
		UBOOL leaning = FALSE;

		UBOOL bTryRun = bRunInput && HeroPawn->CanRun();

		if (bTryRun && !HeroPawn->bWantToRun)
		{
			HeroPawn->TryRun();
		}
		else if (!bTryRun && HeroPawn->bWantToRun)
		{
			HeroPawn->Walk();
		}

		if (!HeroPawn->IsDoingSpecialMove() && !HeroPawn->IsInCamcorderTransition())
		{
			if (HeroPawn->IsRunning())
			{				
				if (bLeanInput && HeroPawn->TryLookBack(bEffectiveLeanLeft))
				{
					bInvalidateLeanInput = TRUE;
					return TRUE;
				}
			}
			else
			{
				if ((bEffectiveLeanLeft != bEffectiveLeanRight) && HeroPawn->TryEnterContextualLean(bEffectiveLeanLeft, bEffectiveLeanRight))
				{
					bInvalidateLeanInput = TRUE;
					return TRUE;
				}

				if (!bInvalidateLeanInput && !appIsNearlyZero(analogLeanInput, KINDA_SMALL_NUMBERF))
				{
					leaning = HeroPawn->TryLean(analogLeanInput);
				}
			}
		}

		if (!leaning)
		{
			HeroPawn->StopLeaning();
		}
	}
	else if (HeroPawn->LocomotionMode == LM_Pushing)
	{
		UBOOL leaning = FALSE;

		if (bEffectiveLeanLeft && !bEffectiveLeanRight)
		{
			leaning = HeroPawn->TryLeanLeftPushing();
		}
		else if (bEffectiveLeanRight && !bEffectiveLeanLeft)
		{
			leaning = HeroPawn->TryLeanRightPushing();
		}

		if (leaning)
		{
			bInvalidateLeanInput = TRUE;
		}
		else if (HeroPawn->bLeaningLeftPushing || HeroPawn->bLeaningRightPushing)
		{
			HeroPawn->StopLeanPushing();
		}
	}
	else if (HeroPawn->LocomotionMode == LM_ContextualLean || HeroPawn->SpecialMove == SMT_EnterContextualLean)
	{
		HeroPawn->UpdateContextualLean(analogLeanInput, playerInputIntent);
	}

	if (HeroPawn->LocomotionMode == LM_LookBack && (!bLeanInput || !bRunInput || !HeroPawn->CanLookBack()))
	{
		// stop looking back
		HeroPawn->TryStopLookBack();
		bLeanInputRight = FALSE;
		bLeanInputLeft = FALSE;
		AnalogLeanInputLeft = 0.0f;
		AnalogLeanInputRight = 0.0f;
	}

	HeroPawn->UpdateLookBackIntent(bLeanInput);

	if (!bLeanInput)
	{
		bInvalidateLeanInput = FALSE;
	}

	if (Inputs.bPressedToggleCamcorder)
	{
		HeroPawn->TryToggleCamcorder();
	}
	else if (Inputs.bPressedToggleNightVision)
	{
		HeroPawn->TryToggleNightVision();
	}
	else if (Inputs.bPressedReloadBatteries)
	{
		HeroPawn->ReloadBatteries();
	}

	if ((Inputs.bPressedZoomIn || Inputs.bPressedZoomOut))
	{
		HeroPawn->ZoomImpulse(Inputs.bPressedZoomIn ? 1.0f : -1.0f);
	}
	else if (Inputs.bStartedActiveZoom)
	{
		HeroPawn->StartedActiveZoom(ZoomInput > 0);
	}

	UpdateTouchZoom(deltaSeconds);

	UBOOL bCanCrouch = HeroPawn->CanCrouch();
	if (bDuck && HeroPawn->CanCrouch() && !HeroPawn->bIsCrouched && HeroPawn->TryCrouch())
	{
		return TRUE;
	}

	if ((!bDuck || !bCanCrouch) && HeroPawn->LocomotionMode == LM_Walk && HeroPawn->bIsCrouched && HeroPawn->TryUncrouch())
	{
		return TRUE;
	}

	if ((HeroPawn->LocomotionMode == LM_Walk || HeroPawn->LocomotionMode == LM_Fall) && HeroPawn->TryPushFromLedge(playerInputIntent))
	{
		return TRUE;
	}
	
	// Block non-interruptible special moves
	if (HeroPawn->IsDoingSpecialMove() && !HeroPawn->CanInterruptSpecialMove())
	{
		return FALSE;	
	}
	
	if (playerInputIntent.SizeSquared2D() >= Square(MinPlayerInputIntentForSpecialMove))
	{
		FVector playerInputDirection = playerInputIntent.SafeNormal2D();

		// Check if we should automatically start a special move
		for (INT idx = 0; idx < HeroPawn->Touching.Num(); idx++)
		{
			AOLGameplayVolume* gameplayVolume = Cast<AOLGameplayVolume>(HeroPawn->Touching(idx));

			if (gameplayVolume && gameplayVolume->IsValid())
			{
				AOLSqueezeVolume* squeezeVolume = Cast<AOLSqueezeVolume>(gameplayVolume);
				if (squeezeVolume && HeroPawn->TryEnterSqueeze(playerInputDirection, squeezeVolume))
				{
					return TRUE;
				}			
			}
		}

		if (HeroPawn->TryEnterLedgeWalk(playerInputDirection))
		{
			return TRUE;
		}	

		if ( (HeroPawn->LocomotionMode == LM_LedgeHang || HeroPawn->LocomotionMode == LM_LedgeWalk) && HeroPawn->TryLedgeTransition(playerInputDirection))
		{		
			return TRUE;
		}

		if (HeroPawn->TryEnterLadder(playerInputDirection))
		{
			bDuck = FALSE;
			return TRUE;
		}	

		if (Inputs.bPressedJump && HeroPawn->LocomotionMode == LM_LedgeWalk && ((playerInputDirection | HeroPawn->Rotation.Vector()) > 0.707f) && HeroPawn->TryJumpFromLedgeWalk(TRUE))
		{
			return TRUE;
		}

		if (HeroPawn->LocomotionMode == LM_Squeeze && HeroPawn->TryExitSqueeze(playerInputDirection))
		{
			return TRUE;
		}

		if (HeroPawn->LocomotionMode == LM_Bed && HeroPawn->TryExitBed(playerInputDirection))
		{
			return TRUE;
		}
	}

	if ( (Inputs.bPressedJump || HeroPawn->Physics == PHYS_Falling) && HeroPawn->TryPassObstacle(playerInputIntent.SafeNormal2D()))
	{
		return TRUE;
	}

	if ( (Inputs.bPressedJump || HeroPawn->Physics == PHYS_Falling) && HeroPawn->TryGrabLedge(playerInputIntent))
	{
		return TRUE;
	}

	if (HeroPawn->Physics == PHYS_Falling && HeroPawn->TryGrabAndClimb(playerInputIntent))
	{
		return TRUE;
	}

	if (bDuck && HeroPawn->LocomotionMode == LM_LedgeWalk && HeroPawn->TryJumpFromLedgeWalk(FALSE))
	{
		bDuck = FALSE;
		return TRUE;
	}

	UBOOL ledgeActionValid = (HeroPawn->LocomotionMode == LM_LedgeHang) || ((HeroPawn->SpecialMove == SMT_GrabLedgeFromAir || HeroPawn->SpecialMove == SMT_GrabLedgeFromGround) && HeroPawn->CanInterruptSpecialMove());
	if (bDuck && ledgeActionValid && HeroPawn->TryDropFromLedge())
	{		
		bDuck = FALSE;
		return TRUE;
	}

	if (HeroPawn->Physics == PHYS_Falling && HeroPawn->TryGrabLadder(playerInputIntent))
	{
		bDuck = FALSE;
		return TRUE;
	}

	if (HeroPawn->LocomotionMode == LM_Ladder && HeroPawn->TryExitLadder())
	{
		return TRUE;
	}

	if (bDuck && HeroPawn->LocomotionMode == LM_Ladder && HeroPawn->TryDropFromLadder())
	{		
		bDuck = FALSE;
		return TRUE;
	}
		
	if (ledgeActionValid && HeroPawn->TryClimbUpLedge(Inputs.bPressedJump, playerInputIntent))
	{
		TutorialManager->ConditionalClimbUpTutorialCompleted();
		return TRUE;
	}
	
	if (Inputs.bPressedJump && HeroPawn->LocomotionMode == LM_Walk && HeroPawn->TryJump(playerInputIntent))
	{
		return TRUE;
	}

	if (HeroPawn->LocomotionMode == LM_Walk && HeroPawn->TryCSA(Inputs.bPressedUseButton))
	{
		return TRUE;
	}

	if (HeroPawn->LocomotionMode == LM_Walk && HeroPawn->TryObjectPickup(Inputs.bPressedUseButton))
	{
		return TRUE;
	}
	
	if (HeroPawn->LocomotionMode == LM_Walk && HeroPawn->TryEnterBed(Inputs.bPressedUseButton))
	{
		return TRUE;
	}	

	if (HeroPawn->LocomotionMode == LM_Walk && HeroPawn->TryOpenAndEnterLocker(Inputs.bPressedUseButton))
	{
		return TRUE;
	}

	if (HeroPawn->LocomotionMode == LM_Locker && HeroPawn->TryOpenAndExitLocker(Inputs.bPressedUseButton))
	{
		return TRUE;
	}
	
	if (HeroPawn->LocomotionMode == LM_Walk)
	{
		UBOOL playerInteractIntent = Inputs.bReleasedUseButton && (UsePressedTime < 0.25f);
		if (HeroPawn->TryDoorInstantInteraction(playerInteractIntent, playerInputIntent)) // Instant Open/Close
		{
			return TRUE;
		}
	}

	UBOOL doorOpenActionAvailable = AvailableInteractions.ContainsItem(PIT_OpenDoor) || AvailableInteractions.ContainsItem(PIT_AutoCloseDoor) || AvailableInteractions.ContainsItem(PIT_LockedDoor);

	if (Inputs.bPressedUseButton && doorOpenActionAvailable)
	{
		bValidDoorHold = TRUE;
	}
	else if (!bUseButtonDown)
	{
		bValidDoorHold = FALSE;
	}
	else if (bValidDoorHold && !doorOpenActionAvailable)
	{
		bValidDoorHold = FALSE;
	}	

	if ((Inputs.bReleasedUseButton || !bUseButtonDown) && HeroPawn->LocomotionMode == LM_Door)
	{
		HeroPawn->StopInteractiveOpen(); // Release held door
		return TRUE;
	}

	if (bValidDoorHold && (UsePressedTime > 0.25f) && (HeroPawn->LocomotionMode == LM_Walk))
	{
		if (HeroPawn->TryDoorInteractiveOpen()) // Interactive Open
		{
			bInvalidateReleasedUse = TRUE; // eat up the holding input, so it can't be reused
			bValidDoorHold = FALSE;
			return TRUE;
		}
	}

	if (!bValidDoorHold && HeroPawn->LocomotionMode == LM_Walk && HeroPawn->TryPushObject(Inputs.bPressedUseButton && !Inputs.bReleasedUseButton))
	{
		return TRUE;
	}

	if (HeroPawn->LocomotionMode == LM_Pushing && (Inputs.bReleasedUseButton || !bUseButtonDown))
	{
		HeroPawn->StopPushing();
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Achievements
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::CheckCollectibleAchievement(UBOOL bFoundDocument, const FString& collectibleName)
{
	
}

void AOLPlayerController::CheckForAchievementOnRemoteEvent(const FName& eventName)
{
}

////////////////////////////////////////////////////////////////////////////////////////////
// Struggle
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::InitStruggleOnEnemy(UOLSeqAct_Struggle* struggleAct)
{
	if (struggleAct->Enemy)
	{
		UAnimNodeSlot* enemyAnimSlot = Cast<UAnimNodeSlot>(struggleAct->Enemy->SkeletalMeshComponent->FindAnimNode("AnimSlot"));
		if (enemyAnimSlot)
		{
			enemyAnimSlot->SetActiveChild(0, 0.1f);
		}

		UOLAnimEnemyStruggle* enemyAnimNode = Cast<UOLAnimEnemyStruggle>(struggleAct->Enemy->SkeletalMeshComponent->FindAnimNode("EnemyStruggle"));

		if (enemyAnimNode && struggleAct->Config.InitIdleAnimEnemy != NAME_None)
		{
			enemyAnimNode->SetPhase(SAP_StartIdle, struggleAct->Config);
		}
	}
}

void AOLPlayerController::ActivateStruggle(UOLSeqAct_Struggle* struggleAct)
{	
	if (HeroPawn->SpecialMove == SMT_EnterStruggle)
	{
		// filter multiple triggers
		return;
	}

	appMemZero(Struggle);
	Struggle.StruggleAct = struggleAct;
	Struggle.Enemy = struggleAct->Enemy;
	Struggle.Config = struggleAct->Config;
	Struggle.SmoothedAnimPlayRate = 0.0f;
	Struggle.RefLocation = struggleAct->RefLocation;
	Struggle.RefDirection = struggleAct->RefDirection;	

	if (Struggle.Enemy)
	{
		Struggle.EnemyAnimNode = Cast<UOLAnimEnemyStruggle>(Struggle.Enemy->SkeletalMeshComponent->FindAnimNode("EnemyStruggle"));

		if (!Struggle.EnemyAnimNode)
		{
			// something is wrong. this isn't going anywhere - abort.
			appMemZero(Struggle);
			struggleAct->bSucceeded = TRUE;
			return;
		}

		Struggle.EnemyAnimNode->SetPhase(SAP_Entry, Struggle.Config);
		Struggle.Enemy->SkeletalMeshComponent->bUpdateSkelWhenNotRendered = TRUE; // force updating, in case we approach backwards
		Struggle.Enemy->SkeletalMeshComponent->RootMotionMode = RMM_Translate;
		Struggle.Enemy->SkeletalMeshComponent->RootMotionRotationMode = RMRM_RotateActor;
	}

	HeroPawn->StartStruggle(Struggle.Config, Struggle.RefLocation, Struggle.RefDirection);	
}

void AOLPlayerController::StruggleEntryCompleted()
{
	Struggle.bActiveStrugging = TRUE;
	Struggle.CycleStartedTime = GWorld->GetTimeSeconds();
	Struggle.LastMouseX = PureMouseX;
	Struggle.SmoothedAnimPlayRate = 1.0f;

	if (Struggle.EnemyAnimNode)
	{
		Struggle.EnemyAnimNode->SetPhase(SAP_Cycle, Struggle.Config);
	}
}

void AOLPlayerController::StruggleExitCompleted()
{
	if (Struggle.StruggleAct)
	{
		Struggle.StruggleAct->bSucceeded = Struggle.bSucceeded;
		Struggle.StruggleAct->bFailed = !Struggle.bSucceeded;
	}

	if (Struggle.bSucceeded && Struggle.EnemyAnimNode)
	{
		Struggle.EnemyAnimNode->SetPhase(SAP_SuccessIdle, Struggle.Config);
	}

	appMemZero(Struggle);

	// no rush at all, but make sure to clean the anim set eventually (can't do it now though cause the anim is blending out)
	HeroPawn->PendingAnimSetUpdateTime = GWorld->GetTimeSeconds() + 10.0f; 	
}

void AOLPlayerController::UpdateStruggle(FLOAT deltaSeconds)
{
	TWEAKABLE FLOAT NoFailCooldownCoeff = 0.25f;

	if (Struggle.bActiveStrugging)
	{
		UBOOL bDone = (GWorld->GetTimeSeconds() > Struggle.CycleStartedTime + Struggle.Config.CycleTime);

		FLOAT timePct = Saturate((GWorld->GetTimeSeconds() - Struggle.CycleStartedTime) / Struggle.Config.CycleTime);
		FLOAT currentSuccessPct = 0.0f;
		
		if (PlayerInput->bUsingGamepad)
		{
			FLOAT winThresold = 0.0f;
			if (Struggle.Config.bNoFail)
			{
				winThresold = StruggleShakesThresholdForWinNoFail; 
			}
			else
			{
				winThresold = StruggleShakesThresholdForWin * Struggle.Config.CycleTime;
			}
			currentSuccessPct = Struggle.NbShakes / winThresold;
		}
		else
		{
			FLOAT winThresold = 0.0f;
			if (Struggle.Config.bNoFail)
			{
				winThresold = StruggleInputThresholdForWinNoFail;
			}
			else
			{
				winThresold = StruggleInputThresholdForWin * Struggle.Config.CycleTime;
			}
			currentSuccessPct = (Struggle.TotalDeltas * Struggle.NbShakes) / winThresold;
		}

		UBOOL success = currentSuccessPct >= 1.0f;
		currentSuccessPct = Saturate(currentSuccessPct);

		UBOOL bStruggleNoFail = FALSE;
		GConfig->GetBool( TEXT("Patches"), TEXT("StruggleNoFail"), (UBOOL&)bStruggleNoFail, GGameIni);

		if (bStruggleNoFail)
		{
			currentSuccessPct = 1.0f;
			success = TRUE;
		}
		
		if (Struggle.Config.bNoFail)
		{
			TWEAKABLE FLOAT StruggleNoFailMaxTime = 15.0f;
			UBOOL bFailSafeExit = (GWorld->GetTimeSeconds() > Struggle.CycleStartedTime + StruggleNoFailMaxTime);

			if (bFailSafeExit)
			{
				bDone = TRUE;
				success = TRUE;
			}
			else
			{
				bDone = (bDone && success); // must succeed to finish, or run through the failsafe time (for e.g. touchpads or weird mouse behaviors)
			}

			if (!bDone)
			{
				// cooldown effect on no-fails
				Struggle.NbShakes = Utils::Approach(Struggle.NbShakes, 0.0f, NoFailCooldownCoeff, deltaSeconds);
				Struggle.TotalDeltas = Utils::Approach(Struggle.TotalDeltas, 0.0f, NoFailCooldownCoeff, deltaSeconds);
			}
		}

		if (bDone)
		{
			Struggle.bActiveStrugging = FALSE;			
			Struggle.bSucceeded = success;

			if (Struggle.EnemyAnimNode)
			{
				Struggle.EnemyAnimNode->SetPhase(success ? SAP_Success : SAP_Fail, Struggle.Config);
			}

			HeroPawn->FinishStruggle(success);
		}
		else
		{
			// Update input

			if (PlayerInput->bUsingGamepad)
			{
				FLOAT rightStickValue = PlayerInput->RawJoyLookRight;

				if ((Struggle.CurrentInputDirection != SID_Left && rightStickValue > 0.9f) || (Struggle.CurrentInputDirection != SID_Right && rightStickValue < -0.9f))
				{
					Struggle.CurrentInputDirection = (Struggle.CurrentInputDirection != SID_Right ? SID_Right : SID_Left);
					Struggle.NbShakes += 1.0f;
				}
			}
			else
			{
				FLOAT deltaInput = (PureMouseX - Struggle.LastMouseX) * (deltaSeconds / 0.016f); // normalized @ 60fps
				Struggle.LastMouseX = PureMouseX;

				TWEAKABLE FLOAT MinInputForDirection = 10.0f;				
				TWEAKABLE FLOAT SmoothingCoeff = 0.9999f;
				TWEAKABLE FLOAT DirThresh = 0.2f;
				FLOAT instantDir = 0.0f;
				if (Abs(deltaInput) > MinInputForDirection)
				{
					instantDir = (deltaInput > 0.0f ? 1.0f : -1.0f);					
				}

				Struggle.SmoothedDirection = Utils::Approach(Struggle.SmoothedDirection, instantDir, SmoothingCoeff, deltaSeconds);
				Struggle.SmoothedDirection = Clamp(Struggle.SmoothedDirection, -1.0f, 1.0f);
				
				if ((Struggle.SmoothedDirection > DirThresh && Struggle.CurrentInputDirection != SID_Left) || (Struggle.SmoothedDirection < -DirThresh && Struggle.CurrentInputDirection != SID_Right))
				{
					Struggle.CurrentInputDirection = (Struggle.CurrentInputDirection != SID_Right ? SID_Right : SID_Left);
					Struggle.NbShakes += 1.0f;
				}

				Struggle.TotalDeltas += Abs(deltaInput);
			}
			
			// Update health

			if (!Struggle.Config.bNoFail)
			{
				FLOAT currentHealth = 100.0f*(1.0f - timePct*0.75f*(1.0f-currentSuccessPct));
				HeroPawn->SetHealth(currentHealth);
			}

			// Update anim rate
			TWEAKABLE FLOAT StruggleApproachCoeff = 0.99f;
			
			FLOAT effectiveMaxRate = LerpClamped(timePct, Struggle.Config.MaxRateStart, Struggle.Config.MaxRateEnd);

			FLOAT successDelta = timePct > 0.1f ? (currentSuccessPct/timePct) : 0.0f;
			FLOAT unboundTargetRate = Struggle.Config.MinRate + (effectiveMaxRate-Struggle.Config.MinRate)*(successDelta - Struggle.Config.SuccessPctForMinRate)/(Struggle.Config.SuccessPctForMaxRate-Struggle.Config.SuccessPctForMinRate);
			FLOAT targetAnimRate = Clamp(unboundTargetRate, Struggle.Config.MinRate, effectiveMaxRate);
			
			Struggle.SmoothedAnimPlayRate = Utils::Approach(Struggle.SmoothedAnimPlayRate, targetAnimRate, StruggleApproachCoeff, deltaSeconds);			
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Audio
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::UpdateMusic(FLOAT DeltaSeconds)
{
	AController* CurrentController = GWorld->GetFirstController();

	EAIMusicState NewState = EAIMS_None;

	if (bOverriddenMusic)
	{
		NewState = (EAIMusicState)OverriddenMusicState;
		AIDistance = OverriddenMusicDistance;
	}
	else
	{
		AOLBot* MostAggressiveBotForMusic = NULL;
		FLOAT DistToMostAggressiveForMusic = -1.0f;
		AOLBot* MostAggressiveBotForStress = NULL;
		FLOAT DistToMostAggressiveForStress = -1.0f;

		AOLBot* Bot = NULL;
		while(CurrentController != NULL)
		{
			Bot = Cast<AOLBot>(CurrentController);

			if (Bot)
			{
				UBOOL bUseForMusic = (Bot->EnemyPawn->bUseForMusic || Bot->EnemyPawn->Modifiers.bUseForMusic);
				UBOOL bUseForStress = bUseForMusic || Bot->EnemyPawn->Modifiers.bForceUseForStressBreath;

				if (bUseForMusic || bUseForStress)
				{
					FLOAT Dist = Location.Distance(Bot->EnemyPawn->Location);

					if (bUseForMusic)
					{
						if (MostAggressiveBotForMusic == NULL
							|| Bot->BehaviorState > MostAggressiveBotForMusic->BehaviorState
							|| (Bot->BehaviorState == MostAggressiveBotForMusic->BehaviorState && (DistToMostAggressiveForMusic == -1.0f || DistToMostAggressiveForMusic > Dist)))
						{
							MostAggressiveBotForMusic = Bot;
							DistToMostAggressiveForMusic = Dist;
						}
					}

					if (bUseForStress)
					{
						if (MostAggressiveBotForStress == NULL
							|| Bot->BehaviorState > MostAggressiveBotForStress->BehaviorState
							|| (Bot->BehaviorState == MostAggressiveBotForStress->BehaviorState && (DistToMostAggressiveForStress == -1.0f || DistToMostAggressiveForStress > Dist)))
						{
							MostAggressiveBotForStress = Bot;
							DistToMostAggressiveForStress = Dist;
						}
					}
				}
			}

			CurrentController = CurrentController->NextController;
		}

		if (MostAggressiveBotForMusic != NULL)
		{
			switch(MostAggressiveBotForMusic->BehaviorState)
			{
			case AIBS_Idle:
			case AIBS_Patrolling:
				NewState = EAIMS_Patrol;
				break;
			case AIBS_Investigating:
				NewState = EAIMS_Investigate;
				break;
			case AIBS_Chasing:
				NewState = EAIMS_Chase;
				break;
			}			
		}
		else
		{
			NewState = EAIMS_None;
		}

		if (MostAggressiveBotForStress)
		{
			AIDistance = DistToMostAggressiveForStress*0.01f;
		}
		else
		{
			AIDistance = 0.0f;
		}
	}

	if (NewState != AIMusic)
	{
		AIMusic = NewState;

		if (MusicAIStateGroup != NAME_None)
		{
			switch(AIMusic)
			{
			case EAIMS_None:
				SetState(MusicAIStateGroup, MusicAIStateNone);
				break;
			case EAIMS_Patrol:
				SetState(MusicAIStateGroup, MusicAIStatePatrol);
				break;
			case EAIMS_Investigate:
				SetState(MusicAIStateGroup, MusicAIStateInvestigate);
				break;
			case EAIMS_Chase:
				AIChaseMusicTimer = AIChaseMusicTimeDelay;
				break;
			}
		}

		if (AIChaseMusicTimer > 0.f && AIMusic != EAIMS_Chase)
		{
			AIChaseMusicTimer = 0.f;
		}
	}
	else if (AIMusic == EAIMS_Chase && AIChaseMusicTimer > 0.f)
	{
		AIChaseMusicTimer -= DeltaSeconds;
		
		if (AIChaseMusicTimer <= 0.f)
		{
			AIChaseMusicTimer = 0.f;

			SetState(MusicAIStateGroup, MusicAIStateChase);
		}
	}

	if (AIDistanceRTPC != NAME_None)
	{
		SetRTPCValue( AIDistanceRTPC, AIDistance );
	}

	if (HeroPawn != NULL)
	{
		AOLScareMoment* Scare = NULL;
		for (INT Idx = 0; Idx < HeroPawn->CachedScares.Num(); ++Idx)
		{
			Scare = HeroPawn->CachedScares(Idx);
			if (Scare != NULL && Scare->bEnabled)
			{
				FLOAT DistToScare = Location.Distance(Scare->Location);
				if (Scare->bPlaying && DistToScare > Scare->Range * 100.0f)
				{
					Scare->eventStopScare();
				}
				else if (!Scare->bPlaying && DistToScare <= Scare->Range * 100.0f)
				{
					Scare->eventStartScare();
				}

				if (Scare->bPlaying)
				{
					Scare->UpdateScareRTPC(DistToScare/100.0f);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// HUD
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::UpdateFade(FLOAT deltaSeconds)
{
	if (bEnableFading && FadeTimeRemaining > 0.0)
	{
		FadeTimeRemaining = Max(FadeTimeRemaining - deltaSeconds, 0.0f);
		if (FadeTime > 0.0)
		{
			FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining/FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
		}

		if (FadeTimeRemaining == 0.0f && FadeAlpha.Y == 0.0f)
		{
			bEnableFading = FALSE;
		}
	}
}

void AOLPlayerController::UpdateOverlay(FLOAT deltaSeconds)
{
	if (ActiveSplashScreen && HUD && HUD->bSplashScreenOpen && (HUD->bSplashScreenReady || ActiveSplashScreen->bAlwaysReady))
	{
		if (PlayerInput->IsPressed(FName(TEXT("Spacebar"))) || PlayerInput->IsPressed(FName(TEXT("LeftMouseButton"))) || PlayerInput->IsPressed(FName(TEXT("Enter"))))
		{
			appMemZero(Inputs);
			ActiveSplashScreen->Continue();
		}
	}

	if (Utils::IsDemo() && GameOverActivatedTimestamp > 0.0f && GWorld->GetRealTimeSeconds() > GameOverActivatedTimestamp + 10.0f)
	{
		// quit to main menu
		GEngine->Exec(*(FString(TEXT("open ")) + Utils::GetOLGame()->DemoMapName.ToString()));
	}
}

void AOLPlayerController::ShowSplashScreen(UOLSeqAct_SplashScreen* splashScreenAction)
{
	ActiveSplashScreen = splashScreenAction;

	if (HUD)
	{
		HUD->ShowSplashScreen();
		eventFullScreenOverlayChanged();
	}	
}

void AOLPlayerController::HideSplashScreen()
{
	if (HUD)
	{
		HUD->bSplashScreenOpen = FALSE;
		eventFullScreenOverlayChanged();
	}
	ActiveSplashScreen = NULL;
}

void AOLPlayerController::StartBlockOnLoading()
{
	if (!bBlockedOnLoading)
	{
		bBlockedOnLoading = TRUE;
		GWorld->GetWorldInfo()->bRequestedBlockOnAsyncLoading = TRUE;
		ShowLoadingOverlay();
	}
}

void AOLPlayerController::StopBlockOnLoading()
{
	if (bBlockedOnLoading)
	{
		bBlockedOnLoading = FALSE;
		HideLoadingOverlay();		

		if (PlayerInput)
		{
			PlayerInput->ResetInput();
		}
	}
}

void AOLPlayerController::KismetRequestedShowLoadingScreen()
{
	if (!bShowingKismetControlledLoadingScreen)
	{
		bShowingKismetControlledLoadingScreen = TRUE;
		ShowLoadingMovie(TRUE);
	}
}

void AOLPlayerController::KismetRequestedHideLoadingScreen()
{
	if (bShowingKismetControlledLoadingScreen)
	{
		bShowingKismetControlledLoadingScreen = FALSE;
		ShowLoadingMovie(FALSE);

		if (PlayerInput)
		{
			PlayerInput->ResetInput();
		}
	}
}

void AOLPlayerController::ShowLoadingOverlay()
{
	if (!bShowingKismetControlledLoadingScreen &&  GFullScreenMovie && !GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE))
	{
		bShowingLoadingOverlay = TRUE;
		GFullScreenMovie->GameThreadPlayMovie(MM_LoopFromMemory, UCONST_LOADING_MOVIE, 0, 6);
	}
}

void AOLPlayerController::HideLoadingOverlay()
{
	if (bShowingLoadingOverlay && GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE))
	{
		GFullScreenMovie->GameThreadStopMovie();
		bShowingLoadingOverlay = FALSE;
	}
}

void AOLPlayerController::TryDingoUnlockAchievement(EOutlastAchievement achievement)
{
#if DINGO
	GOLDingo->EventTryUnlockAchievement(achievement);
#endif
}

void AOLPlayerController::GameOver()
{
	if (HUD)
	{
		HUD->ShowGameOver();
		eventFullScreenOverlayChanged();
		GameOverActivatedTimestamp = GWorld->GetRealTimeSeconds();
	}	
}

void AOLPlayerController::SetNewObjective(FName newObjective, UBOOL bForce)
{
	if (!bForce && CompletedObjectives.ContainsItem(newObjective)) // already completed
	{
		return;
	}

	if (CurrentObjective == newObjective) 
	{
		if (!bForce)
		{
			return; // current objective
		}
	}
	else if (CurrentObjective != NAME_None)
	{
		CompletedObjectives.AddUniqueItem(CurrentObjective);
	}

	CurrentObjective = newObjective;

	if (CurrentObjective != NAME_None)
	{
		if (HUD)
		{
			HUD->ShowNewObjective();
		}

		HeroPawn->TriggerSoundEvent(HeroPawn->SndNewObjective);
	}
}

void AOLPlayerController::AddAvailableInteraction(EPlayerInteractionType interactionType)
{
	AvailableInteractions.AddUniqueItem(interactionType);
}

void AOLPlayerController::ProcessCompletedRecording(AOLRecordingMarker* recordingMarker)
{
	HUD->ShowRecordingCompleteMessage();
	HUD->SetLatestRecording(recordingMarker->MomentName);
	CompletedRecordingMoments.AddUniqueItem(recordingMarker->MomentName);
	UnreadRecordingMoments.AddUniqueItem(recordingMarker->MomentName);
	HeroPawn->TriggerSoundEvent(HeroPawn->SndRecordingCompleted);
	PendingRecordingMarker = NULL;

	CheckCollectibleAchievement(FALSE, recordingMarker->MomentName.ToString());
}

void AOLPlayerController::NativeApplyRemoteRecording(AOLRecordingMarker* Marker)
{
	if (!Marker || Marker->bRecorded)
		return;

	Marker->bRecorded = TRUE;
	ProcessCompletedRecording(Marker);
}

void AOLPlayerController::RecordingCompleted(AOLRecordingMarker* recordingMarker)
{
	recordingMarker->RecordingComplete();

	// Notify script (MultiplayerController overrides this to send the packet to peers)
	struct { AOLRecordingMarker* Marker; } Parms;
	Parms.Marker = recordingMarker;
	ProcessEvent(FindFunctionChecked(FName(TEXT("NotifyRecordingCompleted"))), &Parms);

	if (appIsNearlyZero(recordingMarker->NotificationDelay, KINDA_SMALL_NUMBERF))
	{
		ProcessCompletedRecording(recordingMarker);
	}
	else
	{
		RecordingCompletedTime = GWorld->GetTimeSeconds();
		PendingRecordingMarker = recordingMarker;
	}
}

UBOOL AOLPlayerController::IsPlayerFindableWhileHidden(AOLEnemyPawn* searchingEnemy)
{
	return TRUE;
}

void AOLPlayerController::SetPlayerFoundWhileHidden(AOLEnemyPawn* searchingEnemy)
{
	if (searchingEnemy->IsA(AOLEnemySoldier::StaticClass()))
	{
		bFoundBySoldierWhileHidden = TRUE;
	}
	else if (searchingEnemy->IsA(AOLEnemySurgeon::StaticClass()))
	{
		bFoundBySurgeonWhileHidden = TRUE;
	}
}

void MarkChainRemoteObserver(USequenceOp* Op, int Depth)
{
	if (!Op || Depth > 16)
		return;
	for (INT i = 0; i < Op->OutputLinks.Num(); i++)
	{
		for (INT j = 0; j < Op->OutputLinks(i).Links.Num(); j++)
		{
			USequenceOp* Next = Op->OutputLinks(i).Links(j).LinkedOp;
			if (!Next)
				continue;
			USeqAct_Interp* Interp = Cast<USeqAct_Interp>(Next);
			USeqAct_ToggleCinematicMode* CinMode = Cast<USeqAct_ToggleCinematicMode>(Next);
			USeqAct_ActorFactory* ActorFactory = Cast<USeqAct_ActorFactory>(Next);
			UOLSeqAct_Struggle* Struggle = Cast<UOLSeqAct_Struggle>(Next);
			if (Interp)
			{
				Interp->bRemoteObserverMatinee = TRUE;
				Interp->bObserverOnly = TRUE;
			}
			if (CinMode)
			{
				CinMode->bObserverOnly = TRUE;
			}
			if (ActorFactory)
			{
				ActorFactory->bObserverOnly = TRUE;
			}
			if (Struggle)
			{
				Struggle->bObserverOnly = TRUE;
			}
			MarkChainRemoteObserver(Next, Depth + 1);
		}
	}
}

void UnmarkChainRemoteObserver(USequenceOp* Op, int Depth)
{
	if (!Op || Depth > 16)
		return;
	for (INT i = 0; i < Op->OutputLinks.Num(); i++)
	{
		for (INT j = 0; j < Op->OutputLinks(i).Links.Num(); j++)
		{
			USequenceOp* Next = Op->OutputLinks(i).Links(j).LinkedOp;
			if (!Next)
				continue;
			USeqAct_Interp* Interp = Cast<USeqAct_Interp>(Next);
			USeqAct_ToggleCinematicMode* CinMode = Cast<USeqAct_ToggleCinematicMode>(Next);
			USeqAct_ActorFactory* ActorFactory = Cast<USeqAct_ActorFactory>(Next);
			UOLSeqAct_Struggle* Struggle = Cast<UOLSeqAct_Struggle>(Next);
			if (Interp)
			{
				Interp->bRemoteObserverMatinee = FALSE;
				Interp->bObserverOnly = FALSE;
			}
			if (CinMode)
				CinMode->bObserverOnly = FALSE;
			if (ActorFactory)
				ActorFactory->bObserverOnly = FALSE;
			if (Struggle)
				Struggle->bObserverOnly = FALSE;
			UnmarkChainRemoteObserver(Next, Depth + 1);
		}
	}
}

void AOLPlayerController::ForceActivateTouchEvent(USeqEvent_Touch* TouchEvent)
{
	if (!TouchEvent)
		return;
	AWorldInfo* WI = GWorld ? GWorld->GetWorldInfo() : NULL;
	if (!WI)
		return;
	MarkChainRemoteObserver(TouchEvent, 0);
	TouchEvent->bActive = FALSE;
	debugf(TEXT("[TA] ForceActivateTouchEvent: %s bEnabled=%d TriggerCount=%d MaxTriggerCount=%d ParentSeq=%d"),
		*TouchEvent->GetName(), (INT)TouchEvent->bEnabled, TouchEvent->TriggerCount, TouchEvent->MaxTriggerCount, (INT)(TouchEvent->ParentSequence!=NULL));
	TouchEvent->ActivateEvent(WI, WI);
	debugf(TEXT("[TA] ForceActivateTouchEvent: done"));
}

void AOLPlayerController::ObserverActivateCSA(AOLCSA* CSA, UBOOL bConsumeActivation)
{
	if (!CSA)
		return;
	debugf(TEXT("[CSA] ObserverActivateCSA: CSA=%s Events=%d"), *CSA->GetName(), CSA->GeneratedEvents.Num());
	for (INT i = 0; i < CSA->GeneratedEvents.Num(); i++)
	{
		UOLSeqEvent_CSAActivated* Ev = Cast<UOLSeqEvent_CSAActivated>(CSA->GeneratedEvents(i));
		debugf(TEXT("[CSA]   Event[%d]=%s bEnabled=%d"), i, Ev ? *Ev->GetName() : TEXT("NULL"), Ev ? (INT)Ev->bEnabled : -1);
		if (!Ev || !Ev->bEnabled) continue;
		MarkChainRemoteObserver(Ev, 0);
		Ev->ActivateEvent(CSA, NULL);
		// ActivateEvent already incremented Ev->TriggerCount.
		// CSA->TriggerCount tracks whether this CSA "slot" is consumed for future triggers.
		if (!bConsumeActivation)
			CSA->TriggerCount = Max(CSA->TriggerCount - 1, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Systems
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPlayerController::GetLocationsForStreamingVolumes(TArray<FVector>& viewLocations)
{
	if (bForceLevelReset)
	{
		viewLocations.Empty();
		return;
	}

	if (bTravellingToCheckpoint) // special case when flushing the streaming, after loading a checkpoint, as the hero camera isn't yet set.
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		if (!GameEngine || (!GameEngine->IsPreparingMapChange() && GWorld->StreamingVolumeUpdateDelay == 0))
		{
			AOLGame* olGame = Cast<AOLGame>(GWorld->GetGameInfo());
			check(olGame);
			AOLCheckpoint* cpObject = Utils::GetCheckpointFromName(olGame->CurrentCheckpointName);

			if (cpObject)
			{
				viewLocations.AddItem(cpObject->Location + VecZ(20.0f));
			}
		}

		return;
	}

	if (bDebugFreeCam || bDebugFixedCam)
	{
		viewLocations.AddItem(DebugCamPos);
	}		

	if (HeroPawn)
	{
		// Include both location and camera

		viewLocations.AddItem(HeroPawn->Location);
		
		FRotator dummyRot;
		FLOAT dummyFov;
		FVector camLoc;
		HeroPawn->GetCamera(camLoc, dummyRot, dummyFov);
		viewLocations.AddItem(camLoc);
	}
}

void AOLPlayerController::ResetWorldState()
{
	TArray<AOLPushableObject*> pushables; // delayed to make sure they're initialized after all doors, as the linkeddoor makes a sequence dependency

	LevelsResetAfterPlayerDeath.Empty();

	for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
	{
		ULevelStreaming* LevelStreamingObject = WorldInfo->StreamingLevels(LevelIndex);
		if( LevelStreamingObject && LevelStreamingObject->LoadedLevel && LevelStreamingObject->bIsVisible)
		{
			LevelsResetAfterPlayerDeath.AddItem(LevelStreamingObject->PackageName);
		}
	}

	for (FActorIterator It; It; ++It)
	{
		AActor* actor = *It;
		if (actor && actor->IsAStaticMeshActor())
		{
			// early out of the most common case
			continue;
		}

		AOLDoor* door = Cast<AOLDoor>(*It);
		if (door)
		{
			door->Reset();
			continue;
		}

		AOLPushableObject* pushable = Cast<AOLPushableObject>(*It);
		if (pushable)
		{
			pushables.AddItem(pushable);			
			continue;
		}

		AOLGameplayMarker* marker = Cast<AOLGameplayMarker>(*It);
		if (marker)
		{
			marker->Reset();
			continue;
		}

		AOLCSA* csa = Cast<AOLCSA>(*It);
		if (csa)
		{
			csa->Reset();
			continue;
		}

		AOLDarknessVolume* darknessVolume = Cast<AOLDarknessVolume>(*It);
		if (darknessVolume)
		{
			darknessVolume->Reset();
			continue;
		}

		APhysicsVolume* physVolume = Cast<APhysicsVolume>(*It);
		if (physVolume)
		{
			physVolume->Reset();
			continue;
		}

		ALevelStreamingVolume* levelStreamingVolume = Cast<ALevelStreamingVolume>(*It);
		if (levelStreamingVolume)
		{
			levelStreamingVolume->Reset();
			continue;
		}

		AOLBashableObject* bashable = Cast<AOLBashableObject>(*It);
		if (bashable)
		{
			bashable->Reset();
			continue;
		}

		AOLGameplayItemPickup* gameplayItem = Cast<AOLGameplayItemPickup>(*It);
		if (gameplayItem)
		{
			gameplayItem->Reset();
			continue;
		}

		AOLBatteriesPickupFactory* battery = Cast<AOLBatteriesPickupFactory>(*It);
		if (battery)
		{
			battery->Reset();
			continue;
		}
	}

	// Second pass, after doors have been initialized.
	for (INT i = 0; i < pushables.Num(); i++)
	{
		AOLPushableObject* pushable = pushables(i);
		pushable->Reset();
	}

	// Reset all triggers

	TArray<USequenceObject*> allEvents;
	GWorld->GetGameSequence()->FindSeqObjectsByClass(USequenceEvent::StaticClass(), allEvents);

	for (INT i = 0; i < allEvents.Num(); i++)
	{
		USequenceEvent* ev = Cast<USequenceEvent>(allEvents(i));

		if (ev)
		{
			ev->ActivationTime = 0.0f;
			ev->TriggerCount = 0;
			ev->Instigator = NULL;
		}
	}

	// Reset all counters (increment conditions)

	TArray<USequenceObject*> allCounters;
	GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqCond_Increment::StaticClass(), allCounters);

	for (INT i = 0; i < allCounters.Num(); i++)
	{
		USeqCond_Increment* counter = Cast<USeqCond_Increment>(allCounters(i));

		if (counter)
		{
			counter->Reset();
		}
	}

	// Reset all matinees

	TArray<USequenceObject*> allMatinees;
	GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqAct_Interp::StaticClass(), allMatinees);

	for (INT i = 0; i < allMatinees.Num(); i++)
	{
		USeqAct_Interp* matineeSeq = Cast<USeqAct_Interp>(allMatinees(i));

		if (matineeSeq && matineeSeq->bActive && !matineeSeq->bPlayThroughWorldReset)
		{
			matineeSeq->SetPosition(0.0f, TRUE);
			matineeSeq->Stop();
		}
	}

	// Reset eligible set objectives

	TArray<USequenceObject*> allSetObj;
	GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqAct_SetObjective::StaticClass(), allSetObj);

	for (INT i = 0; i < allSetObj.Num(); i++)
	{
		UOLSeqAct_SetObjective* setObj = Cast<UOLSeqAct_SetObjective>(allSetObj(i));

		if (setObj && setObj->bResetOnPlayerDeath)
		{
			setObj->bHasBeenActivated = FALSE;
		}
	}
}


void AOLPlayerController::ResetLevelState(ULevel* level)
{
	TArray<AOLPushableObject*> pushables; // delayed to make sure they're initialized after all doors, as the linkeddoor makes a sequence dependency

	for (INT i = 0; i < level->Actors.Num(); i++)
	{
		AActor* actor = level->Actors(i);

		if (actor && actor->IsAStaticMeshActor())
		{
			// early out of the most common case
			continue;
		}

		AOLDoor* door = Cast<AOLDoor>(actor);
		if (door)
		{
			door->Reset();
			continue;
		}

		AOLPushableObject* pushable = Cast<AOLPushableObject>(actor);
		if (pushable)
		{
			pushables.AddItem(pushable);			
			continue;
		}

		AOLGameplayMarker* marker = Cast<AOLGameplayMarker>(actor);
		if (marker)
		{
			marker->Reset();
			continue;
		}

		AOLCSA* csa = Cast<AOLCSA>(actor);
		if (csa)
		{
			csa->Reset();
			continue;
		}

		AOLDarknessVolume* darknessVolume = Cast<AOLDarknessVolume>(actor);
		if (darknessVolume)
		{
			darknessVolume->Reset();
			continue;
		}

		APhysicsVolume* physVolume = Cast<APhysicsVolume>(actor);
		if (physVolume)
		{
			physVolume->Reset();
			continue;
		}

		ALevelStreamingVolume* levelStreamingVolume = Cast<ALevelStreamingVolume>(actor);
		if (levelStreamingVolume)
		{
			levelStreamingVolume->Reset();
			continue;
		}

		AOLBashableObject* bashable = Cast<AOLBashableObject>(actor);
		if (bashable)
		{
			bashable->Reset();
			continue;
		}

		AOLGameplayItemPickup* gameplayItem = Cast<AOLGameplayItemPickup>(actor);
		if (gameplayItem)
		{
			gameplayItem->Reset();
			continue;
		}

		AOLBatteriesPickupFactory* battery = Cast<AOLBatteriesPickupFactory>(actor);
		if (battery)
		{
			battery->Reset();
			continue;
		}
	}

	// Second pass, after doors have been initialized.
	for (INT i = 0; i < pushables.Num(); i++)
	{
		AOLPushableObject* pushable = pushables(i);
		pushable->Reset();
	}

	// Reset all triggers

	for (INT seqIdx = 0; seqIdx < level->GameSequences.Num(); seqIdx++)
	{
		USequence* sequence = level->GameSequences(seqIdx);
		if (sequence)
		{
			TArray<USequenceObject*> allEvents;
			sequence->FindSeqObjectsByClass(USequenceEvent::StaticClass(), allEvents);

			for (INT i = 0; i < allEvents.Num(); i++)
			{
				USequenceEvent* ev = Cast<USequenceEvent>(allEvents(i));

				if (ev)
				{
					ev->ActivationTime = 0.0f;
					ev->TriggerCount = 0;
					ev->Instigator = NULL;
				}
			}

			// Reset all counters (increment conditions)

			TArray<USequenceObject*> allCounters;
			sequence->FindSeqObjectsByClass(USeqCond_Increment::StaticClass(), allCounters);

			for (INT i = 0; i < allCounters.Num(); i++)
			{
				USeqCond_Increment* counter = Cast<USeqCond_Increment>(allCounters(i));

				if (counter)
				{
					counter->Reset();
				}
			}
		
			// Reset eligible set objectives

			TArray<USequenceObject*> allSetObj;
			sequence->FindSeqObjectsByClass(UOLSeqAct_SetObjective::StaticClass(), allSetObj);

			for (INT i = 0; i < allSetObj.Num(); i++)
			{
				UOLSeqAct_SetObjective* setObj = Cast<UOLSeqAct_SetObjective>(allSetObj(i));

				if (setObj && setObj->bResetOnPlayerDeath)
				{
					setObj->bHasBeenActivated = FALSE;
				}
			}
		}
	}
}

void AOLPlayerController::ApplyCheckpoint(const FName& checkpointName)
{
	checkSlow(checkpointName == Utils::GetCurrentCheckpointName());

	ResetWorldState();

	AOLGameStateList::ApplyCheckpoint(); // Apply automatic game state, and trigger all ApplyGameState events

	TArray<FName>* dummyCPList = AOLCheckpointList::GetCheckpointList(); // Force refresh checkpoint list

	// Activate kismet events

	if (GWorld->GetGameSequence())
	{
		// Player Spawned
		TArray<USequenceObject*> allPlayerSpawnedEvents;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqEvent_PlayerSpawned::StaticClass(), allPlayerSpawnedEvents);
		for (INT Idx = 0; Idx < allPlayerSpawnedEvents.Num(); Idx++)
		{
			USeqEvent_PlayerSpawned* spawnedEvent = Cast<USeqEvent_PlayerSpawned>(allPlayerSpawnedEvents(Idx));
			if (spawnedEvent != NULL)
			{
				spawnedEvent->CheckActivate(this, this);
			}
		}

		// Spawned at Checkpoint
		TArray<USequenceObject*> allSpawnedAtCheckpointEvents;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_SpawnedAtCheckpoint::StaticClass(), allSpawnedAtCheckpointEvents);
		for (INT Idx = 0; Idx < allSpawnedAtCheckpointEvents.Num(); Idx++)
		{
			UOLSeqEvent_SpawnedAtCheckpoint* spawnedEvent = Cast<UOLSeqEvent_SpawnedAtCheckpoint>(allSpawnedAtCheckpointEvents(Idx));
			if (spawnedEvent != NULL)
			{
				if (spawnedEvent->CheckpointName == checkpointName)
				{
					spawnedEvent->CheckActivate(this, this);
				}
			}
		}

		// Apply Checkpoint State
		TArray<USequenceObject*> allApplyCPStateEvents;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_ApplyCheckpointState::StaticClass(), allApplyCPStateEvents);
		for (INT Idx = 0; Idx < allApplyCPStateEvents.Num(); Idx++)
		{
			UOLSeqEvent_ApplyCheckpointState* applyCPStateEvent = Cast<UOLSeqEvent_ApplyCheckpointState>(allApplyCPStateEvents(Idx));
			if (applyCPStateEvent != NULL)
			{
				AOLCheckpointList::TriggerApplyCheckpointStateEvent(applyCPStateEvent);
			}
		}
	}
}

void AOLPlayerController::InitializeKismetSequence(USequence* levelSequence)
{
	if (bTravellingToCheckpoint)
	{
		// We'll triggers these events once we're done travelling
		return;
	}

	AOLGame* olGame = Cast<AOLGame>(GWorld->GetGameInfo());
	if (olGame && olGame->CurrentCheckpointName != NAME_None) // prevent running in main menu when loading checkpoint map
	{
		for (INT Idx = 0; Idx < levelSequence->SequenceObjects.Num(); Idx++)
		{
			UOLSeqEvent_ApplyGameState* applyGSEvent = Cast<UOLSeqEvent_ApplyGameState>(levelSequence->SequenceObjects(Idx));
			if (applyGSEvent)
			{
				AOLGameStateList::TriggerApplyGameStateEvent(applyGSEvent);
			}

			UOLSeqEvent_ApplyCheckpointState* applyCPStateEvent = Cast<UOLSeqEvent_ApplyCheckpointState>(levelSequence->SequenceObjects(Idx));
			if (applyCPStateEvent && applyCPStateEvent->CheckpointName != FName(TEXT("Male_Torture")))
			{
				AOLCheckpointList::TriggerApplyCheckpointStateEvent(applyCPStateEvent);
			}
		}
	}
}

void OnLevelBecomingVisible(ULevelStreaming* streamingLevel)
{
	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (OLPC)
	{
		OLPC->OnLevelBecomingVisible(streamingLevel);
	}
}

void AOLPlayerController::OnLevelBecomingVisible(ULevelStreaming* streamingLevel)
{
	if (!LevelsResetAfterPlayerDeath.ContainsItem(streamingLevel->PackageName))
	{
		debugf(TEXT(" ### --- Resetting level %s"), *streamingLevel->PackageName.ToString());

		ResetLevelState(streamingLevel->LoadedLevel);
		LevelsResetAfterPlayerDeath.AddItem(streamingLevel->PackageName);
	}

	eventOnLevelBecameVisible(streamingLevel->PackageName.ToString());
}

void AOLPlayerController::LoadedLevelListChanged()
{
	// force update light optim
	LastLightOptimCamPos = FVector(FLT_MIN, FLT_MIN, FLT_MIN);
}

void AOLPlayerController::StartTravelToCheckpoint()
{
	if (bCinematicMode)
	{
		eventSetCinematicMode(NULL, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE);
		bIgnoreLookInput = 0;
		bIgnoreMoveInput = 0;
	}

	bHasCamcorder = TRUE;
	bTravellingToCheckpoint = TRUE;
	bTravelCheckPersistent = TRUE;
	StableLevelsTimestamp = -1.0f;

	AOLGame* olGame = Utils::GetOLGame();

	if (olGame)
	{
		olGame->bSoundOnPause = TRUE;
	}

	eventForcePause(TRUE);

	SetState(LoadingStateGroup, LoadingStateOn);
	PostAkEvent(SndResetMixStates);

	eventClientSetCameraFade(TRUE, FColor(0, 0, 0), FVector2D(1.0f, 0.0f), 3.0f, TRUE);
	
	ShowLoadingMovie(TRUE);
}

void AOLPlayerController::TravelComplete()
{
	bTravellingToCheckpoint = FALSE;
	
	eventClientSetCameraFade(TRUE, FColor(0, 0, 0), FVector2D(1.0f, 0.0f), 3.0f, TRUE);

	PostAkEvent(SndResetMixStates);

	AOLGame* olGame = Cast<AOLGame>(GWorld->GetGameInfo());
	check(olGame && olGame->CurrentCheckpointName != NAME_None);

	Utils::GetFXManager()->ResetEffects();

	ApplyCheckpoint(olGame->CurrentCheckpointName);	

	SetState(LoadingStateGroup, LoadingStateOff);

	if (HeroPawn)
	{
#if DINGO
		GOLDingo->EventPlayerSpawned(olGame->CurrentCheckpointName, olGame->DifficultyMode, HeroPawn->Location);
#endif

		HeroPawn->FindTouchingActors();
	}

	UBOOL bStopMovie = (!bShowingKismetControlledLoadingScreen && GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE) == TRUE);
	UBOOL bNoFocus = (HUD && HUD->bLostFocus);

#if DINGO
	UBOOL bShouldReturnToPressStart = FALSE;
	UBOOL bShouldPause = FALSE;
	GOLDingo->VerifyPostTravelState(bShouldReturnToPressStart, bShouldPause);

	if (bShouldReturnToPressStart)
	{
		debugf(TEXT("## OLPC TravelComplete - Returning to Press Start"));
		ShowLoadingMovie(FALSE);
		UOLEngine* olengine = Cast<UOLEngine>(GEngine);
		olengine->ReturnToPressStartScreen();
	}
	else if (bShouldPause)
	{
		debugf(TEXT("## OLPC TravelComplete - Pausing Game"));
		ShowLoadingMovie(FALSE);
		HUD->eventShowMenuType(EMT_PauseMenu);
	}
	else
#endif
	if (bStopMovie && !bNoFocus)
	{
		ShowLoadingMovie(FALSE, TRUE, 2.0f, 0.0f);
	}
	else if (bNoFocus)
	{
		if (bStopMovie)
		{
			ShowLoadingMovie(FALSE);
		}

		// repause through the lost focus system
		eventForcePause(FALSE);
		HUD->eventOnLostFocusPause(TRUE);
	}
	else
	{
		eventForcePause(FALSE);
	}

	HUD->Reset();
	eventOnTravelComplete();
}

void AOLPlayerController::UpdateTravel(FLOAT deltaTime)
{
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);

	if (bTravelCheckPersistent)
	{
		if (GameEngine != NULL && GameEngine->IsPreparingMapChange())
		{
			GameEngine->CancelPendingMapChange();
		}

		AOLGame* OLGame = Cast<AOLGame>(GWorld->GetGameInfo());
		check(OLGame && OLGame->CurrentCheckpointName != NAME_None);

		AOLCheckpoint* cpObject = Utils::GetCheckpointFromName(OLGame->CurrentCheckpointName);
		check(cpObject);

		// Get Current Persistent Name
		// get the actual persistent level's name
		FName PreviousMapName = GWorld->PersistentLevel->GetOutermost()->GetPureName();

		// look for a persistent streamed in sublevel
		for (INT LevelIndex = 0; LevelIndex < GWorld->GetWorldInfo()->StreamingLevels.Num(); LevelIndex++)
		{
			ULevelStreamingPersistent* PersistentLevel = Cast<ULevelStreamingPersistent>(GWorld->GetWorldInfo()->StreamingLevels(LevelIndex));
			if (PersistentLevel)
			{
				PreviousMapName = PersistentLevel->PackageName;
				// only one persistent level
				break;
			}
		}

		if (GameEngine && cpObject->PersistentLevelName != NAME_None && cpObject->PersistentLevelName != PreviousMapName)
		{
			TArray<FName> LevelNames;
			LevelNames.AddItem(cpObject->PersistentLevelName);

			GameEngine->PrepareMapChange(LevelNames);
			GameEngine->bShouldCommitPendingMapChange = TRUE;
		}

		bTravelCheckPersistent = FALSE;
	}
	else if ( (GameEngine == NULL || !GameEngine->IsPreparingMapChange()) && GWorld->StreamingVolumeUpdateDelay == 0 )
	{
		if (GIsGame)
		{
			GWorld->ProcessLevelStreamingVolumes();
		}

		INT prevAllowLevelLoadOverride = GWorld->AllowLevelLoadOverride;
		GWorld->AllowLevelLoadOverride = 1;
		ClientFlushLevelStreaming();
		GWorld->AllowLevelLoadOverride = prevAllowLevelLoadOverride;

		UBOOL bStableLevels = TRUE;

		for (INT i = 0 ; i < GWorld->GetWorldInfo()->StreamingLevels.Num() ; ++i)
		{
			ULevelStreaming* LevelStreamingObject = GWorld->GetWorldInfo()->StreamingLevels(i);

			if (LevelStreamingObject)
			{
				UBOOL bActive = (LevelStreamingObject->bHasLoadRequestPending || LevelStreamingObject->bHasUnloadRequestPending || LevelStreamingObject->bIsRequestingUnloadAndRemoval);
				bActive = bActive || (LevelStreamingObject->LoadedLevel && LevelStreamingObject->LoadedLevel->bHasVisibilityRequestPending);

				if (bActive)
				{
					bStableLevels = FALSE;
					break;
				}
			}
		}

		if (bStableLevels)
		{		
			if (StableLevelsTimestamp <= 0.0f)
			{
				StableLevelsTimestamp = GWorld->GetRealTimeSeconds();
			}
			else if (GWorld->GetRealTimeSeconds() >= (StableLevelsTimestamp + 1.0f))
			{	
				if (bForceLevelReset)
				{
					bForceLevelReset = FALSE;
					StableLevelsTimestamp = -1;
				}
				else
				{
					TravelComplete();
				}
			}
		}
		else
		{
			StableLevelsTimestamp = -1.0f;
		}
	}
}

void AOLPlayerController::SaveBeforeQuitting()
{
	// Called upon quitting to main menu or leaving game
	SavePersistentState();

	UOLEngine* engine = Cast<UOLEngine>(GEngine);
	FName cpName = Utils::GetCurrentCheckpointName();
	if (engine && cpName != NAME_None && !Utils::IsDemo() && Utils::GetOLGame()->DifficultyMode != EDM_Insane)
	{
		engine->SaveCheckpointImmediate(cpName);
	}

	ClearAllProgress();	
}

void AOLPlayerController::ClearAllProgress()
{
	CurrentObjective = NAME_None;
	CompletedObjectives.Empty();
	CompletedRecordingMoments.Empty();
	UnreadRecordingMoments.Empty();
	TutorialManager->Clear();
	InventoryManager->ClearAll();
	LevelsResetAfterPlayerDeath.Empty();

	AOLGame* olGame = Utils::GetOLGame();
	NumBatteries = (olGame && olGame->DifficultyMode == EDM_Normal) ? DefaultNumBatteries : 1;
	NumBatteriesAtLastCheckpoint = NumBatteries;
	BatteryEnergyAtLastCheckpoint = 1.0f;

	extern AOLGameStateList* GMasterGameStateList;
	if (GMasterGameStateList)
	{
		GMasterGameStateList->ResetAllGameState();
	}

	if (HUD && HUD->CamcorderHUD)
	{
		HUD->CamcorderHUD->RecordingTimeSeconds = 0.0f;
	}
}

void AOLPlayerController::StartNewGameAtCheckpoint(const FString& CheckpointStr, UBOOL bSaveToDisk)
{
	FName CPName(*CheckpointStr);

	// Find the checkpoint in the current world only — mirrors the original UC AllActors search.
	// Do NOT fall back to TObjectIterator: a checkpoint found in a loaded-but-not-in-world
	// package (e.g. the ImGui checkpoint tab) would trigger SaveCheckpoint and corrupt the
	// PendingCheckpointAction that LoadSaveFile already set to Checkpoint_Load.
	AOLCheckpoint* StartCP = NULL;
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpoint* CP = Cast<AOLCheckpoint>(*It);
		if (CP && !CP->IsPendingKill() && CP->CheckpointName == CPName)
		{
			StartCP = CP;
			break;
		}
	}
	if (!StartCP)
		return;

debugf(TEXT("SNGAC: StopAllSounds"));
	if (HUD && HUD->eventIsMainMenuOpen())
		StopAllSounds();

	debugf(TEXT("SNGAC: HideMenu"));
	if (HUD)
	{
		UFunction* HideMenuFn = HUD->FindFunction(FName(TEXT("HideMenu")));
		if (HideMenuFn) HUD->ProcessEvent(HideMenuFn, NULL);
	}

	debugf(TEXT("SNGAC: Ghost"));
	if (bDebugGhost && CheatManager)
	{
		UOLCheatManager* CM = Cast<UOLCheatManager>(CheatManager);
		if (CM)
		{
			UFunction* GhostFn = CM->FindFunction(FName(TEXT("Ghost")));
			if (GhostFn) CM->ProcessEvent(GhostFn, NULL);
		}
	}

	debugf(TEXT("SNGAC: UnPossess+DestroyHero"));
	AOLHero* Hero = HeroPawn;
	eventUnPossess();
	if (Hero) GWorld->DestroyActor(Hero);

	debugf(TEXT("SNGAC: ClearAllProgress"));
	ClearAllProgress();

	UOLEngine* Engine = Cast<UOLEngine>(GEngine);
	AOLGame* CurrentGame = Cast<AOLGame>(GWorld->GetGameInfo());

	if (CurrentGame)
	{
		debugf(TEXT("SNGAC: SaveCheckpoint"));
		if (Engine)
			Engine->SaveCheckpoint(StartCP->CheckpointName, bSaveToDisk);

		CurrentGame->CurrentCheckpointName = StartCP->CheckpointName;

		debugf(TEXT("SNGAC: RestartPlayer"));
		UFunction* RestartFn = CurrentGame->FindFunction(FName(TEXT("RestartPlayer")));
		if (RestartFn)
		{
			struct { APlayerController* PC; } Parms;
			Parms.PC = this;
			CurrentGame->ProcessEvent(RestartFn, &Parms);
		}
		debugf(TEXT("SNGAC: done"));
	}
}

UBOOL AOLPlayerController::ShippingCheat_GiveAllCheckpoints()
{
	UOLEngine* olengine = Cast<UOLEngine>(GEngine);

	if (olengine && HUD->eventIsInCreditsMenu()) // only in the credits
	{
		ClearAllProgress();
		olengine->SaveAllCheckpoints();

		return TRUE;
	}

	return FALSE;
}

void AOLPlayerController::CheatGiveAllCollectibles()
{
	NumBatteriesAtLastCheckpoint = MaxNumBatteries;
	NumBatteries = MaxNumBatteries;
	BatteryEnergyAtLastCheckpoint = 1.0f;
	HeroPawn->CurrentBatterySetEnergy = 1.0f;

	InventoryManager->CollectedDocuments.Empty();
	InventoryManager->CollectedDocuments.AddItem(TEXT("Admin_Doc_AnonymousEmail"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Admin_Doc_ProjectWallrider1"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Admin_Doc_ProjectWallrider2"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Admin_Doc_Reception"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Admin_Doc_BossRoom"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Admin_Doc_Basement"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Prison_Doc_Wernicke1"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Prison_Doc_Wernicke2"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Prison_Doc_Lobby"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Prison_Doc_SecuritySAS"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Sewer_Doc_BeginningSewer"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Sewer_Doc_DeadGuy"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Male_Doc_Experiment"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Male_Doc_Bathroom3rdFloor"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Male_Doc_MainHall"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Male_Doc_LockerRoom"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Male_Doc_Office"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Court_Doc_Pergola"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Court_Doc_Chapel"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Female_Doc_UnderStair"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Female_Doc_Hole"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Revisit_Doc_Office"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Revisit_Doc_Backstage"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Revisit_Doc_Library"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Revisit_Doc_Room"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Lab_Doc_LabRoom"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Lab_Doc_MachineRoom"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Lab_Doc_MajorRoom"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Lab_Doc_LifeLiquid"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Lab_Doc_TallRoom"));
	InventoryManager->CollectedDocuments.AddItem(TEXT("Lab_Doc_CoreRoom"));

	CompletedRecordingMoments.Empty();
	CompletedRecordingMoments.AddItem(TEXT("Admin_RM_Asylum"));
	CompletedRecordingMoments.AddItem(TEXT("Admin_RM_Stevenson"));
	CompletedRecordingMoments.AddItem(TEXT("Admin_RM_StaticTV"));
	CompletedRecordingMoments.AddItem(TEXT("Admin_RM_Witness"));
	CompletedRecordingMoments.AddItem(TEXT("Admin_RM_Chris"));
	CompletedRecordingMoments.AddItem(TEXT("Prison_RM_PriestCell"));
	CompletedRecordingMoments.AddItem(TEXT("Prison_RM_Necro"));
	CompletedRecordingMoments.AddItem(TEXT("Prison_RM_SoldierRippingHeadOff"));
	CompletedRecordingMoments.AddItem(TEXT("Prison_RM_SwarmShower"));
	CompletedRecordingMoments.AddItem(TEXT("Sewer_RM_GuyOnMattress"));
	CompletedRecordingMoments.AddItem(TEXT("Sewer_RM_EndOfSewer"));
	CompletedRecordingMoments.AddItem(TEXT("Male_RM_IsolationRoom"));
	CompletedRecordingMoments.AddItem(TEXT("Male_RM_SurgeonKilling"));
	CompletedRecordingMoments.AddItem(TEXT("Male_RM_TragersDeath"));
	CompletedRecordingMoments.AddItem(TEXT("Male_RM_Pyro"));
	CompletedRecordingMoments.AddItem(TEXT("Male_RM_Kitchen"));
	CompletedRecordingMoments.AddItem(TEXT("Court_RM_Walrider"));
	CompletedRecordingMoments.AddItem(TEXT("Court_RM_Fountain"));
	CompletedRecordingMoments.AddItem(TEXT("Female_RM_DryMachine"));
	CompletedRecordingMoments.AddItem(TEXT("Female_RM_Elevator"));
	CompletedRecordingMoments.AddItem(TEXT("Female_RM_ChuteHole"));
	CompletedRecordingMoments.AddItem(TEXT("Revisit_RM_RecreationHall"));
	CompletedRecordingMoments.AddItem(TEXT("Revisit_RM_PoolRoom"));
	CompletedRecordingMoments.AddItem(TEXT("Revisit_RM_PriestBurning"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_Reception"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_Morphegenic"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_ChrisDeath"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_Sphere"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_Billy"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_LiquidLife"));
	CompletedRecordingMoments.AddItem(TEXT("Lab_RM_BillysDeath"));
}

void AOLPlayerController::GiveCollectibleToPlayer(FName CollectibleName)
{
	if (InventoryManager)
	{
		InventoryManager->AddCollectible(CollectibleName);
		if (HUD)
			HUD->SetLatestDocument(CollectibleName);
	}
}

void AOLPlayerController::GiveGameplayItemToPlayer(FName ItemName)
{
	if (InventoryManager)
	{
		InventoryManager->AddUniqueItem(ItemName);
		if (HUD)
		{
			FString itemText = Localize(TEXT("GameplayItems"), *ItemName.ToString(), TEXT("OLGame"));
			FString text = Localize(TEXT("Messages"), TEXT("PickedUpItem"), TEXT("OLGame"));
			HUD->AddMessage(EHMT_Generic, FString::Printf(*text, *itemText));
		}
	}
}

FLOAT AOLPlayerController::GetHighResTimeSeconds()
{
	// appSeconds() adds 16777216.0 (2^24) to expose float-cast bugs.
	// Subtracting it before the cast keeps the value small enough for float32
	// to retain sub-millisecond precision.
	return (FLOAT)(appSeconds() - 16777216.0);
}

INT AOLPlayerController::GetHighResTimeMs()
{
	// Returns milliseconds as integer — no float precision loss when serialized to string.
	return (INT)((appSeconds() - 16777216.0) * 1000.0);
}

void AOLPlayerController::SavePersistentState()
{
	NumBatteriesAtLastCheckpoint = NumBatteries;
	BatteryEnergyAtLastCheckpoint = HeroPawn->CurrentBatterySetEnergy;
	InventoryManager->SaveBatteries();
}

void AOLPlayerController::NativeCreateCheckpointRecord(FCheckpointRecord& record)
{
	record.CheckpointRecordVersion = 3;

	record.NumBatteries = NumBatteriesAtLastCheckpoint;
	record.BatteryEnergy = BatteryEnergyAtLastCheckpoint;

	record.CurrentObjective = CurrentObjective;
	record.CompletedObjectives = CompletedObjectives;

	record.CompletedRecordingMoments = CompletedRecordingMoments;
	record.UnreadRecordingMoments = UnreadRecordingMoments;

	TArray<FName> activatedGS;
	const TArray<FOLGameState>* gsListPtr = AOLGameStateList::GetGameStateList();
	if (gsListPtr)
	{
		const TArray<FOLGameState>& gsList = *gsListPtr;		

		for (INT i = 0; i < gsList.Num(); i++)
		{
			if (gsList(i).bActivated && gsList(i).bPersistAfterDeath)
			{
				activatedGS.AddItem(gsList(i).Name);
			}
		}
	}

	record.ActivatedGameState = activatedGS;

	// From Inventory Manager
	record.CollectedDocuments = InventoryManager->CollectedDocuments;
	record.UnreadDocuments = InventoryManager->UnreadDocuments;
	record.CollectedBatteries = InventoryManager->SavedBatteryLocs;

	// From Tutorial Manager
	record.bBatteryTutorialComplete = TutorialManager->bBatteryTutorialComplete;
	record.bClimbUpTutorialComplete = TutorialManager->bClimbUpTutorialComplete;
	record.bDocumentTutorialComplete = TutorialManager->bDocumentTutorialComplete;

	record.bFoundBySoldierWhileHidden = bFoundBySoldierWhileHidden;
	record.bFoundBySurgeonWhileHidden = bFoundBySurgeonWhileHidden;

	// Camera recording time (the "00:00:00:00" display in the top left)
	if (HUD && HUD->CamcorderHUD)
	{
		record.RecordingTimeSeconds = HUD->CamcorderHUD->RecordingTimeSeconds;
	}
	else
	{
		record.RecordingTimeSeconds = 0.0f;
	}

	AOLGame* olgame = Utils::GetOLGame();
	check(olgame);

	record.DifficultyMode = olgame->DifficultyMode;

	// Version 2
	//...
	
	// Version 3
	record.bUsedHidingSpot = bUsedHidingSpot;
}

void AOLPlayerController::NativeApplyCheckpointRecord(const FCheckpointRecord& record)
{
	NumBatteries = record.NumBatteries;
	NumBatteriesAtLastCheckpoint = NumBatteries;

	CurrentObjective = record.CurrentObjective;
	CompletedObjectives = record.CompletedObjectives;

	UOLEngine* engine = Cast<UOLEngine>(GEngine);
	check(engine); // hahaha, the check engine light is on!

	// There are two possibilities: we load a new game from the main menu (from empty), or we reload after dying (existing progress)
	// We don't reset recordings or collectibles upon dying, so if they're non-empty, we don't load them from the save game
	if (CompletedRecordingMoments.Num() == 0)
	{
		CompletedRecordingMoments = record.CompletedRecordingMoments;
		UnreadRecordingMoments = record.UnreadRecordingMoments;
	}
	else
	{
		check(engine->bRestartingActiveCheckpoint);
	}

	if (!engine->bRestartingActiveCheckpoint)
	{
		// if restarting after death, we don't touch the gamestate
		AOLGameStateList::SetGameStateList(record.ActivatedGameState);
	}

	if (InventoryManager->CollectedDocuments.Num() == 0)
	{
		InventoryManager->CollectedDocuments = record.CollectedDocuments;
		InventoryManager->UnreadDocuments = record.UnreadDocuments;
	}
	else
	{
		check(engine->bRestartingActiveCheckpoint);
	}

	InventoryManager->SavedBatteryLocs = record.CollectedBatteries;
	
	TutorialManager->bBatteryTutorialComplete = record.bBatteryTutorialComplete;
	TutorialManager->bClimbUpTutorialComplete = record.bClimbUpTutorialComplete;
	TutorialManager->bDocumentTutorialComplete = record.bDocumentTutorialComplete;

	bFoundBySoldierWhileHidden = record.bFoundBySoldierWhileHidden;
	bFoundBySurgeonWhileHidden = record.bFoundBySurgeonWhileHidden;

	// Camera recording time (the "00:00:00:00" display in the top left)
	if (HUD && HUD->CamcorderHUD)
	{
		HUD->CamcorderHUD->RecordingTimeSeconds = record.RecordingTimeSeconds;
	}

	if (!engine->bRestartingActiveCheckpoint)
	{
		AOLGame* olgame = Utils::GetOLGame();
		check(olgame);

		olgame->DifficultyMode = record.DifficultyMode;
	}

	if (record.CheckpointRecordVersion >= 2)
	{	
		BatteryEnergyAtLastCheckpoint = record.BatteryEnergy;
	}
	else
	{
		BatteryEnergyAtLastCheckpoint = 1.0f;
	}

	if (record.CheckpointRecordVersion >= 3)
	{
		bUsedHidingSpot = record.bUsedHidingSpot;
	}
}

void AOLPlayerController::OverrideListenerLocation(FVector& inout_Location, FVector& inout_ProjUp, FVector& inout_ProjRight, FVector& inout_ProjFront) const
{
	// Override if in debug camera and we have a valid pawn
	if ((bDebugFreeCam || bDebugFixedCam) && HeroPawn)
	{
		FRotationTranslationMatrix eyeMtx(HeroPawn->EyeRotation, HeroPawn->EyeLocation);
		eyeMtx.GetAxes(inout_ProjFront, inout_ProjRight, inout_ProjUp);
		inout_Location = eyeMtx.GetOrigin();
	}
}

void AOLPlayerController::UpdateLightingOptimisation(FLOAT deltaSeconds)
{
	if (!HeroPawn || (!bEnableShadowOptimisation && !bEnableLightOptimisation))
	{
		return;
	}

	const FVector& EyeLocation = HeroPawn->EyeLocation;

	FLOAT distSqToLast = EyeLocation.DistanceSquared(LastLightOptimCamPos);

	if (distSqToLast < Square(100.0f))
	{
		return;
	}

	LastLightOptimCamPos = EyeLocation;
		
	INT nbNormals = 0;
	INT nbModulate = 0;
	INT nbEnabled = 0;
	INT nbDisabled = 0;

	for (INT i = 0; i < HeroPawn->CachedLights.Num(); i++)
	{
		ALight* light = Cast<ALight>(HeroPawn->CachedLights(i));

		if (light && light->LightComponent)
		{
			ULightComponent* lightComp = light->LightComponent;
			FLOAT distSq = light->Location.DistanceSquared(EyeLocation);

			if (bEnableLightOptimisation)
			{
				if ((lightComp->MaxDistFromCamForEnabled < KINDA_SMALL_NUMBERF) || distSq <= Square(lightComp->MaxDistFromCamForEnabled))
				{
					if (!lightComp->bEnabled && lightComp->bDisabledByOptimisation)
					{
						FComponentReattachContext ReattachContext(lightComp);
						lightComp->bEnabled = TRUE;
						lightComp->bDisabledByOptimisation = FALSE;
					}
					nbEnabled++;
				}
				else
				{
					if (lightComp->bEnabled)
					{
						FComponentReattachContext ReattachContext(lightComp);
						lightComp->bEnabled = FALSE;
						lightComp->bDisabledByOptimisation = TRUE;
					}
					nbDisabled++;
					continue;
				}
			}

			if (bEnableShadowOptimisation)
			{				
				UBOOL validCandidate = (lightComp->LightShadowMode == LightShadow_Normal || lightComp->bShadowModeOptimized);
				validCandidate = validCandidate && lightComp->CastDynamicShadows && !IsDominantLightType(lightComp->GetLightType());

				if (!validCandidate)
				{
					continue;
				}

				if (appIsNearlyZero(lightComp->MaxDistFromCamForNormalShadows, KINDA_SMALL_NUMBERF) || distSq <= Square(lightComp->MaxDistFromCamForNormalShadows))
				{
					if (lightComp->LightShadowMode != LightShadow_Normal && lightComp->bShadowModeOptimized)
					{
						FComponentReattachContext ReattachContext(lightComp);
						lightComp->LightShadowMode = LightShadow_Normal;
						lightComp->bShadowModeOptimized = FALSE;
					}

					if (lightComp->LightShadowMode == LightShadow_Normal)
					{
						nbNormals++;
					}
				}
				else
				{
					if (lightComp->LightShadowMode != LightShadow_Modulate)
					{
						FComponentReattachContext ReattachContext(lightComp);
						lightComp->LightShadowMode = LightShadow_Modulate;
						lightComp->bShadowModeOptimized = TRUE;
					}

					nbModulate++;
				}
			}
		}
	}
	
	SET_DWORD_STAT(STAT_NumNormalShadowLights, nbNormals);
	SET_DWORD_STAT(STAT_NumOptimShadowLights, nbModulate);
	SET_DWORD_STAT(STAT_NumEnabledLights, nbEnabled);
	SET_DWORD_STAT(STAT_NumDisabledLights, nbDisabled);
}

void AOLPlayerController::ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const
{
	FXManager->GetPostProcessSettings(PPSettings);
}

FLOAT AOLPlayerController::GetGamma()
{
	extern FLOAT GetDisplayGamma();
	return GetDisplayGamma();
}

void AOLPlayerController::SetGamma(FLOAT Gamma)
{
	extern void SetDisplayGamma(FLOAT Gamma);
	SetDisplayGamma(Gamma);
}

void AOLPlayerController::SetVolume(FLOAT VolumeLevel)
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetRTPCValue(TEXT("MASTER_VOLUME"), VolumeLevel * 100.0f, NULL);
	}
}

void AOLPlayerController::StopAllSounds()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->StopAllSounds();
	}
}

UBOOL AOLPlayerController::HearSound( UAkBaseSoundObject* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed )
{
	if (InSoundCue->GetName().InStr(TEXT("CAM_Low_Battery"), FALSE, TRUE) != INDEX_NONE && HUD && HUD->eventIsInPauseMenu())
	{
		return TRUE;
	}

	return Super::HearSound(InSoundCue, SoundPlayer, SoundLocation, bStopWhenOwnerDestroyed);
}

void AOLPlayerController::SetAstoundSoundEnabled(UBOOL bEnabled)
{
	SetState(AstoundSoundGroup, bEnabled ? AstoundSoundOn : AstoundSoundOff);
}

void AOLPlayerController::NativeOnControllerChanged(INT ControllerId, UBOOL bIsConnected)
{
#if DINGO
	if (GEngine && HUD && !bIsConnected && GOLDingo->IsActiveUser(ControllerId) && !GWorld->IsPaused())
	{			
		if (Utils::IsInMainMenu())
		{
			debugf(TEXT("-- Controller disconnected - return to start"));
			UOLEngine* olEngine = Cast<UOLEngine>(GEngine);
			if (olEngine)
			{
				olEngine->ReturnToPressStartScreen();
			}
		}
		else
		{
			debugf(TEXT("-- Controller disconnected - pausing"));
			HUD->eventShowMenuType(EMT_PauseMenu);
		}
	}
#endif
}

void AOLPlayerController::PauseGameOnLostFocus()
{
#if DINGO
	if (GEngine && HUD && !Utils::IsInMainMenu() && !GWorld->IsPaused())
	{			
		debugf(TEXT("-- PauseGameOnLostFocus"));
		HUD->eventShowMenuType(EMT_PauseMenu);
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
// Debug
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLPlayerController::TickPaused(FLOAT deltaSeconds)
{
	if (bTravellingToCheckpoint)
	{
		UpdateTravel(deltaSeconds);
	}

	UpdateOverlay(deltaSeconds);

#if WITH_ORBISCONTROLLEREMULATION || ORBIS
	UpdateOrbisController(deltaSeconds);
#endif

#if !FINAL_RELEASE && !SHIPPING_PC_GAME

	if (HeroPawn)
	{
		DrawDebug();
	}

	if (bDebugFreeCam)
	{
		for (INT InteractionIndex = 0; InteractionIndex < Interactions.Num(); InteractionIndex++)
		{
			if (Interactions(InteractionIndex))
			{
				Interactions(InteractionIndex)->Tick(deltaSeconds);
			}
		}

		ProcessFreeCam(deltaSeconds);

		for (INT InteractionIndex = 0; InteractionIndex < Interactions.Num(); InteractionIndex++)
		{
			if (Interactions(InteractionIndex))
			{
				Interactions(InteractionIndex)->Tick(-1.0f);
			}
		}
	}

#endif

	return TRUE;
}

void AOLPlayerController::ProcessFreeCam(FLOAT deltaSeconds)
{
	FVector	freeCamX, freeCamY, freeCamZ;
	FRotationMatrix rotMat(DebugCamRot);
	rotMat.GetAxes(freeCamX, freeCamY, freeCamZ);

	// aStrafe is zeroed by IsMoveInputIgnored() (active during LM_Cinematic / Kismet cinematic mode).
	// RawJoyRight is captured before that zeroing and holds the raw strafe axis value.
	// Scale it by MoveStrafeSpeed (same scale PostProcessInput would apply) to match aBaseY.
	FLOAT strafeAxis = PlayerInput->aStrafe;
	if (strafeAxis == 0.f && PlayerInput->RawJoyRight != 0.f)
		strafeAxis = PlayerInput->RawJoyRight * PlayerInput->MoveStrafeSpeed;
	FVector delta = PlayerInput->aBaseY*freeCamX + strafeAxis*freeCamY;

	FLOAT freeCamSpeed = DebugFreeCamSpeed;

	if (PlayerInput->bUsingGamepad)
	{
		UBOOL bFast = PlayerInput->IsPressed(FName(TEXT("XboxTypeS_LeftTrigger")));
		UBOOL bSlow = PlayerInput->IsPressed(FName(TEXT("XboxTypeS_LeftShoulder")));

		TWEAKABLE FLOAT FastMult = 10.0f;
		TWEAKABLE FLOAT SlowMult = 0.5f;
		TWEAKABLE FLOAT NrmMult = 1.0f;
		TWEAKABLE FLOAT GhostMult = 0.75f;

		FLOAT multiplier = bFast ? FastMult : (bSlow ? SlowMult : NrmMult);

		if (bDebugGhost)
		{
			multiplier *= GhostMult;
		}

		freeCamSpeed = multiplier*DebugFreeCamSpeed;	
	}
	else
	{
		if (bDebugGhost)
		{
			freeCamSpeed = (PlayerInput->IsCtrlPressed() ? 2.0f*DebugFreeCamSpeed : (PlayerInput->IsShiftPressed() ? 10.0f*DebugFreeCamSpeed : DebugFreeCamSpeed));
		}
		else
		{
			freeCamSpeed = (PlayerInput->IsCtrlPressed() ? 0.6f*DebugFreeCamSpeed : (PlayerInput->IsShiftPressed() ? 10.0f*DebugFreeCamSpeed : 2.0f*DebugFreeCamSpeed));
		}
	
		if (GWorld->IsPaused())
		{
			freeCamSpeed *= 100.0f*deltaSeconds*PlayerInput->MoveForwardSpeed; // Not quite sure why this is necessary... needs cleanup!		
		}
	}

	DebugCamPos += delta * freeCamSpeed;	

	FRotator DeltaRot((INT)PlayerInput->aLookUp, (INT)PlayerInput->aTurn, 0);
	DebugCamRot += DeltaRot;
	DebugCamRot.Pitch = Clamp(FRotator::NormalizeAxis(DebugCamRot.Pitch), -16384, 16383);

	if (bDebugGhost)
	{
		HeroPawn->SetLocation(DebugCamPos - FVector(0, 0, 165.0f));
		HeroPawn->SetRotation(DebugCamRot);
	}
}

FString AOLPlayerController::NativeBuildInviteLink(const FString& IP, const FString& Port, const FString& Room, const FString& Pass)
{
    FString Link = FString::Printf(TEXT("openol://%s:%s?room=%s"), *IP, *Port, *Room);
    if (Pass.Len() > 0)
        Link += FString::Printf(TEXT("&password=%s"), *Pass);
    return Link;
}

UBOOL AOLPlayerController::NativeParseInviteLink(const FString& Link, FString& OutIP, FString& OutPort, FString& OutRoom, FString& OutPass)
{
    if (!Link.StartsWith(TEXT("openol://")))
        return FALSE;

    FString Body = Link.Mid(9);

    FString Host, Query;
    if (!Body.Split(TEXT("?"), &Host, &Query))
        Host = Body;

    FString LocalIP, LocalPort;
    if (Host.Split(TEXT(":"), &LocalIP, &LocalPort))
    {
        OutIP   = LocalIP;
        OutPort = LocalPort;
    }
    else
    {
        OutIP = Host;
    }

    FString Remaining = Query;
    while (Remaining.Len() > 0)
    {
        FString Pair, Rest;
        if (!Remaining.Split(TEXT("&"), &Pair, &Rest))
        {
            Pair      = Remaining;
            Remaining = TEXT("");
        }
        else
        {
            Remaining = Rest;
        }

        FString Key, Val;
        if (!Pair.Split(TEXT("="), &Key, &Val))
            continue;

        if (Key == TEXT("room"))
            OutRoom = Val;
        else if (Key == TEXT("password"))
            OutPass = Val;
    }

    return TRUE;
}

FString AOLPlayerController::NativeGetMySteamID()
{
    if (GSteamUser)
    {
        QWORD SID = (QWORD)GSteamUser->GetSteamID().ConvertToUint64();
        debugf(NAME_Log, TEXT("NativeGetMySteamID: %llu"), (unsigned long long)SID);
        return FString::Printf(TEXT("%llu"), (unsigned long long)SID);
    }
    debugf(NAME_Log, TEXT("NativeGetMySteamID: GSteamUser is NULL"));
    return TEXT("");
}

void AOLPlayerController::NativeOpenSteamFriendsOverlay()
{
    if (GSteamFriends)
        GSteamFriends->ActivateGameOverlay("friends");
}

// Forward declarations for functions defined later in this file.
extern "C" void SetRelayRichPresence(const char* RoomCode);
extern "C" void ClearRelayRichPresence();

void AOLPlayerController::NativeSetRelayRichPresence(const FString& RoomCode)
{
    SetRelayRichPresence(TCHAR_TO_UTF8(*RoomCode));
}

void AOLPlayerController::NativeClearRelayRichPresence()
{
    ClearRelayRichPresence();
}

// ---------------------------------------------------------------------------
// Relay control — start/stop the embedded relay and query its state.
// ---------------------------------------------------------------------------

// RelayThread.h pulls in winsock2, but we already have the guard at the top of this file.
#include "..\..\Multiplayer\Inc\RelayThread.h"

void AOLPlayerController::NativeStartRelay(INT Port)
{
    if (GRelayThread.IsRunning())
        return;
    GRelayThread.StartRelay((WORD)Port, "OpenOL", "openol_relay.db");
    SetRelayRichPresence("DEFAULT");
}

void AOLPlayerController::NativeStopRelay()
{
    GRelayThread.StopRelay();
    ClearRelayRichPresence();
}

UBOOL AOLPlayerController::NativeIsRelayRunning()
{
    return GRelayThread.IsRunning();
}

void AOLPlayerController::NativeConnectLocal(INT Port)
{
    // Local (non-P2P) connect to 127.0.0.1 — used when the player is hosting their own relay.
    // Avoids pulling the entire Multiplayer header chain into the OLGame Unity build.
    extern void GMpConn_ConnectLocal(WORD);
    GMpConn_ConnectLocal((WORD)Port);
}

// ---------------------------------------------------------------------------
// Global helpers declared in OpenOLGlobals.h, implemented here (Steamworks available).
// ---------------------------------------------------------------------------

extern "C" void GetSteamPersonaName(char* Out, int OutMax)
{
    Out[0] = '\0';
    if (!GSteamFriends) return;
    const char* Name = GSteamFriends->GetPersonaName();
    if (!Name || !Name[0]) return;
    _snprintf(Out, OutMax, "%s", Name);
    Out[OutMax - 1] = '\0';
}

extern "C" void SetRelayRichPresence(const char* RoomCode)
{
    if (!GSteamFriends || !GSteamUser) return;
    // Connect string: "<hostSteamID>/<room>" — parsed by OutlastLauncher (cold launch)
    // and OnlineSubsystem Finalize() (hot join).
    QWORD MySteamID = GSteamUser->GetSteamID().ConvertToUint64();
    char ConnectStr[128];
    _snprintf(ConnectStr, sizeof(ConnectStr), "%llu/%s", (unsigned long long)MySteamID, RoomCode);
    GSteamFriends->SetRichPresence("connect",       ConnectStr);
    GSteamFriends->SetRichPresence("steam_display", "#StatusPlayingMultiplayer");
    debugf(NAME_Log, TEXT("RichPresence set: connect='%s'"), ANSI_TO_TCHAR(ConnectStr));
}

extern "C" void ClearRelayRichPresence()
{
    if (!GSteamFriends) return;
    GSteamFriends->ClearRichPresence();
    debugf(NAME_Log, TEXT("RichPresence cleared"));
}

void AOLPlayerController::NativeConnectP2P(const FString& HostSteamID, INT RelayPort,
    const FString& RoomCode, const FString& Password)
{
    // Forward to FMpConnection::ConnectP2P via the extern declared in Multiplayer package.
    // Avoids pulling the entire Multiplayer header chain into the OLGame Unity build.
    typedef void (*FConnectP2PFn)(QWORD, WORD, const FString&, const FString&);
    extern void GMpConn_ConnectP2P(QWORD, WORD, const FString&, const FString&);
    QWORD SteamID = (QWORD)appAtoi64(*HostSteamID);
    if (SteamID != 0)
        GMpConn_ConnectP2P(SteamID, (WORD)RelayPort, RoomCode, Password);
}

void AOLPlayerController::DrawDebug()
{
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	FVector baseLoc = (bDebugFixedCam || bDebugFreeCam || bDebugGhost) ? DebugCamPos : HeroPawn->EyeLocation;

	FLOAT thickness = 5.0f;

	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugGameplay)
	{
		FLOAT MaxObstacleZ = 350.0f;
		FLOAT MaxObstacleDist = 500.0f;
		AOLLedgeMarker* closestHeroLedge = HeroPawn->FindClosestLedge(0.0f, MaxObstacleZ, MaxObstacleDist);

		for (FActorIterator It; It; ++It)
		{
			AActor* actor = *It;
			if (actor && actor->IsAStaticMeshActor())
			{
				// early out of the most common case
				continue;
			}

			// Allow long distance since they're used by enemies and not the hero
			AOLPreferredPathMarker* pathMarker = Cast<AOLPreferredPathMarker>(*It);
			if (pathMarker && pathMarker->IsValid())
			{
				DrawDebugSphere(pathMarker->Location, 5.0f, 6, 252, 73, 8);

				if (pathMarker->Next && pathMarker->Next->IsValid())
				{
					UBOOL bActive = FALSE;
					for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
					{
						AOLEnemyPawn* enemyPawn = Cast<AOLEnemyPawn>(pawn);
						if (enemyPawn && enemyPawn->PreferredPath == pathMarker)
						{	
							bActive = TRUE;
							break;
						}
					}

					if (bActive)
					{
						DrawDebugLine(pathMarker->Location, pathMarker->Next->Location, 248, 252, 8, FALSE, thickness);
					}
					else
					{
						DrawDebugLine(pathMarker->Location, pathMarker->Next->Location, 252, 73, 8, FALSE, thickness);
					}
				}
			}

			TWEAKABLE FLOAT MaxDist = 800.0f; // debug stuff, but was still slowing down framerate quite a bit

			if (baseLoc.DistanceSquared((*It)->Location) > Square(MaxDist))
			{
				continue;
			}

			AOLLedgeMarker* ledgeMarker = Cast<AOLLedgeMarker>(*It);
			if (ledgeMarker)
			{
				if (ledgeMarker == closestHeroLedge)
				{
					DrawDebugSphere(ledgeMarker->Location, 5.0f, 6, 51, 123, 80);
					DrawDebugLine(ledgeMarker->Location, ledgeMarker->Next->Location, 242, 100, 100, FALSE, thickness);
				}
				else if (ledgeMarker->IsValid())
				{
					DrawDebugSphere(ledgeMarker->Location, 5.0f, 6, 51, 123, 80);

					if (ledgeMarker->Next && ledgeMarker->Next->IsValid())
					{
						DrawDebugLine(ledgeMarker->Location, ledgeMarker->Next->Location, 100, 243, 158, FALSE, thickness);
					}
				}
				else
				{
					DrawDebugSphere(ledgeMarker->Location, 5.0f, 6, 242, 61, 61);

					if (ledgeMarker->Next && ledgeMarker->Next->IsValid())
					{
						DrawDebugLine(ledgeMarker->Location, ledgeMarker->Next->Location, 255, 20, 20, FALSE, thickness);
					}
				}
			}

			AOLSqueezeVolume* squeezeVolume = Cast<AOLSqueezeVolume>(*It);
			if (squeezeVolume && squeezeVolume->IsValid())
			{
				DrawDebugLine(squeezeVolume->Node1->Location + FVector(0,0,5.0f), squeezeVolume->Node2->Location + FVector(0,0,5.0f), 0, 196, 255, FALSE, thickness);
				DrawDebugSphere(squeezeVolume->Node1->Location, 15.0f, 6, 0, 156, 210);
				DrawDebugSphere(squeezeVolume->Node2->Location, 15.0f, 6, 0, 156, 210);
			}

			AOLDoor* door = Cast<AOLDoor>(*It);
			if (door)
			{
				DrawDebugSphere(door->GetKnobLocation(), 5.0f, 10, 255, 254, 134);
				DrawDebugLine(door->GetKnobLocation(), door->GetKnobLocation() + 25.0f*door->GetDynamicDirection() , 255, 254, 134);

				DrawDebugSphere(door->GetStaticKnobLocation(), 5.0f, 10, 255, 254, 134);
				DrawDebugLine(door->GetStaticKnobLocation(), door->GetStaticKnobLocation() + 25.0f*door->GetStaticDirection() , 60, 112, 60);

				DrawDebugSphere(door->GetCenterLocation(), 5.0f, 6, 208, 134, 255);
				DrawDebugLine(door->GetCenterLocation() + VecZ(100.0f), door->GetCenterLocation() + 50.0f*door->GetStaticDirection() + VecZ(100.0f), 134, 255, 145);

				DrawDebugLine(door->GetEdgeLocation(), door->GetEdgeLocation() + VecZ(180.0f), 134, 255, 145);
				DrawDebugLine(door->GetPivotLocation(), door->GetPivotLocation() + VecZ(180.0f), 134, 145, 255);
			}

			AOLHidingSpot* hidingSpot = Cast<AOLHidingSpot>(*It);
			if (hidingSpot)
			{
				DrawDebugSphere(hidingSpot->Location + VecZ(5.0f), 5.0f, 6, 134, 255, 145);
				DrawDebugLine(hidingSpot->Location + VecZ(5.0f), hidingSpot->Location + VecZ(5.0f) + 25.0f*hidingSpot->Rotation.Vector(), 134, 255, 145);
			}

			AOLBed* bed = Cast<AOLBed>(*It);
			if (bed)
			{
				DrawDebugSphere(bed->Location + VecZ(5.0f), 5.0f, 6, 255, 134, 145);
				DrawDebugLine(bed->Location + VecZ(5.0f), bed->Location + VecZ(5.0f) + 25.0f*bed->Rotation.Vector(), 255, 134, 145);
			}

			AOLLadderMarker* ladderMarker = Cast<AOLLadderMarker>(*It);
			if (ladderMarker && ladderMarker->OtherMarker && ladderMarker->Location.Z < ladderMarker->OtherMarker->Location.Z)
			{
				if (ladderMarker->IsValid() )
				{
					DrawDebugSphere(ladderMarker->Location, 5.0f, 6, 134, 255, 145);
					DrawDebugSphere(ladderMarker->OtherMarker->Location, 5.0f, 6, 134, 255, 145);
					DrawDebugLine(ladderMarker->Location, ladderMarker->OtherMarker->Location, 134, 255, 145, FALSE, thickness);				
				}
				else
				{
					DrawDebugSphere(ladderMarker->Location, 5.0f, 6, 242, 61, 61);
				}
			}

			AOLCornerMarker* cornerMarker = Cast<AOLCornerMarker>(*It);
			if (cornerMarker)
			{
				DrawDebugSphere(cornerMarker->Location, 5.0f, 6, 255, 154, 66);

				FLOAT handMarkerThickness = 15.0f;
				FLOAT lineLength = 25.0f;
				FLOAT cornerHeight = 137.0f;

				AOLHero* defaultHero = (AOLHero*)AOLHero::StaticClass()->GetDefaultObject();
				FVector baseLocStanding = cornerMarker->Location + VecZ(cornerHeight);

				if (cornerMarker->b3Sided)
				{
					FVector wallDir = cornerMarker->Rotation.Vector();
					FVector perpDir = cornerMarker->Rotation.Right();
					FVector sideOffset = 0.5f*cornerMarker->WallThickness * perpDir;
								
					if (cornerMarker->bCanPeekFromLeftStanding)
					{
						DrawDebugOrientedThickLine(baseLocStanding - sideOffset, baseLocStanding - sideOffset + lineLength*wallDir, 255, 154, 66, FALSE, handMarkerThickness);
					}

					if (cornerMarker->bCanPeekFromRightStanding)
					{
						DrawDebugOrientedThickLine(baseLocStanding + sideOffset, baseLocStanding + sideOffset + lineLength*wallDir, 255, 154, 66, FALSE, handMarkerThickness);
					}

					if (cornerMarker->bCanPeekFromLeftStanding || cornerMarker->bCanPeekFromRightStanding)
					{
						DrawDebugOrientedThickLine(baseLocStanding - sideOffset, baseLocStanding + sideOffset, 255, 154, 66, FALSE, handMarkerThickness);
					}
				}
				else
				{
					FVector leftDir = cornerMarker->Rotation.Vector();
					FVector rightDir = cornerMarker->Rotation.Right();

					if (cornerMarker->bCanPeekFromLeftStanding)
					{
						if (cornerMarker->bRoundedCorner)
						{
							FVector centerPt = baseLocStanding + 2.7f*leftDir + 2.7f*rightDir;
							FVector midPt = baseLocStanding + 6.0f*leftDir + 0.7f*rightDir;
							FVector flexPt = baseLocStanding + 10.8f*leftDir;
							FVector endPt = baseLocStanding + lineLength*leftDir;

							DrawDebugOrientedThickLine(endPt, flexPt, 255, 154, 66, FALSE, handMarkerThickness);
							DrawDebugOrientedThickLine(flexPt, midPt, 255, 154, 66, FALSE, handMarkerThickness);
							DrawDebugOrientedThickLine(midPt, centerPt, 255, 154, 66, FALSE, handMarkerThickness);
						}
						else
						{
							DrawDebugOrientedThickLine(baseLocStanding, baseLocStanding + lineLength*leftDir, 255, 154, 66, FALSE, handMarkerThickness);
						}
					}

					if (cornerMarker->bCanPeekFromRightStanding)
					{
						if (cornerMarker->bRoundedCorner)
						{
							FVector centerPt = baseLocStanding + 2.7f*rightDir + 2.7f*leftDir;
							FVector midPt = baseLocStanding + 6.0f*rightDir + 0.7f*leftDir;
							FVector flexPt = baseLocStanding + 10.8f*rightDir;
							FVector endPt = baseLocStanding + lineLength*rightDir;

							DrawDebugOrientedThickLine(endPt, flexPt, 255, 154, 66, FALSE, handMarkerThickness);
							DrawDebugOrientedThickLine(flexPt, midPt, 255, 154, 66, FALSE, handMarkerThickness);
							DrawDebugOrientedThickLine(midPt, centerPt, 255, 154, 66, FALSE, handMarkerThickness);
						}
						else
						{
							DrawDebugOrientedThickLine(baseLocStanding, baseLocStanding + lineLength*rightDir, 255, 154, 66, FALSE, handMarkerThickness);
						}
					}
				}
			}

			AOLRecordingMarker* recordingMarker = Cast<AOLRecordingMarker>(*It);
			if (recordingMarker)
			{
				if (recordingMarker->IsValid())
				{
					DrawDebugSphere(recordingMarker->Location, recordingMarker->Radius, 12, 134, 255, 145);				
				}
				else
				{
					DrawDebugSphere(recordingMarker->Location, recordingMarker->Radius, 12, 115, 128, 116);
				}
			}
		}	
	}

	if (Utils::GetCheatManager())
	{
		if (SoundEnvManager && Utils::GetCheatManager()->bDebugSoundEnvironment)
		{
			SoundEnvManager->DrawDebug(baseLoc);
		}

		if (HeroPawn && (bDebugFreeCam || bDebugFixedCam) && !bDebugGhost)
		{
			if (false)
			{
				DrawDebugSphere( HeroPawn->Location, 5.0f, 6, 16, 128, 46 );
				DrawDebugSphere( HeroPawn->Mesh->LocalToWorld.GetOrigin(), 2.0f, 6, 128, 16, 16 );
				DrawDebugCylinder( HeroPawn->CylinderComponent->Bounds.Origin - FVector(0, 0, HeroPawn->CylinderComponent->CollisionHeight), HeroPawn->CylinderComponent->Bounds.Origin + FVector(0, 0, HeroPawn->CylinderComponent->CollisionHeight), HeroPawn->CylinderComponent->CollisionRadius, 10, 0, 60, 60);
				DrawDebugSphere( HeroPawn->EyeLocation, 5.0f, 6, 32, 255, 92 );
				DrawDebugLine( HeroPawn->EyeLocation, HeroPawn->EyeLocation + 50.0f*HeroPawn->EyeRotation.Vector(), 32, 255, 92 );

				FVector camPos = HeroPawn->Mesh->GetBoneLocation(Utils::GetCameraBoneName());
				FRotator camRot = HeroPawn->Mesh->GetBoneQuaternion(Utils::GetCameraBoneName()).Rotator();
				DrawDebugSphere(camPos, 3.0f, 6, 235, 60, 60);
				DrawDebugLine(camPos, camPos + 50.0f*camRot.Vector(), 235, 60, 60);
			}

			if (Utils::GetCheatManager()->bDebugCamera)
			{
				HeroPawn->Camera->DrawDebug();
			}
		}

		if (HeroPawn && Utils::GetCheatManager()->bDebugAIPositions)
		{
			HeroPawn->DrawDebugAIPositions();
		}

		Utils::GetCheatManager()->DrawDebug();
	}
#endif
}

// ---------------------------------------------------------------------------
// AOLPlayerController::RayIntersectsOBB (native UC function)
// Tests a ray against a StaticMeshComponent's OBB by transforming the ray
// into local space and doing a slab test against the mesh asset's extents.
// ---------------------------------------------------------------------------
UBOOL AOLPlayerController::RayIntersectsOBB(FVector RayOrigin, FVector RayDir,
                                             UStaticMeshComponent* SMC, FVector& HitPos)
{
    if (!SMC || !SMC->StaticMesh || !SMC->IsAttached())
        return FALSE;

    // Transform ray into component local space (handles rotation + non-uniform scale)
    FMatrix WorldToLocal = SMC->LocalToWorld.Inverse();
    FVector LocalOrigin  = WorldToLocal.TransformFVector(RayOrigin);
    FVector LocalDir     = WorldToLocal.TransformNormal(RayDir);

    // Local-space extents of the mesh asset (unscaled — scale is baked into LocalToWorld)
    FVector E = SMC->StaticMesh->Bounds.BoxExtent;
    FVector C = SMC->StaticMesh->Bounds.Origin;

    FVector BoxMin = C - E;
    FVector BoxMax = C + E;

    FLOAT tMin = -1e10f, tMax = 1e10f, t0, t1, Tmp;

    // X slab
    if (Abs(LocalDir.X) < 0.0001f)
    { if (LocalOrigin.X < BoxMin.X || LocalOrigin.X > BoxMax.X) return FALSE; }
    else
    {
        t0 = (BoxMin.X - LocalOrigin.X) / LocalDir.X;
        t1 = (BoxMax.X - LocalOrigin.X) / LocalDir.X;
        if (t0 > t1) { Tmp = t0; t0 = t1; t1 = Tmp; }
        if (t0 > tMin) tMin = t0;
        if (t1 < tMax) tMax = t1;
        if (tMin > tMax) return FALSE;
    }

    // Y slab
    if (Abs(LocalDir.Y) < 0.0001f)
    { if (LocalOrigin.Y < BoxMin.Y || LocalOrigin.Y > BoxMax.Y) return FALSE; }
    else
    {
        t0 = (BoxMin.Y - LocalOrigin.Y) / LocalDir.Y;
        t1 = (BoxMax.Y - LocalOrigin.Y) / LocalDir.Y;
        if (t0 > t1) { Tmp = t0; t0 = t1; t1 = Tmp; }
        if (t0 > tMin) tMin = t0;
        if (t1 < tMax) tMax = t1;
        if (tMin > tMax) return FALSE;
    }

    // Z slab
    if (Abs(LocalDir.Z) < 0.0001f)
    { if (LocalOrigin.Z < BoxMin.Z || LocalOrigin.Z > BoxMax.Z) return FALSE; }
    else
    {
        t0 = (BoxMin.Z - LocalOrigin.Z) / LocalDir.Z;
        t1 = (BoxMax.Z - LocalOrigin.Z) / LocalDir.Z;
        if (t0 > t1) { Tmp = t0; t0 = t1; t1 = Tmp; }
        if (t0 > tMin) tMin = t0;
        if (t1 < tMax) tMax = t1;
        if (tMin > tMax) return FALSE;
    }

    if (tMin < 0.0f) return FALSE; // box is behind the ray origin in local space

    FVector LocalHit = LocalOrigin + LocalDir * tMin;
    HitPos = SMC->LocalToWorld.TransformFVector(LocalHit);

    // Sanity check in world space: hit must be in front of the ray origin
    // (guards against negative Scale3D flipping local-space ray direction)
    if (((HitPos - RayOrigin) | RayDir) < 0.0f)
        return FALSE;

    return TRUE;
}

