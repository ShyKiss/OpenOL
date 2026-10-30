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

static TMap<FString, FString> GModPackagePathCache;

static FString FindModPackageFileImpl(const FString& PackageName)
{
	// Fast path: already resolved this package name before.
	FString* Cached = GModPackagePathCache.Find(PackageName);
	if (Cached) return *Cached;

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
		{
			GModPackagePathCache.Set(PackageName, Found);
			return Found;
		}
	}

	// Cache negative result too so we don't re-scan for missing packages.
	GModPackagePathCache.Set(PackageName, TEXT(""));
	return TEXT("");
}

FString Utils::FindModPackageFile(const FString& PackageName)
{
	return FindModPackageFileImpl(PackageName);
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
	FString FullPath = FindModPackageFileImpl(MapName);
	if (FullPath.IsEmpty())
	{
		// Map is already in the standard search paths — no registration needed
		return TRUE;
	}
	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);
	return TRUE;
}

// ---------------------------------------------------------------------------
// Map replacement — 00_Replace system
// ---------------------------------------------------------------------------
// Layout: CookedPCConsole/Mods/00_Replace/<Prefix>/<Prefix>_<MapName>.upk
//
// Priority (first match wins, alphabetical within each tier):
//   Tier 1 — condition-specific prefixes (e.g. "MP" when multiplayer is active)
//   Tier 2 — any prefix, used when no tier-1 match found AND not in multiplayer
//             (universal single-player mods)

extern UBOOL GMpConn_IsConnected();

static void CollectActiveReplacePrefixes(TArray<FString>& OutPrefixes)
{
	if (GMpConn_IsConnected())
		OutPrefixes.AddUniqueItem(TEXT("MP"));
}

static FString TryFindInPrefix(const FString& Prefix, const FString& MapName, const FString& ReplaceBase)
{
	const FString CandidateName = Prefix + TEXT("_") + MapName;
	const FString CandidatePath = ReplaceBase + Prefix + TEXT("\\") + CandidateName + TEXT(".upk");
	if (GFileManager->FileSize(*CandidatePath) >= 0)
	{
		GPackageFileCache->CachePackage(*CandidatePath, TRUE, FALSE);
		debugf(TEXT("MapReplace: '%s' -> '%s'"), *MapName, *CandidateName);
		return CandidateName;
	}
	return TEXT("");
}

// Tries to find a replacement for MapName under 00_Replace.
// Returns the replacement package name (without extension) or empty string if none found.
static FString FindMapReplacement(const FString& MapName, const FString& ReplaceBase,
                                  const TArray<FString>& PrefixDirs)
{
	TArray<FString> ActivePrefixes;
	CollectActiveReplacePrefixes(ActivePrefixes);

	// Tier 1: condition-specific prefixes (e.g. MP).
	for (INT i = 0; i < PrefixDirs.Num(); ++i)
	{
		if (ActivePrefixes.FindItemIndex(PrefixDirs(i)) == INDEX_NONE)
			continue;
		FString Result = TryFindInPrefix(PrefixDirs(i), MapName, ReplaceBase);
		if (!Result.IsEmpty())
			return Result;
	}

	// Tier 2: any prefix that is NOT condition-specific, only when not in multiplayer.
	if (!GMpConn_IsConnected())
	{
		for (INT i = 0; i < PrefixDirs.Num(); ++i)
		{
			// Skip known condition-specific prefixes — they require an explicit active condition.
			if (PrefixDirs(i) == TEXT("MP"))
				continue;

			FString Result = TryFindInPrefix(PrefixDirs(i), MapName, ReplaceBase);
			if (!Result.IsEmpty())
				return Result;
		}
	}

	return TEXT("");
}

