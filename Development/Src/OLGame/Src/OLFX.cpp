#include "OLGame.h"
#include "OLUtilities.h"

#include "../../Engine/Src/DynamicLightEnvironmentComponent.h"

IMPLEMENT_CLASS(UOLFXManager);
IMPLEMENT_CLASS(UOLInterpTrackInstPPSEffectParam);
IMPLEMENT_CLASS(UOLInterpTrackPPSEffectParam);
IMPLEMENT_CLASS(AOLFXHolder);

//////////////////////////////////////////////////////////////////////////
// UOLFXManager
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UOLFXManager* UOLFXManager::GetFXManager()
{
	return Utils::GetFXManager();
}

void UOLFXManager::Init()
{
	FXHolder = Cast<AOLFXHolder>(GWorld->SpawnActor(AOLFXHolder::StaticClass(), FName(TEXT("FXHolder"))));
	check(FXHolder);
}

void UOLFXManager::Tick(FLOAT deltaTime)
{
	if (CurrentBlur.bActive)
	{
		if (CurrentBlur.Duration > 0.0f && GWorld->GetTimeSeconds() > CurrentBlur.StartTime + CurrentBlur.Duration)
		{
			appMemZero(CurrentBlur);
		}
	}

	BindToCurrentUberPostProcess();
	UpdateSwarmBlur(deltaTime);
	UpdateCameraGlitch(deltaTime);
	UpdateShatteredGlass(deltaTime);
	UpdateMovieEffect(deltaTime);
}

void UOLFXManager::BindToCurrentUberPostProcess()
{
	if (Utils::GetOLPC())
	{
		ULocalPlayer* LP = Cast<ULocalPlayer>(Utils::GetOLPC()->Player);
		if (LP && LP->PlayerPostProcess)
		{
			CurrentUberPostEffect = Cast<UOLUberPostProcessEffect>(LP->PlayerPostProcess->FindPostProcessEffect(UberPostEffectName));
		}
		else
		{
			CurrentUberPostEffect = NULL;
		}
	}
	else
	{
		CurrentUberPostEffect = NULL;
	}
	if (bGrainDisabled && CurrentUberPostEffect)
		CurrentUberPostEffect->GrainOpacity = 0.0f;
}

UBOOL UOLFXManager::AreEffectsEnabled() const
{
	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC && (OLPC->bDebugFixedCam || OLPC->bDebugFreeCam) && !(OLPC->bDebugGhost))
	{
		return FALSE;
	}

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if (OLPC)
	{
		ULocalPlayer* LP = Cast<ULocalPlayer>(OLPC->Player);
		if (LP && LP->ViewportClient && (LP->ViewportClient->ShowFlags & SHOW_ViewMode_Lit) != SHOW_ViewMode_Lit)
		{
			return FALSE; // No PPS if not lit
		}
	}
#endif

	return TRUE;
}

