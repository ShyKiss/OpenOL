/*=============================================================================
	OLEngine.cpp
=============================================================================*/

#include "OLGame.h"

#if ORBIS
#include "OLOrbis.h"
#endif

#if DINGO
#include "OLDingo.h"
#include "DingoGamepadManager.h"
#endif

//GFx Includes
#if WITH_GFx
#include "ScaleformEngine.h"
#endif //WITH_GFx

// Make sure we run in high-performance on laptops
#ifdef _WINDOWS_
extern "C" 
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

#if WITH_STEAMWORKS
#include "steam/ISteamRemoteStorage.h"
#include "OnlineSubsystemSteamworks.h"
extern ISteamRemoteStorage*			GSteamRemoteStorage;
#endif

extern const INT GWhistleblowerAppId = 273300;

UBOOL IsUsingSteamCloud()
{
#if WITH_STEAMWORKS
	return GSteamRemoteStorage != NULL;
#else
	return FALSE;
#endif
}

IMPLEMENT_CLASS(UOLEngine);

//////////////////////////////////////////////////////////////////////////
// OLEngine
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern UBOOL GSmoothCamera;

void UOLEngine::Init()
{
	Super::Init();

	GSmoothCamera = bSmoothCamera;

#if DINGO
	GCallbackEvent->Register( CALLBACK_SuspendGame, this);
	GCallbackEvent->Register( CALLBACK_ResumeGame, this);
	GCallbackEvent->Register( CALLBACK_EnteringConstrainedMode, this);
	GCallbackEvent->Register( CALLBACK_EnteringFullMode, this);
	GCallbackEvent->Register( CALLBACK_GainedFocus, this);
	GCallbackEvent->Register( CALLBACK_LostFocus, this);
	GCallbackEvent->Register( CALLBACK_VUI_Back, this);
	GCallbackEvent->Register( CALLBACK_VUI_Menu, this);
	GCallbackEvent->Register( CALLBACK_VUI_Play, this);
	GCallbackEvent->Register( CALLBACK_VUI_Pause, this);
#endif
}

void UOLEngine::PreExit()
{
	Super::PreExit();
}

void UOLEngine::InitDLC()
{
	UOLDLCManager* dlcMgr = Cast<UOLDLCManager>(DLCManager);
	if (dlcMgr)
	{
		UBOOL bInstalled = dlcMgr->FindAndInstallDLC();

		if (!bInstalled)
		{
			debugf(TEXT("DLC not installed."));
		}
	}
}

UBOOL UOLEngine::CheckReloadForDLC()
{
	if (GWorld && GWorld->GetTimeSeconds() > NextRefreshDLCTime)
	{
		NextRefreshDLCTime = GWorld->GetTimeSeconds() + 5.0f;

		AOLPlayerController* OLPC = Utils::GetOLPC();

		if (OLPC && OLPC->HUD && OLPC->HUD->eventIsOnMainMenuScreen())
		{		
			UBOOL bDLCJustInstalled = RefreshDLC();

			if (bDLCJustInstalled)
			{
				AOLGame* olgame = Utils::GetOLGame();

				if (olgame)
				{
					olgame->eventTravelToStartupMap();
					return TRUE;
				}
			}		
		}
	}

	return FALSE;
}

