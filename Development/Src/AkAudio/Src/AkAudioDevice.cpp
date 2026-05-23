#include "AkAudioPrivate.h"
#include "AkAudioDevice.h"
#include "AkAudioClasses.h"

#include "AkWwise.EXCERPT.h"

#include "AkDefaultIOHookBlocking.h"

#ifdef WWISE_USE_UNREAL_IO
	#include "AkUnrealIOHookBlocking.h"
#endif

#if defined( NGP ) && !defined( AK_OPTIMIZED )
	#include "ForceNetworkConnection.cpp"
#endif

#if ORBIS
	#include "OrbisThreads.h"
#endif

#define INITBANKNAME ( TEXT("Init") )

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
FLOAT GAudioThreadTime       = 0; // In milliseconds.
FLOAT GAudioThreadTimeRaw    = 0; // In milliseconds.
#if PS4
FLOAT GAudioDSPThreadTime    = 0; // In milliseconds.
FLOAT GAudioDSPThreadTimeRaw = 0; // In milliseconds.
#endif
#endif

class WxAkRadiusBrowser // FIXME: this is an unnatural way of simulating an interface (actual object is in UnrealEd)
{
public:
	void RefreshList();
	static WxAkRadiusBrowser * Get();
};

namespace AK
{
	void * AllocHook( size_t in_size )
	{
		return appMalloc( in_size );
	}
	void FreeHook( void * in_ptr )
	{
		appFree( in_ptr );
	}

#ifdef _WIN32 // only on PC and XBox360
	void * VirtualAllocHook(
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwAllocationType,
		DWORD in_dwProtect
		)
	{
		return VirtualAlloc( in_pMemAddress, in_size, in_dwAllocationType, in_dwProtect );
	}
	void VirtualFreeHook( 
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwFreeType
		)
	{
		VirtualFree( in_pMemAddress, in_size, in_dwFreeType );
	}
#endif // only on PC and XBox360

#ifdef XBOX360
	void * PhysicalAllocHook( 
		size_t in_size,					///< Number of bytes to allocate
		ULONG_PTR in_ulPhysicalAddress, ///< Parameter for XPhysicalAlloc
		ULONG_PTR in_ulAlignment,		///< Parameter for XPhysicalAlloc
		DWORD in_dwProtect				///< Parameter for XPhysicalAlloc
		)
	{
		return XPhysicalAlloc( in_size, in_ulPhysicalAddress, in_ulAlignment, in_dwProtect );
	}
	void PhysicalFreeHook( 
		void * in_pMemAddress	///< Pointer to start of memory allocated with PhysicalAllocHook
		)
	{
		XPhysicalFree( in_pMemAddress );
	}
#endif

#ifdef DINGO
	void * APUAllocHook( 
		size_t in_size,				///< Number of bytes to allocate.
		unsigned int in_alignment	///< Alignment in bytes (must be power of two, greater than or equal to four).
		)
	{
		// mathieu.gauthier: We are using vorbis sound banks, so we dont need
		// to allocate APU memory. As of version 2013, wwise always calls APUAllocHook 
		// even for non-APU memory.
		return appMalloc(in_size /*, in_alignment*/);
	}

	void APUFreeHook( 
		void * in_pMemAddress	///< Virtual address as returned by APUAllocHook.
		)
	{
		appFree(in_pMemAddress);
	}
#endif
}

#if !FINAL_RELEASE && !SHIPPING_PC_GAME

// Super hacky way to access WWise's internal counters, which are 
// only exposed through the comm/profiler interface. Copied the 
// class definition here, made members public. Will not be accurate
// when the real WWise profiling is running since it resets the 
// LastTime member when calling Stamp(). 

namespace AkAudiolibTimer
{
	class AkTimerItem
	{
	public:
		SQWORD ComputedUsage;
		SQWORD ActualUsage;
		SQWORD LastTime;
	};

	extern AkTimerItem timerAudio;
#if PS4
	extern AkTimerItem timerDsp;
#endif
}

namespace AK
{
	extern float g_fFreqRatio;
}

void AkThreadCallback(bool in_bLastCall)
{
	static SQWORD LastActualUsage    = 0;
	static SQWORD LastActualDSPUsage = 0;
	
	SQWORD ActualUsage    = AkAudiolibTimer::timerAudio.ActualUsage;
	GAudioThreadTimeRaw    = (ActualUsage    - LastActualUsage)    / AK::g_fFreqRatio;
	LastActualUsage    = ActualUsage;

#if PS4
	SQWORD ActualDSPUsage = AkAudiolibTimer::timerDsp.ActualUsage;
	GAudioDSPThreadTimeRaw = (ActualDSPUsage - LastActualDSPUsage) / AK::g_fFreqRatio;
	LastActualDSPUsage = ActualDSPUsage;
#endif
}
#endif

#ifdef WWISE_USE_UNREAL_IO
	static CAkUnrealIOHookBlocking g_lowLevelIO;
#else
	// Then, we're using the default CAkDefaultIOHookBlocking implementation that's part
	// of the SDK's sample code
	static CAkDefaultIOHookBlocking g_lowLevelIO;
#endif