void UOLFXManager::GetPostProcessSettings(FPostProcessSettings& PPSettings) const
{
	if (!AreEffectsEnabled())
	{
		return;
	}

	const FPostProcessSettings* ppsSettings = NULL;

	switch (CurrentPPSMode)
	{
	case PPS_Default:
		break;
	case PPS_Camcorder:
		break;
	case PPS_NightVision:
		ppsSettings = &NVPPSSettings;
		break;
	case PPS_Death:
		ppsSettings = &DeathPPSSettings;
		break;
	case PPS_GammaCalibration:
		break;
	default:
		check(0);
	}

	if (ppsSettings)
	{
		PPSettings = *ppsSettings;
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC && OLPC->HUD && OLPC->HUD->eventIsInPauseMenu())
	{
		TWEAKABLE FLOAT PauseBlurSize = 32.0f;

		PPSettings.bOverride_EnableDOF = TRUE;
		PPSettings.bOverride_DOF_BlurKernelSize = TRUE;
		PPSettings.bOverride_DOF_MaxNearBlurAmount = TRUE;
		PPSettings.bOverride_DOF_MaxFarBlurAmount = TRUE;
		PPSettings.bOverride_DOF_MinBlurAmount = TRUE;
		PPSettings.DOF_MinBlurAmount = 1.0f;
		PPSettings.DOF_MaxNearBlurAmount  = 1.0f;
		PPSettings.DOF_MaxFarBlurAmount = 1.0f;			
		PPSettings.DOF_BlurKernelSize = PauseBlurSize;
		PPSettings.bEnableDOF = TRUE;
	}
	else
	{
		UBOOL bBlurAllowed = (CurrentPPSMode == PPS_Default || CurrentPPSMode == PPS_NightVision || CurrentPPSMode == PPS_Camcorder);
		UBOOL bBlurEnabled = bBlurAllowed && (CurrentBlur.bActive || HeatBlur.bActive);

		FLOAT currentBlurAmount = 0.0f;
		FLOAT effectiveDesaturation = 0.0f;

		if (CurrentBlur.bActive)
		{
			FLOAT elapsedTime = (GWorld->GetTimeSeconds() - CurrentBlur.StartTime);

			FLOAT fadeInAlpha = 1.0f;
			FLOAT fadeOutAlpha = 1.0f;
		
			if (CurrentBlur.BlendInTime > 0.0f && elapsedTime < CurrentBlur.BlendInTime)
			{
				// blend in
				fadeInAlpha = Utils::SmootherStep(Clamp(elapsedTime / CurrentBlur.BlendInTime, 0.0f, 1.0f));
			}
		
			if (CurrentBlur.BlendOutTime > 0.0f && CurrentBlur.Duration > 0.0f && elapsedTime > CurrentBlur.Duration - CurrentBlur.BlendOutTime)
			{
				// blend out
				fadeOutAlpha = 1.0f - Utils::SmootherStep(Clamp((elapsedTime - (CurrentBlur.Duration - CurrentBlur.BlendOutTime)) / CurrentBlur.BlendOutTime, 0.0f, 1.0f));
			}

			FLOAT alpha = Min(fadeInAlpha, fadeOutAlpha);
			currentBlurAmount = alpha * CurrentBlur.Amount;
			effectiveDesaturation = CurrentBlur.Desaturation;
		}

		if (HeatBlur.bActive)
		{
			currentBlurAmount = Max(currentBlurAmount, HeatBlur.Amount);
			effectiveDesaturation = Max(effectiveDesaturation, HeatBlur.Desaturation);
		}

		if (bSwarmBlurActive)
		{
			currentBlurAmount = Max(currentBlurAmount, SwarmBlurAmount);
		}

		if (currentBlurAmount > 0.01f)
		{
			UBOOL bWasDofEnabled = PPSettings.bOverride_EnableDOF && PPSettings.bEnableDOF;
			FLOAT prevDesat = PPSettings.bOverride_Scene_Desaturation ? PPSettings.Scene_Desaturation : GWorld->GetWorldInfo()->DefaultPostProcessSettings.Scene_Desaturation;
			PPSettings.bOverride_EnableDOF = TRUE;		
			PPSettings.bOverride_DOF_BlurKernelSize = TRUE;
			PPSettings.bOverride_DOF_MaxNearBlurAmount = TRUE;
			PPSettings.bOverride_DOF_MaxFarBlurAmount = TRUE;
			PPSettings.bOverride_DOF_MinBlurAmount = TRUE;
			PPSettings.bOverride_Scene_Desaturation = PPSettings.bOverride_Scene_Desaturation || !appIsNearlyZero(effectiveDesaturation, KINDA_SMALL_NUMBERF);
			PPSettings.bEnableDOF = TRUE;

			PPSettings.Scene_Desaturation = Lerp(prevDesat, effectiveDesaturation, currentBlurAmount);
			PPSettings.DOF_MinBlurAmount = bWasDofEnabled ? Lerp(PPSettings.DOF_MinBlurAmount, currentBlurAmount, currentBlurAmount) : currentBlurAmount;
			PPSettings.DOF_MaxNearBlurAmount  = bWasDofEnabled ? Lerp(PPSettings.DOF_MaxNearBlurAmount, currentBlurAmount, currentBlurAmount) : currentBlurAmount;
			PPSettings.DOF_MaxFarBlurAmount = bWasDofEnabled ? Lerp(PPSettings.DOF_MaxFarBlurAmount, currentBlurAmount, currentBlurAmount) : currentBlurAmount;	
			PPSettings.DOF_BlurKernelSize = bWasDofEnabled ? Lerp(PPSettings.DOF_BlurKernelSize, 32.0f, currentBlurAmount) : 32.0f;
		}
	}
}

