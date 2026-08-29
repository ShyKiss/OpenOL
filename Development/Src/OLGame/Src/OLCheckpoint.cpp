#include "OLGame.h"

#if ORBIS
#include "OLOrbis.h"
#endif

IMPLEMENT_CLASS(AOLCheckpoint);
IMPLEMENT_CLASS(UOLCheckpointSave);
IMPLEMENT_CLASS(AOLCheckpointList);
IMPLEMENT_CLASS(AOLGameStateList);

AOLCheckpointList* GMasterCheckpointList = NULL;
AOLGameStateList* GMasterGameStateList = NULL;

////////////////////////////////////////////////////////////////////////////////////////////

void AOLCheckpoint::OnPlayerSpawn()
{
	AOLGame* CurrentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	CurrentGame->CurrentCheckpointName = CheckpointName;

	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (OLPC != NULL)
	{
		OLPC->StartTravelToCheckpoint();
	}
}

void AOLCheckpoint::ResetCheckpointDeaths()
{
	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (OLPC != NULL)
	{
		OLPC->NumDeathsSinceLastCheckpoint = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void CheckpointSerializeName(FArchive& Ar, FName& N)
{
	// we cannot safely use the name index as that is not guaranteed to persist across game sessions
	FString NameText;
	if (Ar.IsSaving())
	{
		NameText = N.GetNameString();
	}
	INT Number = N.GetNumber();

	Ar << NameText;
	Ar << Number;

	if (Ar.IsLoading())
	{
		// copy over the name with a name made from the name string and number
		N = FName(*NameText, Number);
	}
}

/** class for reading/writing checkpoint Actor records that handles names as strings */
class FCheckpointActorRecordWriter : public FMemoryWriter
{
public:
	FCheckpointActorRecordWriter(TArray<BYTE>& InBytes)
		: FMemoryWriter(InBytes)
	{}

	virtual FArchive& operator<<( class FName& N )
	{
		CheckpointSerializeName(*this, N);
		return *this;
	}
};
class FCheckpointActorRecordReader : public FMemoryReader
{
public:
	FCheckpointActorRecordReader(TArray<BYTE>& InBytes)
		: FMemoryReader(InBytes)
	{}

	virtual FArchive& operator<<( class FName& N )
	{
		CheckpointSerializeName(*this, N);
		return *this;
	}
};

static FArchive& operator<<(FArchive& Ar, FCheckpointTime& SaveTime)
{
	Ar << SaveTime.Year << SaveTime.Month << SaveTime.Day << SaveTime.SecondsSinceMidnight;
	return Ar;
}

////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Serializes the header data while validating the format
 *
 * @param Ar the archive serializing to/from
 * @param BaseLevelName base "P" level this checkpoint was saved in
 * @param BaseLevelGuid package GUID of that level (to detect incompatibility due to level changes)
 * @param ChapterNum the current chapter at the time the checkpoint was saved
 * @param Difficulty the difficulty the player was using when the checkpoint was set
 * @param SaveTime the time the checkpoint was saved
 *
 * @return true if the format is valid, false otherwise
 */
UBOOL UOLCheckpointSave::SerializeHeader(FArchive& Ar,FString& CheckpointName,FCheckpointTime& SaveTime, UBOOL& bDeprecatedPlayerData, INT& GameDifficulty)
{
	const INT CHECKPOINT_VERSION = 2; // rcharpentier - on PS4 versions 1 and 2 are the same, on PC, version 1 has PlayerData of type DeprecatedCheckpointRecord

	INT DataVersion = CHECKPOINT_VERSION;
	Ar << DataVersion;
	// fail with an empty checkpoint object if the versions don't match

#if !CONSOLE
	bDeprecatedPlayerData = (DataVersion == 1);
#else 
	bDeprecatedPlayerData = FALSE; // not used on other platforms (use CheckpointRecordVersion instead)
#endif

	if (!Ar.IsError())
	{
		Ar << CheckpointName;		
		Ar << SaveTime;

#if DINGO
		Ar << GameDifficulty; // used as metadata, not for actual game purposes (to be refactored)
#endif
		return TRUE;
	}
	return FALSE;
}

void UOLCheckpointSave::SerializeFinalData(FArchive& Ar)
{
	UBOOL bDeprecated = FALSE;

	if (SerializeHeader(Ar,CheckpointName,SaveTime, bDeprecated, GameDifficulty))
	{
		Ar << PlayerData;

		bDeprecatedPlayerData = bDeprecated;
	}
	else
	{
		debugf(NAME_Warning, TEXT("Incompatible checkpoint for %s"), *Filename);
		CheckpointName = TEXT("");
		debugf(NAME_Warning, TEXT("Corrupt checkpoint data (%s)"), *Filename);
	}
}

#if ORBIS
void UOLCheckpointSave::SaveMetaData(struct OrbisSaveMetaData& saveMetaData)
{
	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (!OLPC)
	{
		return;
	}

	appMemZero(saveMetaData);
	
	FName cpName = Utils::GetOLGame()->CurrentCheckpointName;
	FName cpTag = Utils::GetCheckpointTag(cpName);

	saveMetaData.Title = FString(TEXT("Outlast")); // overwritten for "Outlast #"

	EDifficultyMode difficulty = (EDifficultyMode)Utils::GetOLGame()->DifficultyMode;
	switch (difficulty)
	{
	case EDM_Normal:
		saveMetaData.Details = Localize(TEXT("Generic"), TEXT("SaveDifficultyNormal"), TEXT("OLGame"));
		break;
	case EDM_Hard:
		saveMetaData.Details = Localize(TEXT("Generic"), TEXT("SaveDifficultyHard"), TEXT("OLGame"));
		break;
	case EDM_Nightmare:
		saveMetaData.Details = Localize(TEXT("Generic"), TEXT("SaveDifficultyNightmare"), TEXT("OLGame"));
		break;
	case EDM_Insane:
		saveMetaData.Details = Localize(TEXT("Generic"), TEXT("SaveDifficultyInsane"), TEXT("OLGame"));
		break;
	}
	
	if (cpTag != NAME_None)
	{
		saveMetaData.IconFile = FString::Printf(TEXT("Icon%s"), *cpTag.ToString());
		saveMetaData.Subtitle = Localize(TEXT("Locations"), *cpTag.ToString(), TEXT("OLGame"));
	}
	else
	{
		saveMetaData.IconFile = FString(TEXT("IconDefault"));
		saveMetaData.Subtitle = FString(TEXT("[UNKNOWN LOCATION]"));
	}
}
#endif

void UOLCheckpointSave::SaveData(TArray<BYTE>& OutSaveGameData, INT SaveTimeOverride /* = -1 */)
{
	if (CheckpointName == TEXT("") && Utils::GetOLGame())
	{
		CheckpointName = Utils::GetOLGame()->CurrentCheckpointName.ToString();
	}

	debugf(TEXT("--- SAVING CHECKPOINT \"%s\" ---"), *CheckpointName);
	DOUBLE StartTime = appSeconds();

	AWorldInfo* Info = GWorld->GetWorldInfo();

	if (SaveTimeOverride <= 0)
	{
		// save time
		INT DayOfWeek, Hour, Min, Sec, MSec;
		appSystemTime(SaveTime.Year, SaveTime.Month, DayOfWeek, SaveTime.Day, Hour, Min, Sec, MSec);
		SaveTime.SecondsSinceMidnight = Hour * 3600 + Min * 60 + Sec;
	}
	else
	{
		SaveTime.Year = 2013;
		SaveTime.Month = 9;
		SaveTime.Day = 4;
		SaveTime.SecondsSinceMidnight = SaveTimeOverride;
	}

#if DINGO
	if (Utils::GetOLGame())
	{
		GameDifficulty = Utils::GetOLGame()->DifficultyMode;
	}
#endif

	AOLPlayerController* PC = Utils::GetOLPC();
	if (PC != NULL)
	{
		FName CheckpointFunctionName = FName(TEXT("CreateCheckpointRecord"));
		UStruct *CheckpointStruct = FindField<UStruct>(PC->GetClass(),TEXT("CheckpointRecord"));
		UFunction *Function = PC->FindFunction(CheckpointFunctionName);
		check(Function);
		check(CheckpointStruct);

		// allocate space for the checkpoint record struct
		TArray<BYTE> BaseData;
		BaseData.AddZeroed(CheckpointStruct->GetPropertiesSize());
		// call the function to fill in the data
		PC->ProcessEvent(Function, BaseData.GetData(), NULL);
		// serialize the struct into the final data so that e.g. strings are in there as their characters and not data pointers, etc
		{
			FCheckpointActorRecordWriter RawDataWriter(PlayerData);
			RawDataWriter.SetPortFlags(PPF_ForceBinarySerialization);
			CheckpointStruct->SerializeBin(RawDataWriter, BaseData.GetTypedData(), CheckpointStruct->GetPropertiesSize());
		}
		PlayerData.Shrink();
		// clean up constructed elements of the checkpoint record
		for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
		{
			if ((P->GetClass()->ClassCastFlags & CASTCLASS_UStructProperty) && ((UStructProperty*)P)->Struct == CheckpointStruct)
			{
				P->DestroyValue(BaseData.GetData());
				break;
			}
		}
	}

	{
		FMemoryWriter Writer(OutSaveGameData);
		SerializeFinalData(Writer);
	} 
	
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	GLog->Logf(TEXT("--- CHECKPOINT SAVED [%4.2f seconds] ---"), appSeconds() - StartTime);
#endif
}

UBOOL UOLCheckpointSave::LoadData()
{
	DOUBLE StartTime = appSeconds();
	debugf(TEXT("--- LOADING CHECKPOINT \"%s\" ---"), *CheckpointName);

	AOLGame* Game = Cast<AOLGame>(GWorld->GetGameInfo());
	if (Game != NULL)
	{	
		for( FActorIterator It; It; ++It )
		{
			AOLCheckpoint *checkpoint = Cast<AOLCheckpoint>(*It);
			if( checkpoint && !checkpoint->bDeleteMe && checkpoint->CheckpointName.ToString() == CheckpointName)
			{
				Game->CurrentCheckpointName = checkpoint->CheckpointName;
				break;
			}
		}

		if (Game->CurrentCheckpointName == NAME_None)
		{
			debugf(TEXT("- Unable to find Checkpoint, aborting"));
			return FALSE;
		}
	}

	// TODO: Player Controller Info
	AOLPlayerController* PC = Utils::GetOLPC();
	if (PC != NULL)
	{
		if (bDeprecatedPlayerData)
		{
			debugf(TEXT("Loading Deprecated Save Game..."));
		}

		FName CheckpointFunctionName = bDeprecatedPlayerData ? FName(TEXT("ApplyDeprecatedCheckpointRecord")) : FName(TEXT("ApplyCheckpointRecord"));

		UFunction *Function = PC->FindFunction(CheckpointFunctionName);
		UScriptStruct* CheckpointStruct = FindField<UScriptStruct>(PC->GetClass(), bDeprecatedPlayerData ? TEXT("DeprecatedCheckpointRecord") : TEXT("CheckpointRecord"));

		check(Function);
		check(CheckpointStruct);
		check(Function->ParmsSize == Align(CheckpointStruct->GetPropertiesSize(), CheckpointStruct->GetMinAlignment())); // 
		
		// serialize the recorded data into the function's parameter
		TArray<BYTE> EventData;
		EventData.AddZeroed(Function->ParmsSize);
		{
			FCheckpointActorRecordReader Reader(PlayerData);
			Reader.SetPortFlags(PPF_ForceBinarySerialization);
			CheckpointStruct->SerializeBin(Reader, EventData.GetTypedData(), Function->ParmsSize);
		}
		PC->ProcessEvent(Function, EventData.GetData(), NULL);
		// clean up constructed elements of the checkpoint record
		for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
		{
			if ((P->GetClass()->ClassCastFlags & CASTCLASS_UStructProperty) && ((UStructProperty*)P)->Struct == CheckpointStruct)
			{
				P->DestroyValue(EventData.GetData());
				break;
			}
		}
			
		return FALSE;
	}

	debugf(TEXT("--- CHECKPOINT LOADED [%4.2f seconds] ---"), appSeconds() - StartTime);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// AOLCheckpointList
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLCheckpointList::PostBeginPlay()
{
	GMasterCheckpointList = this;
}

void AOLCheckpointList::BeginDestroy()
{
	if (GMasterCheckpointList == this)
		GMasterCheckpointList = NULL;
	Super::BeginDestroy();
}

void AOLCheckpointList::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// update all kismet objects that may change status (valid/invalid depending on if they're in the list)
	if (GWorld->GetGameSequence())
	{
		TArray<USequenceObject*> allApplyCPStateEvents;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_ApplyCheckpointState::StaticClass(), allApplyCPStateEvents);

		// Iterate over them, seeing if the name is the one we typed in.
		for (INT Idx = 0; Idx < allApplyCPStateEvents.Num(); Idx++)
		{
			UOLSeqEvent_ApplyCheckpointState* applyCPStateEvent = Cast<UOLSeqEvent_ApplyCheckpointState>(allApplyCPStateEvents(Idx));
			applyCPStateEvent->UpdateStatus();
		}

		TArray<USequenceObject*> allCPConds;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqCond_Checkpoint::StaticClass(), allCPConds);

		// Iterate over them, seeing if the name is the one we typed in.
		for (INT Idx = 0; Idx < allCPConds.Num(); Idx++)
		{
			UOLSeqCond_Checkpoint* cpCond = Cast<UOLSeqCond_Checkpoint>(allCPConds(Idx));
			cpCond->UpdateStatus();
		}
	}
}