void OLUtils_ApplyMapReplacements(TArray<FName>& InOutLevelNames)
{
	FString CookedPath;
	appGetCookedContentPath(appGetPlatformType(), CookedPath);
	const FString ReplaceBase = CookedPath + TEXT("Mods\\00_Replace\\");

	// Quick exit if the directory doesn't exist or is empty.
	TArray<FString> TopDirs;
	GFileManager->FindFiles(TopDirs, *(ReplaceBase + TEXT("*")), FALSE, TRUE);
	if (TopDirs.Num() == 0)
		return;

	for (INT i = 0; i < InOutLevelNames.Num(); ++i)
	{
		const FString OrigName = InOutLevelNames(i).ToString();
		FString Replacement = FindMapReplacement(OrigName, ReplaceBase, TopDirs);
		if (!Replacement.IsEmpty())
			InOutLevelNames(i) = FName(*Replacement);
	}
}

UBOOL UOLUtils::LoadModPackage(const FString& PackageName)
{
	FString FullPath = FindModPackageFileImpl(PackageName);
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

	FString FullPath = FindModPackageFileImpl(PackageName);
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
	FString FullPath = FindModPackageFileImpl(PackageName);
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
		return UObject::StaticFindObject(ObjectClass, ANY_PACKAGE, *ObjectName, FALSE);
	}

	FString FullPath = FindModPackageFileImpl(PackageName);
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

	// Fallback: locate export by name regardless of outer (handles seekfree-stripped outers).
	if (!Result)
	{
		INT ExactIdx = Linker->FindExportIndex(
			ObjectClass->GetFName(), ObjectClass->GetOuter()->GetFName(),
			LeafName, INDEX_NONE);
		if (ExactIdx != INDEX_NONE)
			Result = Linker->LoadExport(ExactIdx);
	}

	// Last-resort: scan ExportMap by name only, ignoring outer hierarchy.
	if (!Result)
	{
		for (INT xi = 0; xi < Linker->ExportMap.Num(); ++xi)
		{
			if (Linker->ExportMap(xi).ObjectName == LeafName)
			{
				Result = Linker->LoadExport(xi);
				if (Result) break;
			}
		}
	}

	UObject::EndLoad();

	if (!Result)
	{
		debugf(TEXT("LoadObjectFromModPackage: '%s' not found in '%s'"), *LeafName.ToString(), *ObjectName);
		return NULL;
	}
	debugf(TEXT("LoadObjectFromModPackage: pkg=%s obj=%s => %s"), *PackageName, *ObjectName, *Result->GetPathName());
	return Result;
}

UObject* Utils::LoadObjectFromModPackage(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass, ULinkerLoad*& OutLinker)
{
	OutLinker = NULL;
	if (!ObjectClass) return NULL;
	if (PackageName.IsEmpty())
		return UObject::StaticFindObject(ObjectClass, ANY_PACKAGE, *ObjectName, FALSE);

	FString FullPath = FindModPackageFileImpl(PackageName);
	if (FullPath.IsEmpty()) return NULL;
	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);

	UObject::BeginLoad();
	ULinkerLoad* Linker = UObject::GetPackageLinker(NULL, *FullPath, LOAD_NoWarn | LOAD_Quiet, NULL, NULL);
	if (!Linker) { UObject::EndLoad(); return NULL; }

	FName LeafName(*ObjectName);
	INT MeshExpIdx = INDEX_NONE;
	for (INT xi = 0; xi < Linker->ExportMap.Num(); ++xi)
		if (Linker->ExportMap(xi).ObjectName == LeafName) { MeshExpIdx = xi; break; }

	UObject* Result = (MeshExpIdx != INDEX_NONE) ? Linker->LoadExport(MeshExpIdx) : NULL;

	OutLinker = Linker;
	UObject::EndLoad();
	return Result;
}

UObject* UOLUtils::LoadObjectFromModPackage(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass)
{
	return Utils::LoadObjectFromModPackage(PackageName, ObjectName, ObjectClass);
}

// ---------------------------------------------------------------------------
// Async object loader
// ---------------------------------------------------------------------------