void UOLFXManager::SetPPSFromScript(BYTE newPPS)
{
	SetPPS((EPPSMode)newPPS);
}

void UOLFXManager::SetPPS(EPPSMode newPPS)
{
	ULocalPlayer* LP = Cast<ULocalPlayer>(Utils::GetOLPC()->Player);
	if (!LP)
	{
		return;
	}

	CurrentPPSMode = newPPS;

	TWEAKABLE UBOOL AllowCamcorderPPS = TRUE;
	UBOOL bAllowCamcorderPPS = AllowCamcorderPPS;
#if _DEBUG
	if (GWorld && Utils::GetCheatManager() && Utils::GetCheatManager()->bCheatsEnabled)
	{
		// cheat - I don't want the PPS in my gym
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		check(WorldInfo);
		if (WorldInfo->WorldPostProcessChain == NULL)
		{
			bAllowCamcorderPPS = FALSE;
		}
	}
#endif

	UPostProcessChain* ppsChain = NULL;

	switch (newPPS)
	{
	case PPS_Default:
	case PPS_Death:
		ppsChain = DefaultPPSChain;
		break;
	case PPS_Camcorder:
		if (bAllowCamcorderPPS)
		{
#if ORBIS
			// On console we don't have motion blur, so we do the camera color 
			// in the uber shader so we can optimize more.
			ppsChain = CamcorderPPSChainConsole;
#else
			// On PC the camera color is a material effect.
			ppsChain = CamcorderPPSChain;
#endif
		}
		break;
	case PPS_NightVision:
		ppsChain = NVPPSChain;
		break;
	case PPS_GammaCalibration:
		ppsChain = GammaCalibrationPPSChain;
		break;
	default:
		check(0);
	}

	LP->RemoveAllPostProcessingChains();

	CurrentUberPostEffect = NULL;
	if (ppsChain)
	{
		LP->InsertPostProcessingChain(ppsChain, INDEX_NONE, TRUE);
		// The chain is cloned internally in the player, so the cloned one is
		// the one we want to modify.
		if (LP->PlayerPostProcess)
		{
			CurrentUberPostEffect = Cast<UOLUberPostProcessEffect>(LP->PlayerPostProcess->FindPostProcessEffect(UberPostEffectName));
		}
	}

	if (bGrainDisabled && CurrentUberPostEffect)
		CurrentUberPostEffect->GrainOpacity = 0.0f;

	if (newPPS != PPS_GammaCalibration)
	{
		SetEffectVisibility(CameraGlitchEffectName, CameraGlitch.bActive);
		if (CurrentUberPostEffect)
		{
			CurrentUberPostEffect->MovieIntensity = MovieEffect.bActive ? MovieEffect.Intensity : 0.0f;
		}
		UpdateHurtAndShatteredGlassVisibility();
	}
}

void UOLFXManager::ResetEffects()
{	
	if (CurrentUberPostEffect)
	{
		CurrentUberPostEffect->HurtAmount = 0.0f;
		CurrentUberPostEffect->CameraColorEffect = 1.0f;
	}

	if (CameraGlitchMat)
	{
		CameraGlitchMat->SetScalarParameterValue(CameraGlitchParamName, 0.0f);
	}

	CamcorderPPSOpacity = 1.0f;
	CurrentHurtEffect = 0.0f;
	LastSetHurtEffect = 0.0f;
	bShatteredGlassEffectActive = FALSE;

	appMemZero(CameraGlitch);
	CameraGlitch.NextGlitchDelay = 10.0f;

	SetEffectVisibility(CameraGlitchEffectName, FALSE);

	DeactivateNightVisionEffect();

	ResetMovieEffect();
	
	UpdateHurtAndShatteredGlassVisibility();
	
	KillBlur();
}

void UOLFXManager::UpdateHurtAndShatteredGlassVisibility()
{
	if (CurrentUberPostEffect)
	{
		CurrentUberPostEffect->bCameraGlassShattered = bShatteredGlassEffectActive;
		CurrentUberPostEffect->HurtAmount = CurrentHurtEffect;
	}
}

