#pragma once
#include "Multiplayer.h"
#include "PushableChannelPackets.h"

// OnBinaryPushState is a plain C++ method on UPushableChannel (not a UC native function),
// so UMake does not generate its declaration. Declared here for use from MultiplayerController.cpp.
void PushableChannel_OnBinaryPushState(UPushableChannel* Ch, INT SenderID, BYTE* Data, INT DataLen);
void PushableChannel_OnBinaryPushDenied(UPushableChannel* Ch, BYTE* Data, INT DataLen);
