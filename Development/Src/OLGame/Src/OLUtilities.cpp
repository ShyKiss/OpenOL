#include "OLGame.h"
#include "OLUtilities.h"

IMPLEMENT_CLASS(UOLUtils);

AOLPlayerController* GOLPC = NULL;

FString Utils::GetEnumString(const char* enumTypeStr, BYTE val)
{
	UEnum* enumClass = FindObject<UEnum>(ANY_PACKAGE, ANSI_TO_TCHAR(enumTypeStr), TRUE);
	if (enumClass != NULL && val < enumClass->NumEnums())
	{
		return enumClass->GetEnum(val).ToString();
	}
	return TEXT("[unknown]");
}

UBOOL Utils::IsBetweenMarkers(const FVector& pos, const AOLLedgeMarker* node1, const AOLLedgeMarker* node2, UBOOL extendContinuousSegments, FLOAT buffer)
{
	TWEAKABLE FLOAT ContinuousSegmentsMinCosAngle = 0.707f;
	FVector node1Loc = node1->Location;
	FVector node2Loc = node2->Location;
	FVector node1ToNode2 = (node2Loc - node1Loc).SafeNormal2D();

	if (extendContinuousSegments)
	{
		if (node2->Next == node1)
		{
			// swap
			node1 = node2;
			node2 = node1->Next;
		}

		check(node1->Next == node2);

		const AOLLedgeMarker* closestMarker = NULL;
		const AOLLedgeMarker* nextMarker = NULL;

		if (pos.DistanceSquared(node1Loc) < pos.DistanceSquared(node2Loc))
		{
			closestMarker = node1;
			nextMarker = node1->Prev;
		}
		else
		{
			closestMarker = node2;
			nextMarker = node2->Next;
		}

		if (nextMarker)
		{
			FVector nextStretch = (nextMarker->Location - closestMarker->Location).SafeNormal();

			if (Abs(nextStretch | node1ToNode2) > ContinuousSegmentsMinCosAngle)
			{
				// null out the buffer
				buffer = 0.0f;
			}
		}
	}

	return IsBetweenMarkers(pos, node1Loc, node2Loc, buffer);
}

UBOOL Utils::IsBetweenMarkers(const FVector& pos, const FVector& node1, const FVector& node2, FLOAT buffer)
{
	FVector node1ToNode2 = (node2 - node1).SafeNormal2D();
	FVector effectiveNode1 = node1 - buffer*node1ToNode2;
	FVector effectiveNode2 = node2 + buffer*node1ToNode2;
	return (((pos - effectiveNode1) | node1ToNode2) > 0.0f) &&	(((pos - effectiveNode2) | node1ToNode2) < 0.0f);
}

AOLPlayerController* Utils::GetOLPC()
{
	return GOLPC;
}

AOLHero* Utils::GetHero()
{
	AOLPlayerController* OLPC = GetOLPC();
	return OLPC ? OLPC->HeroPawn : NULL;
}

UOLCheatManager* Utils::GetCheatManager()
{
	AOLPlayerController* OLPC = GetOLPC();
	return OLPC ? (UOLCheatManager*)OLPC->CheatManager : NULL;
}

UBOOL Utils::IsDemo()
{
	AOLGame* olgame = Cast<AOLGame>(GWorld->GetGameInfo());
	return olgame && olgame->IsDemo();
}

UBOOL Utils::IsPlayingDLC()
{
	AOLGame* olgame = Cast<AOLGame>(GWorld->GetGameInfo());
	return olgame && olgame->IsPlayingDLC();
}

UBOOL Utils::IsDLCInstalled()
{
	AOLGame* olgame = Cast<AOLGame>(GWorld->GetGameInfo());
	return olgame && olgame->IsDLCInstalled();
}