struct FAsyncObjRequest
{
	FString                     FullPath;    // absolute path on disk (already cached)
	FString                     ObjectName;
	UClass*                     ObjectClass;
	Utils::FAsyncObjectCallback Callback;
	void*                       UserData;
	FThreadSafeCounter*         SyncObj;     // borrowed from PackagePrecacheMap entry; do not delete
	INT                         TicksWaited; // timeout guard
};

static TArray<FAsyncObjRequest> GAsyncObjQueue;

void Utils::LoadObjectFromModPackageAsync(const FString& PackageName, const FString& ObjectName,
                                          UClass* ObjectClass, FAsyncObjectCallback Callback, void* UserData)
{
	if (!ObjectClass || !Callback) return;

	// Already in memory (AlwaysLoaded package or previously loaded — UE3 keeps objects until GC).
	{
		UObject* Found = UObject::StaticFindObject(ObjectClass, ANY_PACKAGE, *ObjectName, FALSE);
		if (Found) { Callback(Found, UserData); return; }
	}

	FString FullPath = FindModPackageFileImpl(PackageName);
	if (FullPath.IsEmpty())
	{
		debugf(TEXT("LoadObjectFromModPackageAsync: '%s' not found"), *PackageName);
		Callback(NULL, UserData);
		return;
	}

	GPackageFileCache->CachePackage(*FullPath, TRUE, FALSE);

	// If this path is already being loaded (or already buffered), just queue a rider.
	ULinkerLoad::FPackagePrecacheInfo* Existing = ULinkerLoad::PackagePrecacheMap.Find(*FullPath);
	if (!Existing)
	{
		// Allocate precache entry — IO thread fills the buffer, then decrements SyncObj to 0.
		ULinkerLoad::FPackagePrecacheInfo& Info = ULinkerLoad::PackagePrecacheMap.Set(*FullPath, ULinkerLoad::FPackagePrecacheInfo());
		Info.SynchronizationObject = new FThreadSafeCounter;
		Info.SynchronizationObject->Increment();

		const INT FileSize = GFileManager->FileSize(*FullPath);
		if (FileSize <= 0)
		{
			ULinkerLoad::PackagePrecacheMap.Remove(*FullPath);
			debugf(TEXT("LoadObjectFromModPackageAsync: bad file size for '%s'"), *FullPath);
			Callback(NULL, UserData);
			return;
		}

		Info.PackageDataSize = FileSize;
		Info.PackageData     = appMalloc(FileSize);

		FIOSystem* IO = GIOManager->GetIOSystem(IOSYSTEM_GenericAsync);
		IO->LoadData(*FullPath, 0, FileSize, Info.PackageData, Info.SynchronizationObject, AIOP_Normal);

		Existing = ULinkerLoad::PackagePrecacheMap.Find(*FullPath);
	}

	FAsyncObjRequest Req;
	Req.FullPath    = FullPath;
	Req.ObjectName  = ObjectName;
	Req.ObjectClass = ObjectClass;
	Req.Callback    = Callback;
	Req.UserData    = UserData;
	Req.SyncObj     = Existing->SynchronizationObject;
	GAsyncObjQueue.AddItem(Req);
}

void Utils::TickAsyncObjectLoader()
{
	// Process all requests whose IO has completed.
	for (INT i = GAsyncObjQueue.Num() - 1; i >= 0; --i)
	{
		FAsyncObjRequest& Req = GAsyncObjQueue(i);

		// Still reading from disk.
		if (Req.SyncObj->GetValue() != 0) continue;

		// IO done — UObject work from RAM; no disk I/O here.
		UObject* Result = NULL;
		if (!UObject::IsInLoad())
		{
			// Reuse LoadObjectFromModPackage but path is already cached + buffered.
			// GetPackageLinker will find the FBufferReader in PackagePrecacheMap.
			Result = Utils::LoadObjectFromModPackage(TEXT(""), Req.ObjectName, Req.ObjectClass);
			if (!Result)
			{
				// Fallback: package not in AlwaysLoaded — load by full path.
				UObject::BeginLoad();
				ULinkerLoad* Linker = UObject::GetPackageLinker(NULL, *Req.FullPath, LOAD_NoWarn | LOAD_Quiet, NULL, NULL);
				if (Linker)
				{
					TArray<FString> Parts;
					Req.ObjectName.ParseIntoArray(&Parts, TEXT("."), TRUE);
					FName LeafName(*Parts(Parts.Num() - 1));
					for (INT xi = 0; xi < Linker->ExportMap.Num(); ++xi)
					{
						if (Linker->ExportMap(xi).ObjectName == LeafName)
						{
							Result = Linker->LoadExport(xi);
							if (Result) break;
						}
					}
				}
				UObject::EndLoad();
			}
		}

		Req.Callback(Result, Req.UserData);
		GAsyncObjQueue.Remove(i, 1);
	}
}

