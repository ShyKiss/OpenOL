#ifndef OLUTILIES_H
#define OLUTILIES_H

#ifdef _DEBUG
#define TWEAKABLE static
#else
#define TWEAKABLE const
#endif

extern class AOLPlayerController* GOLPC;

FORCEINLINE FVector VecZ(FLOAT z) { return FVector(0, 0, z); }
FORCEINLINE FVector Vec2D(const FVector& v) { return FVector(v.X, v.Y, 0); }
template< class T > FORCEINLINE UBOOL IsBetween(T x, T minVal, T maxVal) { return (x >= minVal && x <= maxVal); }
template< class T, class U > FORCEINLINE U MapClamped(T x, T minX, T maxX, U outEdge0, U outEdge1) 
{ 
	return Clamp(outEdge0 + (outEdge1 - outEdge0)*(x - minX)/(maxX - minX), Min(outEdge0, outEdge1), Max(outEdge0, outEdge1));
}
FORCEINLINE FLOAT Unlerp(FLOAT val, FLOAT edge0, FLOAT edge1) // returns 0.0f if val is past edge 0, 1.0f if past edge1, and lerped in between - same as MapClamped(val, edge0, edge1, 0.0f, 1.0f)
{ 
	return Clamp((val - edge0) / (edge1 - edge0), 0.0f, 1.0f);
}
FORCEINLINE FLOAT LerpClamped(FLOAT alpha, FLOAT minVal, FLOAT maxVal) 
{ 
	return MapClamped(alpha, 0.0f, 1.0f, minVal, maxVal);
}
FORCEINLINE FLOAT Saturate(FLOAT value)
{
	return Clamp(value, 0.0f, 1.0f);
}

namespace Utils
{
	FString	GetEnumString(const char* enumTypeStr, BYTE val);
	
	// Returns whether the given position is between the markers. 
	// A positive buffer distance allows being outside of the markers, while a negative value constrains within the markers
	// If extendContinuousSegments is set, the buffer size is reduced to 0 if the segments extends in a continuous line
	UBOOL	IsBetweenMarkers(const FVector& pos, const class AOLLedgeMarker* node1, const class AOLLedgeMarker* node2, UBOOL extendContinuousSegments = TRUE, FLOAT buffer = 10.0f);
	UBOOL	IsBetweenMarkers(const FVector& pos, const FVector& node1, const FVector& node2, FLOAT buffer = 10.0f);

	FORCEINLINE FLOAT SmootherStep(FLOAT t)
	{
		return t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);
	}

	FORCEINLINE FLOAT SmootherStep(FLOAT t, FLOAT a, FLOAT b)
	{
		FLOAT x = Clamp((t - a)/(b - a), 0.0f, 1.0f);
		return SmootherStep(x);
	}

	FORCEINLINE FLOAT SmoothLerp(FLOAT alpha, FLOAT minVal, FLOAT maxVal)
	{
		return LerpClamped(SmootherStep(alpha), minVal, maxVal);
	}

	template<class T> FORCEINLINE T Approach(const T& current, const T& target, FLOAT coeff, FLOAT deltaTime)
	{
		return target + (current - target) * appPow( (1.0f-coeff), deltaTime);
	}

	FORCEINLINE FLOAT ApproachAxis(const FLOAT current, FLOAT target, FLOAT coeff, FLOAT deltaTime) // UNR
	{
		return target + FRotator::NormalizeAxis(current - target) * appPow( (1.0f-coeff), deltaTime);
	}

	FORCEINLINE FRotator RApproach(const FRotator& current, const FRotator& target, FLOAT coeff, FLOAT deltaTime)
	{
		return target + (current - target).GetNormalized() * appPow( (1.0f-coeff), deltaTime);
	}

	FORCEINLINE FLOAT NormalizeRotAngle(FLOAT angle)
	{
		angle = appFmod(angle, 360.0f);

		if (angle <= -180.0f)
		{
			angle += 360.0f;
		}
		else if (angle >= 180.0f)
		{
			angle -= 360.0f;
		}
		checkSlow(angle >= -180.0f && angle <= 180.0f);
		return angle;
	}

	FORCEINLINE FLOAT ApproachAngle(const FLOAT current, FLOAT target, FLOAT coeff, FLOAT deltaTime) // Degrees
	{
		return target + Utils::NormalizeRotAngle(current - target) * appPow( (1.0f-coeff), deltaTime);
	}

	FORCEINLINE FLOAT LerpAngle(const FLOAT a, FLOAT b, FLOAT alpha) // Degrees
	{
		return Utils::NormalizeRotAngle(a + alpha*Utils::NormalizeRotAngle(b - a));
	}

	class AOLPlayerController* GetOLPC();
	class AOLHero* GetHero();
	class UOLCheatManager* GetCheatManager();
	class AOLGame* GetOLGame();
	class UOLSoundEnvironmentManager* GetSoundEnvManager();
	class UOLFXManager* GetFXManager();
	class AOLHUD* GetHUD();
	class UPostProcessChain* GetDefaultPostProcessChain();
	class UPostProcessChain* GetCameraPostProcessChain();
	class UPostProcessChain* GetCameraNVPostProcessChain();

	FLOAT GetAspectRatio();
	FName GetCameraBoneName();	
	UBOOL IsDemo();
	UBOOL IsDLCInstalled();
	UBOOL IsPlayingDLC();
	UBOOL IsInMainMenu();
	UBOOL IsTravelling();
	class AOLCheckpoint* GetCheckpointFromName(FName checkpointName);
	UBOOL IsCheckpointValid(FName checkpointName);
	UBOOL IsCheckpointReached(FName checkpointName);
	UBOOL IsCheckpointUnreached(FName checkpointName);
	UBOOL IsCheckpointCompleted(FName checkpointName);
	UBOOL IsCheckpointDLC(FName checkpointName); // must be called with the dlc checkpoint list loaded (i.e. returns false if dlc isn't installed)
	FName GetCurrentCheckpointName();
	FName GetCheckpointTag(FName checkpointName); // ("Admin_MainHall" -> "Admin")
	void PrintCheckpointList();

	// Replace {OLA_X} tags by the current key binding
	FString TranslateKeyBindings(const FString& text);

	void OutputTextToConsole(const FString& text);

	AActor* SpawnActorFromPackage(const FString& PackageName, UClass* ActorClass, FName ActorTag = NAME_None);
	UObject* LoadObjectFromModPackage(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass);
	// Load any mod package (upk or udk) by short name. No AddToRoot — caller decides lifetime.
	// Returns NULL if not found or failed to load.
	UPackage* LoadModPackage(const FString& PackageName);

}