void UOLFXManager::UpdateHurtEffect(FLOAT deltaTime, FLOAT hurtEffect)
{
	if (!CurrentUberPostEffect)
	{
		return;
	}

	UBOOL wasOn = !appIsNearlyZero(CurrentHurtEffect, KINDA_SMALL_NUMBERF);

	if (!AreEffectsEnabled())
	{
		CurrentHurtEffect = 0.0f;
	}
	else
	{	
		TWEAKABLE FLOAT EffectApproachCoeff = 0.999f;
		CurrentHurtEffect = Utils::Approach(CurrentHurtEffect, hurtEffect, EffectApproachCoeff, deltaTime);
	}

	if (CurrentHurtEffect < 0.01f)
	{
		CurrentHurtEffect = 0.0f;
	}

	if (!appIsNearlyEqual(LastSetHurtEffect, CurrentHurtEffect, 0.005f))
	{
		CurrentUberPostEffect->HurtAmount = CurrentHurtEffect;
		LastSetHurtEffect = CurrentHurtEffect;
	}

	UBOOL isOn = !appIsNearlyZero(CurrentHurtEffect, KINDA_SMALL_NUMBERF);

	if (wasOn != isOn)
	{
		UpdateHurtAndShatteredGlassVisibility();
	}
}

void UOLFXManager::UpdateShatteredGlass(FLOAT deltaTime)
{
	AOLHero* hero = Utils::GetHero();

	if (!hero || !CurrentUberPostEffect)
	{
		return;
	}

	UBOOL bActive = AreEffectsEnabled() && (GSystemSettings.DetailMode > DM_Low) && hero->bCameraCracked && hero->IsCamcorderActive();

	if (bActive)
	{
		if (!hero->LightEnvironment || !hero->LightEnvironment->State)
		{
			bActive = FALSE;
		}
	}

	if (bActive)
	{
		FSHVectorRGB dleCRNS = hero->LightEnvironment->State->GetCurrentRepresentativeNonShadowedLightEnvironment();
		const FSHVector unitLightSH = SHBasisFunction(hero->EyeForward);
		FLinearColor intensity = Dot(dleCRNS, unitLightSH) / Dot(unitLightSH, unitLightSH);
		FLinearColor posIntensity(Max(intensity.R,0.0f), Max(intensity.G,0.0f), Max(intensity.B,0.0f), 1.0f);
		FLOAT totalLuminance = 100.0f*posIntensity.ComputeLuminance();

		CurrentUberPostEffect->CameraGlassLightColor = posIntensity;

		TWEAKABLE FLOAT MinLuminance = 0.0f;
		TWEAKABLE FLOAT MaxLuminance = 80.0f;
		FLOAT lightIntensity = MapClamped(totalLuminance, MinLuminance, MaxLuminance, 0.0f, 1.0f);
		CurrentUberPostEffect->CameraGlassLightIntensity = lightIntensity;
	}

	if (bActive != bShatteredGlassEffectActive)
	{
		bShatteredGlassEffectActive = bActive;
		UpdateHurtAndShatteredGlassVisibility();
	}
}

void UOLFXManager::SetEffectVisibility(FName effectName, UBOOL bVisible)
{
	AOLPlayerController* OLPC = Utils::GetOLPC();
	check(OLPC);
	ULocalPlayer* LP = Cast<ULocalPlayer>(OLPC->Player);
	check(LP);
	UPostProcessChain* ppsChain = LP->GetPostProcessChain(0);
	
	if (ppsChain)
	{
		UPostProcessEffect* effect = ppsChain->FindPostProcessEffect(effectName);

		if (effect)
		{
			effect->bShowInGame = bVisible;
		}
	}
}

void UOLFXManager::TriggerBlur(FLOAT amount, FLOAT duration, FLOAT desaturation, FLOAT blendInTime, FLOAT blendOutTime)
{
	if (amount > 0.0f && duration > 0.0f)
	{
		CurrentBlur.bActive = TRUE;
		CurrentBlur.Duration = duration;
		CurrentBlur.Amount = amount;
		CurrentBlur.StartTime = GWorld->GetTimeSeconds();
		CurrentBlur.BlendInTime = blendInTime;
		CurrentBlur.BlendOutTime = blendOutTime;
		CurrentBlur.Desaturation = desaturation;
	}
}

void UOLFXManager::SetHeatBlur(FLOAT amount, FLOAT desaturation)
{
	amount = amount > 0.05f ? amount : 0.0f;
	HeatBlur.bActive = amount > 0.0f;
	HeatBlur.Amount = amount;
	HeatBlur.Desaturation = desaturation;
	HeatBlur.Duration = -1.0f;
	HeatBlur.StartTime = -1.0f;
	HeatBlur.BlendInTime = -1.0f;
	HeatBlur.BlendOutTime = -1.0f;
}