UBOOL Utils::IsInMainMenu()
{
	for (INT i = 0; i < GWorld->Levels.Num(); i++)
	{
		ULevel* level = GWorld->Levels(i);
		if (level && level->GetOutermost()->GetName().ToLower().InStr(TEXT("mainmenu")) != INDEX_NONE)
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL Utils::IsTravelling()
{
	if (GOLPC)
	{
		return GOLPC->bTravellingToCheckpoint;
	}

	return TRUE; // we must always have an OLPC on steady state
}

AOLGame* Utils::GetOLGame()
{
	return Cast<AOLGame>(GWorld->GetGameInfo());
}

UOLSoundEnvironmentManager* Utils::GetSoundEnvManager()
{
	return GOLPC ? GOLPC->SoundEnvManager : NULL;
}

UOLFXManager* Utils::GetFXManager()
{
	return GOLPC ? GOLPC->FXManager : NULL;
}

AOLHUD* Utils::GetHUD()
{
	return GOLPC ? GOLPC->HUD : NULL;
}

UPostProcessChain* Utils::GetDefaultPostProcessChain()
{
	UOLFXManager* FXManager = GetFXManager();
	return FXManager ? FXManager->DefaultPPSChain : NULL;
}

UPostProcessChain* Utils::GetCameraPostProcessChain()
{
	UOLFXManager* FXManager = GetFXManager();
#if ORBIS
	return FXManager ? FXManager->CamcorderPPSChainConsole : NULL;
#else
	return FXManager ? FXManager->CamcorderPPSChain : NULL;
#endif
}

UPostProcessChain* Utils::GetCameraNVPostProcessChain()
{
	UOLFXManager* FXManager = GetFXManager();
	return FXManager ? FXManager->NVPPSChain : NULL;
}

FName Utils::GetCameraBoneName()
{
	return ((UOLHeroCamera*)(UOLHeroCamera::StaticClass()->GetDefaultObject()))->CameraBoneName;
}

FLOAT Utils::GetAspectRatio()
{
	AOLPlayerController* OLPC = GetOLPC();
	ULocalPlayer* const LP = OLPC ? Cast<ULocalPlayer>(OLPC->Player) : NULL;
	UGameViewportClient* const vpClient = LP ? LP->ViewportClient : NULL;
	
	if (vpClient)
	{
		FVector2D viewportSize(0,0);
		vpClient->GetViewportSize(viewportSize);
		return viewportSize.X/viewportSize.Y;
	}

	return 16.0f/10.0f;
}

AOLCheckpoint* Utils::GetCheckpointFromName(FName checkpointName)
{
	for( FActorIterator It; It; ++It)
	{
		AOLCheckpoint* Checkpoint = Cast<AOLCheckpoint>(*It);
		if (Checkpoint && Checkpoint->CheckpointName == checkpointName)
		{
			return Checkpoint;
		}
	}

	return NULL;
}

UBOOL Utils::IsCheckpointValid(FName checkpointName)
{
	return checkpointName != NAME_None && Utils::GetCheckpointFromName(checkpointName) != NULL && AOLCheckpointList::IsCheckpointInList(checkpointName);
}

UBOOL Utils::IsCheckpointReached(FName checkpointName)
{
	if (checkpointName == NAME_None)
	{
		return FALSE;
	}

	AOLGame* currentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	if (currentGame)
	{	
		return AOLCheckpointList::IsReached(checkpointName, currentGame->CurrentCheckpointName);
	}
	return FALSE;
}

UBOOL Utils::IsCheckpointUnreached(FName checkpointName)
{
	if (checkpointName == NAME_None)
	{
		return FALSE;
	}

	AOLGame* currentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	if (currentGame)
	{	
		return AOLCheckpointList::IsUnreached(checkpointName, currentGame->CurrentCheckpointName);
	}
	return FALSE;
}

UBOOL Utils::IsCheckpointCompleted(FName checkpointName)
{
	if (checkpointName == NAME_None)
	{
		return FALSE;
	}

	AOLGame* currentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	if (currentGame)
	{	
		return AOLCheckpointList::IsCompleted(checkpointName, currentGame->CurrentCheckpointName);
	}
	return FALSE;
}

UBOOL Utils::IsCheckpointDLC(FName checkpointName)
{
	AOLCheckpointList* ownerList = AOLCheckpointList::GetListForCheckpoint(checkpointName);
	return (ownerList && ownerList->GameType == OGT_Whistleblower);
}

FName Utils::GetCurrentCheckpointName()
{
	AOLGame* currentGame = Cast<AOLGame>(GWorld->GetGameInfo());
	if (currentGame)
	{	
		return currentGame->CurrentCheckpointName;
	}

	return NAME_None;
}

FString Utils::TranslateKeyBindings(const FString& originalText)
{
	AOLPlayerController* OLPC = GetOLPC();
	UOLPlayerInput* playerInput = OLPC ? Cast<UOLPlayerInput>(OLPC->PlayerInput) : NULL;

	if (!playerInput)
	{
		return originalText;
	}

	FString text = originalText;

	INT startIdx = text.InStr(TEXT("{OLA_"), FALSE, TRUE, 0);

	while (startIdx != INDEX_NONE)
	{
		INT closingBraceIdx = text.InStr(TEXT("}"), FALSE, FALSE, startIdx+5);

		if (closingBraceIdx == INDEX_NONE)
		{
			return originalText;
		}

		FString fullToken = text.Mid(startIdx, closingBraceIdx - startIdx + 1);
		FString actionName = text.Mid(startIdx + 1, closingBraceIdx - startIdx - 1);

		INT bindIdx = -1;
		FString keyBinding = playerInput->GetBindNameFromCommand(actionName, &bindIdx);

#if CONSOLE
		if (true)
#else
		if (playerInput->bUsingGamepad)
#endif
		{
			while (bindIdx >= 0 && !keyBinding.IsEmpty() && keyBinding.InStr(TEXT("XboxTypeS_"), FALSE, TRUE) == INDEX_NONE)
			{
				bindIdx--;
				keyBinding = playerInput->GetBindNameFromCommand(actionName, &bindIdx);
			}

			// Gamepad: Display internal key codes which will be replaced with images by Scaleform
			const int MAX_IMAGE_SUBSTITUTION_LENGTH = 15;
			keyBinding = FString::Printf(TEXT("{%s}"), *keyBinding.Replace(TEXT("XboxTypeS_"), TEXT("")).Left(MAX_IMAGE_SUBSTITUTION_LENGTH-2));
			text.ReplaceInline(*fullToken, *keyBinding);
			startIdx = text.InStr(TEXT("{OLA_"), FALSE, TRUE, 0);
		}
		else
		{
			while (bindIdx >= 0 && !keyBinding.IsEmpty() && keyBinding.InStr(TEXT("XboxTypeS_"), FALSE, TRUE) != INDEX_NONE)
			{
				bindIdx--;
				keyBinding = playerInput->GetBindNameFromCommand(actionName, &bindIdx);
			}

			// Keyboard: Display localized "friendly" key name
			FString friendlyKey = Localize(TEXT("InputKeys"), *keyBinding, TEXT("OLGame"));
			keyBinding = FString::Printf(TEXT("(%s)"), *friendlyKey);
			text.ReplaceInline(*fullToken, *keyBinding);
			startIdx = text.InStr(TEXT("{OLA_"), FALSE, TRUE, 0);
		}
	}

	return text;
}

void Utils::OutputTextToConsole(const FString& text)
{
	UConsole* console = (GEngine->GameViewport != NULL) ? GEngine->GameViewport->ViewportConsole : NULL;
	if (console)
	{
		console->eventOutputText(text);
	}

	debugf(*text);
}

void Utils::PrintCheckpointList()
{
	INT listCount = 0;
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpointList* cpListActor = Cast<AOLCheckpointList>(*It);
		if (cpListActor)
		{			
			TArray<FName>* cpList = &cpListActor->CheckpointList;
			OutputTextToConsole(FString::Printf(TEXT(" Checkpoint List %d (%s)"), listCount, *Utils::GetEnumString("OutlastGameType", cpListActor->GameType)));
			for (INT i = 0; i < cpList->Num(); i++)
			{
				OutputTextToConsole(FString::Printf(TEXT(" - [%02d] %s"), i, *(*cpList)(i).ToString()));
			}
			OutputTextToConsole(FString::Printf(TEXT(" > %d checkpoints."), cpList->Num()));
			listCount++;
		}
	}

	if (listCount == 0)
	{
		OutputTextToConsole("No checkpoint list");
	}
}

FName Utils::GetCheckpointTag(FName checkpointName)
{
	AOLCheckpoint* cp = GetCheckpointFromName(checkpointName);

	if (cp)
	{
		return cp->Tag;
	}

	return NAME_None;
}

//////////////////////////////////////////////////////////////////////////
// Script utils (UOLUtils)
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UBOOL UOLUtils::IsPS4()
{
#if ORBIS
	return TRUE;
#else
	{
		AOLPlayerController* OLPC = Utils::GetOLPC();
		if (OLPC && OLPC->HUD && OLPC->HUD->bForcePS4UI)
		{
			return TRUE;
		}
	}
	return FALSE;
#endif
}

UBOOL UOLUtils::IsDingo()
{
#if DINGO
	return TRUE;
#else
	return FALSE;
#endif
}

UBOOL UOLUtils::IsConsole()
{
#if ORBIS || DINGO
	return TRUE;
#else
	{
		AOLPlayerController* OLPC = Utils::GetOLPC();
		if (OLPC && OLPC->HUD && OLPC->HUD->bForcePS4UI)
		{
			return TRUE;
		}
	}
	return FALSE;
#endif
}

AOLPlayerController* UOLUtils::GetOLPC()
{
	return Utils::GetOLPC();
}

UBOOL UOLUtils::IsDLCInstalled()
{
	return Utils::IsDLCInstalled();
}

UBOOL UOLUtils::IsPlayingDLC()
{
	return Utils::IsPlayingDLC();
}

UBOOL UOLUtils::IsBindableKey(FName ButtonName)
{
	FString buttonStr = ButtonName.ToString().ToLower();
	return (buttonStr.InStr(TEXT("xbox")) == INDEX_NONE && buttonStr.InStr(TEXT("gamepad")) == INDEX_NONE);
}
