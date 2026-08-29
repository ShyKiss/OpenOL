/*=============================================================================
	LaunchPrivate.h: Unreal launcher.
	Copyright 1998-2012 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if _WINDOWS || PLATFORM_MACOSX || DINGO

#if WITH_EDITOR //have to have the editor compiled in at least
#define HAVE_WXWIDGETS 1
#else
#define HAVE_WXWIDGETS 0
#endif

//@warning: this needs to be the very first include
#if _WINDOWS && WITH_EDITOR
#include "UnrealEd.h"
#endif

#include "Engine.h"
#include "UnIpDrv.h"
#include "DemoRecording.h"

#if _WINDOWS && !DINGO
#include "WinDrv.h"
#endif

#include "LaunchGames.h"
#include "FMallocDebug.h"
#include "FMallocProfiler.h"
#include "ScriptCallstackDecoder.h"
#include "MallocProfilerEx.h"
#include "FMallocProxySimpleTrack.h"
#include "FMallocProxySimpleTag.h"
#include "FMallocThreadSafeProxy.h"
#include "FFeedbackContextAnsi.h"
#include "FCallbackDevice.h"
#include "FConfigCacheIni.h"
#include "LaunchEngineLoop.h"

#if _WINDOWS
#include "FMallocAnsi.h"

#if _WIN64
#include "FMallocTBB.h"
#include "UnThreadingWindows.h"
#else
#include "MallocBinned.h"
#endif
#include "SplashScreen.h"
#include "FFeedbackContextWindows.h"
#include "FFileManagerWindows.h"
#elif PLATFORM_MACOSX
#include "FFileManagerMac.h"
#include "MacThreading.h"
#include "AsyncLoadingMac.h"
#elif DINGO
#include "FMallocDingo.h"
#include "FMallocDingoXMem.h"
#if DINGO_USES_TBB_MALLOC
#include "FMallocTBB.h"
#endif //DINGO_USES_TBB_MALLOC
#include "FFileManagerDingo.h"
#else
#error Please define your platform.
#endif

#endif