TArray<FName>* AOLCheckpointList::GetCheckpointList()
{
	if (GWorld->HasBegunPlay())
	{
		if (GMasterCheckpointList)
		{
			return &GMasterCheckpointList->CheckpointList;
		}
		else
		{
			// No master list. Maybe a gym, maybe the first call after loading a non-DLC checkpoint

			for (FActorIterator It; It; ++It)
			{
				AOLCheckpointList* cpList = Cast<AOLCheckpointList>(*It);
				if (cpList)
				{
					debugf(TEXT("New master checkpoint list found."));
					GMasterCheckpointList = cpList;
					return &cpList->CheckpointList;
				}
			}

			return NULL;
		}
	}
	else
	{
		// In editor, iterate over all actors to find the master list
		for (FActorIterator It; It; ++It)
		{
			AOLCheckpointList* cpList = Cast<AOLCheckpointList>(*It);
			if (cpList)
			{
				return &cpList->CheckpointList;
			}
		}
	}

	return NULL;
}

UBOOL AOLCheckpointList::IsCheckpointInList(const FName& checkpointName)
{
	TArray<FName>* cpList = GetCheckpointList();

	if (cpList)
	{
		return cpList->ContainsItem(checkpointName);
	}
	return FALSE;
}

UBOOL AOLCheckpointList::Script_IsUnreached(const FName& testCheckpoint, const FName& currentCheckpoint)
{
	return IsUnreached(testCheckpoint, currentCheckpoint);
}