/*------------------------------------------------------------------------------------
	UAkAudioDevice constructor and UObject interface.
------------------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UAkAudioDevice);

bool UAkAudioDevice::m_bSoundEngineInitialized = false;

// Always use sound engine when SoundFrame not present

#if defined(AK_SOUNDFRAME) && defined(AK_SOUNDFRAME_PLAYBACK)
	bool UAkAudioDevice::m_bUseSoundEngine = false;
	#define AK_USE_SOUNDENGINE (m_bUseSoundEngine)
#else
	#define AK_USE_SOUNDENGINE (1)
#endif

void UAkAudioDevice::StaticConstructor()
{
}

void UAkAudioDevice::Teardown()
{
	if ( m_bSoundEngineInitialized )
	{
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
		AK::SoundEngine::UnregisterGlobalCallback(AkThreadCallback);
#endif

#ifndef AK_OPTIMIZED
#ifdef _WINDOWS_
    //
    // Terminate Communication Services
    //
    AK::Comm::Term();
#endif
#endif // AK_OPTIMIZED

		//
		// Terminate the music engine
		//
		AK::MusicEngine::Term();

		//
		// Unregister game objects. Since we're about to terminate the sound engine
		// anyway, we don't really have to unregister those game objects here. But
		// in general it is good practice to unregister game objects as soon as they
		// become obsolete, to free up resources.
		//

		if ( AK::SoundEngine::IsInitialized() )
		{
			//
			// Terminate the sound engine
			//
		
			AK::SoundEngine::Term();
		}
		
		g_lowLevelIO.Term();

		// Terminate the streaming manager
		if ( AK::IAkStreamMgr::Get() )
		{   
			AK::IAkStreamMgr::Get()->Destroy();
		}

		// Terminate the Memory Manager
		AK::MemoryMgr::Term();

		m_bSoundEngineInitialized = false;
	}

	// Terminate SoundFrame

#ifdef AK_SOUNDFRAME
	if ( m_pSoundFrame )
	{
		m_pSoundFrame->UnregisterGameObject( DUMMY_GAMEOBJ );
		m_pSoundFrame->Release();
		m_pSoundFrame = NULL;
	}
#endif
}

#define WWISE_READ_SIZE DVD_MIN_READ_SIZE

bool UAkAudioDevice::EnsureInitialized()
{
	if ( m_bSoundEngineInitialized ) 
		return true;

	debugf(	NAME_Log,
			TEXT("Wwise(R) SDK Version %d.%d.%d Build %d. Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved."),
			AK_WWISESDK_VERSION_MAJOR, 
			AK_WWISESDK_VERSION_MINOR, 
			AK_WWISESDK_VERSION_SUBMINOR, 
			AK_WWISESDK_VERSION_BUILD);

	AkMemSettings memSettings;

	// rcharpentier - no idea of the impact of raising this
	memSettings.uMaxNumPools = 80;

	if ( AK::MemoryMgr::Init( &memSettings ) != AK_Success )
        return false;

	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings( stmSettings );
	AK::IAkStreamMgr * pStreamMgr = AK::StreamMgr::Create( stmSettings );
	if ( ! pStreamMgr )
        return false;

	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings( deviceSettings );
	deviceSettings.uGranularity = WWISE_READ_SIZE;
#if ORBIS
	deviceSettings.threadProperties.dwAffinityMask = 1 << WWISE_HWTRHEAD;
	deviceSettings.threadProperties.nPriority = SCE_KERNEL_PRIO_FIFO_HIGHEST + 10; // Slightly lower priority than physics
#elif DINGO
	deviceSettings.threadProperties.dwAffinityMask = 1 << WWISE_HWTRHEAD;
#endif

	if ( g_lowLevelIO.Init( deviceSettings, true ) != AK_Success )
        return false;

	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings( initSettings );
	AK::SoundEngine::GetDefaultPlatformInitSettings( platformInitSettings );

#if ORBIS || DINGO
	platformInitSettings.uLEngineDefaultPoolSize           = 128 * 1024 * 1024; // Default is 16MB
	platformInitSettings.fLEngineDefaultPoolRatioThreshold = 0.8f; // Start panicking when you are 80% full.
	platformInitSettings.threadLEngine.dwAffinityMask      = 1 << WWISE_HWTRHEAD;
	platformInitSettings.threadBankManager.dwAffinityMask  = 1 << WWISE_HWTRHEAD;
	platformInitSettings.threadMonitor.dwAffinityMask      = 1 << WWISE_HWTRHEAD;
#if ORBIS
	platformInitSettings.threadLEngine.nPriority           = SCE_KERNEL_PRIO_FIFO_HIGHEST + 10; // Slightly lower priority than physics
	platformInitSettings.threadBankManager.nPriority       = SCE_KERNEL_PRIO_FIFO_HIGHEST + 5; // Slightly lower priority than physics
	platformInitSettings.threadMonitor.nPriority           = SCE_KERNEL_PRIO_FIFO_HIGHEST + 5; // Slightly lower priority than physics
#elif DINGO
	platformInitSettings.threadLEngine.nPriority           = THREAD_PRIORITY_NORMAL; // Slightly lower priority than physics
	platformInitSettings.threadBankManager.nPriority       = THREAD_PRIORITY_NORMAL;// Slightly lower priority than physics
	platformInitSettings.threadMonitor.nPriority           = THREAD_PRIORITY_NORMAL;// Slightly lower priority than physics
	platformInitSettings.uMaxXMAVoices = 256;
#endif
#endif

#if defined(WIN32) && !defined(DINGO)
	// Make the sound to not be audible when the game is minimized.

	extern HWND GGameWindow;
	//Recall Game window
	if( GGameWindow )
	{
		platformInitSettings.hWnd = GGameWindow;
		platformInitSettings.bGlobalFocus = false;
#if !SHIPPING_PC_GAME && !FINAL_RELEASE
		if (ParseParam(appCmdLine(), TEXT("sndglobalfocus")))
		{
			platformInitSettings.bGlobalFocus = true;
		}
#endif

	}

	if (GEngine && GEngine->ShouldUseLowAudioQuality())
	{
		platformInitSettings.eAudioQuality = AkSoundQuality_Low;
		debugf(	NAME_Log, TEXT("Using low quality sound (24 KHz sample rate)"));
	}
#endif

#ifdef PS3
	// The Wwise sound engine is best using the UnrealEngine GSPURS too instead of creating its own.
	platformInitSettings.pSpurs = GSPURS;
	platformInitSettings.threadBankManager.uStackSize = 3* 8192;
#endif

	if ( AK::SoundEngine::Init( &initSettings, &platformInitSettings ) != AK_Success )
        return false;

	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings( musicInit );

	if ( AK::MusicEngine::Init( &musicInit ) != AK_Success )
        return false;

	// mathieu.gauthier: The comm API seem to fail to initialize on June XDK, it is probably
	// a sign that we need to update Wwise soon.
#if defined(_WINDOWS_) && !defined(DINGO)
#ifndef AK_OPTIMIZED
    //
    // Initialize communications (not in release build!)
    //
    AkCommSettings commSettings;
    AK::Comm::GetDefaultInitSettings( commSettings );
    if ( AK::Comm::Init( commSettings ) != AK_Success )
    {
        assert( ! "Could not initialize communication." );
        return false;
    }
#endif // AK_OPTIMIZED
#endif

	// Register plug-ins.
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_MATRIXREVERB, CreateMatrixReverbFX, CreateMatrixReverbFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_COMPRESSOR, CreateCompressorFX, CreateCompressorFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_EXPANDER, CreateExpanderFX, CreateExpanderFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_DELAY, CreateDelayFX, CreateDelayFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_PARAMETRICEQ, CreateParametricEQFX, CreateParametricEQFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_PEAKLIMITER, CreatePeakLimiterFX, CreatePeakLimiterFXParams );
	
	// plalonde - Outlast Plugins
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_ROOMVERB, CreateRoomVerbFX, CreateRoomVerbFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_METER, CreateMeterFX, CreateMeterFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_FLANGER, CreateFlangerFX, CreateFlangerFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_TREMOLO, CreateTremoloFX, CreateTremoloFXParams );

	AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, AKSOURCEID_TONE, CreateToneSource, CreateToneSourceParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, AKSOURCEID_SINE, CreateSineSource, CreateSineSourceParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, AKSOURCEID_SILENCE, CreateSilenceSource, CreateSilenceSourceParams );

	AK::SoundEngine::RegisterCodec( AKCOMPANYID_AUDIOKINETIC, AKCODECID_VORBIS, CreateVorbisFilePlugin, CreateVorbisBankPlugin );

	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_GENAUDIO, AKEFFECTID_GENAUDIORTI, CreateAstoundSoundRTIFX, CreateAstoundSoundRTIFXParams );

	//
	// Setup banks path
	//
	bUsingDLCBanks = FALSE;
	SetBankDirectory();

	// Init dummy game object
	AK::SoundEngine::RegisterGameObj( DUMMY_GAMEOBJ, "Unreal Global" );

	m_bSoundEngineInitialized = true;

#if defined(AK_SOUNDFRAME) && defined(AK_SOUNDFRAME_PLAYBACK)
	m_bUseSoundEngine = !GIsEditor;
#endif

	LoadAllReferencedBanks();
	
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	AK::SoundEngine::RegisterGlobalCallback(AkThreadCallback);
#endif

	return true;
}

void UAkAudioDevice::SetBankDirectory()
{
#if defined(PS3)

	AkOSChar szPath[ CELL_FS_MAX_FS_PATH_LENGTH ];
	AkOSChar * pszGameName = 0;
	CONVERT_WIDE_TO_OSCHAR( appGetGameName(), pszGameName );

	#ifdef WWISE_USE_UNREAL_IO
		strcpy( szPath, "..\\" );
		strcat( szPath, pszGameName );
		strcat( szPath, "Game\\CookedPS3\\" );
	#else
		strcpy( szPath, "/app_home" );
		strcat( szPath, "/" );
		strcat( szPath, pszGameName );
		strcat( szPath, "Game/CookedPS3/" );
	#endif

	g_lowLevelIO.SetBasePath( szPath );
#elif defined(NGP)
	AkOSChar szPath[ 128 ];
	AkOSChar * pszGameName = 0;
	CONVERT_WIDE_TO_OSCHAR( appGetGameName(), pszGameName );

	#ifdef WWISE_USE_UNREAL_IO
		strcpy( szPath, "..\\" );
		strcat( szPath, pszGameName );
		strcat( szPath, "Game\\CookedNGP\\" );
	#else
		strcpy( szPath, "/host0" );
		strcat( szPath, "/" );
		strcat( szPath, pszGameName );
		strcat( szPath, "Game/CookedNGP/" );
	#endif

	g_lowLevelIO.SetBasePath( szPath );
#else

#ifdef XBOX360
	FString BasePath = FString::Printf(TEXT("..\\%sGame\\CookedXbox360\\"), appGetGameName());
#elif PLATFORM_MACOSX
	FString BasePath = appRootDir() + FString(TEXT("OLGame/CookedMac/"));
#elif ORBIS
	FString BasePath = bUsingDLCBanks ? ConsoleDLCPath : TEXT("../../OLGame/CookedOrbis/");
#elif DINGO
	FString BasePath = bUsingDLCBanks ? ConsoleDLCPath : TEXT("../../OLGame/CookedDingo/");
#else
	FString BasePath = appGameDir();
	if( GUseSeekFreeLoading )
	{
		if (bUsingDLCBanks)
		{
			BasePath += TEXT("CookedPCConsoleDLC\\"); 
		}
		else
		{
			BasePath += TEXT("CookedPCConsole\\"); 
		}		
	}
	else
	{
		// We no longer load the _MainGame variety
		BasePath += TEXT("Content\\WwiseAudio\\Windows\\");
	}
#endif

	AkOSChar * pszPath = 0;
	CONVERT_WIDE_TO_OSCHAR( *BasePath, pszPath );
	g_lowLevelIO.SetBasePath( pszPath );

#endif

	AK::StreamMgr::SetCurrentLanguage( AKTEXT("English(US)") );
}

UBOOL UAkAudioDevice::Init()
{
	EnsureInitialized(); // ensure audiolib is initialized

	// Initialize SoundFrame

#ifdef AK_SOUNDFRAME
	m_pSoundFrame = NULL;
	if( AK::SoundFrame::CreateUEd( this, &m_pSoundFrame ) )
	{
		m_pSoundFrame->Connect();
	}

	m_TimeStamp = 0;
#endif

	debugf(NAME_Init,TEXT("AK Audio Device initialized."));

	return 1;
}

inline void FVectorToAKVector( const FVector & in_vect, AkVector & out_vect )
{
	out_vect.X = -in_vect.X;
	out_vect.Y = in_vect.Z;
	out_vect.Z = in_vect.Y;
}

void UAkAudioDevice::Update( UBOOL Realtime )
{
	if (GIsEditor && !GWorld->HasBegunPlay())
	{
		SetListener(0, FVector(0.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f),  FVector(1.f, 0.f, 0.f));
	}

	// Send position info to AudioLib

	for ( int i = 0; i < m_GameObjects.Num(); ++i )
	{
		UAkComponent * pComponent = m_GameObjects( i );
		AActor* pActor = pComponent->GetOwner();

		if ( pActor )
		{
			FVector sndPos = pActor->Location;
			FVector sndDir = pActor->Rotation.Vector();

			if ( pComponent->BoneName != NAME_None)
			{
				USkeletalMeshComponent* lpSkelMesh = NULL;
				if (pActor->BaseSkelComponent)
				{
					lpSkelMesh = pActor->BaseSkelComponent;      
				}
				else
				{
					//If there isn’t a BaseSkelComponent set (as will be the case with SkeletalMeshActors)
					//Use the first mesh we come to

					for (int Comp = 0; Comp < pActor->Components.Num() && lpSkelMesh == NULL; Comp++)
					{
						lpSkelMesh = Cast<USkeletalMeshComponent>( pActor->Components( Comp ) );
					}
				}

				if (lpSkelMesh)
				{
					INT BoneIndex = lpSkelMesh->MatchRefBone(pComponent->BoneName);
					if(BoneIndex != INDEX_NONE)
					{
						FMatrix BoneMatrix = lpSkelMesh->GetBoneMatrix(BoneIndex);
						BoneMatrix.RemoveScaling();

						sndPos = BoneMatrix.GetOrigin();

						if (pComponent->BoneName == FName(TEXT("NPCMedium-Head")) || pComponent->BoneName == FName(TEXT("NPCLarge-Head")) || pComponent->BoneName == FName(TEXT("Hero-Head")))
						{
							// rcharpentier - fix head orientation for our NPCs
							sndDir = -BoneMatrix.GetAxis(1);
						}
						else
						{
							sndDir = BoneMatrix.GetAxis(0);
						}
					}                                             
				}
			}

			// rcharpentier - hook in virtualization
			{
				extern void UpdateSoundPosition(AActor* actor, FVector& inout_Position, FVector& inout_Direction);
				UpdateSoundPosition(pActor, sndPos, sndDir);
			}

			// rcharpentier - multisources
			{
				extern UBOOL TryGetMultipleSourcePositions(AActor* actor, TArray<FVector>& out_Positions, TArray<FVector>& out_Directions);

				TArray<FVector> soundPositions;
				TArray<FVector> soundDirections;
				if (TryGetMultipleSourcePositions(pActor, soundPositions, soundDirections))
				{
					SetObjectMultiPositions(soundPositions, soundDirections, pComponent);
				}
				else
				{			
					SetObjectPosition( sndPos, sndDir, pComponent );
				}
			}
		}
	}

	if ( m_bSoundEngineInitialized )
	{
		AK::SoundEngine::RenderAudio();

#ifdef AK_SOUNDFRAME
		if (m_pSoundFrame && m_pSoundFrame->IsConnected())
		{
			m_TimeStamp = AK::Monitor::GetTimeStamp();
		}
#endif
	}
}

void UAkAudioDevice::SetObjectMultiPositions( const TArray<FVector>& Locations, const TArray<FVector>& Rotations, UAkComponent* pComponent ) // rcharpentier
{
	enum { MaxGroupMembers = 64 };
	AkSoundPosition soundpositions[MaxGroupMembers];

	check(Locations.Num() == Rotations.Num());
	INT nbSoundPositions = Locations.Num();
	check(nbSoundPositions > 0 && nbSoundPositions <= MaxGroupMembers); // Limited sound sources - this could be bumped up if needed (see other definition of MaxGroupMembers)

	for (INT i = 0; i < nbSoundPositions; i++)
	{
		FVectorToAKVector( Locations(i), soundpositions[i].Position );
		FVectorToAKVector( Rotations(i), soundpositions[i].Orientation );

		if ( soundpositions[i].Orientation.X == 0.0 && soundpositions[i].Orientation.Y == 0.0 && soundpositions[i].Orientation.Z == 0.0 )
		{
			debugf(NAME_Critical,TEXT("Orientation Front vector invalid!") );
		}
	}

	AkGameObjectID GameObjID = (AkGameObjectID) pComponent;
	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::SetMultiplePositions(GameObjID, soundpositions, nbSoundPositions, AK::SoundEngine::MultiPositionType_MultiSources);
		}
	}
#ifdef AK_SOUNDFRAME
	else if( m_pSoundFrame && !GIsPlayInEditorWorld && GIsEditor )
	{
		m_pSoundFrame->SetMultiplePositions(GameObjID, soundpositions, nbSoundPositions, AK::SoundEngine::MultiPositionType_MultiSources);
	}
#endif
}

void UAkAudioDevice::SetObjectPosition( const FVector& Location, const FVector& Rotation, UAkComponent* pComponent )
{
	AkSoundPosition soundpos;
	FVectorToAKVector( Location, soundpos.Position );
	FVectorToAKVector( Rotation, soundpos.Orientation );

	if ( soundpos.Orientation.X == 0.0 && soundpos.Orientation.Y == 0.0 && soundpos.Orientation.Z == 0.0 )
		debugf(NAME_Critical,TEXT("Orientation Front vector invalid!") );

	AkGameObjectID GameObjID = (AkGameObjectID) pComponent;
	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::SetPosition( GameObjID, soundpos );
		}
	}
#ifdef AK_SOUNDFRAME
	else if( m_pSoundFrame && !GIsPlayInEditorWorld && GIsEditor )
	{
		m_pSoundFrame->SetPosition( GameObjID, soundpos );
	}
#endif
}

void UAkAudioDevice::FinishDestroy( void )
{
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		Teardown();
		debugf( NAME_Exit, TEXT( "AK Audio shut down." ) );
	}

	Super::FinishDestroy();
}

/**
 * Special variant of Destroy that gets called on fatal exit. Doesn't really
 * matter on the console so for now is just the same as Destroy so we can
 * verify that the code correctly cleans up everything.
 */
