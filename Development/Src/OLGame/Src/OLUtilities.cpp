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
	// First look in world levels (fast path, covers the common case).
	for (FActorIterator It; It; ++It)
	{
		AOLCheckpoint* Checkpoint = Cast<AOLCheckpoint>(*It);
		if (Checkpoint && Checkpoint->CheckpointName == checkpointName)
			return Checkpoint;
	}

	// Fallback: search all loaded objects — covers checkpoints loaded from
	// packages that are in memory but not spawned into GWorld->Levels
	// (e.g. DLC_Checkpoints loaded for the ImGui checkpoint tab).
	for (TObjectIterator<AOLCheckpoint> It; It; ++It)
	{
		if (It->IsPendingKill()) continue;
		if (It->CheckpointName == checkpointName)
			return *It;
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

static void FindFilesInModSubdir(const FString& SubDir, const TCHAR* Pattern, TArray<FString>& OutNames)
{
	FString CookedPath;
	appGetCookedContentPath(appGetPlatformType(), CookedPath);
	FString SearchDir = CookedPath + SubDir;

	TArray<FString> Files;
	GFileManager->FindFiles(Files, *(SearchDir + TEXT("\\") + Pattern), TRUE, FALSE);
	for (INT i = 0; i < Files.Num(); i++)
	{
		OutNames.AddItem(FFilename(Files(i)).GetBaseFilename());
	}
}

void UOLUtils::GetModMaps(const FString& SubDir, TArray<FString>& MapNames)
{
	MapNames.Empty();
	FindFilesInModSubdir(SubDir, TEXT("*.udk"), MapNames);
	FindFilesInModSubdir(SubDir, TEXT("*.upk"), MapNames);
}

void UOLUtils::GetModPackages(const FString& SubDir, TArray<FString>& PackageNames)
{
	PackageNames.Empty();
	FindFilesInModSubdir(SubDir, TEXT("*.upk"), PackageNames);
}

// Recursively search Dir and all subdirectories for PackageName.upk / .udk.
static FString FindModPackageFileRecursive(const FString& Dir, const FString& PackageName)
{
	// Check in current directory first.
	const FString UpkPath = Dir + PackageName + TEXT(".upk");
	const FString UdkPath = Dir + PackageName + TEXT(".udk");
	if (GFileManager->FileSize(*UpkPath) >= 0) return UpkPath;
	if (GFileManager->FileSize(*UdkPath) >= 0) return UdkPath;

	// Enumerate subdirectories.
	TArray<FString> SubDirs;
	GFileManager->FindFiles(SubDirs, *(Dir + TEXT("*")), FALSE, TRUE);
	for (INT i = 0; i < SubDirs.Num(); ++i)
	{
		FString Found = FindModPackageFileRecursive(Dir + SubDirs(i) + TEXT("\\"), PackageName);
		if (!Found.IsEmpty())
			return Found;
	}
	return TEXT("");
}

static FString FindModPackageFile(const FString& PackageName)
{
	TArray<FString> RootDirs;

	FString CookedPath;
	appGetCookedContentPath(appGetPlatformType(), CookedPath);
	RootDirs.AddItem(CookedPath);

	// Always include DLC directory — checkpoint packages may be needed
	// even when DLC is not installed (e.g. loading DLC checkpoints from base game).
	RootDirs.AddItem(appGameDir() + TEXT("CookedPCConsoleDLC\\"));

	for (INT i = 0; i < RootDirs.Num(); ++i)
	{
		FString Found = FindModPackageFileRecursive(RootDirs(i), PackageName);
		if (!Found.IsEmpty())
			return Found;
	}
	return TEXT("");
}

static UPackage* EnsureModPackageLoaded(const FString& PackageName, const FString& FullPath)
{
	UPackage* Existing = FindObject<UPackage>(NULL, *PackageName, TRUE);
	if (Existing)
	{
		return Existing;
	}

	UPackage* Pkg = UObject::LoadPackage(NULL, *FullPath, LOAD_None);
	if (Pkg)
	{
		Pkg->AddToRoot();
	}
	else
	{
		debugf(TEXT("LoadModPackage: failed to load '%s'"), *PackageName);
	}
	return Pkg;
}

UBOOL UOLUtils::RegisterModMap(const FString& MapName)
{
	FString FullPath = FindModPackageFile(MapName);
	if (FullPath.IsEmpty())
	{
		// Map is already in the standard search paths — no registration needed
		return TRUE;
	}
	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);
	return TRUE;
}