void AOLCheckpointList::Script_TriggerAllApplyCheckpointStateEvents()
{
	if (GWorld == NULL || GWorld->GetGameSequence() == NULL)
		return;
	TArray<USequenceObject*> allEvents;
	GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_ApplyCheckpointState::StaticClass(), allEvents);
	for (INT i = 0; i < allEvents.Num(); i++)
	{
		UOLSeqEvent_ApplyCheckpointState* evt = Cast<UOLSeqEvent_ApplyCheckpointState>(allEvents(i));
		if (evt != NULL)
			AOLCheckpointList::TriggerApplyCheckpointStateEvent(evt);
	}
}

UBOOL AOLCheckpointList::IsUnreached(const FName& testCheckpoint, const FName& currentCheckpoint)
{
	if (testCheckpoint == currentCheckpoint)
	{
		return FALSE;
	}

	TArray<FName>* cpList = GetCheckpointList();

	if (cpList)
	{
		INT testIdx = cpList->FindItemIndex(testCheckpoint);
		INT currentIdx = cpList->FindItemIndex(currentCheckpoint);

		return (testIdx >= 0 && currentIdx >= 0 && currentIdx < testIdx);
	}

	return FALSE;
}

UBOOL AOLCheckpointList::IsReached(const FName& testCheckpoint, const FName& currentCheckpoint)
{
	if (testCheckpoint == currentCheckpoint)
	{
		return TRUE;
	}

	TArray<FName>* cpList = GetCheckpointList();
	if (cpList)
	{
		INT testIdx = cpList->FindItemIndex(testCheckpoint);
		INT currentIdx = cpList->FindItemIndex(currentCheckpoint);

		return (testIdx >= 0 && currentIdx >= 0 && currentIdx >= testIdx);
	}

	return FALSE;
}