void UOLFXManager::KillBlur()
{
	appMemZero(CurrentBlur);
	appMemZero(HeatBlur);
	bSwarmBlurActive = FALSE;
	SwarmBlurAmount = 0.0f;
}

void UOLFXManager::UpdateSwarmBlur(FLOAT deltaTime)
{
	TWEAKABLE FLOAT MaxSwarmBlur = 0.85f;
	TWEAKABLE FLOAT DistForMaxBlur = 1000.0f;
	TWEAKABLE FLOAT DistForMinBlur = 2000.0f;
	APawn* closestSwarm = NULL;
	FLOAT closestDist = FLT_MAX;
	AOLHero* hero = Utils::GetHero();

	if (hero && !hero->IsInNightVision())
	{
		for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
		{
			if (pawn && pawn->IsA(AOLEnemyNanoCloud::StaticClass()))
			{
				FLOAT distToSwarm = hero->Location.Distance(pawn->Location);
				if (distToSwarm < closestDist)
				{
					closestDist = distToSwarm;
					closestSwarm = pawn;
				}
			}
		}
	}

	if (closestSwarm)
	{
		FLOAT cosAngle = Saturate(hero->EyeForward | (closestSwarm->Location - hero->Location).SafeNormal());
		SwarmBlurAmount = cosAngle * MapClamped(closestDist, DistForMaxBlur, DistForMinBlur, MaxSwarmBlur, 0.0f);

		FLOAT lifeTimeScale = MapClamped(GWorld->GetTimeSeconds() - closestSwarm->SpawnTime, 0.0f, 3.0f, 0.0f, 1.0f);
		SwarmBlurAmount *= lifeTimeScale;
		bSwarmBlurActive = TRUE;		
	}
	else
	{
		SwarmBlurAmount = 0.0f;
		bSwarmBlurActive = FALSE;
	}
}

void UOLFXManager::TriggerElectricSparks(const FVector& location, const FVector& normal)
{
	check(FXHolder);
	ElectricSparksParticles->ConditionalDetach();
	FXHolder->AttachComponent(ElectricSparksParticles);
	ElectricSparksParticles->Translation = location;
	ElectricSparksParticles->Rotation = normal.Rotation();
	ElectricSparksParticles->ActivateSystem();
}

void UOLFXManager::ResetMovieEffect()
{
	if (MovieEffect.bActive && MovieEffect.SndEventStop && Utils::GetOLPC())
	{
		Utils::GetOLPC()->PostAkEvent(MovieEffect.SndEventStop);
	}

	if (CurrentUberPostEffect)
	{
		CurrentUberPostEffect->MovieIntensity = 0.0f;
	}

	MovieEffect.bActive = FALSE;	

	UBOOL bSwarmActive = FALSE;
	if (GWorld && GWorld->GetWorldInfo())
	{
		for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
		{
			if (pawn && pawn->IsA(AOLEnemyNanoCloud::StaticClass()))
			{
				bSwarmActive = TRUE;
				break;
			}
		}
	}

	if (!bSwarmActive && MovieEffect.Movie)
	{
		MovieEffect.Movie->Stop();
	}

	MovieEffect.Movie = NULL;
}

void UOLFXManager::TriggerMovieEffect(const UOLSeqAct_MovieEffect* movieEffectParams)
{
	if (GSystemSettings.DetailMode == DM_Low)
	{
		return;
	}

	MovieEffect.StartedTime = GWorld->GetTimeSeconds();
	MovieEffect.Movie = movieEffectParams->Movie;
	MovieEffect.Intensity = movieEffectParams->ScaleIntensity;
	MovieEffect.SndEventStop = movieEffectParams->SndEventStop;

	INT animId = (movieEffectParams->AnimType == MEA_Random) ? RandHelper(3) : (movieEffectParams->AnimType - 1);
	MovieEffect.Anim = (animId == 0 ? movieEffectParams->Anim1 : (animId == 2 ? movieEffectParams->Anim2 : movieEffectParams->Anim3));

	if (MovieEffect.Movie)
	{
		if (movieEffectParams->bStartAtRandomFrame)
		{
			MovieEffect.Movie->PlayRandom();
		}
		else
		{
			MovieEffect.Movie->Play();
		}
	}	

	MovieEffect.Duration = 2.0f; // max time of any anim
	MovieEffect.bActive = TRUE;

	if (CurrentUberPostEffect)
	{
		CurrentUberPostEffect->MovieIntensity = MovieEffect.Intensity;
	}

	if (movieEffectParams->SndEventPlay && Utils::GetOLPC())
	{
		Utils::GetOLPC()->PostAkEvent(movieEffectParams->SndEventPlay);
	}
}