UBOOL UOLEngine::RefreshDLC()
{
	UOLDLCManager* dlcMgr = Cast<UOLDLCManager>(DLCManager);
	if (dlcMgr)
	{
		UBOOL bWasInstalled = dlcMgr->IsDLCInstalled();

		if (!bWasInstalled)
		{
			UBOOL bIsInstalled = dlcMgr->FindAndInstallDLC();

			if (bIsInstalled)
			{
				debugf(TEXT("## DLC has been just installed, reloading main menu"));
				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL UOLEngine::ShouldShowNewDLCGame()
{
	UOLDLCManager* dlcMgr = Cast<UOLDLCManager>(DLCManager);
	if (!dlcMgr)
	{
		return FALSE;
	}

	if (dlcMgr->IsDLCInstalled())
	{
		return TRUE;
	}

#if ORBIS
	if (!dlcMgr->IsOrbisDLCOwned()) // show the option if the content is NOT owned - if owned but not installed we don't have anything meaningful to show
	{
		return GOLOrbis->IsDLCAllowed();
	}
#elif DINGO
	return TRUE;
#elif WITH_STEAMWORKS
		if (GSteamworksClientInitialized && GSteamUtils && GSteamApps)
		{
			UBOOL bOwned = GSteamApps->BIsSubscribedApp(GWhistleblowerAppId);
			return (!bOwned && GSteamUtils->IsOverlayEnabled()); // we don't own it but can show the store overlay
		}	
#endif

	return FALSE;
}

UBOOL UOLEngine::TryStartDLCGame()
{
	UOLDLCManager* dlcMgr = Cast<UOLDLCManager>(DLCManager);
	if (!dlcMgr)
	{
		return FALSE;
	}

	UBOOL	bReallyInstalled = dlcMgr->IsDLCInstalled();
	
#if ORBIS
	
	if (bReallyInstalled)
	{
		return TRUE;
	}
	else if (!dlcMgr->IsOrbisDLCOwned())
	{
		// not owned, show the store page
		GOLOrbis->ShowDLCStorePage();
		ActiveOrbisDialog = ODT_PSStore;
		return FALSE;
	}

#elif DINGO

	if (bReallyInstalled)
	{
		return TRUE;
	}
	else
	{
		GOLDingo->ShowDLCStorePage();
		return FALSE;
	}

#elif WITH_STEAMWORKS
		
	if (GUseSeekFreeLoading && GSteamworksClientInitialized && GSteamUtils && GSteamApps && GSteamFriends)
	{
		UBOOL bOwned = GSteamApps->BIsSubscribedApp(GWhistleblowerAppId);
		UBOOL bSteamSaysInstalled = bOwned && GSteamApps->BIsDlcInstalled(GWhistleblowerAppId);
				
		debugf(TEXT("## DLC owned: %d, Steam installed: %d, really installed: %d"), bOwned, bSteamSaysInstalled, bReallyInstalled);

		if (bOwned && bReallyInstalled)
		{
			return TRUE;
		}
		else if (!bOwned && GSteamUtils->IsOverlayEnabled())
		{
			GSteamFriends->ActivateGameOverlayToStore(GWhistleblowerAppId, k_EOverlayToStoreFlag_None);
			debugf(TEXT("## Activating store overlay"));
		}

		return FALSE;
	}
#endif
	
	return bReallyInstalled;
}

void UOLEngine::SetDefaultURL(FURL& inout_DefaultURL) 
{
	AOLGame* defaultGame = Cast<AOLGame>(AOLGame::StaticClass()->GetDefaultObject());
	UOLDLCManager* dlcMgr = Cast<UOLDLCManager>(DLCManager);

	FName mapName = defaultGame->DefaultMapName;

	if (defaultGame->bIsDemo)
	{
		mapName = defaultGame->DemoMapName;
	}
	else if (dlcMgr && dlcMgr->IsDLCInstalled())
	{
		mapName = defaultGame->DLCInstalledMapName;
	}

	inout_DefaultURL.DefaultLocalMap = mapName.ToString();
	inout_DefaultURL.Map = inout_DefaultURL.DefaultLocalMap;
}

void UOLEngine::PlayStartupMovie()
{
	GFullScreenMovie->GameThreadInitiateStartupSequence();

	if ( !ParseParam(appCmdLine(), TEXT("nostartupmovies")) )
	{
		UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();

		if (StartupMovieSound && AudioDevice)
		{
			AudioDevice->PostEvent(StartupMovieSound, NULL, NAME_None);
			AudioDevice->Update(FALSE); // force an update to process the event
		}
	}
}

static FSystemSettings GPendingSystemSettings;

void UOLEngine::Tick(FLOAT DeltaSeconds)
{
	{
		// check for a pending checkpoint action
		if (PendingCheckpointAction == Checkpoint_SaveToMemory || PendingCheckpointAction == Checkpoint_SaveToDisk)
		{
			SaveCheckpointData(PendingCheckpointName, PendingCheckpointAction == Checkpoint_SaveToDisk);
		}
		else if (PendingCheckpointAction == Checkpoint_Load)
		{
			UBOOL bOk = LoadCheckpointData();

			if (!bOk)
			{
				eventRestartPlayer(Utils::GetOLPC());
			}
		}
		PendingCheckpointAction = Checkpoint_Default;
	}

	if (bPendingGraphicalSettingsChange)
	{
		GSystemSettings.ApplyNewSettings(GPendingSystemSettings, TRUE);
		bPendingGraphicalSettingsChange = FALSE;
	}

	if (PendingNewLanguage >= 0)
	{
		UBOOL wasJPN = (appStricmp(TEXT("JPN"), GetLanguage()) == 0);
		static UBOOL hasBeenJPN = FALSE;
		hasBeenJPN = wasJPN || hasBeenJPN;

		FString newLangStr = LanguageIdxToString((ELanguage)PendingNewLanguage);
		GConfig->SetString(TEXT("Engine.Engine"), TEXT("Language"), *newLangStr, GEngineIni);
		GConfig->Flush( FALSE, GEngineIni );

		if (!hasBeenJPN) // Switching from Japanese needs to exit and start the game again (can't get the fontlibs to work)
		{
			SetLanguage(*newLangStr, TRUE);

	#if WITH_GFx
			GGFxEngine->InitLocalization();
			GGFxEngine->InitFontlib();
	#endif

			PendingNewLanguage = -1;

			if (GamePlayers.Num() > 0 && GamePlayers(0))
			{
				GamePlayers(0)->Exec(TEXT("reloadmenu"), *GLog);
			}
		}
	}

#if ORBIS
	GOLOrbis->Tick(DeltaSeconds);
	ProcessOrbisDialogs(DeltaSeconds);
#endif

#if DINGO
	ProcessDingoCallbacks();

	GOLDingo->Tick(DeltaSeconds);

	if (GOLDingo->UserInitState == OLDingo::UIS_AsyncTasksDone)
	{
		AOLPlayerController* OLPC = Utils::GetOLPC();
		GOLDingo->UserInitState = OLDingo::UIS_Initialized;
		GOLDingo->ReadProfileSettings(OLPC->ProfileSettings);
		OLPC->eventUpdateLocalCacheOfProfileSettings(OLPC->ProfileSettings);
		eventOnDingoUserInitialized(GOLDingo->UserInitResult);		
	}

#endif

	CheckReloadForDLC();

	Super::Tick(DeltaSeconds);
}

void UOLEngine::StartCurrentCheckpoint()
{
	// request a pending load, to be executed at beginning of next frame
	check(CurrentCheckpointSave);
	PendingCheckpointAction = Checkpoint_Load;
	bRestartingActiveCheckpoint = TRUE;
}

UBOOL UOLEngine::LoadSaveFile(const FString& Filename) // Windows/Mac
{
#if !CONSOLE
	if (!GWorld->HasBegunPlay() || !AOLCheckpointList::GetCheckpointList())
	{
		warnf(TEXT("### Can't load save file: No checkpoint list"));
		return FALSE;
	}

	if (Filename.Len() > 0 && (!CurrentCheckpointSave || Filename != CurrentCheckpointSave->Filename))
	{
		TArray<BYTE> saveData;
		ReadSaveData(Filename, saveData);
		SetCurrentSaveData(Filename, saveData);
	}

	if (CurrentCheckpointSave)
	{
		PendingCheckpointAction = Checkpoint_Load;
		bRestartingActiveCheckpoint = FALSE;
		return TRUE;
	}

#endif
	return FALSE;
}


void UOLEngine::SaveCheckpointImmediate(FName CheckpointName)
{
	CreateNewSaveFile();
	SaveCheckpointData(CheckpointName, TRUE);
}

void UOLEngine::SaveCheckpoint(FName CheckpointName, UBOOL bSaveToDisk)
{
	AOLGame* olGame = Utils::GetOLGame();

	if (Utils::IsDemo() || (olGame && olGame->DifficultyMode == EDM_Insane))
	{
		bSaveToDisk = FALSE; // denied
	}

#if DINGO
	GOLDingo->EventPlayerReachedCheckpoint(olGame->DifficultyMode, CheckpointName); 
#endif

	CreateNewSaveFile();
	PendingCheckpointAction = bSaveToDisk ? Checkpoint_SaveToDisk : Checkpoint_SaveToMemory;
	PendingCheckpointName = CheckpointName;
}


void UOLEngine::SaveAllCheckpoints()
{
#if !ORBIS
	
	INT cpCount = 0;

	TArray<AOLCheckpointList*> cpLists;
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpointList* cpListActor = Cast<AOLCheckpointList>(*It);
		if (cpListActor)
		{
			const TArray<FName>& cpList = cpListActor->CheckpointList;
			for (INT i = 0; i < cpList.Num(); i++)
			{
				const FName& cpName = cpList(i);
				CurrentCheckpointSave = ConstructObject<UOLCheckpointSave>(UOLCheckpointSave::StaticClass(),this,NAME_None);
				CurrentCheckpointSave->Filename = FString::Printf(TEXT("OL_CP_%s"), *cpName.ToString());

				// and save the data
				CurrentCheckpointSave->CheckpointName = cpName.ToString();
				TArray<BYTE> SaveGameData;
				CurrentCheckpointSave->SaveData(SaveGameData, ++cpCount);

				WriteSaveData(CurrentCheckpointSave->Filename, SaveGameData);
			}
		}
	}

#endif
}

void UOLEngine::DebugSaveGame(const FString& Filename)
{
	CurrentCheckpointSave = ConstructObject<UOLCheckpointSave>(UOLCheckpointSave::StaticClass(),this,NAME_None);
	CurrentCheckpointSave->Filename = Filename;

	check(CurrentCheckpointSave != NULL);

	Utils::GetOLGame()->CurrentCheckpointName = FName(TEXT("TestCheckpoint"));
	CurrentCheckpointSave->CheckpointName = FString(TEXT("TestCheckpoint"));
	TArray<BYTE> SaveGameData;
	CurrentCheckpointSave->SaveData(SaveGameData);

	WriteSaveData(CurrentCheckpointSave->Filename, SaveGameData);
}

void UOLEngine::DebugLoadGame(const FString& Filename)
{
	CurrentCheckpointSave = ConstructObject<UOLCheckpointSave>(UOLCheckpointSave::StaticClass(),this,NAME_None);
	CurrentCheckpointSave->Filename = Filename;
	LoadSaveFile(Filename);
}

//////////////////////////////////////////////////////////////////////////
// ORBIS
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#if ORBIS
void UOLEngine::ProcessOrbisDialogs(FLOAT deltaSeconds)
{
	OrbisDialogResult dialogResult = GOLOrbis->TickSystemDialogs(deltaSeconds);

	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (dialogResult != ODR_Inactive)
	{
		if (ActiveOrbisDialog == ODT_None)
		{
			// a dialog was triggered behind our back
			if (OLPC && !bPausedForSystemDialog)
			{
				OLPC->eventForcePause(TRUE);
				bPausedForSystemDialog = TRUE;
			}
			ActiveOrbisDialog = ODT_SystemError;
		}
		else if (dialogResult != ODR_InProgress)
		{
			// a dialog is done

			UBOOL bSuccess = (dialogResult == ODR_Success);

			if (ActiveOrbisDialog == ODT_SelectSaveLocation)
			{
				eventFinishedSaveDataDialog(bSuccess);
				ActiveOrbisDialog = ODT_None;

				if (bPausedForSystemDialog)
				{
					OLPC->eventForcePause(FALSE);
					bPausedForSystemDialog = FALSE;
				}
			}
			else if (ActiveOrbisDialog == ODT_LoadGame)
			{
				if (bSuccess)
				{
					check(SaveDataBuffer.Num() > 0);

					if (!Utils::IsDLCInstalled())
					{
						// check whether we're about to load a DLC save
						FString cpNameStr;
						FMemoryReader Ar(SaveDataBuffer);
						UBOOL bDeprecated = FALSE;
						FCheckpointTime dummyTime;
						INT dummyDifficulty = 0;
						UOLCheckpointSave::SerializeHeader(Ar, cpNameStr, dummyTime, bDeprecated, dummyDifficulty);

						FName cpName(*cpNameStr);
						UBOOL bCheckpointExists = AOLCheckpointList::GetListForCheckpoint(cpName) != NULL;

						if (!bCheckpointExists)
						{
							warnf(TEXT("Invalid checkpoint in save data. Trying to load a DLC checkpoint but DLC isn't installed?"));
							bSuccess = FALSE;

							GOLOrbis->ShowBadCheckpointMessage();
							ActiveOrbisDialog = ODT_LoadedBadCheckpoint;
						}
					}

					if (bSuccess)
					{
						SetCurrentSaveData(FString(TEXT("Outlast.sav")), SaveDataBuffer);
						SaveDataBuffer.Empty();

						PendingCheckpointAction = Checkpoint_Load;
						bRestartingActiveCheckpoint = FALSE;	

						if (OLPC)
						{
							OLPC->StopAllSounds();
						}

						eventFinishedSaveDataDialog(TRUE);
						ActiveOrbisDialog = ODT_None;
					}
				}
				else
				{
					eventFinishedSaveDataDialog(FALSE);
					ActiveOrbisDialog = ODT_None;
				}
			}
			else if (ActiveOrbisDialog == ODT_LoadedBadCheckpoint)
			{
				ActiveOrbisDialog = ODT_None;
				NativeSelectAndLoadGame();
			}
			else if (ActiveOrbisDialog == ODT_PSStore)
			{
				ActiveOrbisDialog = ODT_None;
			}
			else
			{
				check(ActiveOrbisDialog == ODT_SystemError);

				if (dialogResult == ODR_WriteSaveFailure)
				{
					// Saving failed while playing - prompt the user to choose a different save
					GOLOrbis->SelectAndSaveGame(&SaveDataBuffer);
					ActiveOrbisDialog = ODT_SelectSaveLocation;
				}
				else
				{
					if (OLPC && bPausedForSystemDialog)
					{
						OLPC->eventForcePause(FALSE);
						bPausedForSystemDialog = FALSE;
					}
					ActiveOrbisDialog = ODT_None;
				}
			}			
		}
	}
}
#endif

UBOOL UOLEngine::PS4_LoadCurrentSaveFile()
{
#if ORBIS 
	if (!GWorld->HasBegunPlay() || !AOLCheckpointList::GetCheckpointList())
	{
		warnf(TEXT("### Can't load save file: No checkpoint list"));
		return FALSE;
	}

	TArray<BYTE> saveData;
	UBOOL bSuccess = GOLOrbis->ReadSaveGame(saveData);

	if (bSuccess)
	{
		if (!Utils::IsDLCInstalled())
		{
			// check whether we're about to load a DLC save
			FString cpNameStr;
			FMemoryReader Ar(saveData);
			UBOOL bDeprecated = FALSE;
			FCheckpointTime dummyTime;
			INT dummyDifficulty = 0;
			UOLCheckpointSave::SerializeHeader(Ar, cpNameStr, dummyTime, bDeprecated, dummyDifficulty);

			FName cpName(*cpNameStr);
			UBOOL bCheckpointExists = AOLCheckpointList::GetListForCheckpoint(cpName) != NULL;

			if (!bCheckpointExists)
			{
				warnf(TEXT("Invalid checkpoint in save data. Trying to load a DLC checkpoint but DLC isn't installed?"));
				bSuccess = FALSE;

				GOLOrbis->ShowBadCheckpointMessage();
				ActiveOrbisDialog = ODT_LoadedBadCheckpoint;
			}
		}

		if (bSuccess)
		{
			SetCurrentSaveData(FString(TEXT("Outlast.sav")), saveData);
	
			PendingCheckpointAction = Checkpoint_Load;
			bRestartingActiveCheckpoint = FALSE;
			return TRUE;
		}
	}
#endif

	return FALSE;
}

UBOOL UOLEngine::PS4_LoadMostRecentSaveFile()
{
#if ORBIS
	GOLOrbis->UseMostRecentSaveFile();
	return PS4_LoadCurrentSaveFile();
#endif
	return FALSE;
}

void UOLEngine::NativeSelectSaveLocation(const FString& startCP)
{
#if ORBIS

	check(ActiveOrbisDialog == ODT_None);
	  
	if (CurrentCheckpointSave == NULL)
	{
		CreateNewSaveFile();
	}
	check(CurrentCheckpointSave);

	SaveDataBuffer.Empty();
	CurrentCheckpointSave->CheckpointName = startCP;
	Utils::GetOLGame()->CurrentCheckpointName = FName(*CurrentCheckpointSave->CheckpointName);
	
	CurrentCheckpointSave->SaveData(SaveDataBuffer);
	CurrentCheckpointSave->SaveMetaData(GOLOrbis->GetMetaDataBuffer());
	
	GOLOrbis->SelectAndSaveGame(&SaveDataBuffer);

	ActiveOrbisDialog = ODT_SelectSaveLocation;

#endif
}

void UOLEngine::NativeSelectAndLoadGame()
{
#if ORBIS

	if (!GWorld->HasBegunPlay() || !AOLCheckpointList::GetCheckpointList())
	{
		warnf(TEXT("### Can't load save file: No checkpoint list"));
		return ;
	}

	check(ActiveOrbisDialog == ODT_None);

	SaveDataBuffer.Empty();	
	GOLOrbis->SelectAndLoadGame(&SaveDataBuffer);

	ActiveOrbisDialog = ODT_LoadGame;

#endif
}

//////////////////////////////////////////////////////////////////////////
// DINGO
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UBOOL UOLEngine::ShouldOpenPressStartScreen()
{
#if DINGO
	return GOLDingo->ShouldOpenPressStartScreen();	
#endif
	return FALSE;
}

void UOLEngine::Dingo_AllowAllControllersInput()
{
#if DINGO
	GOLDingo->AllowAllControllersInput();
#endif
}

void UOLEngine::DingoNative_StartInitializeUser()
{
#if DINGO
	GOLDingo->StartInitializeUser();
#endif
}

BYTE UOLEngine::Dingo_LoadMostRecentSaveFile()
{
#if DINGO
	RefreshSaveFiles();

	if (SaveFiles.Num() > 0)
	{
		return Dingo_LoadSaveGame(SaveFiles(0).Filename);
	}
#endif
	return LGR_LoadError;
}

BYTE UOLEngine::Dingo_LoadSaveGame(const FString& SaveFilename)
{
#if DINGO
	if (!GWorld->HasBegunPlay() || !AOLCheckpointList::GetCheckpointList())
	{
		warnf(TEXT("### Can't load save file: No checkpoint list"));
		return LGR_LoadError;
	}

	TArray<BYTE> saveData;
	UBOOL bSuccess = GOLDingo->GetSaveGameData(SaveFilename, saveData);

	if (!Utils::IsDLCInstalled())
	{
		// check whether we're about to load a DLC save
		FString cpNameStr;
		FMemoryReader Ar(saveData);
		UBOOL bDeprecated = FALSE;
		FCheckpointTime dummyTime;
		INT dummyDifficulty = 0;
		UOLCheckpointSave::SerializeHeader(Ar, cpNameStr, dummyTime, bDeprecated, dummyDifficulty);

		FName cpName(*cpNameStr);
		UBOOL bCheckpointExists = AOLCheckpointList::GetListForCheckpoint(cpName) != NULL;

		if (!bCheckpointExists)
		{
			warnf(TEXT("Invalid checkpoint in save data. Trying to load a DLC checkpoint but DLC isn't installed?"));
			return LGR_BadCheckpoint;
		}
	}

	if (bSuccess)
	{
		SetCurrentSaveData(FString(TEXT("Outlast.sav")), saveData);

		GOLDingo->SetSavingFilename(SaveFilename);

		PendingCheckpointAction = Checkpoint_Load;
		bRestartingActiveCheckpoint = FALSE;
		return LGR_Success;
	}
	
#endif
	return LGR_LoadError;
}

UBOOL UOLEngine::Dingo_StartNewGameWithSave(const FString& StartCP, const FString& SaveFilename)
{
#if DINGO
	if (CurrentCheckpointSave == NULL)
	{
		CreateNewSaveFile();
	}
	check(CurrentCheckpointSave);

	SaveDataBuffer.Empty();
	CurrentCheckpointSave->CheckpointName = StartCP;
	Utils::GetOLGame()->CurrentCheckpointName = FName(*CurrentCheckpointSave->CheckpointName);

			// and save the data
	CurrentCheckpointSave->SaveData(SaveDataBuffer);

	GOLDingo->SetSavingFilename(SaveFilename);
	GOLDingo->WriteSaveGameAsync(SaveDataBuffer);

	Utils::GetOLPC()->eventStartNewGameAtCheckpoint(StartCP, TRUE);

	return TRUE;
#endif
	return FALSE;
}

UBOOL UOLEngine::Dingo_StartNewGameWithNewSave(const FString& StartCP)
{
#if DINGO
	FString SaveFilename = GOLDingo->GetNewSaveName();
	return Dingo_StartNewGameWithSave(StartCP, SaveFilename);
#endif
	return FALSE;
}

UBOOL UOLEngine::Dingo_SaveGameImmediate(const FString& SaveFilename)
{
#if DINGO
	if (CurrentCheckpointSave == NULL)
	{
		CreateNewSaveFile();
	}
	check(CurrentCheckpointSave);

	SaveDataBuffer.Empty();
	CurrentCheckpointSave->CheckpointName = Utils::GetCurrentCheckpointName().ToString();
	Utils::GetOLGame()->CurrentCheckpointName = FName(*CurrentCheckpointSave->CheckpointName);

	// and save the data
	CurrentCheckpointSave->SaveData(SaveDataBuffer);

	GOLDingo->SetSavingFilename(SaveFilename);
	GOLDingo->WriteSaveGameAsync(SaveDataBuffer);

	return TRUE;
#endif
	return FALSE;
}

UBOOL UOLEngine::Dingo_SaveGameImmediateToNewSave()
{
#if DINGO
	FString SaveFilename = GOLDingo->GetNewSaveName();
	return Dingo_SaveGameImmediate(SaveFilename);
#endif
	return FALSE;
}

FString UOLEngine::Dingo_GetCheckpointTag(FName checkpointName)
{
	// hacked, we can't use actors since they may not be loaded for a DLC vs. base scenario

	FString checkpointStr = checkpointName.ToString();
	INT underscoreIdx = checkpointStr.InStr(TEXT("_"));

	if (underscoreIdx >= 0)
	{
		FString prefix = checkpointStr.Left(underscoreIdx);
		FString lowerPrefix = prefix.ToLower();

		if (lowerPrefix == TEXT("revisit"))
		{
			return TEXT("AdminRevisit");
		}
		else if (checkpointStr.ToLower() == TEXT("lab_afterexperiment"))
		{
			return TEXT("DLC_Lab");
		}
		else if (lowerPrefix == TEXT("dlc"))
		{
			return TEXT("DLC_Lab");
		}
		else if (lowerPrefix == TEXT("hospital"))
		{
			return TEXT("DLC_Hospital");
		}
		else if (lowerPrefix == TEXT("courtyard1"))
		{
			return TEXT("DLC_Courtyard1");
		}
		else if (lowerPrefix == TEXT("prisonrevisit"))
		{
			return TEXT("DLC_Prison");
		}
		else if (lowerPrefix == TEXT("courtyard2"))
		{
			return TEXT("DLC_Courtyard2");
		}
		else if (lowerPrefix == TEXT("building2"))
		{
			return TEXT("DLC_Building2");
		}
		else if (lowerPrefix == TEXT("malerevisit"))
		{
			return TEXT("DLC_Exit");
		}
		else if (lowerPrefix == TEXT("adminblock"))
		{
			return TEXT("DLC_Exit");
		}
		else
		{
			return prefix;
		}
	}
	else if (checkpointStr.ToLower() == TEXT("startgame"))
	{
		return TEXT("Admin");
	}

	debugf(TEXT("## Can't determine tag for checkpoint %s"), *checkpointStr);

	return TEXT("");
}

FString UOLEngine::Dingo_GetSaveFileIconName(FName checkpointName)
{
	// hacked, we can't use actors since they may not be loaded for a DLC vs. base scenario

	FString checkpointStr = checkpointName.ToString().ToLower();
	INT underscoreIdx = checkpointStr.InStr(TEXT("_"));

	if (underscoreIdx >= 0)
	{
		FString prefix = checkpointStr.Left(underscoreIdx);

		if (prefix == TEXT("admin"))
		{
			return TEXT("si01");
		}
		else if (prefix == TEXT("prison"))
		{
			return TEXT("si02");
		}
		else if (prefix == TEXT("sewer"))
		{
			return TEXT("si03");
		}
		else if (prefix == TEXT("male"))
		{
			return TEXT("si04");
		}
		else if (prefix == TEXT("courtyard"))
		{
			return TEXT("si05");
		}
		else if (prefix == TEXT("female"))
		{
			return TEXT("si06");
		}
		else if (prefix == TEXT("revisit"))
		{
			return TEXT("si07");
		}
		else if (prefix == TEXT("lab"))
		{
			if (checkpointStr == TEXT("lab_afterexperiment"))
			{
				return TEXT("si09");
			}
			else
			{
				return TEXT("si08");
			}
		}
		else if (prefix == TEXT("dlc"))
		{
			return TEXT("si09");
		}
		else if (prefix == TEXT("hospital"))
		{
			return TEXT("si10");
		}
		else if (prefix == TEXT("courtyard1"))
		{
			return TEXT("si11");
		}
		else if (prefix == TEXT("prisonrevisit"))
		{
			return TEXT("si13");
		}
		else if (prefix == TEXT("courtyard2"))
		{
			return TEXT("si12");
		}
		else if (prefix == TEXT("building2"))
		{
			return TEXT("si14");
		}
		else if (prefix == TEXT("malerevisit"))
		{
			return TEXT("si15");
		}
		else if (prefix == TEXT("adminblock"))
		{
			return TEXT("si15");
		}
	}
	else if (checkpointStr == TEXT("startgame"))
	{
		return TEXT("si01");
	}

	return TEXT("siNone");
}

UBOOL UOLEngine::Dingo_DeleteSaveGame(const FString& SaveFilename)
{
#if DINGO
	GOLDingo->DeleteSaveGameAsync(SaveFilename);
	return TRUE;
#endif
	return FALSE;
}

INT UOLEngine::Dingo_OnInitialPressStart(INT ControllerId)
{
#if DINGO
	INT realControllerId = GOLDingo->OnInitialPressStart(ControllerId);
	ULocalPlayer* Player = GEngine->GamePlayers(0);
	if (Player)
	{
		Player->ControllerId = realControllerId;
	}
	return realControllerId;
#else
	return ControllerId;
#endif
}

UBOOL UOLEngine::Dingo_ShouldShowLoginUI(INT ControllerId)
{
#if DINGO
	return GOLDingo->ShouldShowLoginUI(ControllerId);
#endif
	return FALSE;
}

UBOOL UOLEngine::Dingo_ShowHelpUI()
{
#if DINGO
	return GOLDingo->ShowHelpUI();
#endif
	return FALSE;
}

UBOOL UOLEngine::DingoNative_ShowLoginUIAndInitializeUser()
{
#if DINGO
	return GOLDingo->ShowLoginUI();
#endif
	return FALSE;
}

FString UOLEngine::Dingo_GetActiveGamertag()
{
#if DINGO

	FString playerName = GOLDingo->GetActiveGamertag();
		
	if (playerName.Len() > 16)
	{
		playerName = playerName.Left(16) + TEXT("...");
	}

	return playerName;

#endif
	return TEXT("");
}

void UOLEngine::ReturnToPressStartScreen()
{
#if DINGO
	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (!OLPC)
	{
		return;
	}

	DingoGamepadManager::GetInstance().UpdateCachedPairings();
	GDingoOSS->ForceUserStateRefresh();

	if (Utils::IsTravelling())
	{
		debugf(TEXT("## ReturnToPressStartScreen - ignored because we're travelling"));
	}
	else if (Utils::IsInMainMenu())
	{
		GOLDingo->ResetUser();

		if (OLPC->HUD)
		{
			OLPC->HUD->eventReturnToPressStartMenu();
		}
	}
	else
	{
		OLPC->SaveBeforeQuitting();

		GOLDingo->ResetUser();

		AOLGame* olGame = Utils::GetOLGame();
		if (olGame)
		{
			olGame->eventTravelToStartupMap();
		}
	}
#endif
}


//////////////////////////////////////////////////////////////////////////
// Steam
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UOLEngine::DeleteAllLocalSaves()
{
#if !CONSOLE
	TArray<FString> Filenames;
	GFileManager->FindFiles(Filenames, *FString::Printf(TEXT("%sSaveData\\OL_*.sav"), *appGameDir()), TRUE, FALSE);
	for (INT i = 0; i < Filenames.Num(); i++)
	{
		// FindFiles returns bare filenames (e.g. "OL_slot0.sav"); strip extension before passing to DeleteSaveFile.
		FString Name = Filenames(i);
		if (Name.Right(4) == TEXT(".sav"))
			Name = Name.Left(Name.Len() - 4);
		DeleteSaveFile(Name);
	}
#endif
}

INT UOLEngine::DeleteAllCloudSaves()
{
#if WITH_STEAMWORKS
	if (!GSteamRemoteStorage)
	{
		return -1;
	}

	TArray<FString> Filenames;
	FindSteamCloudSaveFiles(Filenames);

	for (INT i = 0; i < Filenames.Num(); i++)
	{
		DeleteSteamCloudFile(TCHAR_TO_UTF8(*Filenames(i)));
	}

	return Filenames.Num();
#else
	return 0;
#endif
}

UBOOL UOLEngine::WriteToSteamCloud(const char* Filename, const TArray<BYTE>& SaveData)
{
#if WITH_STEAMWORKS
	if (!GSteamRemoteStorage)
	{
		return FALSE;
	}

	TWEAKABLE INT MaxTotalFileCount = 500;
	INT fileCount = GSteamRemoteStorage->GetFileCount();	

	INT saveCount = fileCount - 1; // remove one for the profile, the rest are saves
	INT maxSaveCount = MaxTotalFileCount - 2; // profile, and the one we're about to write
	if (saveCount > maxSaveCount)
	{
		// Clear the oldest save game, until we get to max 500 files.
		TArray<FString> Filenames;
		FindSteamCloudSaveFiles(Filenames);

		while (saveCount > maxSaveCount) // note: looping for safety's sake, but technically this should never loop (as we enforce the max file cap)
		{
			SQWORD oldestTime = -1;
			INT oldestFilenameIdx = -1;

			for (INT i = 0; i < Filenames.Num(); i++) 
			{
				SQWORD timestamp = GSteamRemoteStorage->GetFileTimestamp(TCHAR_TO_UTF8(*Filenames(i)));

				if (oldestFilenameIdx < 0 || timestamp < oldestTime)
				{
					oldestFilenameIdx = i;
					oldestTime = timestamp;
				}
			}

			if (oldestFilenameIdx >= 0)
			{
				DeleteSteamCloudFile(TCHAR_TO_UTF8(*Filenames(oldestFilenameIdx)));
				Filenames.Remove(oldestFilenameIdx);
			}

			saveCount--;
		}
	}

	bool bSuccess = GSteamRemoteStorage->FileWrite(Filename, SaveData.GetData(), SaveData.Num());

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if (!bSuccess)
	{
		warnf(TEXT("WriteToSteamCloud failed for %s"), UTF8_TO_TCHAR(Filename));
	}
#endif

	return bSuccess;
#else
	return FALSE;
#endif
}

UBOOL UOLEngine::ReadFromSteamCloud(const char* Filename, TArray<BYTE>& out_SaveData)
{
#if WITH_STEAMWORKS
	if (!GSteamRemoteStorage)
	{
		return FALSE;
	}

	INT fileSize = GSteamRemoteStorage->GetFileSize(Filename);

	if (fileSize <= 0)
	{
#if !SHIPPING_PC_GAME
		warnf(TEXT("ReadFromSteamCloud failed for %s"), UTF8_TO_TCHAR(Filename));
		return FALSE;
#endif
	}

	out_SaveData.Empty();
	out_SaveData.AddZeroed(fileSize);

	INT dataRead = GSteamRemoteStorage->FileRead(Filename, out_SaveData.GetData(), fileSize);

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if (dataRead != fileSize)
	{
		warnf(TEXT("ReadFromSteamCloud FAILED for %s - dataRead: %d, fileSize: %d "), Filename, dataRead, fileSize);
	}
#endif

	return dataRead == fileSize;

#else
	return FALSE;
#endif
}

UBOOL UOLEngine::DeleteSteamCloudFile(const char* Filename)
{
#if WITH_STEAMWORKS
	if (!GSteamRemoteStorage)
	{
		return FALSE;
	}

	GSteamRemoteStorage->FileDelete(Filename);

	return TRUE;

#else
	return FALSE;
#endif
}

UBOOL UOLEngine::FindSteamCloudSaveFiles(TArray<FString>& FileNames)
{
#if WITH_STEAMWORKS
	if (!GSteamRemoteStorage)
	{
		return FALSE;
	}

	INT fileCount = GSteamRemoteStorage->GetFileCount();
	FileNames.Empty();
	
	for (INT i = 0; i < fileCount; i++)
	{
		INT dummySize = 0;
		FString filename = FString(UTF8_TO_TCHAR(GSteamRemoteStorage->GetFileNameAndSize(i, &dummySize)));

		if (filename.EndsWith(FString(TEXT(".sav"))))
		{
			FileNames.AddItem(filename);
		}
	}

	return TRUE;

#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////
// Internal
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UOLEngine::SaveCheckpointData(const FName& CheckpointName, UBOOL bSaveToDisk)
{
	// create a new one if necessary
	if (CurrentCheckpointSave == NULL)
	{
		CreateNewSaveFile();
	}
	check(CurrentCheckpointSave != NULL);
	// and save the data
	CurrentCheckpointSave->CheckpointName = CheckpointName.ToString();
	SaveDataBuffer.Empty();
	CurrentCheckpointSave->SaveData(SaveDataBuffer);
	
	if (bSaveToDisk)
	{
#if ORBIS
		CurrentCheckpointSave->SaveMetaData(GOLOrbis->GetMetaDataBuffer());
		GOLOrbis->WriteSaveGameAsync(SaveDataBuffer, GOLOrbis->GetMetaDataBuffer());
#elif DINGO
		GOLDingo->WriteSaveGameAsync(SaveDataBuffer);
#else
		WriteSaveData(CurrentCheckpointSave->Filename, SaveDataBuffer);
#endif
	}
}

UBOOL UOLEngine::LoadCheckpointData()
{
	// reset level load override as it might have been set by LoadMap()
	GWorld->AllowLevelLoadOverride = 0;

	if (CurrentCheckpointSave != NULL)
	{
		return CurrentCheckpointSave->LoadData();
	}

	return FALSE;
}

void UOLEngine::CreateNewSaveFile()
{
	CurrentCheckpointSave = ConstructObject<UOLCheckpointSave>(UOLCheckpointSave::StaticClass(),this,NAME_None);

#if CONSOLE
	CurrentCheckpointSave->Filename = FString(TEXT("Outlast.sav"));
#else
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
	appSystemTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);

	CurrentCheckpointSave->Filename = FString::Printf(TEXT("OL_%i-%i-%i_%i-%i-%i"), Year, Month, Day, Hour, Min, Sec);
#endif
}

void UOLEngine::SetCurrentSaveData(const FString& Filename, const TArray<BYTE>& CheckpointData)
{
	check(Filename.Len() > 0);
	CurrentCheckpointSave = ConstructObject<UOLCheckpointSave>(UOLCheckpointSave::StaticClass(),this,NAME_None);
	CurrentCheckpointSave->Filename = Filename;
	
	// If this is empty, then the checkpoint is a new one so don't validate
	if (CheckpointData.Num())
	{
		FMemoryReader Ar((TArray<BYTE>&)CheckpointData);
		// Now serialize the data into the checkpoint object
		CurrentCheckpointSave->SerializeFinalData(Ar);
		if (Ar.IsError())
		{
			debugf(NAME_Error,TEXT("Failed to read checkpoint file"));
			CurrentCheckpointSave = NULL;
		}
	}
}

UBOOL UOLEngine::ReadSaveData(const FString& Filename, TArray<BYTE>& out_SaveData)
{
#if CONSOLE
	return FALSE;
#else
	if (IsUsingSteamCloud())
	{
		FString fullPath = FString::Printf(TEXT("%s.sav"), *Filename);
		return ReadFromSteamCloud(TCHAR_TO_UTF8(*fullPath), out_SaveData);
	}
	else
	{
		UBOOL bResult = FALSE;

		FArchive* Ar = GFileManager->CreateFileReader(*FString::Printf(TEXT("%sSaveData\\%s.sav"), *appGameDir(), *Filename));
		if (Ar != NULL)
		{
			out_SaveData.Empty(Ar->TotalSize());
			out_SaveData.AddZeroed(Ar->TotalSize());
			Ar->Serialize(out_SaveData.GetData(), Ar->TotalSize());

			if (!Ar->IsError())
			{
				//debugf(TEXT("Successfully read %i bytes"), Ar->TotalSize());
				bResult = TRUE;
			}
			else
			{
				debugf(NAME_Error, TEXT("Failed to read save data!"));
			}

			delete Ar;
		}

		return bResult;
	}
#endif
}

void UOLEngine::WriteSaveData(const FString& Filename, const TArray<BYTE>& SaveData)
{
#if !CONSOLE
	if (IsUsingSteamCloud())
	{
		FString fullPath = FString::Printf(TEXT("%s.sav"), *Filename);
		WriteToSteamCloud(TCHAR_TO_UTF8(*fullPath), SaveData);
	}
	else
	{
		FArchive* Ar = GFileManager->CreateFileWriter(*FString::Printf(TEXT("%sSaveData\\%s.sav"), *appGameDir(), *Filename));
		if (Ar != NULL)
		{
			Ar->Serialize((void*)SaveData.GetData(), SaveData.Num());

			if (!Ar->IsError())
			{
				debugf(TEXT("Successfully wrote %i bytes"), Ar->TotalSize());
			}
			else
			{
				debugf(NAME_Error, TEXT("Failed to write save data!"));
			}

			delete Ar;
		}
	}
#endif
}

void UOLEngine::DeleteSaveFile(const FString& Filename)
{
#if !CONSOLE
	if (IsUsingSteamCloud())
	{
		FString fullPath = FString::Printf(TEXT("%s.sav"), *Filename);
		DeleteSteamCloudFile(TCHAR_TO_UTF8(*fullPath));
	}
	else
	{
		UBOOL ok = GFileManager->Delete(*FString::Printf(TEXT("%sSaveData\\%s.sav"), *appGameDir(), *Filename));
		if (ok)
		{
			debugf(TEXT("Successfully deleted save data '%s'"), *Filename);
		}
		else
		{
			debugf(NAME_Error, TEXT("Failed to delete save data '%s'!"), *Filename);
		}
	}
#endif
}

IMPLEMENT_COMPARE_CONSTREF( FSaveFileInfo, OLGame, { return A.Time.Compare(B.Time); } )

INT FCheckpointTime::Compare(const FCheckpointTime Other) const
{
	if (Year == Other.Year)
	{
		if (Month == Other.Month)
		{
			if (Day == Other.Day)
			{
				return SecondsSinceMidnight < Other.SecondsSinceMidnight ? 1 : -1;
			}
			else
			{
				return Day < Other.Day ? 1 : -1;
			}
		}
		else
		{
			return Month < Other.Month ? 1 : -1;
		}
	}
	else
	{
		return Year < Other.Year ? 1 : -1;
	}
}

UBOOL UOLEngine::HasValidSaveGame()
{
#if ORBIS
	TArray<FString> savegames;
	UBOOL bOk = GOLOrbis->EnumerateSaveGames(savegames);
	return (bOk && savegames.Num() > 0);
#elif DINGO
	if (GOLDingo->UserInitState == OLDingo::UIS_Initialized)
	{
		return GDingoOSS->ContentInterfaceImpl->ExistingSaveFiles.Num() > 0;
	}
	return FALSE;
#else
	return SaveFiles.Num() > 0;
#endif
}

void UOLEngine::RefreshSaveFiles()
{
#if DINGO
	SaveFiles.Empty();
	
	if (GOLDingo->UserInitState == OLDingo::UIS_Initialized)
	{
		TArray<FString> saveFilenames;

		GOLDingo->EnumerateSaveGames(saveFilenames);

		SaveFiles.AddZeroed(saveFilenames.Num());

		for (INT i = 0; i < saveFilenames.Num(); ++i)
		{
			const FString& filenameStr = saveFilenames(i);
			SaveFiles(i).Filename = FFilename(filenameStr);

			TArray<BYTE> SaveData;
			GOLDingo->GetSaveGameData(saveFilenames(i), SaveData);

			FString CheckpointName;
			FMemoryReader Ar(SaveData);
			UBOOL bDeprecated = FALSE;
			INT difficulty = 0;
			UOLCheckpointSave::SerializeHeader(Ar, CheckpointName, SaveFiles(i).Time, bDeprecated, difficulty);

			SaveFiles(i).CheckpointName = FName(*CheckpointName);

			const TArray<TCHAR>& charArray = filenameStr.GetCharArray();
			const TCHAR& lastChar = charArray(charArray.Num() - 2);
			INT saveId = lastChar - L'0';
			
			if (saveId >= 1 && saveId <= 5)
			{
				SaveFiles(i).SaveFileId = saveId;
			}
			else
			{
				debugf(TEXT("Invalid save id for filename: %s"), *filenameStr);
			}

			SaveFiles(i).Difficulty = difficulty;
		}

		// Sort list by newest first
		Sort<USE_COMPARE_CONSTREF( FSaveFileInfo, OLGame )>( SaveFiles.GetTypedData(), SaveFiles.Num() );
	}
#elif !CONSOLE
	// Get all files in SaveFile Directory
	TArray<FString> Filenames;
	if (IsUsingSteamCloud())
	{
		FindSteamCloudSaveFiles(Filenames);
	}
	else
	{
		GFileManager->FindFiles(Filenames, *FString::Printf(TEXT("%sSaveData\\OL_*.sav"), *appGameDir()), true, false);
	}

	SaveFiles.Empty();

	TArray<AOLCheckpointList*> cpLists;
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpointList* cpList = Cast<AOLCheckpointList>(*It);
		if (cpList)
		{
			cpLists.AddItem(cpList);
		}
	}
		
	for (INT filenameIdx = 0; filenameIdx < Filenames.Num(); ++filenameIdx)
	{
		FSaveFileInfo saveInfo;
		appMemZero(saveInfo);
		saveInfo.Filename = FFilename(Filenames(filenameIdx)).GetBaseFilename();

		TArray<BYTE> SaveData;
		ReadSaveData(saveInfo.Filename, SaveData);

		FString CheckpointName;
		FMemoryReader Ar(SaveData);
		UBOOL bDeprecated = FALSE;
		INT dummyDifficulty = 0;
		UOLCheckpointSave::SerializeHeader(Ar, CheckpointName, saveInfo.Time, bDeprecated, dummyDifficulty);

		saveInfo.CheckpointName = FName(*CheckpointName);

		UBOOL bCheckpointExists = FALSE;
		
		for (INT i = 0; i < cpLists.Num(); i++)
		{
			AOLCheckpointList* cpList = cpLists(i);
			INT testIdx = cpList->CheckpointList.FindItemIndex(saveInfo.CheckpointName);
			if (testIdx >= 0)
			{
				bCheckpointExists = TRUE;
				break;
			}
		}

		if (cpLists.Num() > 0 && !bCheckpointExists)
		{
			// Invalid checkpoint
			//debugf(TEXT("Skipping save %s - checkpoint %s not found (DLC isn't installed?)"), *saveInfo.Filename, *CheckpointName);
		}
		else
		{
			SaveFiles.AddItem(saveInfo);
		}
	}

	// Sort list by newest first
	Sort<USE_COMPARE_CONSTREF( FSaveFileInfo, OLGame )>( SaveFiles.GetTypedData(), SaveFiles.Num() );
#endif
}

UBOOL UOLEngine::UsingFixedSaveLocation()
{
#if ORBIS
	return TRUE;
#else
	return FALSE;
#endif
}

UBOOL UOLEngine::UpdateProfileFromSystemSettings(UOLProfileSettings* userConfiguredSettings)
{
#if CONSOLE
	return TRUE;
#else
	UBOOL ok = userConfiguredSettings->SetProfileSettingValueId(PSI_Fullscreen, GSystemSettings.bFullscreen);
	ok = ok && userConfiguredSettings->SetProfileSettingValueId(PSI_VSync, GSystemSettings.bUseVSync);

	{
		INT fpsCap = FPSC_62;
		FLOAT maxFPS = GEngine->MaxSmoothedFrameRate;
		if      (maxFPS <= 0.f)   fpsCap = FPSC_Unlimited;
		else if (maxFPS <= 30.f)  fpsCap = FPSC_30;
		else if (maxFPS <= 62.f)  fpsCap = FPSC_62;
		else if (maxFPS <= 120.f) fpsCap = FPSC_120;
		else if (maxFPS <= 144.f) fpsCap = FPSC_144;
		else if (maxFPS <= 240.f) fpsCap = FPSC_240;
		else                      fpsCap = FPSC_Unlimited;
		ok = ok && userConfiguredSettings->SetProfileSettingValueId(PSI_MaxFPS, fpsCap);
	}

	INT resX = GSystemSettings.ResX;
	INT resY = GSystemSettings.ResY;

	EScreenResolution resolution = SR_Other;

	if (bUseCustomResolution)
	{
		resolution = SR_Other;
	}
	else
	{
		for (INT i = 0; i < userConfiguredSettings->ScreenResolutions.Num(); i++)
		{
			if (resX == userConfiguredSettings->ScreenResolutions(i).Width && resY == userConfiguredSettings->ScreenResolutions(i).Height)
			{
				resolution = EScreenResolution(i);
				break;
			}
		}
	}
	
	if (resolution == SR_Other)
	{
		debugf(TEXT("### Using other resolution: %d x %d"), resX, resY);
	}

	debugf(TEXT("### UpdateProfileFromSystemSettings - Current resolution is %s (%d x %d, fullscreen: %s)"), *Utils::GetEnumString("EScreenResolution", resolution), resX, resY, GSystemSettings.bFullscreen ? TEXT("yes") : TEXT("no"));

	ok = ok && userConfiguredSettings->SetProfileSettingValueId(PSI_Resolution, resolution);

	ok = ok && UpdateProfileKeyBindingsFromSystem(userConfiguredSettings);

	return ok;
#endif
}

UBOOL UOLEngine::UpdateProfileKeyBindingsFromSystem(UOLProfileSettings* userConfiguredSettings)
{
	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (!OLPC)
	{
		return FALSE;
	}

	UPlayerInput* playerInput = OLPC->PlayerInput;
	if (!playerInput)
	{
		return FALSE;
	}

	UBOOL ok = TRUE;

	INT startBindingsIdx = PSI_KB_MoveForward;
	INT endBindingsIdx = PSI_KB_ShowEvidenceMenu;

	for (INT i = startBindingsIdx; i <= endBindingsIdx; i++)
	{
		FString settingName = Utils::GetEnumString("EProfileSettingID", i);
		FString actionName = settingName.Replace(TEXT("PSI_KB_"), TEXT("OLA_")); 

		INT bindIdx = -1;
		FString keyBinding = playerInput->GetBindNameFromCommand(actionName, &bindIdx);

		while (bindIdx >= 0 && !keyBinding.IsEmpty() && keyBinding.InStr(TEXT("XboxTypeS_"), FALSE, TRUE) != INDEX_NONE)
		{
			bindIdx--;
			keyBinding = playerInput->GetBindNameFromCommand(actionName, &bindIdx);
		}

		UBOOL thisBindingOk = userConfiguredSettings->SetProfileSettingValueString(i, keyBinding);
		ok = ok && thisBindingOk;
	}

	return ok;
}

INT UOLEngine::LanguageStrToIdx(const TCHAR* langStr)
{
	INT lang = -1;

	if (appStricmp(langStr, TEXT("INT")) == 0)
	{
		lang = EL_English;
	}
	else if (appStricmp(langStr, TEXT("FRA")) == 0)
	{
		lang = EL_French;
	}
	else if (appStricmp(langStr, TEXT("ESN")) == 0)
	{
		lang = EL_Spanish;
	}
	else if (appStricmp(langStr, TEXT("ITA")) == 0)
	{
		lang = EL_Italian;
	}
	else if (appStricmp(langStr, TEXT("DEU")) == 0)
	{
		lang = EL_German;
	}
	else if (appStricmp(langStr, TEXT("RUS")) == 0)
	{
		lang = EL_Russian;
	}
	else if (appStricmp(langStr, TEXT("POL")) == 0)
	{
		lang = EL_Polish;
	}
	else if (appStricmp(langStr, TEXT("PTB")) == 0)
	{
		lang = EL_Brazilian;
	}
	else if (appStricmp(langStr, TEXT("JPN")) == 0)
	{
		lang = EL_Japanese;
	}

	return lang;
}

FString UOLEngine::LanguageIdxToString(ELanguage language)
{
	switch (language)
	{
	case EL_English:
		return FString(TEXT("INT"));
	case EL_French:
		return FString(TEXT("FRA"));
	case EL_Spanish:
		return FString(TEXT("ESN"));
	case EL_Italian:
		return FString(TEXT("ITA"));
	case EL_German:
		return FString(TEXT("DEU"));
	case EL_Russian:
		return FString(TEXT("RUS"));
	case EL_Polish:
		return FString(TEXT("POL"));
	case EL_Brazilian:
		return FString(TEXT("PTB"));
	case EL_Japanese:
		return FString(TEXT("JPN"));
	}

	return FString(TEXT("?"));
}

UBOOL UOLEngine::ApplySystemSettings(UOLProfileSettings* userConfiguredSettings)
{
#if CONSOLE
	return TRUE;
#else
	INT textureQualityLevel = 2;
	INT shadowQualityLevel = 2;
	INT effectsQualityLevel = 2;
	INT bFullScreen = GSystemSettings.bFullscreen;
	INT bUseVSync = GSystemSettings.bUseVSync;
	INT language = -1;
	INT fpsCap = FPSC_62;

	EScreenResolution resolution = SR_Other;

	UBOOL ok = userConfiguredSettings->GetProfileSettingValueId(PSI_TextureQuality, textureQualityLevel);
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_ShadowsQuality, shadowQualityLevel);
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_EffectsQuality, effectsQualityLevel);
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_Fullscreen, bFullScreen);
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_VSync, bUseVSync);
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_Resolution, *(INT*)&resolution);
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_Language, language);
	userConfiguredSettings->GetProfileSettingValueId(PSI_MaxFPS, fpsCap);


	if (ok)
	{
		GPendingSystemSettings = GSystemSettings;
		FSystemSettings& newSettings = GPendingSystemSettings;

		newSettings.bUseVSync = (UBOOL)bUseVSync;
		newSettings.bFullscreen = (UBOOL)bFullScreen;	

		if (bUseCustomResolution)
		{
			newSettings.ResX = CustomResX;
			newSettings.ResY = CustomResY;

			debugf(TEXT("### Resolution overriden by custom setting: %d x %d"), CustomResX, CustomResY);
		}
		else if (resolution == SR_Other)
		{		
#ifdef _WINDOWS_
			extern INT GPrimaryMonitorWidth;
			extern INT GPrimaryMonitorHeight;

			newSettings.ResX = GPrimaryMonitorWidth;
			newSettings.ResY = GPrimaryMonitorHeight;

			debugf(TEXT("### Using resolution from primary monitor: %d x %d"), GPrimaryMonitorWidth, GPrimaryMonitorHeight);
#else
			warnf(TEXT("### Using primary monitor resolution not implemented on mac!"));
#endif
		}
		else
		{
			check(resolution >= 0 && resolution < userConfiguredSettings->ScreenResolutions.Num());
			FScreenResolutionInfo& screenResInfo = userConfiguredSettings->ScreenResolutions(resolution);

			newSettings.ResX = screenResInfo.Width;
			newSettings.ResY = screenResInfo.Height;
		}

		debugf(TEXT("### Applying resolution %d x %d (Fullscreen: %s)"), newSettings.ResX, newSettings.ResY, bFullScreen ? TEXT("yes") : TEXT("no"));

		FString textureSectionName = FString::Printf(TEXT("SystemSettingsBucket%d"), textureQualityLevel+1);
		FString shadowSectionName = FString::Printf(TEXT("SystemSettingsBucket%d"), shadowQualityLevel+1);
		FString effectsSectionName = FString::Printf(TEXT("SystemSettingsBucket%d"), effectsQualityLevel+1);

		// Textures
		GSystemSettings.GetIntSettingFromIni(textureSectionName, FString(TEXT("MaxAnisotropy")), newSettings.MaxAnisotropy);
		newSettings.TextureLODSettings.Initialize( GSystemSettingsIni, *textureSectionName );

		// Effects
		GSystemSettings.GetFloatSettingFromIni(effectsSectionName, FString(TEXT("DecalCullDistanceScale")), newSettings.DecalCullDistanceScale);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("SHSecondaryLighting")), newSettings.bAllowSHSecondaryLighting);

		if (bDisableMotionBlur)
		{
			newSettings.bAllowMotionBlur = FALSE;
		}
		else
		{
			GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("MotionBlur")), newSettings.bAllowMotionBlur);
		}

		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("MotionBlurPause")), newSettings.bAllowMotionBlurPause);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("AmbientOcclusion")), newSettings.bAllowAmbientOcclusion);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("Distortion")), newSettings.bAllowDistortion);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("FilteredDistortion")), newSettings.bAllowFilteredDistortion);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("DropParticleDistortion")), newSettings.bAllowParticleDistortionDropping);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("LensFlares")), newSettings.bAllowLensFlares);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("AllowRadialBlur")), newSettings.bAllowRadialBlur);
		GSystemSettings.GetIntSettingFromIni(effectsSectionName, FString(TEXT("DetailMode")), newSettings.DetailMode);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("bAllowFracturedDamage")), newSettings.bAllowFracturedDamage);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("Bloom")), newSettings.bAllowBloom);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("UnbatchedDecals")), newSettings.bAllowUnbatchedDecals);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("bAllowDownsampledTranslucency")), newSettings.bAllowDownsampledTranslucency);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("AllowSkin")), newSettings.bAllowSkin);
		GSystemSettings.GetBoolSettingFromIni(effectsSectionName, FString(TEXT("bAllowPostprocessFXAA")), newSettings.bAllowPostprocessFXAA);

		// Shadows
		GSystemSettings.GetBoolSettingFromIni(shadowSectionName, FString(TEXT("CompositeDynamicLights")), newSettings.bUseCompositeDynamicLights);
		GSystemSettings.GetBoolSettingFromIni(shadowSectionName, FString(TEXT("DynamicShadows")), newSettings.bAllowDynamicShadows);
		GSystemSettings.GetIntSettingFromIni(shadowSectionName, FString(TEXT("ShadowFilterQualityBias")), newSettings.ShadowFilterQualityBias);
		GSystemSettings.GetIntSettingFromIni(shadowSectionName, FString(TEXT("MinShadowResolution")), newSettings.MinShadowResolution);
		GSystemSettings.GetIntSettingFromIni(shadowSectionName, FString(TEXT("MaxShadowResolution")), newSettings.MaxShadowResolution);
		GSystemSettings.GetIntSettingFromIni(shadowSectionName, FString(TEXT("MaxWholeSceneDominantShadowResolution")), newSettings.MaxWholeSceneDominantShadowResolution);
		GSystemSettings.GetIntSettingFromIni(shadowSectionName, FString(TEXT("ShadowFadeResolution")), newSettings.ShadowFadeResolution);
		GSystemSettings.GetFloatSettingFromIni(shadowSectionName, FString(TEXT("ShadowFadeExponent")), newSettings.ShadowFadeExponent);
		GSystemSettings.GetFloatSettingFromIni(shadowSectionName, FString(TEXT("ShadowTexelsPerPixel")), newSettings.ShadowTexelsPerPixel);
		GSystemSettings.GetBoolSettingFromIni(shadowSectionName, FString(TEXT("bEnableForegroundShadowsOnWorld")), newSettings.bEnableForegroundShadowsOnWorld);
		GSystemSettings.GetBoolSettingFromIni(shadowSectionName, FString(TEXT("bAllowWholeSceneDominantShadows")), newSettings.bAllowWholeSceneDominantShadows);

		bPendingGraphicalSettingsChange = TRUE;

		static const FLOAT FPSCapValues[] = { 30.f, 62.f, 120.f, 144.f, 240.f, 0.f };
		if (fpsCap >= 0 && fpsCap < ARRAY_COUNT(FPSCapValues))
		{
			GEngine->MaxSmoothedFrameRate = FPSCapValues[fpsCap];
			GEngine->bSmoothFrameRate = (FPSCapValues[fpsCap] > 0.f);
		}

		INT curLang = LanguageStrToIdx(GetLanguage());

		if (language >= 0 && language != curLang)
		{
			PendingNewLanguage = language;
		}
	}

	return ok;