UBOOL AOLCheckpointList::IsCompleted(const FName& testCheckpoint, const FName& currentCheckpoint)
{
	if (testCheckpoint == currentCheckpoint)
	{
		return FALSE;
	}

	TArray<FName>* cpList = GetCheckpointList();
	if (cpList)
	{
		INT testIdx = cpList->FindItemIndex(testCheckpoint);
		INT currentIdx = cpList->FindItemIndex(currentCheckpoint);

		return (testIdx >= 0 && currentIdx >= 0 && currentIdx > testIdx);
	}

	return FALSE;
}

void AOLCheckpointList::TriggerApplyCheckpointStateEvent(UOLSeqEvent_ApplyCheckpointState* applyCPStateEvent)
{
	if (applyCPStateEvent != NULL && Utils::GetCheckpointFromName(applyCPStateEvent->CheckpointName) != NULL)
	{
		FName currentCheckpoint = Utils::GetCurrentCheckpointName();

		TArray<INT> activateIndices;

		if (applyCPStateEvent->CheckpointName == currentCheckpoint)
		{
			activateIndices.AddItem(2); // Current
		}
		else if (AOLCheckpointList::IsUnreached(applyCPStateEvent->CheckpointName, currentCheckpoint))
		{
			activateIndices.AddItem(0); // Unreached
		}

		if (AOLCheckpointList::IsReached(applyCPStateEvent->CheckpointName, currentCheckpoint))
		{
			activateIndices.AddItem(1); // Reached
		}

		if (AOLCheckpointList::IsCompleted(applyCPStateEvent->CheckpointName, currentCheckpoint))
		{
			activateIndices.AddItem(3); // Completed
		}

		if (activateIndices.Num() > 0)
		{
			applyCPStateEvent->CheckActivate(GMasterCheckpointList, Utils::GetOLPC(), FALSE, &activateIndices);
		}
	}
}