void UAkAudioDevice::ShutdownAfterError( void )
{
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		Teardown();
		debugf( NAME_Exit, TEXT( "UAkAudioDevice::ShutdownAfterError" ) );
	}

	Super::ShutdownAfterError();
}

void UAkAudioDevice::SetListener( INT ViewportIndex, const FVector& Location, const FVector& Up, const FVector& Right, const FVector& Front )
{
	AkListenerPosition position;

	FVectorToAKVector( Location, position.Position );
	FVectorToAKVector( Front, position.OrientationFront );
	FVectorToAKVector( Up, position.OrientationTop );

	if ( position.OrientationFront.X == 0.0 && position.OrientationFront.Y == 0.0 && position.OrientationFront.Z == 0.0 )
		debugf(NAME_Critical,TEXT("Orientation Front vector invalid!") );

	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::SetListenerPosition( position, ViewportIndex );
		}
	}
#ifdef AK_SOUNDFRAME
	else
	{
		if( m_pSoundFrame && GIsEditor )
			m_pSoundFrame->SetListenerPosition( position, ViewportIndex );
	}
#endif
}

void UAkAudioDevice::StopAllSounds( UBOOL bShouldStopUISounds )
{
#ifdef AK_SOUNDFRAME
	if ( !(AK_USE_SOUNDENGINE) )
	{
		if ( m_pSoundFrame )
		{
			m_pSoundFrame->StopAll();
		}
	}
#endif

	if ( m_bSoundEngineInitialized )
	{
		AK::SoundEngine::StopAll();
	}
}