UBOOL UOLUtils::LoadModPackage(const FString& PackageName)
{
	FString FullPath = FindModPackageFile(PackageName);
	if (FullPath.IsEmpty())
	{
		debugf(TEXT("LoadModPackage: '%s' not found"), *PackageName);
		return FALSE;
	}

	// Register so that "open MapName" can find the file by short name
	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);

	// Map packages (.udk) are loaded later by the engine's travel machinery
	if (!FullPath.EndsWith(TEXT(".udk")))
	{
		if (!EnsureModPackageLoaded(PackageName, FullPath))
		{
			return FALSE;
		}
	}

	return TRUE;
}

UPackage* Utils::LoadModPackage(const FString& PackageName)
{
	// Return already-loaded package if present.
	UPackage* Existing = FindObject<UPackage>(NULL, *PackageName, TRUE);
	if (Existing)
		return Existing;

	FString FullPath = FindModPackageFile(PackageName);
	if (FullPath.IsEmpty())
	{
		debugf(TEXT("Utils::LoadModPackage: '%s' not found in search dirs"), *PackageName);
		return NULL;
	}

	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);
	UPackage* Pkg = UObject::LoadPackage(NULL, *FullPath, LOAD_None);
	if (!Pkg)
		debugf(TEXT("Utils::LoadModPackage: LoadPackage failed for '%s'"), *PackageName);
	return Pkg;
}

AActor* Utils::SpawnActorFromPackage(const FString& PackageName, UClass* ActorClass, FName ActorTag)
{
	if (!ActorClass || !GWorld)
		return NULL;

	// Load the package (no-op if already in memory).
	FString FullPath = FindModPackageFile(PackageName);
	if (FullPath.IsEmpty())
	{
		debugf(TEXT("SpawnActorFromPackage: '%s' not found"), *PackageName);
		return NULL;
	}

	UPackage* Pkg = EnsureModPackageLoaded(PackageName, FullPath);
	if (!Pkg)
		return NULL;

	// Find the first actor of the requested class in the package.
	AActor* SrcActor = NULL;
	for (TObjectIterator<AActor> It; It; ++It)
	{
		if (It->GetOutermost() != Pkg)
			continue;
		if (!It->IsA(ActorClass))
			continue;
		if (ActorTag != NAME_None && It->Tag != ActorTag)
			continue;
		SrcActor = *It;
		break;
	}

	if (!SrcActor)
	{
		debugf(TEXT("SpawnActorFromPackage: no '%s' actor found in '%s'"),
			*ActorClass->GetName(), *PackageName);
		return NULL;
	}

	// Spawn a fresh instance in the current world and copy all properties.
	AActor* DstActor = GWorld->SpawnActor(ActorClass, NAME_None,
		SrcActor->Location, SrcActor->Rotation);
	if (!DstActor)
	{
		debugf(TEXT("SpawnActorFromPackage: SpawnActor failed for '%s'"), *ActorClass->GetName());
		return NULL;
	}

	// Copy all non-component properties from SrcActor to DstActor.
	for (TFieldIterator<UProperty> It(ActorClass); It; ++It)
	{
		UProperty* Prop = *It;
		if (Prop->PropertyFlags & (CPF_Component | CPF_Transient | CPF_Native))
			continue;
		BYTE* Src = (BYTE*)SrcActor + Prop->Offset;
		BYTE* Dst = (BYTE*)DstActor + Prop->Offset;
		Prop->CopyCompleteValue(Dst, Src, NULL, DstActor);
	}
	return DstActor;
}