#endif
}

UBOOL UOLEngine::ApplyKeyBindings(UOLProfileSettings* userConfiguredSettings)
{
	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (!OLPC)
	{
		return FALSE;
	}

	UOLPlayerInput* playerInput = Cast<UOLPlayerInput>(OLPC->PlayerInput);

	if (!playerInput)
	{
		return FALSE;
	}

	UBOOL ok = TRUE;

#if !CONSOLE
	// keyboard and mouse
	{
		INT startBindingsIdx = PSI_KB_MoveForward;
		INT endBindingsIdx = PSI_KB_ShowEvidenceMenu;

		for (INT i = startBindingsIdx; ok && i <= endBindingsIdx; i++)
		{
			FString keyBinding;
			ok = userConfiguredSettings->GetProfileSettingValueString(i, keyBinding);

			if (ok && !keyBinding.IsEmpty())
			{
				FString settingName = Utils::GetEnumString("EProfileSettingID", i);
				FString actionName = settingName.Replace(TEXT("PSI_KB_"), TEXT("OLA_")); 

				// Remove old binding
				playerInput->eventUnBindNoSave(actionName);

				// Add new binding
				playerInput->eventSetBindNoSave(FName(*keyBinding), actionName);
			}
		}
	}
#endif

	// gamepad

	INT gamepadConfigId = 0;
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_GamepadConfig, gamepadConfigId);
	
	if (ok)
	{
		playerInput->GamepadConfig = gamepadConfigId;
		const TArray<FKeyBind>& gamepadBindings = (gamepadConfigId == GC_TypeA ? playerInput->GPBindingsA : (gamepadConfigId == GC_TypeB ? playerInput->GPBindingsB : playerInput->GPBindingsC));

		for (INT i = 0; i < gamepadBindings.Num(); i++)
		{
			const FKeyBind& keybind = gamepadBindings(i);
			playerInput->eventSetBindNoSave(keybind.Name, keybind.Command);
		}
	}	

	// southpaw

	INT bSouthpaw = 0;
	ok = ok && userConfiguredSettings->GetProfileSettingValueId(PSI_Southpaw, bSouthpaw);

	if (ok)
	{
		if (bSouthpaw)
		{
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_LeftX")), playerInput->LookXCommand);
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_LeftY")), playerInput->SouthpawLookYCommand);
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_RightX")), playerInput->StrafeCommand);
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_RightY")), playerInput->SouthpawMoveCommand);
		}
		else
		{
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_LeftX")), playerInput->StrafeCommand);
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_LeftY")), playerInput->MoveCommand);
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_RightX")), playerInput->LookXCommand);
			playerInput->eventSetBindNoSave(FName(TEXT("XboxTypeS_RightY")), playerInput->LookYCommand);
		}
	}	

	playerInput->SaveConfig();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Dingo specific implementation
