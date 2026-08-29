/*=============================================================================
	LightSceneInfo.cpp: Light scene info implementation.
	Copyright 1998-2012 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LightRendering.h"

void FLightSceneInfoCompact::Init(FLightSceneInfo* InLightSceneInfo)
{
	LightSceneInfo = InLightSceneInfo;
	LightEnvironment = InLightSceneInfo->LightEnvironment;
	LightingChannels = InLightSceneInfo->LightingChannels;
	FSphere BoundingSphere(
		InLightSceneInfo->GetOrigin(),
		InLightSceneInfo->GetRadius() > 0.0f ?
			InLightSceneInfo->GetRadius() :
			FLT_MAX
		);
	appMemcpy(&BoundingSphereVector,&BoundingSphere,sizeof(BoundingSphereVector));
	Color = InLightSceneInfo->Color;

	bStaticShadowing = InLightSceneInfo->bStaticShadowing;
	bCastDynamicShadow = InLightSceneInfo->bCastDynamicShadow;
	bCastStaticShadow = InLightSceneInfo->bCastStaticShadow;
	bProjectedShadows = InLightSceneInfo->bProjectedShadows;
	bStaticLighting = InLightSceneInfo->bStaticLighting;
	bCastCompositeShadow = InLightSceneInfo->bCastCompositeShadow;
}

FLinearColor FLightSceneInfo::GetDirectIntensity(const FVector& Point) const
{
	return Color;
}

/** Composites the light's influence into an SH vector. */
void FLightSceneInfo::CompositeInfluence(const FVector& Point, FSHVectorRGB& CompositeSH) const
{
	const FLinearColor DirectIntensity = GetDirectIntensity(Point);
	const FVector LightVector = ((FVector)Position - Point * Position.W).SafeNormal();
	const FSHVectorRGB LightEnvironment = SHBasisFunction(LightVector) * DirectIntensity;
	CompositeSH += LightEnvironment;
}