INT AOLCheckpointList::GetCheckpointIdx(const FName& testCheckpoint)
{
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpointList* cpList = Cast<AOLCheckpointList>(*It);
		if (cpList)
		{
			INT cpIdx = cpList->CheckpointList.FindItemIndex(testCheckpoint);
			if (cpIdx >= 0)
			{
				return cpIdx;
			}
		}
	}

	return -1;
}

AOLCheckpointList* AOLCheckpointList::GetListForCheckpoint(const FName& testCheckpoint)
{
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpointList* cpList = Cast<AOLCheckpointList>(*It);
		if (cpList)
		{
			INT testIdx = cpList->CheckpointList.FindItemIndex(testCheckpoint);
			if (testIdx >= 0)
			{
				return cpList;
			}
		}
	}

	return NULL;
}

void AOLCheckpointList::SetEffectiveList(UBOOL bPlayingDLC)
{
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpointList* cpList = Cast<AOLCheckpointList>(*It);
		if (cpList)
		{
			UBOOL bDLCList = (cpList->GameType == OGT_Whistleblower);

			if (bDLCList == bPlayingDLC)
			{
				GMasterCheckpointList = cpList;
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AOLGameStateList
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLGameStateList::PostBeginPlay()
{
	GMasterGameStateList = this;
}

void AOLGameStateList::BeginDestroy()
{
	if (GMasterGameStateList == this)
		GMasterGameStateList = NULL;
	Super::BeginDestroy();
}

void AOLGameStateList::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// update all kismet objects that may change status (valid/invalid depending on if they're in the list)
	if (GWorld->GetGameSequence())
	{
		TArray<USequenceObject*> allApplyGSStateEvents;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_ApplyGameState::StaticClass(), allApplyGSStateEvents);
		for (INT Idx = 0; Idx < allApplyGSStateEvents.Num(); Idx++)
		{
			UOLSeqEvent_ApplyGameState* applyGSState = Cast<UOLSeqEvent_ApplyGameState>(allApplyGSStateEvents(Idx));
			applyGSState->UpdateStatus();
		}

		TArray<USequenceObject*> allActivateGS;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqAct_ActivateGameState::StaticClass(), allActivateGS);
		for (INT Idx = 0; Idx < allActivateGS.Num(); Idx++)
		{
			UOLSeqAct_ActivateGameState* activateGS = Cast<UOLSeqAct_ActivateGameState>(allActivateGS(Idx));
			activateGS->UpdateStatus();
		}

		TArray<USequenceObject*> allGSConds;
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqCond_GameState::StaticClass(), allGSConds);
		for (INT Idx = 0; Idx < allGSConds.Num(); Idx++)
		{
			UOLSeqCond_GameState* gsCond = Cast<UOLSeqCond_GameState>(allGSConds(Idx));
			gsCond->UpdateStatus();
		}
	}
}