UObject* Utils::LoadObjectFromModPackage(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass)
{
	if (!ObjectClass)
	{
		return NULL;
	}

	// Empty PackageName: object is already in memory (e.g. AlwaysLoaded OLGame.upk)
	if (PackageName.IsEmpty())
	{
		UObject* Result = UObject::StaticFindObject(ObjectClass, ANY_PACKAGE, *ObjectName, FALSE);
		if (!Result)
		{
			debugf(TEXT("LoadObjectFromModPackage: '%s' not found in memory"), *ObjectName);
		}
		return Result;
	}

	FString FullPath = FindModPackageFile(PackageName);
	if (FullPath.IsEmpty())
	{
		debugf(TEXT("LoadObjectFromModPackage: '%s' not found in search dirs"), *PackageName);
		return NULL;
	}

	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);

	// GetPackageLinker requires an active load context (GObjBeginLoadCount > 0)
	UObject::BeginLoad();
	ULinkerLoad* Linker = UObject::GetPackageLinker(NULL, *FullPath, LOAD_NoWarn | LOAD_Quiet, NULL, NULL);
	if (!Linker)
	{
		UObject::EndLoad();
		debugf(TEXT("LoadObjectFromModPackage: failed to open linker for '%s'"), *PackageName);
		return NULL;
	}

	UPackage* Pkg = CastChecked<UPackage>(Linker->LinkerRoot);

	// Walk the dot-separated ObjectName to find the export index of each part,
	// then call CreateByOuterIndex only for the final leaf object.
	TArray<FString> Parts;
	ObjectName.ParseIntoArray(&Parts, TEXT("."), TRUE);

	// CurrentOuterIndex is a 0-based ExportMap index; ROOTPACKAGE_INDEX(0) means LinkerRoot.
	// ExportMap.OuterIndex stores 1-based positive for exports, so we compare against
	// (CurrentOuterExportIdx + 1) except for the root where OuterIndex == 0.
	INT CurrentOuterExportIdx = INDEX_NONE; // INDEX_NONE = we're at package root

	for (INT PartIdx = 0; PartIdx < Parts.Num() - 1; PartIdx++)
	{
		FName PartName(*Parts(PartIdx));
		INT Found = INDEX_NONE;
		INT ExpectedOuterIndex = (CurrentOuterExportIdx == INDEX_NONE) ? 0 : (CurrentOuterExportIdx + 1);

		for (INT ExportIdx = 0; ExportIdx < Linker->ExportMap.Num(); ExportIdx++)
		{
			const FObjectExport& Exp = Linker->ExportMap(ExportIdx);
			if (Exp.ObjectName == PartName && Exp.OuterIndex == ExpectedOuterIndex)
			{
				Found = ExportIdx;
				break;
			}
		}

		if (Found == INDEX_NONE)
		{
			// Seekfree packages strip intermediate outers from ExportMap.
			// Fall back to searching from the package root.
			CurrentOuterExportIdx = INDEX_NONE;
			break;
		}
		CurrentOuterExportIdx = Found;
	}

	// CreateByOuterIndex takes a 0-based outer export index (adds +1 internally), or 0 for root
	INT OuterIdxForCreate = (CurrentOuterExportIdx == INDEX_NONE) ? 0 : CurrentOuterExportIdx;
	FName LeafName(*Parts(Parts.Num() - 1));
	UObject* Result = Linker->CreateByOuterIndex(ObjectClass, LeafName, OuterIdxForCreate, LOAD_None, FALSE);
	UObject::EndLoad();

	if (!Result)
	{
		debugf(TEXT("LoadObjectFromModPackage: '%s' not found in '%s'"), *LeafName.ToString(), *ObjectName);
		return NULL;
	}
	return Result;
}

UObject* UOLUtils::LoadObjectFromModPackage(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass)
{
	return Utils::LoadObjectFromModPackage(PackageName, ObjectName, ObjectClass);
}