FLightSceneInfo::FLightSceneInfo(const ULightComponent* Component)
	: LightComponent(Component)
	, LightGuid(Component->LightGuid)
	, LightmapGuid(Component->LightmapGuid)
	, WorldToLight(Component->WorldToLight)
	, LightToWorld(Component->LightToWorld)
	, Position(Component->GetPosition())
	, Color(FLinearColor(Component->LightColor) * Component->Brightness)
	, LightingChannels(Component->LightingChannels)
	, StaticPrimitiveList(NULL)
	, DynamicPrimitiveList(NULL)
	, Id(INDEX_NONE)
	, bProjectedShadows(Component->HasProjectedShadowing())
	, bStaticLighting(Component->HasStaticLighting())
	, bStaticShadowing(Component->HasStaticShadowing())
	, bCastDynamicShadow(Component->CastShadows && Component->CastDynamicShadows && (!GIsGame || GSystemSettings.DetailMode >= Component->MinDetailModeForDynamicShadows))
	, bCastCompositeShadow(Component->bCastCompositeShadow)
	, bCastStaticShadow(Component->CastShadows && Component->CastStaticShadows)
	, bNonModulatedSelfShadowing(Component->bNonModulatedSelfShadowing)
	, bSelfShadowOnly(Component->bSelfShadowOnly)
	, bAllowPreShadow(Component->bAllowPreShadow)
	, bOwnerSelected(Component->IsOwnerSelected())
	, bPrecomputedLightingIsValid(Component->bPrecomputedLightingIsValid)
	, bExplicitlyAssignedLight(Component->bExplicitlyAssignedLight)
	, bAllowCompositingIntoDLE(Component->bAllowCompositingIntoDLE)
	, bRenderLightShafts(Component->bRenderLightShafts)
	, bIsHeroSpotLight(Component->bIsHeroLight)
	, LightEnvironment(Component->LightEnvironment)
	, LightType(Component->GetLightType())
	, LightShadowMode(Component->LightShadowMode)
	, ShadowProjectionTechnique(Component->ShadowProjectionTechnique)
	, ShadowFilterQuality(Component->ShadowFilterQuality)
	, MinShadowResolution(Component->MinShadowResolution)
	, MaxShadowResolution(Component->MaxShadowResolution)
	, ShadowFadeResolution(Component->ShadowFadeResolution)
	, ContactShadowTransition(Component->ContactShadowTransition)
	, NumUnbuiltInteractions(0)
	, LevelName(Component->GetOutermost()->GetFName())
	, ModShadowColor(Component->ModShadowColor)
	, ModShadowFadeoutTime(Component->ModShadowFadeoutTime)
	, ModShadowFadeoutExponent(Component->ModShadowFadeoutExponent)
	, DistanceFieldShadowMapPenumbraSize(0.01f)
	, DistanceFieldShadowMapShadowExponent(1.0f)
	, OcclusionDepthRange(Component->OcclusionDepthRange)
	, BloomScale(Component->BloomScale)
	, BloomThreshold(Component->BloomThreshold)
	, BloomScreenBlendThreshold(Component->BloomScreenBlendThreshold)
	, BloomTint(Component->BloomTint)
	, RadialBlurPercent(Component->RadialBlurPercent)
	, OcclusionMaskDarkness(Component->OcclusionMaskDarkness)
	, LightComponentName(Component->GetOwner() ? Component->GetOwner()->GetFName() : Component->GetFName())
	, Scene(Component->GetScene()->GetRenderScene())
	, CacheLastUseFrame(0)
	, bForceUpdateTransform(TRUE)
{
	// LightShadow_ModulateBetter is deprecated
	if (Component->LightShadowMode == LightShadow_ModulateBetter)
	{
		LightShadowMode = LightShadow_Modulate;
	}

	// In order to allow interactive manipulation of shadow casting lights they turn into a more proper mode (e.g. normal shadows
	// instead of the cheaper modulated shadows) to better match the precomputed result.
	if(!Component->bPrecomputedLightingIsValid && (Component->HasStaticShadowing() || Component->HasStaticLighting()))
	{
		LightShadowMode = LightShadow_Normal;
	}

	check(!bIsHeroSpotLight || LightType == (BYTE)LightType_Spot);

	if ( Component->Function && Component->Function->SourceMaterial && Component->Function->SourceMaterial->GetMaterial()->bUsedAsLightFunction )
	{
		LightFunctionScale = Component->Function->Scale;
		LightFunctionDisabledBrightness = Component->Function->DisabledBrightness;
		LightFunction = Component->Function->SourceMaterial->GetRenderProxy(FALSE);
	}
	else
	{
		LightFunctionScale = FVector(1,1,1);
		LightFunctionDisabledBrightness = 1.0f;
		LightFunction = NULL;
	}

	MarkCachedShadowMapDirty();
}

void FLightSceneInfo::AddToScene()
{
	const FLightSceneInfoCompact& LightSceneInfoCompact = Scene->Lights(Id);

	if (GetNumViewDependentWholeSceneShadows() > 0)
	{
		Scene->NumWholeSceneShadowLights++;
	}
	if (IsDirectionLightWithLightFunction())
	{
		Scene->NumDirectionalLightFunctions++;
	}

    if(LightEnvironment)
    {
		FLightEnvironmentSceneInfo& LightEnvironmentSceneInfo = Scene->GetLightEnvironmentSceneInfo(LightEnvironment);

		// For a light in a light environment, attach it to primitives which are in the same light environment.
		for(INT PrimitiveIndex = 0;PrimitiveIndex < LightEnvironmentSceneInfo.Primitives.Num();PrimitiveIndex++)
		{
			FPrimitiveSceneInfo* PrimitiveSceneInfo = LightEnvironmentSceneInfo.Primitives(PrimitiveIndex);
			if(LightSceneInfoCompact.AffectsPrimitive(FPrimitiveSceneInfoCompact(PrimitiveSceneInfo)))
			{
				// create light interaction and add to light/primitive lists
				FLightPrimitiveInteraction::Create(this,PrimitiveSceneInfo);
			}
		}

	    // Add the light to the light environment's light list.
	    LightEnvironmentSceneInfo.Lights.AddItem(this);
    }
	else
	{
		// Add the light to the scene's light octree.
		Scene->LightOctree.AddElement(LightSceneInfoCompact);

		// Find primitives that the light affects in the primitive octree.
		FMemMark MemStackMark(GRenderingThreadMemStack);
		for(FScenePrimitiveOctree::TConstElementBoxIterator<SceneRenderingAllocator> PrimitiveIt(
				Scene->PrimitiveOctree,
				GetBoundingBox()
				);
			PrimitiveIt.HasPendingElements();
			PrimitiveIt.Advance())
		{
			CreateLightPrimitiveInteraction(LightSceneInfoCompact, PrimitiveIt.GetCurrentElement());
		}
	}

	if (bIsHeroSpotLight)
	{
		GHeroLightSceneInfo = this;
	}
}