TArray<FOLGameState>* AOLGameStateList::GetGameStateList()
{
	if (GWorld->HasBegunPlay())
	{
		if (GMasterGameStateList)
		{
			return &GMasterGameStateList->GameStateList;
		}
		else
		{
			// No master list. Maybe a gym, maybe the first call after loading a non-DLC checkpoint

			for (FActorIterator It; It; ++It)
			{
				AOLGameStateList* gsList = Cast<AOLGameStateList>(*It);
				if (gsList)
				{
					debugf(TEXT("New master gamestate list found."));
					GMasterGameStateList = gsList;
					return &gsList->GameStateList;
				}
			}

			return NULL;
		}
	}
	else
	{
		// In editor, iterate over all actors to find the master list
		for (FActorIterator It; It; ++It)
		{
			AOLGameStateList* gsList = Cast<AOLGameStateList>(*It);
			if (gsList)
			{
				return &gsList->GameStateList; 
			}
		}
	}

	return NULL;
}

void AOLGameStateList::SetGameStateList(const TArray<FName>& activatedGS)
{
	if (!GWorld->HasBegunPlay() || !GMasterGameStateList)
	{
		warnf(TEXT("SetGameStateList: no master game state list, skipping"));
		return;
	}

	GMasterGameStateList->ResetAllGameState();

	for (INT i = 0; i < activatedGS.Num(); i++)
	{
		GMasterGameStateList->ActivateGameState(activatedGS(i), FALSE);
	}
}

UBOOL AOLGameStateList::IsGameStateValid(const FName& gsName)
{
	FOLGameState* gs = GetGS(gsName);

	if (gs)
	{
		if (gs->AutoActivateOnCheckpointReached != NAME_None)
		{
			return Utils::IsCheckpointValid(gs->AutoActivateOnCheckpointReached);
		}

		return TRUE;
	}

	return FALSE;
}

UBOOL AOLGameStateList::IsGameStateActivated(const FName& gsName)
{
	FOLGameState* gs = GetGS(gsName);

	if (gs)
	{
		return gs->bActivated;
	}

	return FALSE;
}