// ---------------------------------------------------------------------------
// Non-blocking staged mesh loader
//
// FArchiveAsync does its disk read + LZO decompression on the IO thread, but
// FArchiveAsync::Serialize busy-waits (appSleep(0)) whenever the requested range
// is outside the current 128KB precache buffer.  That busy-wait is the whole
// source of the hitching.
//
// Precache() itself never blocks: it kicks off the async read and returns FALSE
// until the data has landed.  So the rule here is simply: never call LoadExport
// until Precache() has returned TRUE for that export's serial range.  The game
// thread then either returns immediately (IO still in flight) or does work that
// is already in RAM.
//
// Memory cost stays at the 2x128KB the engine already keeps; packages are never
// read into RAM whole, and FlushCache() releases even those buffers on reset.
// ---------------------------------------------------------------------------

// Per-package map of mesh export index -> SMC export index that references it.
// Avoids re-loading every SMC in a map package on each preview.
struct FPkgSMCCache
{
	FString          FullPath;
	TArray<INT>      SMCIndices;    // all StaticMeshComponent exports in the package
	TMap<INT, INT>   MeshToSMC;     // mesh export index -> SMC export index
	UBOOL            bScanned;      // SMCIndices filled in

	FPkgSMCCache() : bScanned(FALSE) {}
};

static FPkgSMCCache GPkgSMCCache;

// Per-tick budget for walking StaticMeshComponent exports.  Keeps the frame cost
// bounded while still covering dozens of SMCs in a couple of ticks.
static const DOUBLE SMC_SCAN_BUDGET_SECONDS = 0.002;

// Suppresses synchronous shader compilation for the duration of a scope.
//
// Loading a MaterialInstance runs InitStaticPermutation -> CacheResourceShaders,
// which compiles any static permutation missing from the shader cache right on
// the game thread.  For assets whose permutations were never cooked (the
// "Failed to compile Material Instance ... Default Material will be used"
// warnings) that costs tens to hundreds of milliseconds, which is what makes
// the preview hitch on some meshes and not others.
//
// NOT CURRENTLY USED - too blunt.  GForceMinimalShaderCompilation skips the
// caching for every material in scope, including the ones that would have
// compiled fine, and they then stay without a shader map: enabling it around
// the preview load left much of the game untextured.  A usable fix has to be
// per-material (skip only permutations known to be missing from the cache)
// rather than a global switch.  Kept here for that later investigation.
struct FScopedNoShaderCompile
{
	UBOOL bSaved;
	FScopedNoShaderCompile()  : bSaved(GForceMinimalShaderCompilation) { GForceMinimalShaderCompilation = TRUE; }
	~FScopedNoShaderCompile() { GForceMinimalShaderCompilation = bSaved; }
};

// Returns TRUE once the export's serial range is resident in the loader's
// precache buffer.  Never blocks the game thread.
static UBOOL PrecacheExport(ULinkerLoad* Linker, INT ExportIdx)
{
	if (!Linker || !Linker->Loader) return TRUE;
	if (!Linker->ExportMap.IsValidIndex(ExportIdx)) return TRUE;
	const FObjectExport& Exp = Linker->ExportMap(ExportIdx);
	if (Exp.SerialSize <= 0) return TRUE;
	// Already resident (e.g. object loaded earlier) - nothing to wait for.
	if (Exp._Object && !Exp._Object->HasAnyFlags(RF_NeedLoad)) return TRUE;
	return Linker->Loader->Precache(Exp.SerialOffset, Exp.SerialSize);
}

