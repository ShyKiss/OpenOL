/*=============================================================================
	OLGame.cpp
=============================================================================*/

#include "OLGame.h"
#include "EngineAnimClasses.h"
#include "UDKBaseAnimationClasses.h"
#include "OLGameAIClasses.h"
#include "OLGameAnimClasses.h"
#include "OLGameSequenceClasses.h"


//////////////////////////////////////////////////////////////////////////
// Preprocessor stuff
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	The following must be done once per package.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName OLGAME_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "OLGameClasses.h"
#undef AUTOGENERATE_NAME
#include "OLGameAIClasses.h"
#include "OLGameAnimClasses.h"
#include "OLGameSequenceClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "OLGameClasses.h"
#undef AUTOGENERATE_NAME
#include "OLGameAIClasses.h"
#include "OLGameAnimClasses.h"
#include "OLGameSequenceClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsOLGame( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_OLGAME;
	AUTO_INITIALIZE_REGISTRANTS_OLGAME_AI;
	AUTO_INITIALIZE_REGISTRANTS_OLGAME_ANIM;
	AUTO_INITIALIZE_REGISTRANTS_OLGAME_SEQUENCE;
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesOLGame()
{
	#define NAMES_ONLY
    #define AUTOGENERATE_NAME(name) OLGAME_##name = FName(TEXT(#name));
		#include "OLGameNames.h"
	#undef AUTOGENERATE_NAME
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#include "OLGameClasses.h"
	#include "OLGameAIClasses.h"
	#include "OLGameAnimClasses.h"
	#include "OLGameSequenceClasses.h"
	#undef AUTOGENERATE_FUNCTION	
	#undef NAMES_ONLY
}


#if CHECK_NATIVE_CLASS_SIZES
#if _MSC_VER
#pragma optimize( "", off )
#endif

void AutoCheckNativeClassSizesOLGame( UBOOL& Mismatch )
{
#define NAMES_ONLY
#define AUTOGENERATE_NAME( name )
#define AUTOGENERATE_FUNCTION( cls, idx, name )
#define VERIFY_CLASS_SIZES
#include "OLGameClasses.h"
#include "OLGameAIClasses.h"
#include "OLGameAnimClasses.h"
#include "OLGameSequenceClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY
#undef VERIFY_CLASS_SIZES
}

#if _MSC_VER
#pragma optimize( "", on )
#endif
#endif

IMPLEMENT_CLASS(UOLGameViewportClient);
IMPLEMENT_CLASS(AOLGame);
IMPLEMENT_CLASS(UOLTypes);

/**
 * Game-specific code to handle DLC being added or removed
 *
 * @param bContentWasInstalled TRUE if DLC was installed, FALSE if it was removed
 */
void appOnDownloadableContentChanged(UBOOL bContentWasInstalled)
{
}

//////////////////////////////////////////////////////////////////////////
// UOLGameViewportClient
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UBOOL UOLGameViewportClient::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent EventType,FLOAT AmountDepressed,UBOOL bGamepad)
{
	// rcharpentier - patch cheat to allow unblocking all checkpoints from the credits menu
	if (EventType == IE_Pressed && Key == FName(TEXT("F6")))
	{
		if (Utils::GetOLPC() && Utils::GetOLPC()->ShippingCheat_GiveAllCheckpoints())
		{
			return true;
		}
	}

	return Super::InputKey(Viewport, ControllerId, Key, EventType, AmountDepressed, bGamepad);
}

//////////////////////////////////////////////////////////////////////////
// OLGame
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLGame::PostBeginPlay()
{
	Super::PostBeginPlay();

	VoiceManager = ConstructObject<UOLVoiceManager>(UOLVoiceManager::StaticClass(), this);
	
	if (VoiceManager)
	{
		VoiceManager->Init();
	}		
}

void AOLGame::BeginDestroy()
{
	if (VoiceManager)
	{
		VoiceManager->Deinit();
	}

	Super::BeginDestroy();
}

