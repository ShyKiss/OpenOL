#include "OLGame.h"
#include "OLImGui_Tabs.h"

IMPLEMENT_CLASS(UOLCheatManager);

// Used for single stepping
UBOOL GDelayedSingleStep = FALSE;
UBOOL GDelayedPause = FALSE;

extern UBOOL GShowEditorSprites;

void UOLCheatManager::SetShowEditorSprites(UBOOL bShow)
{
    GShowEditorSprites = bShow;
}

UOLCheatManager* UOLCheatManager::GetCheatManager()
{
	return Utils::GetCheatManager();
}

void UOLCheatManager::DebugGameplay()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugGameplay = !bDebugGameplay;
}

void UOLCheatManager::DebugSoundEnvironment(const FString& filter)
{
	if (bCheatsEnabled)
	{
		if (bDebugSoundEnvironment && filter != DebugSoundEnvFilter)
		{
			DebugSoundEnvFilter = filter;
			return; // we're just setting the filter - don't toggle the debug mode
		}

		DebugSoundEnvFilter = filter;
		bDebugSoundEnvironment = !bDebugSoundEnvironment;		

		ULocalPlayer* LP = Cast<ULocalPlayer>(Utils::GetOLPC()->Player);
		if (LP)
		{
			if (bDebugSoundEnvironment)
			{
				LP->ViewportClient->ShowFlags |= SHOW_Volumes;
			}
			else
			{
				LP->ViewportClient->ShowFlags &= ~SHOW_Volumes;
			}

			// Show the snd env volumes
			if (AllowDebugViewmodes())
			{
				for( TObjectIterator<UBrushComponent> It; It; ++It )
				{
					UBrushComponent* BrushComponent = *It;
					AOLSoundEnvironmentVolume* sndEnv = Cast<AOLSoundEnvironmentVolume>( BrushComponent->GetOwner() );

					// Only bother with volume brushes that belong to the world's scene
					if( sndEnv && BrushComponent->GetScene() == GWorld->Scene )
					{
						if (BrushComponent->HiddenGame && bDebugSoundEnvironment)
						{
							sndEnv->bHidden = FALSE;
							BrushComponent->SetHiddenGame(FALSE);
						}
						else if (!BrushComponent->HiddenGame && !bDebugSoundEnvironment)
						{
							sndEnv->bHidden = TRUE;
							BrushComponent->SetHiddenGame(TRUE);
						}
					}
				}
			}
		}
	}
}

void UOLCheatManager::SingleFrame()
{
	if (!bCheatsEnabled)
	{
		return;
	}
	GDelayedSingleStep = TRUE;
	bPausedForFreeCam = FALSE;
}

void UOLCheatManager::GhostPawn(UBOOL ghostPawn)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	AOLPlayerController* OLPC = GetOuterAOLPlayerController();
	if (ghostPawn)
	{
		OLPC->HeroPawn->SetCollision(FALSE, FALSE, FALSE);
		OLPC->HeroPawn->setPhysics(PHYS_Custom);
		OLPC->HeroPawn->bCollideWorld = FALSE;
		OLPC->HeroPawn->SetPushesRigidBodies(FALSE);
		OLPC->HeroPawn->SetHidden(TRUE);
		OLPC->HeroPawn->bIsGhost = TRUE;		
	}
	else
	{
		OLPC->HeroPawn->SetCollision(TRUE, TRUE, FALSE);
		OLPC->HeroPawn->setPhysics(PHYS_Falling);
		OLPC->HeroPawn->SetLocation(OLPC->DebugCamPos - FVector(0, 0, OLPC->HeroPawn->EyeHeight));
		OLPC->HeroPawn->SetRotation(OLPC->DebugCamRot);
		OLPC->HeroPawn->EyeLocation = OLPC->DebugCamPos;
		OLPC->HeroPawn->EyeRotation = OLPC->DebugCamRot;
		OLPC->HeroPawn->Camera->SetView(OLPC->DebugCamPos, OLPC->DebugCamRot);
		OLPC->HeroPawn->bCollideWorld = TRUE;
		OLPC->HeroPawn->SetPushesRigidBodies(OLPC->HeroPawn->DefaultPawn->bPushesRigidBodies);
		OLPC->HeroPawn->SetHidden(FALSE);
		OLPC->HeroPawn->bIsGhost = FALSE;
	}
}