#ifdef AK_SOUNDFRAME
void UAkAudioDevice::OnSoundObjectNotif( 
	Notif in_eNotif,					///< Notification type
	AkUniqueID in_soundObjectID			///< Unique ID of the sound object
	)
{

#if WITH_EDITOR
	if ( in_eNotif == IClient::Notif_Changed )
	{
		WxAkRadiusBrowser * pBrowser = WxAkRadiusBrowser::Get();
		if ( pBrowser )
			pBrowser->RefreshList();
	}
#endif // WITH_EDITOR
}
#endif

AKRESULT UAkAudioDevice::ClearBanks()
{
	for( TObjectIterator<UAkBank> It; It; ++It )
	{
		(*It)->Unload();
	}

	if ( m_bSoundEngineInitialized )
	{
		return AK::SoundEngine::ClearBanks();
	}
	else
	{
		return AK_Success;
	}
}

AKRESULT UAkAudioDevice::LoadBank(
	const TCHAR *       in_pszString,			///< Name of the bank to load
	AkMemPoolId         in_memPoolId,			///< Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	AkBankID &          out_bankID				///< Returned bank ID
	)
{
	if( EnsureInitialized() ) // ensure audiolib is initialized
		return AK::SoundEngine::LoadBank( in_pszString, in_memPoolId, out_bankID );
	return AK_Fail;
}

AKRESULT UAkAudioDevice::LoadBank(
    const TCHAR *       in_pszString,           ///< Name/path of the bank to load
	AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
	void *              in_pCookie,				///< Callback cookie (reserved to user, passed to the callback function)
    AkMemPoolId         in_memPoolId,			///< Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	AkBankID &          out_bankID				///< Returned bank ID
    )
{
	if( EnsureInitialized() ) // ensure audiolib is initialized
	{
		return AK::SoundEngine::LoadBank( in_pszString, in_pfnBankCallback, in_pCookie, in_memPoolId, out_bankID );
	}
	return AK_Fail;
}

AKRESULT UAkAudioDevice::UnloadBank(
    const TCHAR *       in_pszString,			///< Name of the bank to unload
    AkMemPoolId *       out_pMemPoolId		    ///< Returned memory pool ID used with LoadBank() (can pass NULL)
    )
{
	if ( m_bSoundEngineInitialized )
	{
		return AK::SoundEngine::UnloadBank( in_pszString, out_pMemPoolId );
	}
	return AK_Fail;
}