void AOLGame::OnTravelToStartupMap()
{
#if DINGO
	if (GOLDingo)
	{
		GOLDingo->UpdateSessionState(OLDingo::SS_Inactive);
	}
#endif
}

void AOLGame::HandlePaused()
{
	if (GWorld->GetGameSequence())
	{
		TArray<USequenceObject*> MatineeActions;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqAct_Interp::StaticClass(), MatineeActions);

		for (INT Idx = 0; Idx < MatineeActions.Num(); Idx++)
		{
			USeqAct_Interp* MatineeAction = CastChecked<USeqAct_Interp>( MatineeActions(Idx) );
			check(MatineeAction);

			for (INT j = 0; j < MatineeAction->GroupInst.Num(); ++j)
			{
				UInterpGroupInst* GroupInst = MatineeAction->GroupInst(j);

				for (INT k = 0; k < GroupInst->TrackInst.Num(); ++k)
				{
					UInterpTrackInstAkEvent* AkEventTrackInst = Cast<UInterpTrackInstAkEvent>(GroupInst->TrackInst(k));
					UInterpTrackAkEvent* AkEventTrack = Cast<UInterpTrackAkEvent>(GroupInst->Group->InterpTracks(k));
					if (AkEventTrackInst && AkEventTrack)
					{
						AkEventTrack->PauseAkEvent(AkEventTrackInst);
					}
				}
			}
		}
	}

	for (TObjectIterator<UTextureMovie> It; It; ++It)
	{
		UTextureMovie* TextureMovie = *It;
		if (!TextureMovie->Stopped && !TextureMovie->Paused)
		{
			TextureMovie->Pause();
			TextureMovie->bShouldResume = TRUE;
		}
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC)
	{
		if (bSoundOnPause)
		{
			bSoundOnPause = FALSE;
			OLPC->PostAkEvent(SndPause);
		}
		else
		{
			OLPC->PostAkEvent(SndPauseNoSound);
		}
	}

#if DINGO
	if (GOLDingo)
	{
		GOLDingo->UpdateSessionState(OLDingo::SS_Paused);
	}
#endif
}

void AOLGame::HandleUnpaused()
{
	if (GWorld->GetGameSequence())
	{
		TArray<USequenceObject*> MatineeActions;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqAct_Interp::StaticClass(), MatineeActions);

		for (INT Idx = 0; Idx < MatineeActions.Num(); Idx++)
		{
			USeqAct_Interp* MatineeAction = CastChecked<USeqAct_Interp>( MatineeActions(Idx) );
			check(MatineeAction);

			for (INT j = 0; j < MatineeAction->GroupInst.Num(); ++j)
			{
				UInterpGroupInst* GroupInst = MatineeAction->GroupInst(j);

				for (INT k = 0; k < GroupInst->TrackInst.Num(); ++k)
				{
					UInterpTrackInstAkEvent* AkEventTrackInst = Cast<UInterpTrackInstAkEvent>(GroupInst->TrackInst(k));
					UInterpTrackAkEvent* AkEventTrack = Cast<UInterpTrackAkEvent>(GroupInst->Group->InterpTracks(k));
					if (AkEventTrackInst && AkEventTrack)
					{
						AkEventTrack->UnpauseAkEvent(AkEventTrackInst);
					}
				}
			}
		}
	}
	
	for (TObjectIterator<UTextureMovie> It; It; ++It)
	{
		UTextureMovie* TextureMovie = *It;
		if (TextureMovie->Paused && TextureMovie->bShouldResume)
		{
			TextureMovie->Play();
			TextureMovie->bShouldResume = FALSE;
		}
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC)
	{
		if (bSoundOnPause)
		{
			bSoundOnPause = FALSE;
			OLPC->PostAkEvent(SndUnpause);
		}
		else
		{
			OLPC->PostAkEvent(SndUnpauseNoSound);
		}
	}

#if DINGO
	if (GOLDingo && CurrentCheckpointName != NAME_None && OLPC && OLPC->HeroPawn)
	{
		GOLDingo->UpdateSessionState(OLDingo::SS_Active);
	}