/**
 * If the light affects the primitive, create an interaction, and process children 
 * 
 * @param LightSceneInfoCompact Compact representation of the light
 * @param PrimitiveSceneInfoCompact Compact representation of the primitive
 */
void FLightSceneInfo::CreateLightPrimitiveInteraction(const FLightSceneInfoCompact& LightSceneInfoCompact, const FPrimitiveSceneInfoCompact& PrimitiveSceneInfoCompact)
{
	if(	LightSceneInfoCompact.AffectsPrimitive(PrimitiveSceneInfoCompact))
	{
		// create light interaction and add to light/primitive lists
		FLightPrimitiveInteraction::Create(this,PrimitiveSceneInfoCompact.PrimitiveSceneInfo);
	}

#if USE_MASSIVE_LOD
	// recursively check for interactions with child primitives
	for (INT ChildIndex = 0; ChildIndex < PrimitiveSceneInfoCompact.ChildPrimitives.Num(); ChildIndex++)
	{
		CreateLightPrimitiveInteraction(LightSceneInfoCompact, *PrimitiveSceneInfoCompact.ChildPrimitives(ChildIndex));
	}
#endif
}


void FLightSceneInfo::RemoveFromScene()
{
	if (GetNumViewDependentWholeSceneShadows() > 0)
	{
		Scene->NumWholeSceneShadowLights--;
	}
	if (IsDirectionLightWithLightFunction())
	{
		Scene->NumDirectionalLightFunctions--;
	}
	if(LightEnvironment)
	{
		FLightEnvironmentSceneInfo& LightEnvironmentSceneInfo = Scene->GetLightEnvironmentSceneInfo(LightEnvironment);

		// Remove the light from its light environment's light list.
		LightEnvironmentSceneInfo.Lights.RemoveItem(this);

		// If the light environment scene info is now empty, free it.
		if(!LightEnvironmentSceneInfo.Lights.Num() && !LightEnvironmentSceneInfo.Primitives.Num())
		{
			Scene->LightEnvironments.Remove(LightEnvironment);
		}
	}
	else
	{
		// Remove the light from the octree.
		Scene->LightOctree.RemoveElement(OctreeId);
	}

	if (bIsHeroSpotLight && GHeroLightSceneInfo == this)
	{
		GHeroLightSceneInfo = NULL;
	}

	// Detach the light from the primitives it affects.
	Detach();
}