AKRESULT UAkAudioDevice::UnloadBank(
    const TCHAR *       in_pszString,           ///< Name of the bank to unload
	AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
	void *              in_pCookie 				///< Callback cookie (reserved to user, passed to the callback function)
    )
{
	if ( m_bSoundEngineInitialized )
	{
		return AK::SoundEngine::UnloadBank( in_pszString, NULL, in_pfnBankCallback, in_pCookie );
	}
	return AK_Fail;
}

AKRESULT UAkAudioDevice::LoadInitBank()
{
	AkBankID BankID;
	return LoadBank( INITBANKNAME, AK_DEFAULT_POOL_ID, BankID );
}

AKRESULT UAkAudioDevice::UnloadInitBank()
{
	return UnloadBank( INITBANKNAME );
}

void UAkAudioDevice::LoadAllReferencedBanks()
{
	LoadInitBank();

	// Load any banks that are in memory that haven't been loaded yet
	for( TObjectIterator<UAkBank> It; It; ++It )
	{
		(*It)->Load();
	}
}

void UAkAudioDevice::ReloadAllReferencedBanks()
{
	StopAllSounds();

	for( TObjectIterator<UAkBank> It; It; ++It )
	{
		(*It)->Unload();
	}

	UnloadInitBank();

	LoadAllReferencedBanks();
}

UBOOL UAkAudioDevice::ConditionalReloadAllBanks(UBOOL bForDLC)
{
	if (bUsingDLCBanks == bForDLC)
	{
		return FALSE; // already set
	}

	bUsingDLCBanks = bForDLC;
	SetBankDirectory();
	ReloadAllReferencedBanks();

	if (bForDLC)
	{
		debugf(TEXT("Reloaded all WWise Banks for DLC"));
	}
	else
	{
		debugf(TEXT("Reloaded all WWise Banks for Main Game"));
	}

	return TRUE;
}

void UAkAudioDevice::RegisterAkEventHandler(FName in_PackageName, class IInterface_AkEventHandler* in_pHandler)
{
	if (in_PackageName != NAME_None && in_pHandler)
	{
		if (m_PackageHandlers.HasKey(in_PackageName))
		{
			warnf(TEXT("Trying to register multiple handlers for %s."), *in_PackageName.ToString());
		}
		else
		{
			m_PackageHandlers.Set(in_PackageName, in_pHandler);
		}
	}
}

void UAkAudioDevice::UnregisterAkEventHandler(FName in_PackageName, class IInterface_AkEventHandler* in_pHandler)
{
	if (in_PackageName != NAME_None && in_pHandler)
	{
		IInterface_AkEventHandler** CurrentHandler = m_PackageHandlers.Find(in_PackageName);
		if (CurrentHandler && *CurrentHandler == in_pHandler)
		{
			m_PackageHandlers.Remove(in_PackageName);
		}
	}
}

AkPlayingID UAkAudioDevice::PostEvent(
		UAkEvent * in_pEvent, 
		AActor * in_pActor, 
		FName in_nameBone, 
		AkUInt32 in_uFlags /*= 0*/,					///< Bitmask: see \ref AkCallbackType
		AkCallbackFunc in_pfnCallback /*= NULL*/,	///< Callback function
		void * in_pCookie /*= NULL*/,				///< Callback cookie that will be sent to the callback function along with additional information.
		bool in_bStopWhenOwnerDestroyed /*= false*/,
		bool in_checkHandler /*= true*/
        )
{
	AkPlayingID playingID = AK_INVALID_PLAYING_ID;

	if ( in_pEvent )
	{
		UBOOL Handled = FALSE;
		if (in_checkHandler && in_pEvent->bUseVOSystem)
		{
			IInterface_AkEventHandler** Handler = m_PackageHandlers.Find(in_pEvent->GetOutermost()->GetPureName());
			if(Handler && *Handler && (*Handler)->HandleEvent(in_pEvent, in_pActor, in_nameBone, playingID, in_uFlags, in_pfnCallback, in_pCookie, in_bStopWhenOwnerDestroyed))
			{
				Handled = TRUE;
			}
		}

		if (!Handled)
		{
			// PostEvent must be bound to a game object. Passing DUMMY_GAMEOBJ as default game object.
			AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
			if( GetGameObjectID( in_pActor, in_nameBone, GameObjID, in_bStopWhenOwnerDestroyed ) == AK_Success )
			{
				FString eventName( in_pEvent->GetName() );
				playingID = PostEventInternal( *eventName, GameObjID, in_uFlags, in_pfnCallback, in_pCookie );

				// rcharpentier - dirty hack, but whatever. See if we should create a sound environment for this actor.
				{
					extern void RegisterSoundEmitterForActor(AActor* actor);
					RegisterSoundEmitterForActor(in_pActor);
				}
			}
		}
	}
	return playingID;
}

AkPlayingID UAkAudioDevice::PostEventInternal(
		const TCHAR * in_pszEvent, 
		AkGameObjectID in_GameObjectID, 
		AkUInt32 in_uFlags /*= 0*/,					///< Bitmask: see \ref AkCallbackType
		AkCallbackFunc in_pfnCallback /*= NULL*/,	///< Callback function
		void * in_pCookie /*= NULL*/				///< Callback cookie that will be sent to the callback function along with additional information.
        )
{
	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			return AK::SoundEngine::PostEvent( in_pszEvent, in_GameObjectID, in_uFlags, in_pfnCallback, in_pCookie );
		}
		else
		{
			return AK_INVALID_PLAYING_ID;
		}
	}
	else
	{
#ifdef AK_SOUNDFRAME
		bool bSuccess = false;
		if( m_pSoundFrame )
		{
			// @todo:vraghu@ak 
			// This is a hack so we can continue to play events even 
			// after generating sound banks in UnrealEd.
			// It's a fairly benign hack, but still needs to go at some point.
			m_pSoundFrame->RegisterGameObject( in_GameObjectID, L"No Name" );

			bSuccess = m_pSoundFrame->PlayEvents( &in_pszEvent, 1, in_GameObjectID );
		}
		// The soundframe does not give back the playing ID to be returned.
		// returning AK_INVALID_PLAYING_ID+1 so it is not considered as an error, but this returned value cannot be used to reference
		// the newly instanciated playback.
		if( bSuccess )
			return AK_INVALID_PLAYING_ID+1;
		else if ( m_bSoundEngineInitialized )// fallback to sound engine
		{
			return AK::SoundEngine::PostEvent( in_pszEvent, in_GameObjectID, in_uFlags, in_pfnCallback, in_pCookie );
		}
#endif	
	}
	return AK_INVALID_PLAYING_ID;
}