FOLGameState* AOLGameStateList::GetGS(const FName& gsName)
{
	TArray<FOLGameState>* gsList = GetGameStateList();

	if (gsList)
	{
		for (INT i = 0; i < gsList->Num(); i++)
		{
			FOLGameState& gs = (*gsList)(i);
			if (gs.Name == gsName)
			{
				return &gs;
			}
		}
	}

	return FALSE;
}

void AOLGameStateList::ActivateGameState(const FName& gsName, UBOOL bTriggerApplyEvents)
{
	FOLGameState* gs = GetGS(gsName);

	if (gs)
	{
		gs->bActivated = TRUE;

		debugf(NAME_DevGameState, TEXT("GS - Activating %s (apply now: %d)"), *gsName.ToString(), bTriggerApplyEvents);
		
		if (bTriggerApplyEvents && GWorld->GetGameSequence())
		{
			TArray<USequenceObject*> allApplyEvents;
			GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_ApplyGameState::StaticClass(), allApplyEvents);

			for (INT Idx = 0; Idx < allApplyEvents.Num(); Idx++)
			{
				UOLSeqEvent_ApplyGameState* applyEvent = Cast<UOLSeqEvent_ApplyGameState>(allApplyEvents(Idx));
				if (applyEvent != NULL)
				{
					if (applyEvent->GameStateName == gsName)
					{
						applyEvent->CheckActivate(GMasterGameStateList, Utils::GetOLPC());
					}
				}
			}
		}
	}
	else
	{
		warnf(TEXT("### Game state %s not found!"), *gsName.ToString());
	}
}

void AOLGameStateList::ResetAllGameState()
{
	// Reset everything (including persisting) - e.g. when loading a savegame
	
	debugf(NAME_DevGameState, TEXT("Resetting GameState"));

	TArray<FOLGameState>* gsList = GetGameStateList();

	if (gsList)
	{
		for (INT i = 0; i < gsList->Num(); i++)
		{
			FOLGameState& gs = (*gsList)(i);

			if (gs.bActivated)
			{
				debugf(NAME_DevGameState, TEXT(" ... [%d] %s"), i, *gs.Name.ToString());
			}

			gs.bActivated = FALSE;
		}
	}
}

void AOLGameStateList::TriggerApplyGameStateEvent(UOLSeqEvent_ApplyGameState* applyGSEvent)
{
	FOLGameState* gs = GetGS(applyGSEvent->GameStateName);

	if (gs)
	{
		TArray<INT> activateIndices;

		if (gs->bActivated)
		{
			activateIndices.AddItem(1); // On
		}
		else
		{
			activateIndices.AddItem(0); // Off
		}

		debugf(NAME_DevGameState, TEXT("GS - Triggering ApplyGameState for %s with %s"), *gs->Name.ToString(), gs->bActivated ? TEXT("ON") : TEXT("OFF"));

		applyGSEvent->CheckActivate(GMasterGameStateList, Utils::GetOLPC(), FALSE, &activateIndices);
	}
}

void AOLGameStateList::ApplyCheckpoint()
{
	// activate all as prescribed by checkpoint (no deactivation)

	debugf(NAME_DevGameState, TEXT("GS - Applying checkpoint %s"), *Utils::GetCurrentCheckpointName().ToString());

	TArray<FOLGameState>* gsList = GetGameStateList();

	if (gsList)
	{
		for (INT i = 0; i < gsList->Num(); i++)
		{
			FOLGameState& gs = (*gsList)(i);

			if (gs.AutoActivateOnCheckpointReached != NAME_None && Utils::IsCheckpointReached(gs.AutoActivateOnCheckpointReached))
			{
				debugf(NAME_DevGameState, TEXT(" ... [%d] Activating %s (>= %s)"), i, *gs.Name.ToString(), *gs.AutoActivateOnCheckpointReached.ToString());
				gs.bActivated = TRUE;
			}
		}

		if (GWorld->GetGameSequence())
		{
			TArray<USequenceObject*> allApplyEvents;
			GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_ApplyGameState::StaticClass(), allApplyEvents);

			for (INT Idx = 0; Idx < allApplyEvents.Num(); Idx++)
			{
				UOLSeqEvent_ApplyGameState* applyEvent = Cast<UOLSeqEvent_ApplyGameState>(allApplyEvents(Idx));
				if (applyEvent)
				{
					TriggerApplyGameStateEvent(applyEvent);					
				}
			}
		}
	}
}