void UOLCheatManager::GiveItem(const FString& ItemName)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	GetOuterAOLPlayerController()->InventoryManager->AddUniqueItem(FName(*ItemName));
}

void UOLCheatManager::SetGamma(FLOAT newGamma)
{
	extern void SetDisplayGamma(FLOAT Gamma);
	extern void SaveDisplayGamma();

	SetDisplayGamma(newGamma);
	SaveDisplayGamma();
}

void UOLCheatManager::ResetDoors()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	for (FActorIterator It; It; ++It)
	{
		AOLDoor* door = Cast<AOLDoor>(*It);
		if (door)
		{
			door->SetTargetOpenAngle(door->InitialOpenAngle);
		}
	}
}

void UOLCheatManager::ResetPushables()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	for (FActorIterator It; It; ++It)
	{
		AOLPushableObject* pushable = Cast<AOLPushableObject>(*It);
		if (pushable)
		{
			pushable->SetDisplacement(0.0f);
		}
	}
}

void UOLCheatManager::ResetWorldState()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC)
	{
		OLPC->ResetWorldState();
	}
}


void UOLCheatManager::ApplyCP(const FString& CPName)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	FName checkpoint(*CPName);

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (!OLPC)
	{
		return;
	}

	if (!Utils::IsCheckpointValid(checkpoint))
	{
		OLPC->eventClientMessage(TEXT("Invalid checkpoint name. Must be on a OLCheckpoint actor and ordered in the OLCheckpointList"));
		return;
	}

	AOLGame* currentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	currentGame->CurrentCheckpointName = checkpoint;

	OLPC->ApplyCheckpoint(checkpoint);
}

void UOLCheatManager::DeleteAllSaves()
{
	if (!bCheatsEnabled)
	{
		return;
	}
	
	UOLEngine* olengine = Cast<UOLEngine>(GEngine);

	if (!olengine)
	{
		return;
	}

	olengine->DeleteAllCloudSaves();
	olengine->DeleteAllLocalSaves();
}

void UOLCheatManager::SaveAllCheckpoints()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (!OLPC)
	{
		return;
	}

	UOLEngine* olengine = Cast<UOLEngine>(GEngine);

	if (!olengine)
	{
		return;
	}

	OLPC->ClearAllProgress();
	olengine->DeleteAllCloudSaves();
	olengine->SaveAllCheckpoints();
}

void UOLCheatManager::NativeMakeFullProfile()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	UOLEngine* olengine = Cast<UOLEngine>(GEngine);

	if (!olengine)
	{
		return;
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (!OLPC)
	{
		return;
	}
	
	OLPC->CheatGiveAllCollectibles();

	olengine->SaveCheckpointImmediate(FName(TEXT("Lab_BigTowerDone")));
}

void UOLCheatManager::ToggleMute()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bMuted = !bMuted;

	AOLPlayerController* OLPC = GetOuterAOLPlayerController();

	if (bMuted)
	{
		OLPC->eventClientMessage(TEXT("Mute ON"));
	}
	else
	{
		OLPC->eventClientMessage(TEXT("Mute OFF"));
	}

	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetRTPCValue(TEXT("MASTER_VOLUME"), bMuted ? 0.0f : 100.0f, NULL);
	}
}

void UOLCheatManager::AddDebugTrajectoryPoint(FDebugTrajectoryPoint point)
{
	DebugTrajectoryPoints.AddItem(point);
}