// Begin Outlast Change - plalonde
void UAkAudioDevice::StopPlayingID(
	AkPlayingID in_playingID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve
	)
{
	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::StopPlayingID( in_playingID, in_uTransitionDuration, in_eFadeCurve );
		}
	}
	else
	{
#ifdef AK_SOUNDFRAME
		bool bSuccess = false;
		if( m_pSoundFrame )
		{
			bSuccess = m_pSoundFrame->StopPlayingID( in_playingID, in_uTransitionDuration, in_eFadeCurve );
		}
		if( !bSuccess && m_bSoundEngineInitialized )
		{
			AK::SoundEngine::StopPlayingID( in_playingID, in_uTransitionDuration, in_eFadeCurve );
		}
#endif	
	}
}
// End Outlast Change

// Begin Outlast Change - plalonde
AKRESULT UAkAudioDevice::ExecuteActionOnEvent(
	class UAkEvent * in_pEvent, 
	AkActionType in_action,
	AActor * in_pActor, 
	FName in_nameBone /*= NAME_None*/, 
	AkTimeMs in_uTransitionDuration /*= 0*/,
	AkCurveInterpolation in_eFadeCurve /*= AkCurveInterpolation_Linear*/, 
	AkPlayingID in_playingID /*= AK_INVALID_PLAYING_ID*/
	)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if ( in_pEvent && eResult == AK_Success)
	{
		FString eventName( in_pEvent->GetName() );
		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::ExecuteActionOnEvent( *eventName, (AK::SoundEngine::AkActionOnEventType)in_action, GameObjID, in_uTransitionDuration, in_eFadeCurve, in_playingID );
			}
		}
		else
		{
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;
			/*if( m_pSoundFrame ) // plalonde - Not using this yet since there isn't an overload that takes a string.
			{
			bSuccess = m_pSoundFrame->ExecuteActionOnEvent( *eventName, (AK::SoundEngine::AkActionOnEventType)in_action, GameObjID, in_uTransitionDuration, in_eFadeCurve );
			}*/
			if( !bSuccess && m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::ExecuteActionOnEvent( *eventName, (AK::SoundEngine::AkActionOnEventType)in_action, GameObjID, in_uTransitionDuration, in_eFadeCurve, in_playingID );
			}
#endif	
		}
	}
	return in_pEvent == NULL ? AK_Fail : eResult;
}
// End Outlast Change

AKRESULT UAkAudioDevice::PostTrigger( 
	const TCHAR * in_pszTrigger,		///< Name of the trigger
	AActor * in_pActor,					///< Associated game object ID
	FName in_nameBone
	)
{
	AkGameObjectID GameObjID = AK_INVALID_GAME_OBJECT;
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{
		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::PostTrigger( in_pszTrigger, GameObjID );
			}
		}
		else
		{	
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;

			if( m_pSoundFrame )
				bSuccess = m_pSoundFrame->PostTrigger( in_pszTrigger, GameObjID );

			if( !bSuccess && m_bSoundEngineInitialized )
				eResult = AK::SoundEngine::PostTrigger( in_pszTrigger, GameObjID );// fallback to sound engine
#endif			
		}
	}
	return eResult;
} 

AKRESULT UAkAudioDevice::SetRTPCValue( 
	const TCHAR * in_pszRtpcName,				///< Name of the RTPC
	AkRtpcValue in_value, 						///< Value to set
	AActor * in_pActor,							///< Associated game object ID
	FName in_nameBone
	)
{
	AkGameObjectID GameObjID = AK_INVALID_GAME_OBJECT;
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{
		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::SetRTPCValue( in_pszRtpcName, in_value, GameObjID );
			}
		}
		else
		{
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;

			if( m_pSoundFrame )
				bSuccess = m_pSoundFrame->SetRTPCValue( in_pszRtpcName, in_value, GameObjID );

			if( !bSuccess && m_bSoundEngineInitialized )
				eResult = AK::SoundEngine::SetRTPCValue( in_pszRtpcName, in_value, GameObjID ); // fallback to sound engine
#endif	
			
		}
	}
	return eResult;
}

AKRESULT UAkAudioDevice::SetState( 
	const TCHAR * in_pszStateGroup,				///< Name of the state group
	const TCHAR * in_pszState 					///< Name of the state
    )
{
	AKRESULT eResult = AK_Success;
	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			eResult = AK::SoundEngine::SetState( in_pszStateGroup, in_pszState );
		}
	}
	else
	{
#ifdef AK_SOUNDFRAME
		bool bSuccess = false;

		if( m_pSoundFrame )
			bSuccess = m_pSoundFrame->SetCurrentState( in_pszStateGroup, in_pszState );

		if( !bSuccess && m_bSoundEngineInitialized )
			eResult = AK::SoundEngine::SetState( in_pszStateGroup, in_pszState );// fallback to sound engine
#endif	
	}
	return eResult;
}

AKRESULT UAkAudioDevice::SetSwitch( 
	const TCHAR * in_pszSwitchGroup,			///< Name of the switch group
	const TCHAR * in_pszSwitchState, 			///< Name of the switch
	AActor * in_pActor,							///< Associated game object ID
	FName in_nameBone
	)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	// Switches must be bound to a game object. passing DUMMY_GAMEOBJ as default game object.
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{
		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::SetSwitch( in_pszSwitchGroup, in_pszSwitchState, GameObjID );
			}
		}
		else
		{
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;

			if( m_pSoundFrame )
				bSuccess = m_pSoundFrame->SetCurrentSwitch( in_pszSwitchGroup, in_pszSwitchState, GameObjID );

			if( !bSuccess && m_bSoundEngineInitialized )
				eResult = AK::SoundEngine::SetSwitch( in_pszSwitchGroup, in_pszSwitchState, GameObjID );// fallback to sound engine
#endif	
		}
	}
	return eResult;
}

// rcharpentier
AKRESULT UAkAudioDevice::SetObstructionAndOcclusionOnAllComponents(
	FLOAT obstruction,
	FLOAT occlusion,
	AActor * in_pActor
	)
{
	AKRESULT eResult = AK_Success;
	
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	
	for ( INT i = 0; i < in_pActor->AllComponents.Num(); ++i )
	{
		UAkComponent * pComponent = Cast<UAkComponent>( in_pActor->AllComponents( i ) );
		if (pComponent) 
		{
			if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
			{
				if ( m_bSoundEngineInitialized )
				{
					eResult = AK::SoundEngine::SetObjectObstructionAndOcclusion( (AkGameObjectID)pComponent, 0/*ListenerIndex0*/, obstruction, occlusion );
				}
			}
			else
			{
#ifdef AK_SOUNDFRAME
				bool bSuccess = false;

				if( m_pSoundFrame )
				{
					bSuccess = m_pSoundFrame->SetObjectObstructionAndOcclusion( (AkGameObjectID)pComponent, 0/*ListenerIndex0*/, obstruction, occlusion );
				}

				if( !bSuccess && m_bSoundEngineInitialized )
				{
					eResult = AK::SoundEngine::SetObjectObstructionAndOcclusion( (AkGameObjectID)pComponent, 0/*ListenerIndex0*/, obstruction, occlusion );
				}
#endif	
			}

			if (eResult != AK_Success)
			{
				return eResult;
			}
		}
	}

	return eResult;
}