//////////////////////////////////////////////////////////////////////////

void UOLEngine::Send( ECallbackEventType InType )
{
#if DINGO
	PendingCallbacks.AddItem(InType);
#endif
}

void UOLEngine::Send( ECallbackEventType InType, const FVector& InVector )
{
#if DINGO
	PendingCallbacks.AddItem(InType);
#endif
}

#if DINGO
void UOLEngine::ProcessDingoCallbacks()
{
	AOLPlayerController* OLPC = Utils::GetOLPC();

	for (INT i = 0; i < PendingCallbacks.Num(); i++)
	{
		BYTE callbackType = PendingCallbacks(i);

		switch (callbackType)
		{
		case CALLBACK_EnteringConstrainedMode:
			// @igs(arm) - Here is where your game would handle a reduction in CPU/GPU resources. The game also stops receiving input until
			//             it re-enters Full mode.
			//             As an example we simply pause the game
			//DeferredCommands.AddUniqueItem(TEXT("ShowPauseMenu"));

			{
				debugf(TEXT("### CALLBACK_EnteringConstrainedMode"));		

				if (OLPC)
				{
					OLPC->PauseGameOnLostFocus();
				}	

				break;
			}

		case CALLBACK_EnteringFullMode:
			// @igs(arm) - Here is where your game would handle the return to full CPU/GPU availability

			debugf(TEXT("### CALLBACK_EnteringFullMode"));
			break;

		case CALLBACK_GainedFocus:
			{
				// When gaining or re-gaining focus a title may return to a normal state of emphasis on interactive elements 
				/*if (GEngine && GEngine->GameViewport)
				{
				UUDKGameViewportClient* UDKGameViewportClient = Cast<UUDKGameViewportClient>(GEngine->GameViewport->GetUObject());
				if (UDKGameViewportClient)
				{
				UDKGameViewportClient->bDimScreen = FALSE;
				}
				}
				*/

				debugf(TEXT("### CALLBACK_GainedFocus"));
				break;
			}

		case CALLBACK_LostFocus:
			{
				// When a title is not in focus because the user has snapped another title into focus, the title must 
				// de-emphasize interactive elements such as menus or on-screen controls.
				// As an example we simply pause the game
				/*if (GEngine && GEngine->GameViewport)
				{
				UUDKGameViewportClient* UDKGameViewportClient = Cast<UUDKGameViewportClient>(GEngine->GameViewport->GetUObject());
				if (UDKGameViewportClient)
				{
				UDKGameViewportClient->bDimScreen = TRUE;
				}
				}
				*/
			
				debugf(TEXT("### CALLBACK_LostFocus"));

				if (OLPC)
				{
					OLPC->PauseGameOnLostFocus();
				}

				if (GOLDingo)
				{
					GOLDingo->UpdateSessionState(OLDingo::SS_Paused);
				}
				break;
			}

		case CALLBACK_VUI_Back:
			{
				if (!Utils::IsTravelling() && (GWorld->IsPaused() || Utils::IsInMainMenu()))
				{
					if (Utils::GetOLPC() && Utils::GetOLPC()->HUD)
					{
						debugf(TEXT("Received back speech command: simulating 'B' input"));
						Utils::GetOLPC()->HUD->eventSimulateBackInput();
					}
				}
				break;
			}
		case CALLBACK_VUI_Menu:
		case CALLBACK_VUI_Pause:
			{
				if (!Utils::IsInMainMenu() && !Utils::IsTravelling() && !GWorld->IsPaused() && Utils::GetOLPC())
				{
					debugf(TEXT("Received menu speech command: Opening pause menu"));
					Utils::GetOLPC()->HUD->eventShowMenuType(EMT_PauseMenu);
				}
				break;
			}
		case CALLBACK_VUI_Play:
			{
				if (!Utils::IsInMainMenu() && !Utils::IsTravelling() && GWorld->IsPaused() && Utils::GetOLPC())
				{
					debugf(TEXT("Received play speech command: Closing pause menu"));
					Utils::GetOLPC()->HUD->eventClosePauseMenu();
				}
				break;
			}		
		default:
			warnf(NAME_Dev, TEXT("Received event callback for unsupported event code: %d"), (INT)callbackType);
			break;
		}
	}

	PendingCallbacks.Reset();
}
#endif

void UOLEngine::SetSmoothCamera(UBOOL bEnabled)
{
	GSmoothCamera = bEnabled;
	bSmoothCamera = bEnabled;
	SaveConfig();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