// Staged state for LoadStaticMeshWithMaterialsStep.
struct FMeshLoadState
{
	enum EStage
	{
		Stage_Idle,
		Stage_PrecacheMesh,
		Stage_LoadMesh,
		Stage_ScanSMC,
		Stage_PrecacheSMC,
		Stage_LoadSMC,
		Stage_Done
	};

	FString      PackageName;
	FString      MeshName;
	FString      FullPath;
	ULinkerLoad* Linker;
	INT          MeshExpIdx;
	INT          SMCCursor;      // index into GPkgSMCCache.SMCIndices
	INT          KnownSMCIdx;    // cached SMC export index, INDEX_NONE if unknown
	UStaticMesh* Mesh;
	EStage       Stage;

	FMeshLoadState()
		: Linker(NULL), MeshExpIdx(INDEX_NONE), SMCCursor(0)
		, KnownSMCIdx(INDEX_NONE), Mesh(NULL), Stage(Stage_Idle) {}
};

static FMeshLoadState GMeshLoad;

// Copies SMC->Materials into the mesh elements that have no material of their own.
static void PatchElementsFromSMC(UStaticMesh* Mesh, UStaticMeshComponent* SMC)
{
	if (!Mesh || !SMC || Mesh->LODModels.Num() == 0) return;
	FStaticMeshRenderData& LOD0 = Mesh->LODModels(0);
	for (INT ei = 0; ei < LOD0.Elements.Num(); ++ei)
	{
		if (LOD0.Elements(ei).Material) continue;
		INT Slot = LOD0.Elements(ei).MaterialIndex;
		UMaterialInterface* Mat = (Slot >= 0 && Slot < SMC->Materials.Num())
			? SMC->Materials(Slot) : NULL;
		if (!Mat && SMC->Materials.Num() > 0) Mat = SMC->Materials(0);
		if (Mat) LOD0.Elements(ei).Material = Mat;
	}
}

// TRUE if every LOD0 element already has a material.
static UBOOL MeshHasAllMaterials(UStaticMesh* Mesh)
{
	if (!Mesh || Mesh->LODModels.Num() == 0) return TRUE;
	const FStaticMeshRenderData& LOD0 = Mesh->LODModels(0);
	for (INT ei = 0; ei < LOD0.Elements.Num(); ++ei)
		if (!LOD0.Elements(ei).Material) return FALSE;
	return TRUE;
}

void Utils::BeginStaticMeshLoad(const FString& PackageName, const FString& MeshName)
{
	GMeshLoad             = FMeshLoadState();
	GMeshLoad.PackageName = PackageName;
	GMeshLoad.MeshName    = MeshName;
	GMeshLoad.Stage       = FMeshLoadState::Stage_PrecacheMesh;
}

// Abandon an in-flight load.  Deliberately does NOT call FlushCache(): that waits
// on outstanding IO requests and would reintroduce the very stall we are avoiding.
// The precache buffers are reused by the next load or freed with the linker.
void Utils::ResetStaticMeshLoad()
{
	GMeshLoad = FMeshLoadState();
}

// Release the loader's precache buffers.  Only safe to call when no load is in
// flight (it blocks on pending IO), so use it when closing the preview UI.
void Utils::FlushStaticMeshLoadCache()
{
	if (GMeshLoad.Linker && GMeshLoad.Linker->Loader)
		GMeshLoad.Linker->Loader->FlushCache();
	GMeshLoad = FMeshLoadState();
}

