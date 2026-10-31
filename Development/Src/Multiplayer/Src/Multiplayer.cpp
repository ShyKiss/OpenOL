/*=============================================================================
    Multiplayer.cpp: Multiplayer package registration.
=============================================================================*/

#include "Multiplayer.h"
#include "HeroChannel.h"
#include "MultiplayerHUD.h"

// GMultiplayerController, GMultiplayerHero, GMpConn defined in MultiplayerLink.cpp.
FHeroChannelTicker        GHeroChannelTicker;
FHeroChannelReceiveTicker GHeroChannelReceiveTicker;

IMPLEMENT_CLASS(UHeroChannel);
IMPLEMENT_CLASS(UDoorChannel);
IMPLEMENT_CLASS(UEnemyChannel);
IMPLEMENT_CLASS(UPushableChannel);
IMPLEMENT_CLASS(UWorldChannel);
IMPLEMENT_CLASS(AMultiplayerHero);
IMPLEMENT_CLASS(AMultiplayerController);
IMPLEMENT_CLASS(AMultiplayerHUD);

#if WITH_UE3_NETWORKING

#define STATIC_LINKING_MOJO 1

// Register names and functions.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName MULTIPLAYER_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "MultiplayerClasses.h"
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#undef NAMES_ONLY

// Import natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "MultiplayerClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants.
 */
static void MpReloadConfigCallback()
{
    GMpConn.bIsConnected  = FALSE;
    GMpConn.bIsHandshaked = FALSE;
    GMpConn.bResolved     = FALSE;
    GResolveInfo          = NULL;
    if (GMultiplayerController)
        GMultiplayerController->OnDisconnected();
    GMpConn.Connect();
}

void AutoInitializeRegistrantsMultiplayer( INT& Lookup )
{
    AUTO_INITIALIZE_REGISTRANTS_MULTIPLAYER;
    GReloadConfigCallback = &MpReloadConfigCallback;

    extern void SimNode_Init();
    SimNode_Init();
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesMultiplayer()
{
    #define NAMES_ONLY
    #define AUTOGENERATE_NAME(name) MULTIPLAYER_##name = FName(TEXT(#name));
        #include "MultiplayerNames.h"
    #undef AUTOGENERATE_NAME
    #define AUTOGENERATE_FUNCTION(cls,idx,name)
        #include "MultiplayerClasses.h"
    #undef AUTOGENERATE_FUNCTION
    #undef NAMES_ONLY
}

#endif // WITH_UE3_NETWORKING