// ---------------------------------------------------------------------------
// UTF-8 helpers shared by multiplayer packet encode/decode
// ---------------------------------------------------------------------------

// Encode TCHAR string to UTF-8 bytes. Returns byte count written.
// Dst must have at least MaxBytes bytes available.
FORCEINLINE INT TCHARToUTF8(BYTE* Dst, INT MaxBytes, const TCHAR* Src)
{
    INT Out = 0;
    for (INT i = 0; Src[i] && Out < MaxBytes - 3; ++i)
    {
        unsigned int C = (unsigned int)(unsigned short)Src[i];
        if (C < 0x80)
            { Dst[Out++] = (BYTE)C; }
        else if (C < 0x800)
            { Dst[Out++] = (BYTE)(0xC0 | (C >> 6)); Dst[Out++] = (BYTE)(0x80 | (C & 0x3F)); }
        else
            { Dst[Out++] = (BYTE)(0xE0 | (C >> 12)); Dst[Out++] = (BYTE)(0x80 | ((C >> 6) & 0x3F)); Dst[Out++] = (BYTE)(0x80 | (C & 0x3F)); }
    }
    return Out;
}

// Decode UTF-8 bytes into OutStr.
FORCEINLINE void UTF8ToFString(const BYTE* Src, INT SrcLen, FString& OutStr)
{
    TCHAR Buf[256]; INT Out = 0;
    for (INT i = 0; i < SrcLen && Out < 255; )
    {
        BYTE B0 = Src[i];
        unsigned int C;
        if ((B0 & 0x80) == 0)                          { C = B0; i += 1; }
        else if ((B0 & 0xE0) == 0xC0 && i+1 < SrcLen) { C = ((B0&0x1F)<<6)|(Src[i+1]&0x3F); i += 2; }
        else if ((B0 & 0xF0) == 0xE0 && i+2 < SrcLen) { C = ((B0&0x0F)<<12)|((Src[i+1]&0x3F)<<6)|(Src[i+2]&0x3F); i += 3; }
        else { i++; continue; }
        Buf[Out++] = (TCHAR)C;
    }
    Buf[Out] = 0;
    OutStr = FString(Buf);
}

enum EOLStatGroups
{
	STATGROUP_OLAudio = STATGROUP_LicenseeFirstStatGroup
};

enum EOLAudioStats
{
	STAT_SoundEnvironmentManagerTickTime = STAT_LicenseeFirstStat,
	STAT_UpdateListenerVolumes,
	STAT_UpdateReverb,
	STAT_UpdateDynamicActor,
	STAT_UpdateStaticActor,
	STAT_UpdateDynamicAudioParams,
	STAT_UpdateMultiPositionSources
};

#endif