// Advances the staged load by one step.  Returns TRUE when finished (OutMesh is
// then the result, possibly NULL on failure); FALSE means "IO still pending,
// call again next tick".  Never blocks.
UBOOL Utils::StepStaticMeshLoad(UStaticMesh*& OutMesh)
{
	OutMesh = NULL;

	// Any stage that finishes routes through Stage_Done so the result is
	// reported from exactly one place.
	if (GMeshLoad.Stage == FMeshLoadState::Stage_Done)
	{
		OutMesh = GMeshLoad.Mesh;
		return TRUE;
	}

	switch (GMeshLoad.Stage)
	{
	case FMeshLoadState::Stage_Idle:
		return TRUE;

	case FMeshLoadState::Stage_PrecacheMesh:
	{
		if (UObject::IsInLoad()) return FALSE;

		GMeshLoad.FullPath = FindModPackageFileImpl(GMeshLoad.PackageName);
		if (GMeshLoad.FullPath.IsEmpty()) { GMeshLoad.Stage = FMeshLoadState::Stage_Done; return FALSE; }
		GPackageFileCache->CachePackage(*GMeshLoad.FullPath, TRUE, FALSE);

		UObject::BeginLoad();
		GMeshLoad.Linker = UObject::GetPackageLinker(
			NULL, *GMeshLoad.FullPath, LOAD_NoWarn | LOAD_Quiet, NULL, NULL);
		UObject::EndLoad();
		if (!GMeshLoad.Linker) { GMeshLoad.Stage = FMeshLoadState::Stage_Done; return FALSE; }

		// Locate the mesh export once.
		static const FName NAME_SM(TEXT("StaticMesh"));
		FName LeafName(*GMeshLoad.MeshName.Mid(GMeshLoad.MeshName.InStr(TEXT("."), TRUE) + 1));
		for (INT xi = 0; xi < GMeshLoad.Linker->ExportMap.Num(); ++xi)
		{
			const FObjectExport& Ex = GMeshLoad.Linker->ExportMap(xi);
			if (Ex.ObjectName != LeafName) continue;
			FName CN;
			if (Ex.ClassIndex < 0) CN = GMeshLoad.Linker->ImportMap(-Ex.ClassIndex-1).ObjectName;
			else if (Ex.ClassIndex > 0) CN = GMeshLoad.Linker->ExportMap(Ex.ClassIndex-1).ObjectName;
			if (CN == NAME_SM) { GMeshLoad.MeshExpIdx = xi; break; }
		}
		if (GMeshLoad.MeshExpIdx == INDEX_NONE) { GMeshLoad.Stage = FMeshLoadState::Stage_Done; return FALSE; }

		GMeshLoad.Stage = FMeshLoadState::Stage_LoadMesh;
		return FALSE;
	}

	case FMeshLoadState::Stage_LoadMesh:
	{
		if (UObject::IsInLoad()) return FALSE;
		// Wait (without blocking) until the mesh bytes are in the precache buffer.
		if (!PrecacheExport(GMeshLoad.Linker, GMeshLoad.MeshExpIdx)) return FALSE;

		UObject::BeginLoad();
		GMeshLoad.Mesh = Cast<UStaticMesh>(GMeshLoad.Linker->LoadExport(GMeshLoad.MeshExpIdx));
		UObject::EndLoad();

		// A mesh whose export was created but not serialized yet (abandoned load,
		// RF_NeedLoad still set) has no LODModels.  Handing it to the preview trips
		// check(LODModels.Num() >= 1) in UStaticMeshComponent::GetNumElements, so
		// force a blocking Preload to finish it rather than publish a shell.
		if (GMeshLoad.Mesh && GMeshLoad.Mesh->HasAnyFlags(RF_NeedLoad)
		    && GMeshLoad.Mesh->GetLinker())
		{
			UObject::BeginLoad();
			GMeshLoad.Mesh->GetLinker()->Preload(GMeshLoad.Mesh);
			UObject::EndLoad();
		}
		if (GMeshLoad.Mesh && GMeshLoad.Mesh->LODModels.Num() == 0)
			GMeshLoad.Mesh = NULL;

		if (!GMeshLoad.Mesh || MeshHasAllMaterials(GMeshLoad.Mesh))
		{
			GMeshLoad.Stage = FMeshLoadState::Stage_Done;
			return FALSE;
		}
		GMeshLoad.Stage = FMeshLoadState::Stage_ScanSMC;
		return FALSE;
	}

	case FMeshLoadState::Stage_ScanSMC:
	{
		// Reuse the cached SMC scan when we are still on the same package.
		if (GPkgSMCCache.FullPath != GMeshLoad.FullPath)
		{
			GPkgSMCCache = FPkgSMCCache();
			GPkgSMCCache.FullPath = GMeshLoad.FullPath;
		}

		if (!GPkgSMCCache.bScanned)
		{
			static const FName NAME_SMC(TEXT("StaticMeshComponent"));
			for (INT xi = 0; xi < GMeshLoad.Linker->ExportMap.Num(); ++xi)
			{
				const FObjectExport& Ex = GMeshLoad.Linker->ExportMap(xi);
				FName CN;
				if (Ex.ClassIndex < 0) CN = GMeshLoad.Linker->ImportMap(-Ex.ClassIndex-1).ObjectName;
				else if (Ex.ClassIndex > 0) CN = GMeshLoad.Linker->ExportMap(Ex.ClassIndex-1).ObjectName;
				if (CN == NAME_SMC) GPkgSMCCache.SMCIndices.AddItem(xi);
			}
			GPkgSMCCache.bScanned = TRUE;
		}

		// Known mapping from a previous preview: go straight to that SMC.
		INT* Cached = GPkgSMCCache.MeshToSMC.Find(GMeshLoad.MeshExpIdx);
		GMeshLoad.KnownSMCIdx = Cached ? *Cached : INDEX_NONE;
		GMeshLoad.SMCCursor   = 0;
		GMeshLoad.Stage       = FMeshLoadState::Stage_PrecacheSMC;
		return FALSE;
	}

	case FMeshLoadState::Stage_PrecacheSMC:
	case FMeshLoadState::Stage_LoadSMC:
	{
		if (UObject::IsInLoad()) return FALSE;

		// Walk SMCs within a per-tick time budget instead of one per tick: a map
		// package holds dozens of them, and stepping one at a time would stretch a
		// single preview across a second or more of ticks.
		const DOUBLE StartTime = appSeconds();

		while (appSeconds() - StartTime < SMC_SCAN_BUDGET_SECONDS)
		{
			INT SMCIdx = GMeshLoad.KnownSMCIdx;
			if (SMCIdx == INDEX_NONE)
			{
				if (GMeshLoad.SMCCursor >= GPkgSMCCache.SMCIndices.Num())
				{
					// No SMC references this mesh - keep it without materials.
					GMeshLoad.Stage = FMeshLoadState::Stage_Done;
					return FALSE;
				}
				SMCIdx = GPkgSMCCache.SMCIndices(GMeshLoad.SMCCursor);
			}

			// Chunk not resident yet - yield the frame, resume next tick.
			if (!PrecacheExport(GMeshLoad.Linker, SMCIdx))
			{
				GMeshLoad.Stage = FMeshLoadState::Stage_PrecacheSMC;
				return FALSE;
			}

			UObject::BeginLoad();
			UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(GMeshLoad.Linker->LoadExport(SMCIdx));
			UObject::EndLoad();

			if (SMC && SMC->StaticMesh == GMeshLoad.Mesh && SMC->Materials.Num() > 0)
			{
				PatchElementsFromSMC(GMeshLoad.Mesh, SMC);
				GPkgSMCCache.MeshToSMC.Set(GMeshLoad.MeshExpIdx, SMCIdx);
				GMeshLoad.Stage = FMeshLoadState::Stage_Done;
				return FALSE;
			}

			// Cached mapping went stale - fall back to a full scan.
			if (GMeshLoad.KnownSMCIdx != INDEX_NONE)
			{
				GPkgSMCCache.MeshToSMC.Remove(GMeshLoad.MeshExpIdx);
				GMeshLoad.KnownSMCIdx = INDEX_NONE;
				GMeshLoad.SMCCursor   = 0;
			}
			else
			{
				++GMeshLoad.SMCCursor;
			}
		}

		GMeshLoad.Stage = FMeshLoadState::Stage_PrecacheSMC;
		return FALSE;
	}

	case FMeshLoadState::Stage_Done:
	default:
		OutMesh = GMeshLoad.Mesh;
		return TRUE;
	}
}

