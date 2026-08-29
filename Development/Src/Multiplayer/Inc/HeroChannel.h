#pragma once
#include "Multiplayer.h"
#include "HeroChannelPackets.h"

#define HERO_STATE_MIN_SIZE 62

// Virtual SMT codes used only in the multiplayer layer for corner-peek transitions.
// Not part of ESpecialMoveType; sent as NewSpecialMove in ApplySpecialMoveTransition.
#define SMT_MP_WallToPeek   118
#define SMT_MP_LeavePeek    119
#define SMT_MP_WallExit     120
#define SMT_MP_WallTransition 121

// ============================================================================
// Binary read/write helpers — LE, BYTE* buffer, returns new offset
// ============================================================================

FORCEINLINE INT PutU8 (BYTE* B, INT N, INT V)  { B[N]=(BYTE)(V&0xFF); return N+1; }
FORCEINLINE INT PutU16(BYTE* B, INT N, INT V)  { B[N]=(BYTE)(V&0xFF); B[N+1]=(BYTE)((V>>8)&0xFF); return N+2; }
FORCEINLINE INT PutI16(BYTE* B, INT N, INT V)  { return PutU16(B,N,V); }
FORCEINLINE INT PutI32(BYTE* B, INT N, INT V)  { B[N]=(BYTE)(V&0xFF); B[N+1]=(BYTE)((V>>8)&0xFF); B[N+2]=(BYTE)((V>>16)&0xFF); B[N+3]=(BYTE)((V>>24)&0xFF); return N+4; }
FORCEINLINE INT PutF32(BYTE* B, INT N, FLOAT V){ DWORD U; appMemcpy(&U,&V,4); return PutI32(B,N,(INT)U); }

FORCEINLINE INT ReadU8 (const BYTE* B, INT N, INT& V) { V=(INT)B[N]; return N+1; }
FORCEINLINE INT ReadU16(const BYTE* B, INT N, INT& V) { V=(INT)B[N]|((INT)B[N+1]<<8); return N+2; }
FORCEINLINE INT ReadI16(const BYTE* B, INT N, INT& V) { INT U=(INT)B[N]|((INT)B[N+1]<<8); V=(U>=0x8000)?(U-0x10000):U; return N+2; }
FORCEINLINE INT ReadI32(const BYTE* B, INT N, INT& V) { DWORD U=(DWORD)B[N]|((DWORD)B[N+1]<<8)|((DWORD)B[N+2]<<16)|((DWORD)B[N+3]<<24); V=(INT)U; return N+4; }
FORCEINLINE INT ReadF32(const BYTE* B, INT N, FLOAT& V){ DWORD U=(DWORD)B[N]|((DWORD)B[N+1]<<8)|((DWORD)B[N+2]<<16)|((DWORD)B[N+3]<<24); appMemcpy(&V,&U,4); return N+4; }

// ============================================================================
// Decoded hero state — filled once by BuildDecodedState, applied to dummy
// ============================================================================

struct FHeroStatePacket
{
    FVector  Loc;
    FVector  Vel;
    FRotator Rot;
    INT      Health;
    INT      CamcorderState;
    INT      SpecialMove;
    INT      LocomotionMode;
    UBOOL    bCrouched;
    UBOOL    bHeatShielding;
    FLOAT    HeatDistance;
    FLOAT    LadderDelta;
    FLOAT    LedgeHangDelta;
    FLOAT    LedgeWalkDelta;
    FLOAT    SqueezeDelta;
    UBOOL    bHobbling;
    FLOAT    HobblingIntensity;
    FLOAT    TargetHobblingIntensity;
    UBOOL    bLimping;
    FLOAT    CurrentLean;
    FLOAT    PeekingRatio;
    FLOAT    CornerIKStrength;
    UBOOL    bCamMeshVisible;
    UBOOL    bLeftAnim;
    INT      EyeYaw;
    UBOOL    bInDarkness;
    UBOOL    bIsGhost;
    UBOOL    bParrying;
    FLOAT    ParryEnemyDist;
    FLOAT    ParryEnemyRelYaw;
    // 0=None, 1=Water, 2=Blood — surface material under feet at last footstep
    INT      FootstepSurface;
    // Struggle cycle anim play rate (0 when not struggling)
    FLOAT    StrugglePlayRate;
    // Optional tail — only present on first packet (len > HERO_STATE_MIN_SIZE)
    TCHAR    Nick[33];
    UBOOL    bHasNick;
};

// Free functions — helpers not exposed in UC/UMake-generated header.
void              BuildStatePacket(AOLHero* Hero, UBOOL bSendingJumpGroundZ, FLOAT JumpGroundZ, BYTE* Out, INT& OutLen);
UBOOL             DecodeBinaryState(const BYTE* Data, INT DataLen, FHeroStatePacket& Out);
AMultiplayerHero* SpawnDummy(AMultiplayerController* Controller);

// ============================================================================
// FHeroChannelTicker — drives UHeroChannel::SendGlobalState() every engine tick
// without requiring UHeroChannel to inherit FTickableObject (which would conflict
// with the UMake-generated class definition in MultiplayerClasses.h).
// ============================================================================

class FHeroChannelTicker : public FTickableObject
{
public:
    UHeroChannel* Channel;

    FHeroChannelTicker() : Channel(NULL) {}

    virtual void  Tick(FLOAT DeltaTime);
    virtual UBOOL IsTickable() const;
    virtual UBOOL IsTickableWhenPaused() const { return FALSE; }
};

// Second ticker — runs even without a local hero (receive-side interpolation).
class FHeroChannelReceiveTicker : public FTickableObject
{
public:
    UHeroChannel* Channel;

    FHeroChannelReceiveTicker() : Channel(NULL) {}

    virtual void  Tick(FLOAT DeltaTime);
    virtual UBOOL IsTickable() const;
    virtual UBOOL IsTickableWhenPaused() const { return FALSE; }
};

extern FHeroChannelReceiveTicker GHeroChannelReceiveTicker;

extern FHeroChannelTicker GHeroChannelTicker;

// Called from TryCSA at the moment of activation — sends SMT_CSA packet immediately
// instead of waiting for the next tick (which would miss instant CSAs).
// AnimStart/AnimFwd are the expectedAnimStart/expectedAnimFwd computed in TryCSA.
void MpSendCSAActivation(AOLCSA* CSA, const FVector& AnimStart, const FVector& AnimFwd);