void UOLFXManager::UpdateMovieEffect(FLOAT deltaTime)
{	
	if (MovieEffect.bActive)
	{
		if (!AreEffectsEnabled() || (GSystemSettings.DetailMode == DM_Low) || (GWorld->GetTimeSeconds() - MovieEffect.StartedTime) > MovieEffect.Duration)
		{
			ResetMovieEffect();
		}
	}

	FLOAT currentIntensity = 0.0f;

	if (MovieEffect.bActive)
	{
		FLOAT elapsedTime = GWorld->GetTimeSeconds() - MovieEffect.StartedTime;
				
		for (INT i = 1; i < MovieEffect.Anim.Num() - 1; i++)
		{
			if (MovieEffect.Anim(i).X > elapsedTime)
			{
				FLOAT leftTime = MovieEffect.Anim(i-1).X;
				FLOAT rightTime = MovieEffect.Anim(i).X;
				FLOAT leftVal = MovieEffect.Anim(i-1).Y;
				FLOAT rightVal = MovieEffect.Anim(i).Y;

				currentIntensity = MapClamped(elapsedTime, leftTime, rightTime, leftVal, rightVal);
				currentIntensity = Saturate(currentIntensity * MovieEffect.Intensity);
				break;
			}
		}

		if (CurrentUberPostEffect)
		{
			CurrentUberPostEffect->MovieIntensity = currentIntensity;
		}
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();	
	if (AudioDevice && MovieEffectRTPC != NAME_None)
	{
		AudioDevice->SetRTPCValue(*MovieEffectRTPC.ToString(), currentIntensity * 100.0f, NULL);
	}
}

UBOOL UOLFXManager::IsPlayingMovieEffect()
{
	if (GWorld)
	{
		return MovieEffect.bActive;
	}

	return FALSE;
}

void UOLFXManager::UpdateCameraGlitch(FLOAT deltaTime)
{
	AOLHero* hero = Utils::GetHero();

	if (!hero || !CameraGlitchMat)
	{
		return;
	}

	TWEAKABLE FLOAT MinGlitchDuration = 0.25f;
	TWEAKABLE FLOAT MaxGlitchDuration = 1.25f;
	TWEAKABLE FLOAT MinGlitchDelay = 10.0f;
	TWEAKABLE FLOAT MaxGlitchDelay = 180.0f;
	TWEAKABLE FLOAT GlitchSineFreq = 5.0f;

	UBOOL wasOn = CameraGlitch.bActive;

	FLOAT glitchAmount = 0.0f;

	if (AreEffectsEnabled() && (GSystemSettings.DetailMode > DM_Low) && hero->bCameraCracked && hero->IsCamcorderActive() && hero->PreciseHealth > 99.0f)
	{	
		CameraGlitch.NextGlitchDelay -= deltaTime;

		if (CameraGlitch.NextGlitchDelay <= 0.0f)
		{
			// start a new glitch
			CameraGlitch.bActive = TRUE;
			CameraGlitch.StartedTime = GWorld->GetTimeSeconds();
			CameraGlitch.Duration = RandRange(MinGlitchDuration, MaxGlitchDuration);
			CameraGlitch.NextGlitchDelay = CameraGlitch.Duration + RandRange(MinGlitchDelay, MaxGlitchDelay);
			CameraGlitch.GlitchType = RandHelper(CGT_MAX);

			hero->TriggerSoundEvent(SndCameraGlitch);
		}

		if (CameraGlitch.bActive)
		{
			FLOAT elapsedTime = GWorld->GetTimeSeconds() - CameraGlitch.StartedTime;

			if (elapsedTime >= CameraGlitch.Duration)
			{
				CameraGlitch.bActive = FALSE;
			}
			else
			{
				switch (CameraGlitch.GlitchType)
				{
				case CGT_OnOff:
					glitchAmount = 1.0f;
					break;
				case CGT_LinearDrop:
					glitchAmount = 1.0f - Saturate(elapsedTime / CameraGlitch.Duration);
					break;
				case CGT_Sine:
					glitchAmount = 0.5f + 0.5f*Saturate(elapsedTime * 2.0f * PI * GlitchSineFreq / CameraGlitch.Duration);
					break;
				}
			}
		}
	}
	
	if (!appIsNearlyEqual(CameraGlitch.CurrentGlitchEffect, glitchAmount, 0.005f) || (glitchAmount <= 0.0f && CameraGlitch.CurrentGlitchEffect >= KINDA_SMALL_NUMBERF))
	{
		CameraGlitchMat->SetScalarParameterValue(CameraGlitchParamName, glitchAmount);
		CameraGlitch.CurrentGlitchEffect = glitchAmount;
		hero->SetAudioValue(CameraGlitchIntensity, 100.0f*glitchAmount);
	}

	UBOOL isOn = CameraGlitch.bActive;
	if (wasOn != isOn)
	{
		SetEffectVisibility(CameraGlitchEffectName, isOn);
	}
}

void UOLFXManager::ActivateNightVisionEffect(UBOOL bPowered)
{
	if (CurrentPPSMode != PPS_NightVision)
	{
		SetPPS(PPS_NightVision);
	
		for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
		{
			if (C->Pawn)
			{
				AOLEnemyPawn* enemyPawn = Cast<AOLEnemyPawn>(C->Pawn);

				if (enemyPawn && enemyPawn->Mesh)
				{
					SetFXForEnemyPawn(enemyPawn);				
				}
			}
		}
	}

	for (INT i = 0; i < NVSensitiveMaterials.Num(); i++)
	{
		NVSensitiveMaterials(i)->SetScalarParameterValue(NVParamName, 1.0f);
		NVSensitiveMaterials(i)->SetScalarParameterValue(NVLightParamName, bPowered ? 1.0f : 0.0f);
	}

}

void UOLFXManager::DeactivateNightVisionEffect()
{
	SetPPS(PPS_Default);

	for (INT i = 0; i < NVSensitiveMaterials.Num(); i++)
	{
		NVSensitiveMaterials(i)->SetScalarParameterValue(NVParamName, 0.0f);
		NVSensitiveMaterials(i)->SetScalarParameterValue(NVLightParamName, 0.0f);
	}

	for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
	{
		if (C->Pawn)
		{
			AOLEnemyPawn* enemyPawn = Cast<AOLEnemyPawn>(C->Pawn);

			if (enemyPawn)
			{
				SetFXForEnemyPawn(enemyPawn);				
			}
		}
	}
}

void UOLFXManager::ActivateCamcorderEffect()
{
	SetPPS(PPS_Camcorder);

	for (INT i = 0; i < NVSensitiveMaterials.Num(); i++)
	{
		NVSensitiveMaterials(i)->SetScalarParameterValue(NVParamName, 0.0f);
	}

	for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
	{
		if (C->Pawn)
		{
			AOLEnemyPawn* enemyPawn = Cast<AOLEnemyPawn>(C->Pawn);

			if (enemyPawn)
			{
				SetFXForEnemyPawn(enemyPawn);
			}
		}
	}
}

void UOLFXManager::SetFXForEnemyPawn(AOLEnemyPawn* enemyPawn)
{
	switch (CurrentPPSMode)
	{
	case PPS_NightVision:
		{
			enemyPawn->EnableNightVisionEffect();				
		}
		break;
	default:
		{
			enemyPawn->DisableNightVisionEffect();			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// OLInterpTrackFloatEffectParam
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/** Add a new keyframe at the specified time. Returns index of new keyframe. */
INT UOLInterpTrackPPSEffectParam::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UOLInterpTrackPPSEffectParam::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UOLInterpTrackPPSEffectParam::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	FLOAT NewFloatValue = FloatTrack.Eval(NewPosition, 0.f);

	if (PropertyType == TPP_MaterialInstance)
	{
		if (EffectMaterial && ParamName != NAME_None)
		{
			EffectMaterial->SetScalarParameterValue(ParamName, NewFloatValue);
		}

		if (EffectName != NAME_None && GIsGame && Utils::GetOLPC())
		{
			ULocalPlayer* LP = Cast<ULocalPlayer>(Utils::GetOLPC()->Player);
			check(LP);
			UPostProcessChain* ppsChain = LP->GetPostProcessChain(0);

			if (ppsChain)
			{
				UPostProcessEffect* effect = ppsChain->FindPostProcessEffect(EffectName); // TODO: cache this
				if (effect)
				{
					effect->bShowInGame = !appIsNearlyZero(NewFloatValue, KINDA_SMALL_NUMBERF);
				}
			}
		}
	}
	else if (PropertyType == TPP_UberMovieIntensity)
	{
		UOLFXManager* FXManager = Utils::GetFXManager();
		if (GIsGame && FXManager && FXManager->CurrentUberPostEffect)
		{
			FXManager->CurrentUberPostEffect->MovieIntensity = NewFloatValue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// OLInterpTrackInstFloatEffectParam
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UOLInterpTrackInstPPSEffectParam::InitTrackInst(UInterpTrack* Track)
{
	Super::InitTrackInst(Track);
	SaveActorState(Track);
}

void UOLInterpTrackInstPPSEffectParam::TermTrackInst(UInterpTrack* Track)
{
	RestoreActorState(Track);
	Super::TermTrackInst(Track);
}

/** Called before Interp editing to put object back to its original state. */
void UOLInterpTrackInstPPSEffectParam::SaveActorState(UInterpTrack* Track)
{
	UOLInterpTrackPPSEffectParam* ParamTrack = CastChecked<UOLInterpTrackPPSEffectParam>(Track);
	
	if (ParamTrack->PropertyType == TPP_MaterialInstance)
	{
		if (ParamTrack->EffectMaterial && ParamTrack->ParamName != NAME_None)
		{
			ParamTrack->EffectMaterial->GetScalarParameterValue(ParamTrack->ParamName, PreviousParamValue);
		}

		if (ParamTrack->EffectName != NAME_None && GIsGame && Utils::GetOLPC())
		{
			ULocalPlayer* LP = Cast<ULocalPlayer>(Utils::GetOLPC()->Player);
			check(LP);
			UPostProcessChain* ppsChain = LP->GetPostProcessChain(0);

			if (ppsChain)
			{
				UPostProcessEffect* effect = ppsChain->FindPostProcessEffect(ParamTrack->EffectName);
				if (effect)
				{
					PreviousEffectShown = effect->bShowInGame;
				}
			}
		}
	}
	else if (ParamTrack->PropertyType == TPP_UberMovieIntensity)
	{
		UOLFXManager* FXManager = Utils::GetFXManager();
		if (GIsGame && FXManager && FXManager->CurrentUberPostEffect)
		{
			PreviousParamValue = FXManager->CurrentUberPostEffect->MovieIntensity;
		}
	}
}

/** Restore the saved state of this Actor. */
void UOLInterpTrackInstPPSEffectParam::RestoreActorState(UInterpTrack* Track)
{
	UOLInterpTrackPPSEffectParam* ParamTrack = CastChecked<UOLInterpTrackPPSEffectParam>(Track);
	
	if (ParamTrack->PropertyType == TPP_MaterialInstance)
	{
		if (ParamTrack->EffectMaterial && ParamTrack->ParamName != NAME_None)
		{
			ParamTrack->EffectMaterial->GetScalarParameterValue(ParamTrack->ParamName, PreviousParamValue);
		}

		if (ParamTrack->EffectName != NAME_None && GIsGame && Utils::GetOLPC())
		{
			ULocalPlayer* LP = Cast<ULocalPlayer>(Utils::GetOLPC()->Player);
			check(LP);
			UPostProcessChain* ppsChain = LP->GetPostProcessChain(0);
			if (ppsChain)
			{
				UPostProcessEffect* effect = ppsChain->FindPostProcessEffect(ParamTrack->EffectName);
				if (effect)
				{
					effect->bShowInGame = PreviousEffectShown;
				}
			}
		}
	}
	else if (ParamTrack->PropertyType == TPP_UberMovieIntensity)
	{
		UOLFXManager* FXManager = Utils::GetFXManager();
		if (GIsGame && FXManager && FXManager->CurrentUberPostEffect)
		{
			FXManager->CurrentUberPostEffect->MovieIntensity = PreviousParamValue;
		}
	}
}