// Load a UStaticMesh by name, patching Elements[].Material from any SMC in the same
// package that references this mesh.  Works for art packages (material in mesh) and
// map packages (material only in StaticMeshComponent::Materials).
// Blocking variant, kept for callers outside the preview path.
UStaticMesh* Utils::LoadStaticMeshWithMaterials(const FString& PackageName, const FString& MeshName)
{
	// Load the mesh via the established path.
	UStaticMesh* Result = Cast<UStaticMesh>(
		LoadObjectFromModPackage(PackageName, MeshName, UStaticMesh::StaticClass()));
	if (!Result) return NULL;

	// If Elements already carry materials, nothing more to do.
	if (Result->LODModels.Num() == 0) return Result;
	FStaticMeshRenderData& LOD0 = Result->LODModels(0);
	UBOOL bNeedsPatch = FALSE;
	for (INT ei = 0; ei < LOD0.Elements.Num(); ++ei)
		if (!LOD0.Elements(ei).Material) { bNeedsPatch = TRUE; break; }
	if (!bNeedsPatch) return Result;

	// Elements have no material: this is a map package where materials live in SMC.
	// Open the linker, find all SMC exports, load them one by one until we find
	// one whose StaticMesh matches ours, then copy its Materials into Elements.
	FString FullPath = FindModPackageFileImpl(PackageName);
	if (FullPath.IsEmpty()) return Result;

	UObject::BeginLoad();
	ULinkerLoad* Linker = UObject::GetPackageLinker(NULL, *FullPath, LOAD_NoWarn | LOAD_Quiet, NULL, NULL);
	if (!Linker) { UObject::EndLoad(); return Result; }

	static const FName NAME_SMC(TEXT("StaticMeshComponent"));
	TArray<INT> SMCIndices;
	for (INT xi = 0; xi < Linker->ExportMap.Num(); ++xi)
	{
		const FObjectExport& Ex = Linker->ExportMap(xi);
		FName CN;
		if (Ex.ClassIndex < 0) CN = Linker->ImportMap(-Ex.ClassIndex-1).ObjectName;
		else if (Ex.ClassIndex > 0) CN = Linker->ExportMap(Ex.ClassIndex-1).ObjectName;
		if (CN == NAME_SMC) SMCIndices.AddItem(xi);
	}

	// Load all SMCs in one pass so EndLoad resolves all cross-references at once.
	for (INT i = 0; i < SMCIndices.Num(); ++i)
		Linker->LoadExport(SMCIndices(i));
	UObject::EndLoad();

	for (INT i = 0; i < SMCIndices.Num(); ++i)
	{
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(
			Linker->ExportMap(SMCIndices(i))._Object);
		if (!SMC || SMC->StaticMesh != Result || SMC->Materials.Num() == 0)
			continue;

		for (INT ei = 0; ei < LOD0.Elements.Num(); ++ei)
		{
			if (LOD0.Elements(ei).Material) continue;
			INT Slot = LOD0.Elements(ei).MaterialIndex;
			UMaterialInterface* Mat = (Slot >= 0 && Slot < SMC->Materials.Num())
				? SMC->Materials(Slot) : NULL;
			if (!Mat && SMC->Materials.Num() > 0) Mat = SMC->Materials(0);
			if (Mat) LOD0.Elements(ei).Material = Mat;
		}
		break;
	}

	return Result;
}