void FLightSceneInfo::Detach()
{
	check(IsInRenderingThread());

	// implicit linked list. The destruction will update this "head" pointer to the next item in the list.
	while(StaticPrimitiveList)
	{
		FLightPrimitiveInteraction::Destroy(StaticPrimitiveList);
	}
	// implicit linked list. The destruction will update this "head" pointer to the next item in the list.
	while(DynamicPrimitiveList)
	{
		FLightPrimitiveInteraction::Destroy(DynamicPrimitiveList);
	}

#if USE_MASSIVE_LOD
	// clear all orphaned actors, but it may cause problems with a hierarchy because they could have children
	// that will be tacked onto the orphaned primitive map, so we make a duplicate, and repeat until no
	// more were added
	while (OrphanedPrimitiveMap.Num())
	{
		TMultiMap<UPrimitiveComponent*, FLightPrimitiveInteraction*> OrphanedCopy = OrphanedPrimitiveMap;
		// clear out the orphaned/pending interactions (this will add and remove from the orphaned primitive map)
		for (TMultiMap<UPrimitiveComponent*, FLightPrimitiveInteraction*>::TIterator It(OrphanedCopy); It; ++It)
		{
			FLightPrimitiveInteraction::Destroy(It.Value());
		}
	}
#endif

	// Clear the light's static mesh draw lists.
	for(INT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
	{
		GetDPGInfo(DPGIndex)->DetachStaticMeshes();
	}
}

void FLightSceneInfo::DetachPrimitiveShared(const FLightPrimitiveInteraction& Interaction) 
{
	FPrimitiveSceneInfo* PrimitiveSceneInfo = Interaction.GetPrimitiveSceneInfo();
	if (PrimitiveSceneInfo->BrightestDominantLightSceneInfo == this)
	{
		// Note: Only handling switching to another dominant light when detaching a dominant light in the editor,
		// Because this should rarely be possible in game and is difficult to handle due to the dominant light being merged into the base pass.
		if (GIsEditor && !GIsGame)
		{
			FLOAT MaxBrightness = 0.0f;
			FLightSceneInfo* BrightestDominantLight = NULL;
			for( const FLightPrimitiveInteraction* CurrentInteraction = PrimitiveSceneInfo->LightList; 
				CurrentInteraction; 
				CurrentInteraction = CurrentInteraction->GetNextLight() )
			{
				if (CurrentInteraction->GetLight() != Interaction.GetLight() && IsDominantLightType(CurrentInteraction->GetLight()->LightType))
				{
					const FLinearColor CurrentIntensity = CurrentInteraction->GetLight()->GetDirectIntensity(PrimitiveSceneInfo->Bounds.Origin);
					if (!BrightestDominantLight || CurrentIntensity.GetMax() > MaxBrightness)
					{
						MaxBrightness = CurrentIntensity.GetMax();
						BrightestDominantLight = CurrentInteraction->GetLight();
					}
				}
			}

			PrimitiveSceneInfo->BrightestDominantLightSceneInfo = BrightestDominantLight;
			PrimitiveSceneInfo->BeginDeferredUpdateStaticMeshes();
			check(PrimitiveSceneInfo->BrightestDominantLightSceneInfo != this);
		}
		else
		{
			PrimitiveSceneInfo->BrightestDominantLightSceneInfo = NULL;
			PrimitiveSceneInfo->BeginDeferredUpdateStaticMeshes();
		}
	}
	if (bIsHeroSpotLight && (PrimitiveSceneInfo->LightEnvironment == NULL || !PrimitiveSceneInfo->bCastDynamicShadow))
	{
		PrimitiveSceneInfo->HeroLightCount--;
	}
	if (PrimitiveSceneInfo->DynamicLightSceneInfo == this)
	{
		PrimitiveSceneInfo->DynamicLightSceneInfo = NULL;
		// Update the primitive's static meshes, to ensure they use the right version of the base pass shaders.
		PrimitiveSceneInfo->BeginDeferredUpdateStaticMeshes();
	}
}

void FLightSceneInfo::AttachPrimitiveShared(const FLightPrimitiveInteraction& Interaction)
{
	FPrimitiveSceneInfo* PrimitiveSceneInfo = Interaction.GetPrimitiveSceneInfo();
	if (IsDominantLightType(LightType))
	{
		check(!PrimitiveSceneInfo->AffectingDominantLight || LightComponent == PrimitiveSceneInfo->AffectingDominantLight);
		if (PrimitiveSceneInfo->BrightestDominantLightSceneInfo)
		{
			// If a dominant light is already affecting this primitive, replace it if the current domiant light is brighter at the bounds origin
			const FLinearColor OtherIntensity = PrimitiveSceneInfo->BrightestDominantLightSceneInfo->GetDirectIntensity(PrimitiveSceneInfo->Bounds.Origin);
			const FLinearColor CurrentIntensity = GetDirectIntensity(PrimitiveSceneInfo->Bounds.Origin);
			if (OtherIntensity.GetMax() < CurrentIntensity.GetMax())
			{
				PrimitiveSceneInfo->BrightestDominantLightSceneInfo = this;
				PrimitiveSceneInfo->BeginDeferredUpdateStaticMeshes();
			}
		}
		else
		{
			PrimitiveSceneInfo->BrightestDominantLightSceneInfo = this;
		}

		if (GOnePassDominantLight && PrimitiveSceneInfo->BrightestDominantLightSceneInfo == this)
		{
			// Apply the dominant light in the base pass if using one pass dominant lights
			PrimitiveSceneInfo->DynamicLightSceneInfo = this;

			// Update the primitive's static meshes, to ensure they use the right version of the base pass shaders.
			PrimitiveSceneInfo->BeginDeferredUpdateStaticMeshes();
		}
	}
	if (bIsHeroSpotLight && (PrimitiveSceneInfo->LightEnvironment == NULL || !PrimitiveSceneInfo->bCastDynamicShadow))
	{
		PrimitiveSceneInfo->HeroLightCount++;
	}
}

void FLightSceneInfo::MarkCachedShadowMapDirty(INT SplitIndex, UBOOL bDirty)
{
	if (SplitIndex == -1)
	{
		CachedShadowMapDirtyFlags = bDirty ? 0xffffffff : 0x00000000;
	}
	else
	{
		if (bDirty)
		{
			CachedShadowMapDirtyFlags |= (1 << SplitIndex);
		}
		else
		{
			CachedShadowMapDirtyFlags &= ~(1 << SplitIndex);
		}
	}
}

UBOOL FLightSceneInfo::IsCachedShadowMapDirty(INT SplitIndex)
{
	return (SplitIndex == -1 && CachedShadowMapDirtyFlags != 0) ||
		   (((1 << SplitIndex) & CachedShadowMapDirtyFlags) != 0);
}

void FLightSceneInfo::ResolveShadowDepthToCache(UBOOL bOnePassPointLightShadow, INT SplitIndex, INT ShadowResolution)
{
	// We only support this optimization on systems with 
	if (!GSupportsDepthTextures)
	{
		return;
	}

	if (LightType == (BYTE)LightType_Point || LightType == (BYTE)LightType_DominantPoint)
	{
		if (!IsValidRef(CachedShadowMapCube) || ShadowResolution != CachedShadowMapCube->Resolution)
		{
			CachedShadowMapCube = GSceneRenderTargets.AllocateShadowMapCacheCube(ShadowResolution);
			// Cache is full, cant resolve.
			if (!IsValidRef(CachedShadowMapCube))
			{
				return;
			}
		}
		check(CachedShadowMapCube->Resolution == ShadowResolution);
		GSceneRenderTargets.ResolveShadowMapCubeToCache(CachedShadowMapCube, SplitIndex);
		MarkCachedShadowMapDirty(SplitIndex, FALSE);
		CacheLastUseFrame = GFrameNumberRenderThread;
	}
	else if (LightType == (BYTE)LightType_Spot || LightType == (BYTE)LightType_DominantSpot)
	{
		if (!IsValidRef(CachedShadowMap2D) || ShadowResolution != CachedShadowMap2D->Resolution)
		{
			CachedShadowMap2D = GSceneRenderTargets.AllocateShadowMapCache2D(ShadowResolution);
			// Cache is full, cant resolve.
			if (!IsValidRef(CachedShadowMap2D))
			{
				return;
			}
		}
		check(CachedShadowMap2D->Resolution == ShadowResolution);
		GSceneRenderTargets.ResolveShadowMap2DToCache(CachedShadowMap2D);
		MarkCachedShadowMapDirty(SplitIndex, FALSE);
		CacheLastUseFrame = GFrameNumberRenderThread;
	}
	else
	{
		return;
	}
}

UBOOL FLightSceneInfo::RestoreShadowDepthFromCache(UBOOL bOnePassPointLightShadow, INT SplitIndex, INT ShadowResolution)
{
	if (!GSupportsDepthTextures || IsCachedShadowMapDirty(SplitIndex))
	{
		return FALSE;
	}

	if ((LightType == (BYTE)LightType_Point || LightType == (BYTE)LightType_DominantPoint) && IsValidRef(CachedShadowMapCube))
	{
		// Does not match our cached resolution, need to re-render the shadow map.
		if (ShadowResolution != CachedShadowMapCube->Resolution)
		{
			CachedShadowMapCube = NULL;
			return FALSE;
		}
		GSceneRenderTargets.RestoreShadowMapCubeFromCache(CachedShadowMapCube, SplitIndex);
		CacheLastUseFrame = GFrameNumberRenderThread;
		return TRUE;
	}
	else if ((LightType == (BYTE)LightType_Spot || LightType == (BYTE)LightType_DominantSpot) && IsValidRef(CachedShadowMap2D))
	{
		// Does not match our cached resolution, need to re-render the shadow map.
		if (ShadowResolution != CachedShadowMap2D->Resolution)
		{
			CachedShadowMap2D = NULL;
			return FALSE;
		}
		GSceneRenderTargets.RestoreShadowMap2DFromCache(CachedShadowMap2D);
		CacheLastUseFrame = GFrameNumberRenderThread;
		return TRUE;
	}

	return FALSE;
}

UBOOL FLightSceneInfo::ReleaseUnusedCachedShadowMaps()
{
	if ((IsValidRef(CachedShadowMap2D) || 
		 IsValidRef(CachedShadowMapCube)) &&
		(GFrameNumberRenderThread - CacheLastUseFrame) > 180)
	{
		CachedShadowMap2D = NULL;
		CachedShadowMapCube = NULL;
	}

	return TRUE;
}

/** Determines whether two bounding spheres intersect. */
FORCEINLINE UBOOL AreSpheresNotIntersecting(
	const VectorRegister& A_XYZ,
	const VectorRegister& A_Radius,
	const VectorRegister& B_XYZ,
	const VectorRegister& B_Radius
	)
{
	const VectorRegister DeltaVector = VectorSubtract(A_XYZ,B_XYZ);
	const VectorRegister DistanceSquared = VectorDot3(DeltaVector,DeltaVector);
	const VectorRegister MaxDistance = VectorAdd(A_Radius,B_Radius);
	const VectorRegister MaxDistanceSquared = VectorMultiply(MaxDistance,MaxDistance);
	return VectorAnyGreaterThan(DistanceSquared,MaxDistanceSquared);
}

/**
* Tests whether this light's modulated shadow affects the given primitive by doing a bounds check.
*
* @param CompactPrimitiveSceneInfo - The primitive to test.
* @return True if the modulated shadow affects the primitive.
*/
UBOOL FLightSceneInfoCompact::AffectsModShadowPrimitive(const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo) const
{
	// Check if the light's bounds intersect the primitive's bounds.
	if(AreSpheresNotIntersecting(
		BoundingSphereVector,
		VectorReplicate(BoundingSphereVector,3),
		VectorLoadFloat3(&CompactPrimitiveSceneInfo.Bounds.Origin),
		VectorLoadFloat1(&CompactPrimitiveSceneInfo.Bounds.SphereRadius)
		))
	{
		return FALSE;
	}

	if(!LightSceneInfo->AffectsBounds(CompactPrimitiveSceneInfo.Bounds))
	{
		return FALSE;
	}

	return TRUE;
}

/**
* Tests whether this light affects the given primitive.  This checks both the primitive and light settings for light relevance
* and also calls AffectsBounds.
*
* @param CompactPrimitiveSceneInfo - The primitive to test.
* @return True if the light affects the primitive.
*/
UBOOL FLightSceneInfoCompact::AffectsPrimitive(const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo) const
{
	// Check if the light's bounds intersect the primitive's bounds.
	if(AreSpheresNotIntersecting(
		BoundingSphereVector,
		VectorReplicate(BoundingSphereVector,3),
		VectorLoadFloat3(&CompactPrimitiveSceneInfo.Bounds.Origin),
		VectorLoadFloat1(&CompactPrimitiveSceneInfo.Bounds.SphereRadius)
		))
	{
		return FALSE;
	}

	if(!CompactPrimitiveSceneInfo.bAcceptsLights)
	{
		return FALSE;
	}

	const FPrimitiveSceneInfo* PrimitiveSceneInfo = CompactPrimitiveSceneInfo.PrimitiveSceneInfo;
	PREFETCH(PrimitiveSceneInfo);
	PREFETCH(LightSceneInfo);

	// Dynamic lights that affect the default light environment will also affect primitives in other light environments,
	// unless they're being composited into the light environments.
	const UBOOL bCompositeDynamicLight =
		!CompactPrimitiveSceneInfo.bLightEnvironmentForceNonCompositeDynamicLights
		&& GSystemSettings.bUseCompositeDynamicLights
		// Only composite lights that have compositing enabled (most lights. not Muzzle Flashes)
		&& LightSceneInfo->bAllowCompositingIntoDLE
		// Don't composite dominant lights on primitives that allow dominant light influence
		&& !(IsDominantLightType(LightSceneInfo->LightType) && LightSceneInfo->LightComponent == PrimitiveSceneInfo->AffectingDominantLight)
		// Don't composite lights that have a light function
		&& (LightSceneInfo->LightFunction == NULL)
		// Force dominant lights on primitives that don't allow dominant light influence to be composited (ie not rendered in a lighting pass)
		|| IsDominantLightType(LightSceneInfo->LightType) 
		&& (!PrimitiveSceneInfo->bAllowDominantLightInfluence || PrimitiveSceneInfo->AffectingDominantLight && LightSceneInfo->LightComponent != PrimitiveSceneInfo->AffectingDominantLight);

	const ULightEnvironmentComponent* PrimitiveLightEnvironment = CompactPrimitiveSceneInfo.LightEnvironment;
	if(!LightEnvironment && !bStaticLighting && !bCompositeDynamicLight)
	{
		PrimitiveLightEnvironment = NULL;
	}

	if( PrimitiveLightEnvironment != LightEnvironment )
	{
		return FALSE;
	}

	if( !LightingChannels.OverlapsWith( CompactPrimitiveSceneInfo.LightingChannels ) )
	{
		return FALSE;
	}

	// Cull based on information in the full scene infos.

	if(!LightSceneInfo->AffectsBounds(CompactPrimitiveSceneInfo.Bounds))
	{
		return FALSE;
	}

	if( !PrimitiveSceneInfo->bAcceptsDynamicLights && !bStaticShadowing )
	{
		return FALSE; 
	}

	// If the primitive has an OverrideLightComponent specified, only that light can affect it
	if (PrimitiveSceneInfo->OverrideLightComponent && PrimitiveSceneInfo->OverrideLightComponent != LightSceneInfo->LightComponent
		|| !PrimitiveSceneInfo->OverrideLightComponent && LightSceneInfo->bExplicitlyAssignedLight)
	{
		return FALSE;
	}

	return TRUE;
}

/** Sets the bReceiveDynamicShadowsParameter parameter only. */
void FForwardShadowingShaderParameters::SetReceiveShadows(FShader* PixelShader, UBOOL bReceiveDynamicShadows) const
{
	SetPixelShaderBool(PixelShader->GetPixelShader(), bReceiveDynamicShadowsParameter, bReceiveDynamicShadows);
}

/** Sets all parameters except bReceiveDynamicShadowsParameter. */
void FForwardShadowingShaderParameters::Set(
	const FSceneView& View,
	FShader* PixelShader,
	UBOOL bOverrideDynamicShadowsOnTranslucency,
	const FProjectedShadowInfo* TranslucentPreShadowInfo,
	UBOOL bIsTranslucentPreShadow) const
{
	if (bUsesLightAttenuationBufferParameter.IsBound())
	{
		SetPixelShaderBool(PixelShader->GetPixelShader(), bUsesLightAttenuationBufferParameter, GLightRenderingShadowInfo == NULL || GSceneRenderTargets.bLightAttenuationEnabled);
	}

	if (TranslucentPreShadowInfo)
	{
		// The shadow should have either rendered its depths this frame or have its depths in the preshadow cache
		checkSlow(!bIsTranslucentPreShadow || TranslucentPreShadowInfo->bRendered || TranslucentPreShadowInfo->bDepthsCached);
		// If the shadow is in the preshadow cache it must have its depth buffer up to date
		checkSlow(!bIsTranslucentPreShadow || !TranslucentPreShadowInfo->bAllocatedInPreshadowCache || TranslucentPreShadowInfo->bDepthsCached);

		// Set the transform from screen coordinates to shadow depth texture coordinates.
		const FMatrix ScreenToShadow = TranslucentPreShadowInfo->GetScreenToShadowMatrix(View, bIsTranslucentPreShadow, TRUE);
		SetPixelShaderValue(PixelShader->GetPixelShader(),ScreenToShadowMatrixParameter,ScreenToShadow);

		const FIntPoint ShadowBufferSize = TranslucentPreShadowInfo->GetShadowBufferResolution(bIsTranslucentPreShadow);
		SetPixelShaderValue(PixelShader->GetPixelShader(),ShadowBufferAndTexelSizeParameter,
			FVector4(ShadowBufferSize.X, ShadowBufferSize.Y, 1.0f / ShadowBufferSize.X, 1.0f / ShadowBufferSize.Y));
		FVector2D ShadowOverrideAndTransitionScale(1.0f, TranslucentPreShadowInfo->LightSceneInfo->ContactShadowTransition);
		SetPixelShaderValue(PixelShader->GetPixelShader(),ShadowOverrideFactorParameter, ShadowOverrideAndTransitionScale);

		if (bIsTranslucentPreShadow)
		{
			SetTextureParameter(
				PixelShader->GetPixelShader(),
				ShadowDepthTextureParameter,
				TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSupportsDepthTextures ? 
				GSceneRenderTargets.GetTranslucencyShadowDepthZTexture(TranslucentPreShadowInfo->bAllocatedInPreshadowCache) : 
				GSceneRenderTargets.GetTranslucencyShadowDepthColorTexture(TranslucentPreShadowInfo->bAllocatedInPreshadowCache)
				);
		}
		else
		{
			SetTextureParameter(
				PixelShader->GetPixelShader(),
				ShadowDepthTextureParameter,
				TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSupportsDepthTextures ? 
					GSceneRenderTargets.GetShadowDepthZTexture(FALSE, FALSE, FALSE, TranslucentPreShadowInfo->bUsesCachedShadowMap ? ShadowBufferSize.X : -1) : 
					GSceneRenderTargets.GetShadowDepthColorTexture(FALSE, FALSE)
				);
		}
	}
	else if (bOverrideDynamicShadowsOnTranslucency)
	{
		SetPixelShaderValue(PixelShader->GetPixelShader(),ScreenToShadowMatrixParameter,FMatrix::Identity);

		SetPixelShaderValue(PixelShader->GetPixelShader(),ShadowBufferAndTexelSizeParameter,
			FVector4(1.0f, 1.0f, 1.0f, 1.0f));

		// Set the override factor to 0 to force the whole object to be shadowed, as that is multiplied by the final shadow factor
		FVector2D ShadowOverrideAndTransitionScale(0.0f, 0.0f);
		SetPixelShaderValue(PixelShader->GetPixelShader(),ShadowOverrideFactorParameter, ShadowOverrideAndTransitionScale);

		SetTextureParameter(
			PixelShader->GetPixelShader(),
			ShadowDepthTextureParameter,
			TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
			GWhiteTexture->TextureRHI
			);
	}
}