// rcharpentier
AKRESULT UAkAudioDevice::SetObstructionAndOcclusion(
	FLOAT obstruction,
	FLOAT occlusion,
	AActor * in_pActor,
	FName in_nameBone
	)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{		
		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::SetObjectObstructionAndOcclusion( GameObjID, 0/*ListenerIndex0*/, obstruction, occlusion );
			}
		}
		else
		{
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;

			if( m_pSoundFrame )
			{
				bSuccess = m_pSoundFrame->SetObjectObstructionAndOcclusion( GameObjID, 0/*ListenerIndex0*/, obstruction, occlusion );
			}

			if( !bSuccess && m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::SetObjectObstructionAndOcclusion( GameObjID, 0/*ListenerIndex0*/, obstruction, occlusion );
			}
#endif	
		}
	}
	return eResult;
}

// rcharpentier
AKRESULT UAkAudioDevice::SetGameObjectAuxSendValuesOnAllComponents(
	const TArray<AkAuxBusValue>& in_auxValues,
	AActor * in_pActor
	)
{
	AKRESULT eResult = AK_Success;

	TArray<AkAuxSendValue> SendValues(in_auxValues.Num());

	for (INT Idx = 0; Idx < in_auxValues.Num(); ++Idx)
	{
		SendValues(Idx).auxBusID = AK::SoundEngine::GetIDFromString( in_auxValues(Idx).AuxBus );
		SendValues(Idx).fControlValue = in_auxValues(Idx).ControlValue;
	}

	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;

	for ( INT i = 0; i < in_pActor->AllComponents.Num(); ++i )
	{
		UAkComponent * pComponent = Cast<UAkComponent>( in_pActor->AllComponents( i ) );
		if (pComponent) 
		{
			if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
			{
				if ( m_bSoundEngineInitialized )
				{
					eResult = AK::SoundEngine::SetGameObjectAuxSendValues( (AkGameObjectID)pComponent, SendValues.GetData(), SendValues.Num() );
				}
			}
			else
			{
#ifdef AK_SOUNDFRAME
				bool bSuccess = false;

				if( m_pSoundFrame )
				{
					bSuccess = m_pSoundFrame->SetGameObjectAuxSendValues( (AkGameObjectID)pComponent, SendValues.GetData(), SendValues.Num() );
				}

				if( !bSuccess && m_bSoundEngineInitialized )
				{
					eResult = AK::SoundEngine::SetGameObjectAuxSendValues( (AkGameObjectID)pComponent, SendValues.GetData(), SendValues.Num() );// fallback to sound engine
				}
#endif	
			}

			if (eResult != AK_Success)
			{
				return eResult;
			}
		}
	}

	return eResult;
}

// plalonde
AKRESULT UAkAudioDevice::SetGameObjectAuxSendValues(
	const TArray<AkAuxBusValue>& in_auxValues,
	AActor * in_pActor,
	FName in_nameBone
	)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	// Switches must be bound to a game object. passing DUMMY_GAMEOBJ as default game object.
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{
		TArray<AkAuxSendValue> SendValues(in_auxValues.Num());

		for (INT Idx = 0; Idx < in_auxValues.Num(); ++Idx)
		{
			SendValues(Idx).auxBusID = AK::SoundEngine::GetIDFromString( in_auxValues(Idx).AuxBus );
			SendValues(Idx).fControlValue = in_auxValues(Idx).ControlValue;
		}

		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::SetGameObjectAuxSendValues( GameObjID, SendValues.GetData(), SendValues.Num() );
			}
		}
		else
		{
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;

			if( m_pSoundFrame )
				bSuccess = m_pSoundFrame->SetGameObjectAuxSendValues( GameObjID, SendValues.GetData(), SendValues.Num() );

			if( !bSuccess && m_bSoundEngineInitialized )
				eResult = AK::SoundEngine::SetGameObjectAuxSendValues( GameObjID, SendValues.GetData(), SendValues.Num() );// fallback to sound engine
#endif	
		}
	}
	return eResult;
}

AKRESULT UAkAudioDevice::SetGameObjectOutputBusVolume(
	AkReal32 in_volume,
	AActor * in_pActor,
	FName in_nameBone
	)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	// Switches must be bound to a game object. passing DUMMY_GAMEOBJ as default game object.
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{
		if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
		{
			if ( m_bSoundEngineInitialized )
			{
				eResult = AK::SoundEngine::SetGameObjectOutputBusVolume( GameObjID, in_volume );
			}
		}
		else
		{
#ifdef AK_SOUNDFRAME
			bool bSuccess = false;

			if( m_pSoundFrame )
				bSuccess = m_pSoundFrame->SetGameObjectOutputBusVolume( GameObjID, in_volume );

			if( !bSuccess && m_bSoundEngineInitialized )
				eResult = AK::SoundEngine::SetGameObjectOutputBusVolume( GameObjID, in_volume );// fallback to sound engine
#endif	
		}
	}
	return eResult;
}

AKRESULT UAkAudioDevice::ActivateOcclusion(
		bool in_bActivate,
		AActor * in_pActor, 
		FName in_nameBone
		)
{
	AKRESULT eResult = AK_Success;

	// Occlusion not activated. Unreal occlusion detection system ends up considering most things as occluded.
	// Enable the following code to start using it.
#if 0
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	// Switches must be bound to a game object. passing DUMMY_GAMEOBJ as default game object.
	AKRESULT eResult = GetGameObjectID( in_pActor, in_nameBone, GameObjID );
	if( eResult == AK_Success )
	{
		if ( m_bSoundEngineInitialized )
		{
			eResult = AK::SoundEngine::SetObjectObstructionAndOcclusion( GameObjID, 0/*ListenerIndex0*/, 0.f, in_bActivate?0.5f:0.0f );
		}
	}
#endif
	return eResult;
}

void UAkAudioDevice::GetProjectAudioFileRoot(
		FString& out_sRoot
		)
{
#ifdef AK_SOUNDFRAME
	out_sRoot = m_pSoundFrame->GetCurrentProjectOriginalRoot();
	if( out_sRoot != TEXT("") )
	{
		// replace the existing setting only if a new path is available
		GConfig->SetString(TEXT("Wwise.Options"), TEXT("LinkedProjectOriginalPath"), *out_sRoot, GEditorIni);
	}
#endif
}

#ifdef AK_SOUNDFRAME

void UAkAudioDevice::OnConnect( 
		bool in_bConnect		///< True if Wwise is connected, False if it is not
		)
{
	if ( in_bConnect && !( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld ) )
	{
		// Make sure all game objects are registered
		FString dummyOriginalRoot;
		GetProjectAudioFileRoot( dummyOriginalRoot );// Simply update the path on connection.

		m_pSoundFrame->RegisterGameObject( DUMMY_GAMEOBJ, L"Unreal Global" );

		for ( int i = 0; i < m_GameObjects.Num(); ++i )
		{
			m_pSoundFrame->RegisterGameObject( (AkGameObjectID)( m_GameObjects( i ) ) ); // FIXME: game object names
		}
	}
}