void AOLGameStateList::OnPlayerDeath()
{
	debugf(NAME_DevGameState, TEXT("GS - OnPlayerDeath"));

	// reset all non-persisting, if checkpoint is unreached

	TArray<FOLGameState>* gsList = GetGameStateList();

	if (gsList)
	{
		for (INT i = 0; i < gsList->Num(); i++)
		{
			FOLGameState& gs = (*gsList)(i);

			if (gs.bActivated && !gs.bPersistAfterDeath && (gs.AutoActivateOnCheckpointReached == NAME_None || !Utils::IsCheckpointReached(gs.AutoActivateOnCheckpointReached)))
			{
				debugf(NAME_DevGameState, TEXT(" ... [%d] Deactivating %s"), i, *gs.Name.ToString());
				gs.bActivated = FALSE;
			}
		}
	}
}

void AOLGameStateList::SetEffectiveList(UBOOL bPlayingDLC)
{
	for (FActorIterator It; It; ++It)
	{
		AOLGameStateList* gsList = Cast<AOLGameStateList>(*It);
		if (gsList)
		{
			UBOOL bDLCList = (gsList->GameType == OGT_Whistleblower);

			if (bDLCList == bPlayingDLC)
			{
				GMasterGameStateList = gsList;
				break;
			}
		}
	}
}


void AOLGameStateList::DumpGameState()
{
	Utils::OutputTextToConsole("Game State: ");

	TArray<FOLGameState>* gsList = GetGameStateList();

	if (gsList)
	{
		for (INT i = 0; i < gsList->Num(); i++)
		{
			FOLGameState& gs = (*gsList)(i);
			Utils::OutputTextToConsole(FString::Printf(TEXT(" - [%02d] %s: %s"), i, *(*gsList)(i).Name.ToString(), gs.bActivated ? TEXT("ON") : TEXT("OFF")));
		}
	}
}

void AOLGameStateList::DisplayDebug(UCanvas* canvas)
{
	if (!Utils::GetCheatManager())
	{
		return;
	}

#if !SHIPPING_PC_GAME && !FINAL_RELEASE

	TWEAKABLE FLOAT PosY = 15.0f;
	TWEAKABLE FLOAT YL = 18.0f;
	TWEAKABLE FLOAT col0 = 10.0f;	
	TWEAKABLE FLOAT colWidth = 300.0f;

	FLOAT posY = PosY;
	FLOAT posX = col0;

	canvas->SetDrawColor(0, 100, 185);

	canvas->SetPos(col0, posY);
	canvas->DrawText(FString::Printf(TEXT("Game State (checkpoint: %s)"), *Utils::GetCurrentCheckpointName().ToString()));
	posY += YL;	

	FLOAT maxX = canvas->SizeX - 50.0f;
	FLOAT maxY = canvas->SizeY - 50.0f;

	DrawLine2D(canvas->Canvas, FVector2D(col0-5.0f, posY), FVector2D(maxX, posY), canvas->DrawColor);
	posY += YL;			

	TArray<FOLGameState>* gsList = GetGameStateList();

	if (gsList)
	{
		for (INT i = 0; i < gsList->Num(); i++)
		{
			FOLGameState& gs = (*gsList)(i);
			if (gs.bActivated)
			{
				canvas->SetDrawColor(32, 255, 150);
			}
			else
			{
				canvas->SetDrawColor(200, 200, 200);
			}
						
			canvas->SetPos(posX, posY);

			canvas->DrawText(FString::Printf(TEXT("[%02d] %s"), i, *gs.Name.ToString()));
	
			posY += YL;	

			if (posY >= maxY)
			{
				posY = PosY + YL + YL;
				posX += colWidth;
			}
		}
	}

#endif
}