#endif
}

AOLCheckpoint* AOLGame::MatchCheckpoint(const FString& PortalName)
{
	if (PortalName.Len() > 0)
	{
		return Utils::GetCheckpointFromName(FName(*PortalName));
	}
	return NULL;
}

void AOLGame::UpdateGameType()
{
	// Outlast 2 prepro - we're not playing DLC.
	bIsPlayingDLC = FALSE;

	// Update whether we're playing DLC

//	UBOOL bKnownGameType = FALSE;
//
//#if !SHIPPING_PC_GAME
//	if (bForcePlayingDLC && Utils::GetCheatManager() && Utils::GetCheatManager()->bCheatsEnabled)
//	{
//		bIsPlayingDLC = TRUE;
//		bKnownGameType = TRUE;
//	}
//#endif
//
//	if (!bKnownGameType && CurrentCheckpointName != NAME_None)
//	{
//		AOLCheckpointList* ownerList = AOLCheckpointList::GetListForCheckpoint(CurrentCheckpointName);
//		if (ownerList)
//		{
//			bIsPlayingDLC = (ownerList->GameType == OGT_Whistleblower);
//			bKnownGameType = TRUE;
//
//			debugf(TEXT("Starting at checkpoint \"%s\" - DLC: %s"), *CurrentCheckpointName.ToString(), bIsPlayingDLC ? TEXT("yes") : TEXT("no"));
//		}
//	}
//
//	if (!bKnownGameType) 
//	{
//		// Not sure, so check whether we're in the main menu
//		for (INT i = 0; i < GWorld->Levels.Num(); i++)
//		{
//			ULevel* level = GWorld->Levels(i);
//			if (level && level->GetOutermost()->GetName().ToLower().InStr(TEXT("mainmenu")) != INDEX_NONE)
//			{
//				bIsPlayingDLC = FALSE; // Main menu is not considered DLC
//				bKnownGameType = TRUE;
//			}
//		}
//	}
//
//	if (!bKnownGameType) 
//	{
//		// Still not sure, so check whether the persistent starts with dlc_
//		if (GWorld->PersistentLevel && GWorld->PersistentLevel->GetOutermost()->GetName().ToLower().InStr(TEXT("dlc_")) != INDEX_NONE)
//		{
//			bIsPlayingDLC = TRUE; 
//			bKnownGameType = TRUE;
//
//			debugf(TEXT("Playing DLC - invalid checkpoint but DLC persistent"), *CurrentCheckpointName.ToString(), bIsPlayingDLC ? TEXT("yes") : TEXT("no"));
//		}
//	}
//
//	if (!bKnownGameType)
//	{
//		bIsPlayingDLC = FALSE; // if still not sure, e.g. a gym, we're not considered DLC
//	}
//
//	UAkAudioDevice* akDevice = UAkAudioDevice::Get();
//	if (GIsRunning && akDevice) // GIsRunning to prevent updating during the loading sequence - cuts off the bink movie sound
//	{
//		// Reload the sounds banks if we're switching main <-> dlc
//		akDevice->ConditionalReloadAllBanks(bIsPlayingDLC);
//	}
//	
//	AOLCheckpointList::SetEffectiveList(bIsPlayingDLC);
//	AOLGameStateList::SetEffectiveList(bIsPlayingDLC);
}

UBOOL AOLGame::IsDemo()
{
	return bIsDemo;
}

UBOOL AOLGame::IsPlayingDLC()
{
	return bIsPlayingDLC;
}

UBOOL AOLGame::IsDLCInstalled()
{
	if (GIsEditor)
	{
		return TRUE;
	}

	UOLEngine* olEngine = Cast<UOLEngine>(GEngine);
	if (!olEngine)
	{
		return FALSE;
	}

	UOLDLCManager* dlcMgr = Cast<UOLDLCManager>(olEngine->DLCManager);
	if (dlcMgr)
	{
		return dlcMgr->IsDLCInstalled();
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