void UOLCheatManager::DrawDebug()
{
	AOLPlayerController* OLPC = GetOuterAOLPlayerController();

	if (bDebugTrajectory)
	{
		FLOAT curTime = GWorld->GetTimeSeconds();

		TWEAKABLE FLOAT MaxDebugTrajectoryLifetime = 4.0f;
		TWEAKABLE FLOAT MinPointSize = 5.0f;
		TWEAKABLE FLOAT MaxPointSize = 12.0f;

		FLinearColor colors[] = {
			FLinearColor(0.2f, 0.93f, 0.0f), // DTT_Walking
			FLinearColor(0.93f, 0.92f, 0.0f), // DTT_Falling
			FLinearColor(0.93f, 0.0f, 0.0f), // DTT_AdjustPosition
			FLinearColor(0.0f, 0.4f, 0.93f), // DTT_ProceduralAnim
			FLinearColor(0.0f, 0.93f, 0.88f), // DTT_SpecialMove
			FLinearColor(1.0f, 1.0f, 1.0f), // DTT_Camera
			FLinearColor(1.0f, 1.0f, 1.0f), // DTT_Other
		};
		
		for (INT i = 0; i < DebugTrajectoryPoints.Num(); i++)
		{
			FDebugTrajectoryPoint& point = DebugTrajectoryPoints(i);

			if (curTime > point.TimeStamp + MaxDebugTrajectoryLifetime)
			{
				// expired
				DebugTrajectoryPoints.Remove(i);
				i--;
				continue;
			}
			
			FLOAT pointSize = Lerp(MaxPointSize, MinPointSize, Clamp(point.Speed / 400.0f, 0.0f, 1.0f));

			FLinearColor color = colors[point.PointType];
			
			if (point.PointType == DTT_Camera)
			{
				FLOAT interpolator = Clamp(point.Speed / 600.0f, 0.0f, 1.0f);
				color = FLinearColor(1.0f - interpolator, interpolator, 0.0f);
			}
			
			OLPC->DrawDebugPoint(point.Position, pointSize, color);
		}
	}
}

void UOLCheatManager::cplist()
{
	Utils::PrintCheckpointList();
}

void UOLCheatManager::ActivateGS(FName gsName)
{
	AOLGameStateList::ActivateGameState(gsName, TRUE);
	Utils::GetOLPC()->eventClientMessage(FString::Printf(TEXT("%s activated"), *gsName.ToString()));
}

void UOLCheatManager::ResetGS()
{
	extern AOLGameStateList* GMasterGameStateList;
	GMasterGameStateList->ResetAllGameState();
	AOLGameStateList::ApplyCheckpoint();
	Utils::GetOLPC()->eventClientMessage(TEXT("All game state reset to current checkpoint"));
}

UBOOL UOLCheatManager::IsViewModeUnlit()
{
#if !FINAL_RELEASE
	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC)
	{
		ULocalPlayer* LP = Cast<ULocalPlayer>(OLPC->Player);
		if (LP && LP->ViewportClient && (LP->ViewportClient->ShowFlags & SHOW_ViewMode_Lit) != SHOW_ViewMode_Lit)
		{
			return TRUE;
		}
	}
#endif

	return FALSE;
}

void UOLCheatManager::ReloadSoundBanks(UBOOL bDLC)
{
	UAkAudioDevice* akDevice = UAkAudioDevice::Get();
	if (akDevice)
	{
		akDevice->ConditionalReloadAllBanks(bDLC);
	}
}

void UOLCheatManager::LoadDLCSoundBank(const FString& BankName)
{
	UAkAudioDevice* akDevice = UAkAudioDevice::Get();
	if (!akDevice || Utils::IsPlayingDLC())
	{
		return;
	}

	FString DLCPath = appGameDir() + TEXT("CookedPCConsoleDLC\\");
	FString MainPath = appGameDir() + TEXT("CookedPCConsole\\");

	akDevice->SetBasePath(DLCPath);

	AkOSChar* pszBankName = 0;
	CONVERT_WIDE_TO_OSCHAR(*BankName, pszBankName);

	AkBankID BankID;
	AKRESULT Result = AK::SoundEngine::LoadBank(pszBankName, AK_DEFAULT_POOL_ID, BankID);

	akDevice->SetBasePath(MainPath);
}

void UOLCheatManager::SetLanguage(const FString& LanguageCode)
{
	UOLEngine* olEngine = Cast<UOLEngine>(GEngine);
	AOLPlayerController* OLPC = GetOuterAOLPlayerController();
	
	if (olEngine)
	{
		INT newLangIdx = olEngine->LanguageStrToIdx(*LanguageCode);

		if (newLangIdx)
		{
			olEngine->PendingNewLanguage = newLangIdx;
			OLPC->eventClientMessage(FString::Printf(TEXT("Language set to %s"), *LanguageCode));
		}
		else
		{
			OLPC->eventClientMessage(TEXT("Invalid language code"));
		}
	}
}


#if DINGO
#include "OLDingo.h"
#include "OnlineSubsystemDingo.h"
#endif

void UOLCheatManager::DingoTest(INT id)
{
#if DINGO
	
#endif
}