UBOOL UAkAudioDevice::IsConnected() const
{
	if (m_pSoundFrame != NULL)
	{
		return m_pSoundFrame->IsConnected();
	}
	
	return FALSE;
}

AkTimeMs UAkAudioDevice::GetCaptureTimeStamp()
{
	return m_TimeStamp;
}
#endif

#define GAME_OBJECT_MAX_STRING_SIZE 512

void UAkAudioDevice::RegisterComponent( UAkComponent * in_pComponent )
{
	m_GameObjects.AddItem( in_pComponent );

	FString objectName;
	
	if (in_pComponent->BoneName == NAME_None)
	{
		objectName = in_pComponent->GetOwner()->GetFName().ToString();
	}
	else
	{
		objectName = FString::Printf(TEXT("%s_(%s)"), *in_pComponent->GetOwner()->GetFName().ToString(), *in_pComponent->BoneName.ToString());
	}
	const TCHAR * tszName = *objectName;

	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
#ifndef AK_OPTIMIZED
		char szName[ GAME_OBJECT_MAX_STRING_SIZE ];
#if defined( __PPU__) || defined( NGP ) || defined( __MACOSX__ )
		wcstombs( szName, tszName, GAME_OBJECT_MAX_STRING_SIZE );
#else
		size_t nConverted;
		wcstombs_s( &nConverted, szName, GAME_OBJECT_MAX_STRING_SIZE, tszName, GAME_OBJECT_MAX_STRING_SIZE );
#endif
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::RegisterGameObj( (AkGameObjectID) in_pComponent, szName );
		}
#else // AK_OPTIMIZED
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::RegisterGameObj( (AkGameObjectID) in_pComponent );
		}
#endif // AK_OPTIMIZED
	}
	else
	{
#ifdef AK_SOUNDFRAME
		if( m_pSoundFrame )
			m_pSoundFrame->RegisterGameObject( (AkGameObjectID) in_pComponent, tszName );

#ifndef AK_OPTIMIZED
		char szName[ GAME_OBJECT_MAX_STRING_SIZE ];
#if defined( __PPU__) || defined( NGP ) || defined( __MACOSX__ )
		wcstombs( szName, tszName, GAME_OBJECT_MAX_STRING_SIZE );
#else
		size_t nConverted;
		wcstombs_s( &nConverted, szName, GAME_OBJECT_MAX_STRING_SIZE, tszName, GAME_OBJECT_MAX_STRING_SIZE );
#endif
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::RegisterGameObj( (AkGameObjectID) in_pComponent, szName );
		}
#else // AK_OPTIMIZED
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::RegisterGameObj( (AkGameObjectID) in_pComponent );
		}
#endif // AK_OPTIMIZED

#endif // AK_SOUNDFRAME
	}
}

void UAkAudioDevice::UnregisterComponent( UAkComponent * in_pComponent )
{
	for ( int i = 0; i < m_GameObjects.Num(); ++i )
	{
		if ( in_pComponent == m_GameObjects( i ) )
		{
			m_GameObjects.RemoveSwap( i );

			if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
			{
				if ( m_bSoundEngineInitialized )
				{
					AK::SoundEngine::UnregisterGameObj( (AkGameObjectID) in_pComponent );
				}
			}
#ifdef AK_SOUNDFRAME
			else if( m_pSoundFrame )
				m_pSoundFrame->UnregisterGameObject( (AkGameObjectID) in_pComponent );
#endif

			return;
		}
	}
}

UAkAudioDevice * UAkAudioDevice::Get()
{
	return GEngine && GEngine->Client ? (UAkAudioDevice *) GEngine->Client->GetAkAudioDevice() : NULL;
}

UAkComponent * UAkAudioDevice::GetAkComponent( AActor * in_pActor, FName in_nameBone, bool in_bStopWhenOwnerDestroyed /*= false*/ )
{
	// Find an existing AkComponent to reuse

	for ( INT i = 0; i < in_pActor->AllComponents.Num(); ++i )
	{
		UAkComponent * pComponent = Cast<UAkComponent>( in_pActor->AllComponents( i ) );
		if ( pComponent && in_nameBone == pComponent->BoneName ) 
			return pComponent;
	}

	// Create one otherwise.

	UAkComponent * pComponent = ConstructObject<UAkComponent>( UAkComponent::StaticClass(), in_pActor );

	pComponent->BoneName = in_nameBone;
	pComponent->bStopWhenOwnerDestroyed = in_bStopWhenOwnerDestroyed;
	pComponent->ConditionalAttach( GWorld->Scene, in_pActor, in_pActor->LocalToWorld() );	
	
	return pComponent;
}

UBOOL UAkAudioDevice::IsAkComponentValid( UAkComponent* in_pComponent )
{
	INT Index;
	return m_GameObjects.FindItem(in_pComponent, Index);
}

AKRESULT UAkAudioDevice::GetGameObjectID( AActor * in_pActor, FName in_nameBone, AkGameObjectID& io_GameObject, bool in_bStopWhenOwnerDestroyed /*= false*/ )
{
	if ( in_pActor )
	{
		UAkComponent * pComponent = GetAkComponent( in_pActor, in_nameBone, in_bStopWhenOwnerDestroyed );
		if ( pComponent )
		{
			io_GameObject = (AkGameObjectID) pComponent;
			return AK_Success;
		}
		else
			return AK_Fail;
	}

	// we do not modify io_GameObject, letting it to the specified default value.
	return AK_Success;
}

void UAkAudioDevice::StopGameObject( AkGameObjectID in_GameObjID )
{
	if ( AK_USE_SOUNDENGINE || GIsPlayInEditorWorld )
	{
		if ( m_bSoundEngineInitialized )
		{
			AK::SoundEngine::StopAll( in_GameObjID );
		}
	}
#ifdef AK_SOUNDFRAME
	else if( m_pSoundFrame )
	{
		m_pSoundFrame->StopAll( in_GameObjID );
	}
#endif
}

void UAkAudioDevice::Flush( FSceneInterface* SceneToFlush )
{
	// Stop all audio components attached to the scene
	for( INT ComponentIndex = m_GameObjects.Num() - 1; ComponentIndex >= 0; ComponentIndex-- )
	{
		UAkComponent* AudioComponent = m_GameObjects( ComponentIndex );
		if( AudioComponent )
		{
			FSceneInterface* ComponentScene = AudioComponent->GetScene();
			if( SceneToFlush == NULL || ComponentScene == NULL || ComponentScene == SceneToFlush )
			{
				AudioComponent->Stop();
			}
		}
	}

	UAkAudioDevice::StopGameObject( DUMMY_GAMEOBJ );
}

void UAkAudioDevice::CancelEventCallbackCookie( void* in_cookie )
{
	if ( m_bSoundEngineInitialized )
	{
		AK::SoundEngine::CancelEventCallbackCookie( in_cookie );
	}
}