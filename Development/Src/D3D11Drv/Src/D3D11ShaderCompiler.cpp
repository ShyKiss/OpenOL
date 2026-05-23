/*=============================================================================
	D3D11ShaderCompiler.cpp: D3D shader compiler implementation.
	Copyright 1998-2012 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D11DrvPrivate.h"

#if !UE3_LEAN_AND_MEAN

#if _WIN64
#pragma pack(push,16)
#else
#pragma pack(push,8)
#endif
#define D3D_OVERLOADS 1
#include "D3Dcompiler.h"
#include <d3d11Shader.h>
#pragma pack(pop)

//
//@igs(nrm):  Durango-specific shader pre-compilation support
//
// The Durango shader compiler uses the exact same API interface as D3D, it just compiles all the way down to Durango microcode instead of D3D bytecode (and supports a few extra defines and switches).
// After looking at our options for adding Durango shader pre compilation, I decided to use the built-in PC D3D11 shader compiler interface and just when we detect we are compiling for the SP_DINGO target,
// Load up the Durango DLL and use the functions from that DLL instead of the built int one.  So long as we don't need to do any substantial Durango specific shader processing work, this solution should work cleanly.
// 
//  If the Durango shader compiler path diverges significantly, it would make sense to move it to Dingo Tools and make it use the 'Console' shader compiler path.  This would require some rearchitecting of that
// path though since it doesn't support compiling hull/domain/geometry/compile shaders and it doesn't understand constant buffers.
//
// @NOTE:  You _must_ use the *_TargetWrapper functions throughout the code to make sure Durango is supported
//
typedef HRESULT (WINAPI *pD3DReflect)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);
typedef HRESULT (WINAPI *pD3DGetDebugInfo)(LPCVOID pSrcData, _In_ SIZE_T SrcDataSize, _Out_ ID3DBlob** ppDebugInfo);
typedef HRESULT (WINAPI *pD3DGetBlobPart)(LPCVOID pSrcData, _In_ SIZE_T SrcDataSize, _In_ D3D_BLOB_PART Part, _In_ UINT Flags, _Out_ ID3DBlob** ppPart);
typedef HRESULT (WINAPI *pD3DStripShader)(LPCVOID pShaderBytecode, _In_ SIZE_T BytecodeLength, _In_ UINT uStripFlags, _Out_ ID3DBlob** ppStrippedBlob);

// The handle to the Durango shader compiler DLL and the function pointers we get out of the Durango DLL
HMODULE DurangoD3DCompilerDLL = NULL;
pD3DPreprocess DurangoD3DPreprocessFunc = NULL;
pD3DCompile DurangoD3DCompileFunc = NULL;
pD3DReflect DurangoD3DReflectFunc = NULL;
pD3DGetDebugInfo DurangoD3DGetDebugInfo = NULL;
pD3DGetBlobPart DurangoD3DGetBlobPart = NULL;
pD3DStripShader DurangoD3DStripShaderFunc = NULL;

// Call this before making any calls to Durango specific shader compile functions to make sure the Durango DLL can be loaded and get the function pointers out of it.
void InitializeDurangoShaderCompileDLL()
{
	if( DurangoD3DCompilerDLL == NULL )
	{
		TCHAR DurangoSDKPath[MAX_PATH];
		//DWORD Result = GetEnvironmentVariable(TEXT("DurangoXDK"), DurangoSDKPath, MAX_PATH);
		//if( !Result )
		//{
		//	appErrorf( TEXT("Unable to find %%DurangoXDK%% Environment variable...  Make sure you have installed the DurangoXDK!") );
		//}

		FString DurangoDLLPath = FString::Printf( TEXT("%s\\xdk\\FXC\\amd64\\D3DCompiler_46_xdk.dll"), DurangoSDKPath );
		//DurangoD3DCompilerDLL = LoadLibraryEx( *DurangoDLLPath, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR );
		DurangoD3DCompilerDLL = LoadLibraryEx(L"C:\\Program Files (x86)\\Microsoft Durango XDK\\180713\\xdk\\FXC\amd64\\D3DCompiler_47_xdk.dll", NULL, 0x00000100);
		if( !DurangoD3DCompilerDLL )
		{
			appErrorf( TEXT("Unable to load Durango shader compiler DLL from %%DurangoXDK%%\\xdk\\FXC\\amd64\\D3DCompiler_46_xdk.dll...  Make sure you have installed the DurangoXDK!") );
		}

#pragma warning(push)
#pragma warning(disable:4191) // disable: unsafe conversion from 'type of expression' to 'type required'
		DurangoD3DPreprocessFunc = (pD3DPreprocess)GetProcAddress( DurangoD3DCompilerDLL, "D3DPreprocess" );
		DurangoD3DCompileFunc = (pD3DCompile)GetProcAddress( DurangoD3DCompilerDLL, "D3DCompile" );
		DurangoD3DReflectFunc = (pD3DReflect)GetProcAddress( DurangoD3DCompilerDLL, "D3DReflect" );
		DurangoD3DStripShaderFunc = (pD3DStripShader)GetProcAddress( DurangoD3DCompilerDLL, "D3DStripShader" );
		DurangoD3DGetDebugInfo = (pD3DGetDebugInfo)GetProcAddress( DurangoD3DCompilerDLL, "D3DGetDebugInfo" );
		DurangoD3DGetBlobPart = (pD3DGetBlobPart)GetProcAddress( DurangoD3DCompilerDLL, "D3DGetBlobPart" );
#pragma warning(pop)

		if( (DurangoD3DPreprocessFunc == NULL) || 
			(DurangoD3DCompileFunc == NULL) || 
			(DurangoD3DReflectFunc == NULL) ||
			(DurangoD3DStripShaderFunc == NULL) ||
			(DurangoD3DGetDebugInfo == NULL) ||
			(DurangoD3DGetBlobPart == NULL)
			)
		{
			appErrorf( TEXT("Unexpected Error!!! Unable to find required function(s) in Durango Shader compiler DLL.\n!") );
		}
	}
}

// Wrapper around D3DProcess that routes to the appropriate DLL depending on FShaderTarget
HRESULT WINAPI D3DPreprocess_TargetWrapper(
	FShaderTarget Target,
	_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
	_In_ SIZE_T SrcDataSize,
	_In_opt_ LPCSTR pSourceName,
	_In_opt_ CONST D3D_SHADER_MACRO* pDefines,
	_In_opt_ ID3DInclude* pInclude,
	_Out_ ID3DBlob** ppCodeText,
	_Out_opt_ ID3DBlob** ppErrorMsgs)
{
	if( Target.Platform == SP_DINGO )
	{
		// Make sure the Durango shader compiler DLL has been loaded
		InitializeDurangoShaderCompileDLL();

		return DurangoD3DPreprocessFunc( pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, ppCodeText, ppErrorMsgs );
	}
	else
	{
		return D3DPreprocess( pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, ppCodeText, ppErrorMsgs );
	}
}

// Wrapper around D3DCompile that routes to the appropriate DLL depending on FShaderTarget
HRESULT WINAPI D3DCompile_TargetWrapper(
	FShaderTarget					Target,
	LPCVOID                         pSrcData,
	SIZE_T                          SrcDataSize,
	LPCSTR                          pFileName,
	CONST D3D_SHADER_MACRO*         pDefines,
	ID3DInclude*                    pInclude,
	LPCSTR                          pEntrypoint,
	LPCSTR                          pTarget,
	UINT                            Flags1,
	UINT                            Flags2,
	ID3DBlob**                      ppCode,
	ID3DBlob**                      ppErrorMsgs)
{
	if( Target.Platform == SP_DINGO )
	{
		// Make sure the Durango shader compiler DLL has been loaded
		InitializeDurangoShaderCompileDLL();

		return DurangoD3DCompileFunc( pSrcData, SrcDataSize, pFileName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs );
	}
	else
	{
		return D3DCompile( pSrcData, SrcDataSize, pFileName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs );
	}
}

// Wrapper around D3DReflect that routes to the appropriate DLL depending on FShaderTarget
HRESULT WINAPI D3DReflect_TargetWrapper(FShaderTarget Target, LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector)
{
	if( Target.Platform == SP_DINGO )
	{
		// Make sure the Durango shader compiler DLL has been loaded
		InitializeDurangoShaderCompileDLL();

		//@hack: the GUID for the ID3D11ShaderReflection COM interface has changed between PC and Dingo and we can't really include both d3d11.h files, so this GUID is extracted from the Durango headers (specificially d3d11shader.h)
		const GUID IID_ID3D11ShaderReflection_Dingo = { 0x8d536ca1, 0x0cca, 0x4956, {0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84} };

		return DurangoD3DReflectFunc( pSrcData, SrcDataSize, IID_ID3D11ShaderReflection_Dingo, ppReflector );
	}
	else
	{
		return D3DReflect( pSrcData, SrcDataSize, pInterface, ppReflector );
	}
}

// Wrapper around D3DStripShader that routes to the appropriate DLL depending on FShaderTarget
HRESULT WINAPI D3DGetDebugInfo_TargetWrapper(FShaderTarget Target, LPCVOID pSrcData, SIZE_T SrcDataSize, ID3DBlob** ppDebugInfo)
{
	if( Target.Platform == SP_DINGO )
	{
		// Make sure the Durango shader compiler DLL has been loaded
		InitializeDurangoShaderCompileDLL();

		//return DurangoD3DGetDebugInfo( pSrcData, SrcDataSize, ppDebugInfo );
		return DurangoD3DGetBlobPart(pSrcData, SrcDataSize, /*D3D_BLOB_PDB*/(D3D_BLOB_PART)9, 0, ppDebugInfo);
	}
	else
	{
		return D3DGetDebugInfo(pSrcData, SrcDataSize, ppDebugInfo);
	}
}

// Wrapper around D3DStripShader that routes to the appropriate DLL depending on FShaderTarget
HRESULT WINAPI D3DStripShader_TargetWrapper(FShaderTarget Target, LPCVOID pSrcData, SIZE_T SrcDataSize, UINT StripFlags, ID3DBlob** OutStrippedData)
{
	if( Target.Platform == SP_DINGO )
	{
		// Make sure the Durango shader compiler DLL has been loaded
		InitializeDurangoShaderCompileDLL();

		return DurangoD3DStripShaderFunc( pSrcData, SrcDataSize, StripFlags, OutStrippedData );
	}
	else
	{
		return D3DStripShader(pSrcData, SrcDataSize, StripFlags, OutStrippedData);
	}
}

/**
 * An implementation of the D3DX include interface to access a FShaderCompilerEnvironment.
 */
class FD3D11IncludeEnvironment : public ID3DInclude
{
public:

	STDMETHOD(Open)(D3D_INCLUDE_TYPE Type,LPCSTR Name,LPCVOID ParentData,LPCVOID* Data,UINT* Bytes)
	{
		FString Filename(ANSI_TO_TCHAR(Name));

		if (appStrcmp(*Filename, TEXT("Material.usf")) == 0)
		{
			check(Environment.MaterialShaderCode);
			const INT Length = strlen(Environment.MaterialShaderCode) + 1;
			ANSICHAR* AnsiFileContents = new ANSICHAR[Length];
			appStrncpyANSI(AnsiFileContents, Environment.MaterialShaderCode, Length);
			*Data = (LPCVOID)AnsiFileContents;
			check(Length > 1);
			*Bytes = Length - 1;
		}
		else
		{
			FString FileContents;

			FString* OverrideContents = Environment.IncludeFiles.Find(*Filename);
			if(OverrideContents)
			{
				FileContents = *OverrideContents;
			}
			else if (appStrcmp(*Filename, TEXT("VertexFactory.usf")) == 0)
			{
				check(Environment.VFFileName);
				FileContents = LoadShaderSourceFile(Environment.VFFileName);
			}
			else
			{
				FileContents = LoadShaderSourceFile(*Filename);
			}

			// Convert the file contents to ANSI.
			FTCHARToANSI ConvertToAnsiFileContents(*FileContents);
			ANSICHAR* AnsiFileContents = new ANSICHAR[ConvertToAnsiFileContents.Length() + 1];
			appStrncpyANSI( AnsiFileContents, (ANSICHAR*)ConvertToAnsiFileContents, ConvertToAnsiFileContents.Length() + 1 );

			// Write the result to the output parameters.
			*Data = (LPCVOID)AnsiFileContents;
			*Bytes = ConvertToAnsiFileContents.Length();
		}

		return S_OK;
	}

	STDMETHOD(Close)(LPCVOID Data)
	{
		delete [] Data;
		return S_OK;
	}

	FD3D11IncludeEnvironment(const FShaderCompilerEnvironment& InEnvironment):
		Environment(InEnvironment)
	{}

private:

	FShaderCompilerEnvironment Environment;
};

/**
 * TranslateCompilerFlag - translates the platform-independent compiler flags into D3DX defines
 * @param CompilerFlag - the platform-independent compiler flag to translate
 * @return DWORD - the value of the appropriate D3DX enum
 */
static DWORD TranslateCompilerFlagD3D11(ECompilerFlags CompilerFlag)
{
	switch(CompilerFlag)
	{
	case CFLAG_PreferFlowControl: return D3DCOMPILE_PREFER_FLOW_CONTROL;
	case CFLAG_Debug: return D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	case CFLAG_AvoidFlowControl: return D3DCOMPILE_AVOID_FLOW_CONTROL;
	default: return 0;
	};
}

/**
 * D3D11CreateShaderCompileCommandLine - takes shader parameters used to compile with the DX10
 * compiler and returns an fxc command to compile from the command line
 */
static FString D3D11CreateShaderCompileCommandLine(
	FShaderTarget Target,
	const FString& ShaderPath, 
	const FString& IncludePath, 
	const TCHAR* EntryFunction, 
	const TCHAR* ShaderProfile, 
	D3D_SHADER_MACRO *Macros,
	DWORD CompileFlags,
	UBOOL bPreprocessedCommandLine
	)
{
	FString FXCCommandline;

	// fxc is our command line compiler
	if( Target.Platform == SP_DINGO )
	{
		InitializeDurangoShaderCompileDLL();	// Technically this isn't needed to complete this funciton, but doing this will verify that the Durango XDK is properly installed

		// Choose the Dingo version of fxc if we're compiling for the Dingo platform and not the PC
		FXCCommandline = FString(TEXT("\"%DurangoXDK%\\bin\\fxc\" ")) + ShaderPath;
	}
	else
	{
		FXCCommandline = FString(TEXT("\"%DXSDK_DIR%\\Utilities\\bin\\x86\\fxc\" ")) + ShaderPath;
	}

	if (!bPreprocessedCommandLine)
	{
		// add definitions
		if(Macros != NULL)
		{
			for (INT i = 0; Macros[i].Name != NULL; i++)
			{
				FXCCommandline += FString(TEXT(" /D ")) + ANSI_TO_TCHAR(Macros[i].Name) + TEXT("=") + ANSI_TO_TCHAR(Macros[i].Definition);
			}
		}
	}

	// add the entry point reference
	FXCCommandline += FString(TEXT(" /E ")) + EntryFunction;

	if (!bPreprocessedCommandLine)
	{
		// add the include path
		FXCCommandline += FString(TEXT(" /I ")) + IncludePath;
	}

	// go through and add other switches
	if(CompileFlags & D3DCOMPILE_PREFER_FLOW_CONTROL)
	{
		CompileFlags &= ~D3DCOMPILE_PREFER_FLOW_CONTROL;
		FXCCommandline += FString(TEXT(" /Gfp"));
	}
	if(CompileFlags & D3DCOMPILE_DEBUG)
	{
		CompileFlags &= ~D3DCOMPILE_DEBUG;
		FXCCommandline += FString(TEXT(" /Zi"));
	}
	if(CompileFlags & D3DCOMPILE_SKIP_OPTIMIZATION)
	{
		CompileFlags &= ~D3DCOMPILE_SKIP_OPTIMIZATION;
		FXCCommandline += FString(TEXT(" /Od"));
	}
	if(CompileFlags & D3DCOMPILE_AVOID_FLOW_CONTROL)
	{
		CompileFlags &= ~D3DCOMPILE_AVOID_FLOW_CONTROL;
		FXCCommandline += FString(TEXT(" /Gfa"));
	}
	if(CompileFlags & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)
	{
		CompileFlags &= ~D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
		FXCCommandline += FString(TEXT(" /Gec"));
	}
	if(CompileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL3)
	{
		CompileFlags &= ~D3DCOMPILE_OPTIMIZATION_LEVEL3;
		FXCCommandline += FString(TEXT(" /O3"));
	}
	checkf(CompileFlags == 0, TEXT("Unhandled shader compiler flag!"));

	// add the target instruction set
	FXCCommandline += FString(TEXT(" /T ")) + ShaderProfile;

	// add a pause on a newline
	FXCCommandline += FString(TEXT(" \r\n pause"));
	return FXCCommandline;
}

/**
 * Uses D3D11 to PreProcess a shader (resolve all #includes and #defines) and dumps it out for debugging
 */
static void D3D11PreProcessShader(
	FShaderTarget Target,
	const TCHAR* SourceFilename,
	const FString& SourceFile,
	const TArray<D3D_SHADER_MACRO>& Macros,
	FD3D11IncludeEnvironment& IncludeEnvironment,
	const TCHAR* ShaderPath
	)
{
#if !NO_D3DCOMPILER
	TRefCountPtr<ID3DBlob> ShaderText;
	TRefCountPtr<ID3DBlob> PreProcessErrors;

	FTCHARToANSI AnsiSourceFile(*SourceFile);

	HRESULT PreProcessHR = D3DPreprocess_TargetWrapper(
		Target,
		(ANSICHAR*)AnsiSourceFile,
		AnsiSourceFile.Length(),
		NULL,
		&Macros(0),
		&IncludeEnvironment,
		ShaderText.GetInitReference(),
		PreProcessErrors.GetInitReference());

	if(FAILED(PreProcessHR))
	{
		warnf( NAME_Warning, TEXT("Preprocess failed for shader %s: %s"), SourceFilename, ANSI_TO_TCHAR(PreProcessErrors->GetBufferPointer()) );
	}
	else
	{
		appSaveStringToFile(
			FString(ANSI_TO_TCHAR(ShaderText->GetBufferPointer())), 
			*(FString(ShaderPath) * FString(SourceFilename) + TEXT(".pre")));
	}
#endif
}

/**
 * Filters out unwanted shader compile warnings
 */
static void D3D11FilterShaderCompileWarnings(const FString& CompileWarnings, TArray<FString>& FilteredWarnings)
{
	TArray<FString> WarningArray;
	FString OutWarningString = TEXT("");
	CompileWarnings.ParseIntoArray(&WarningArray, TEXT("\n"), TRUE);
	
	//go through each warning line
	for (INT WarningIndex = 0; WarningIndex < WarningArray.Num(); WarningIndex++)
	{
		//suppress "warning X3557: Loop only executes for 1 iteration(s), forcing loop to unroll"
		if (WarningArray(WarningIndex).InStr(TEXT("X3557")) == INDEX_NONE
			// "warning X3205: conversion from larger type to smaller, possible loss of data"
			// Gets spammed when converting from float to half
			&& WarningArray(WarningIndex).InStr(TEXT("X3205")) == INDEX_NONE)
		{
			FilteredWarnings.AddUniqueItem(WarningArray(WarningIndex));
		}
	}
}

/** Compiles a D3D11 shader through the D3DX Dll */
static UBOOL D3D11CompileShaderThroughDll(
	FShaderTarget Target,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	const TCHAR* ShaderProfile,
	DWORD CompileFlags,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	TArray<D3D_SHADER_MACRO>& Macros,
	TArray<FString>& FilteredErrors
	)
{
#if NO_D3DCOMPILER
	return FALSE;
#else
	TRefCountPtr<ID3DBlob> Shader;
	TRefCountPtr<ID3DBlob> Errors;

	const FString SourceFile = LoadShaderSourceFile(SourceFilename);
	FD3D11IncludeEnvironment IncludeEnvironment(Environment);

	FTCHARToANSI AnsiSourceFile(*SourceFile);
	HRESULT Result = D3DCompile_TargetWrapper(
		Target,
		(ANSICHAR*)AnsiSourceFile,
		AnsiSourceFile.Length(),
		TCHAR_TO_ANSI(SourceFilename),
		&Macros(0),
		&IncludeEnvironment,
		TCHAR_TO_ANSI(FunctionName),
		TCHAR_TO_ANSI(ShaderProfile),
		CompileFlags,
		0,
		Shader.GetInitReference(),
		Errors.GetInitReference()
		);

	if (FAILED(Result))
	{
		// Copy the error text to the output.
		void* ErrorBuffer = Errors ? Errors->GetBufferPointer() : NULL;
		if (ErrorBuffer)
		{
			D3D11FilterShaderCompileWarnings(ANSI_TO_TCHAR(ErrorBuffer), FilteredErrors);
		}
		else
		{
			FilteredErrors.AddItem(TEXT("Compile Failed without warnings!"));
		}
		return FALSE;
	}
	else
	{
		UINT NumShaderBytes = Shader->GetBufferSize();
		Output.Code.Empty(NumShaderBytes);
		Output.Code.Add(NumShaderBytes);
		appMemcpy(&Output.Code(0),Shader->GetBufferPointer(),NumShaderBytes);

		return TRUE;
	}
#endif
}

//@zombie_ps4_begin
INT GetArrayIndex( const FString& InName )
{
	const INT BracketLoc =  InName.InStr(TEXT("["));

	if( BracketLoc != INDEX_NONE )
	{
		FString RightString = InName.Mid(BracketLoc + 1, InName.Len() - BracketLoc - 2 );
		return appAtoi(*RightString);
	}

	return INDEX_NONE;
}

FString StripBrackets( const char* InName )
{
	FString OutString = InName;

	const INT BracketLoc =  OutString.InStr(TEXT("["));

	if( BracketLoc != INDEX_NONE )
	{
		OutString = OutString.Left(BracketLoc);
	}

	return OutString;
}
//@zombie_ps4_end

/** Enqueues compilation of a D3D11 shader through a worker process */
static void D3D11BeginCompilingShaderThroughWorker(
	INT JobId,
	UINT ThreadId,
	FShaderTarget Target,
	const TCHAR* SourceFilename,
	LPCSTR FunctionName,
	LPCSTR ShaderProfile,
	LPCSTR IncludePath,
	DWORD CompileFlags,
	const FShaderCompilerEnvironment& Environment,
	TArray<D3D_SHADER_MACRO>& Macros
	)
{
	// Set up the Worker Job Type based on platform
	EWorkerJobType JobType = (Target.Platform == SP_DINGO) ? WJT_DingoShader : WJT_D3D11Shader;

	TRefCountPtr<FBatchedShaderCompileJob> BatchedJob = new FBatchedShaderCompileJob(JobId, ThreadId, JobType);
	TArray<BYTE>& WorkerInput = BatchedJob->WorkerInput;
	// Presize to avoid lots of allocations
	WorkerInput.Empty(1000);
	// Setup the input for the worker app, everything that is needed to compile the shader.
	// Note that any format changes here also need to be done in the worker
	// Write a job type so the worker app can know which compiler to invoke
	WorkerInputAppendValue(JobType, WorkerInput);
	// Version number so we can detect stale data
	const BYTE D3D11ShaderCompileWorkerInputVersion = 0;
	WorkerInputAppendValue(D3D11ShaderCompileWorkerInputVersion, WorkerInput);
	WorkerInputAppendMemory(TCHAR_TO_ANSI(SourceFilename), appStrlen(SourceFilename), WorkerInput);
	const FString SourceFile = LoadShaderSourceFile(SourceFilename);
	FTCHARToANSI AnsiSourceFile(*SourceFile);
	WorkerInputAppendMemory((ANSICHAR*)AnsiSourceFile, AnsiSourceFile.Length(), WorkerInput);
	WorkerInputAppendMemory(FunctionName, appStrlen(FunctionName) * sizeof(CHAR), WorkerInput);
	WorkerInputAppendMemory(ShaderProfile, appStrlen(ShaderProfile) * sizeof(CHAR), WorkerInput);
	WorkerInputAppendValue(CompileFlags, WorkerInput);
	WorkerInputAppendMemory(IncludePath, appStrlen(IncludePath) * sizeof(CHAR), WorkerInput);

	Environment.AddIncludesForWorker(WorkerInput);

	INT NumMacros = Macros.Num() - 1;
	WorkerInputAppendValue(NumMacros, WorkerInput);

	for (INT MacroIndex = 0; MacroIndex < Macros.Num() - 1; MacroIndex++)
	{
		D3D_SHADER_MACRO CurrentMacro = Macros(MacroIndex);
		check( CurrentMacro.Name);
		WorkerInputAppendMemory(CurrentMacro.Name, appStrlen(CurrentMacro.Name), WorkerInput);
		WorkerInputAppendMemory(CurrentMacro.Definition, appStrlen(CurrentMacro.Definition), WorkerInput);
	}

	TArray<BYTE> WorkerOutput;
	// Invoke the worker
	GShaderCompilingThreadManager->BeginWorkerCompile(BatchedJob);
}

/** Processes the results of a D3D11 shader compilation. */
static UBOOL FinishCompilingD3D11Shader(
	UBOOL bShaderCompileSucceeded,
	FShaderTarget Target,
	const TArray<FString>& FilteredErrors,
	FShaderCompilerOutput& Output)
{
#if NO_D3DCOMPILER
	return FALSE;
#else
	for (INT ErrorIndex = 0; ErrorIndex < FilteredErrors.Num(); ErrorIndex++)
	{
		const FString& CurrentError = FilteredErrors(ErrorIndex);
		FShaderCompilerError NewError;
		// Extract the filename and line number from the shader compiler error message for PC whose format is:
		// "d:\UnrealEngine3\Binaries\BasePassPixelShader(30,7): error X3000: invalid target or usage string"
		INT FirstParenIndex = CurrentError.InStr(TEXT("("));
		INT LastParenIndex = CurrentError.InStr(TEXT("):"));
		if (FirstParenIndex != INDEX_NONE 
			&& LastParenIndex != INDEX_NONE
			&& LastParenIndex > FirstParenIndex)
		{
			FFilename ErrorFileAndPath = CurrentError.Left(FirstParenIndex);
			if (ErrorFileAndPath.GetExtension().ToUpper() == TEXT("USF"))
			{
				NewError.ErrorFile = ErrorFileAndPath.GetCleanFilename();
			}
			else
			{
				NewError.ErrorFile = ErrorFileAndPath.GetCleanFilename() + TEXT(".usf");
			}

			NewError.ErrorLineString = CurrentError.Mid(FirstParenIndex + 1, LastParenIndex - FirstParenIndex - appStrlen(TEXT("(")));
			NewError.StrippedErrorMessage = CurrentError.Right(CurrentError.Len() - LastParenIndex - appStrlen(TEXT("):")));
		}
		else
		{
			NewError.StrippedErrorMessage = CurrentError;
		}
		Output.Errors.AddItem(NewError);
	}

	if (bShaderCompileSucceeded)
	{
		ID3D11ShaderReflection* Reflector = NULL;
		VERIFYD3D11RESULT(D3DReflect_TargetWrapper(Target, Output.Code.GetData(),Output.Code.Num(),IID_ID3D11ShaderReflection, (void**) &Reflector) );

		// Read the constant table description.
		D3D11_SHADER_DESC ShaderDesc;
		Reflector->GetDesc(&ShaderDesc);

//@zombie_ps4_begin
		TMap<FString,D3D11_SHADER_INPUT_BIND_DESC> ShaderMap;
		for (UINT ResourceIndex = 0; ResourceIndex < ShaderDesc.BoundResources; ResourceIndex++)
		{
			D3D11_SHADER_INPUT_BIND_DESC BindDesc;
			Reflector->GetResourceBindingDesc(ResourceIndex, &BindDesc);

			ShaderMap.Set(BindDesc.Name, BindDesc);
		}
//@zombie_ps4_end

		// Add parameters for shader resources (constant buffers, textures, samplers, etc. */
		for (UINT ResourceIndex = 0; ResourceIndex < ShaderDesc.BoundResources; ResourceIndex++)
		{
			D3D11_SHADER_INPUT_BIND_DESC BindDesc;
			Reflector->GetResourceBindingDesc(ResourceIndex, &BindDesc);
			if (BindDesc.Type == D3D_SIT_CBUFFER || BindDesc.Type == D3D_SIT_TBUFFER)
			{
				const UINT CBIndex = BindDesc.BindPoint;
				ID3D11ShaderReflectionConstantBuffer* ConstantBuffer = Reflector->GetConstantBufferByName(BindDesc.Name);
				D3D11_SHADER_BUFFER_DESC CBDesc;
				ConstantBuffer->GetDesc(&CBDesc);

				if (CBDesc.Size > GConstantBufferSizes[CBIndex])
				{
					appErrorf(TEXT("Set GConstantBufferSizes[%d] to >= %d"), CBIndex, CBDesc.Size);
				}

				// Track all of the variables in this constant buffer.
				for (UINT ConstantIndex = 0; ConstantIndex < CBDesc.Variables; ConstantIndex++)
				{
					ID3D11ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex(ConstantIndex);
					D3D11_SHADER_VARIABLE_DESC VariableDesc;
					Variable->GetDesc(&VariableDesc);
					if (VariableDesc.uFlags & D3D_SVF_USED)
					{
						Output.ParameterMap.AddParameterAllocation(
							ANSI_TO_TCHAR(VariableDesc.Name),
							CBIndex,
							VariableDesc.StartOffset,
							VariableDesc.Size,
							0
							);
					}
				}
			}
			else if (BindDesc.Type == D3D10_SIT_TEXTURE)
			{
//@zombie_ps4_begin
				const FString TextureName =  FString(BindDesc.Name);
				const FString BaseSamplerName = StripBrackets(TCHAR_TO_ANSI(*TextureName));
				const INT ArrayIndex = GetArrayIndex(TextureName);

				FString ExpectedSamplerName = BaseSamplerName + TEXT("Sampler");

				if(ArrayIndex != INDEX_NONE)
				{
					ExpectedSamplerName = FString::Printf(TEXT("%s[%i]"), *ExpectedSamplerName, ArrayIndex );
				}

				D3D11_SHADER_INPUT_BIND_DESC* TestSamp = ShaderMap.Find( ExpectedSamplerName );

				// Add a parameter for a texture without a matching sampler (SamplerState in HLSL)
				// If the texture has a matching sampler (sampler2D in HLSL), it will be handled in the D3D_SIT_SAMPLER branch
				if (!TestSamp)
//@zombie_ps4_end
				{
					UINT BindCount;
					TCHAR OfficialName[1024];
					appStrcpy(OfficialName, ANSI_TO_TCHAR(BindDesc.Name));

					if (Target.Platform == SP_PCD3D_SM5 || Target.Platform == SP_DINGO)
					{
						BindCount = 1;

						// Assign the name and optionally strip any "[#]" suffixes
						TCHAR *BracketLocation = appStrchr(OfficialName, TEXT('['));
						if (BracketLocation)
						{
							*BracketLocation = 0;	

							const INT NumCharactersBeforeArray = BracketLocation - OfficialName;

							// In SM5, for some reason, array suffixes are included in Name, i.e. "LightMapTextures[0]", rather than "LightMapTextures"
							// Additionally elements in an array are listed as SEPERATE bound resources.
							// However, they are always contiguous in resource index, so iterate over the samplers and textures of the initial association
							// and count them, identifying the bindpoint and bindcounts

							while (ResourceIndex + 1 < ShaderDesc.BoundResources)
							{
								D3D11_SHADER_INPUT_BIND_DESC BindDesc2;
								Reflector->GetResourceBindingDesc(ResourceIndex + 1, &BindDesc2);

							if (BindDesc2.Type == D3D_SIT_TEXTURE && strncmp(BindDesc2.Name, BindDesc.Name, NumCharactersBeforeArray) == 0)
								{
									BindCount++;
									// Skip over this resource since it is part of an array
									ResourceIndex++;
								}
								else
								{
									break;
								}
							}
						}
					}
					else
					{
						BindCount = BindDesc.BindCount;
					}

					// Add a parameter for the texture only, the sampler index will be invalid
					Output.ParameterMap.AddParameterAllocation(
						OfficialName,
						0,
						BindDesc.BindPoint,
						BindCount,
						USHRT_MAX
						);
				}
			}
			else if (BindDesc.Type == D3D11_SIT_UAV_RWTYPED)
			{
				TCHAR OfficialName[1024];
				UINT BindCount = 1;
				appStrcpy(OfficialName, ANSI_TO_TCHAR(BindDesc.Name));
	
				// todo: arrays are not yet supported

				Output.ParameterMap.AddParameterAllocation(
					OfficialName,
					0,
					BindDesc.BindPoint,
					BindCount,
					USHRT_MAX
					);
			}
			else if (BindDesc.Type == D3D_SIT_SAMPLER)
			{
				UBOOL bHasMatchingTexture = FALSE;

//@zombie_ps4_begin
				// In SM5, for some reason, array suffixes are included in Name, i.e. "LightMapTextures[0]", rather than "LightMapTextures"
				// Additionally elements in an array are listed as SEPERATE bound resources.
				// However, they are always contiguous in resource index, so iterate over the samplers and textures of the initial association
				// and count them, identifying the bindpoint and bindcounts
				const FString SamplerName =  FString(BindDesc.Name);
				const FString ExpectedTextureName = FString(BindDesc.Name).Replace(TEXT("Sampler"),TEXT(""));
				const FString BaseTextureName = StripBrackets(TCHAR_TO_ANSI(*ExpectedTextureName));

				const INT ArrayIndex = GetArrayIndex(SamplerName);
				
				// Not an array, just return the matching texture
				if( ArrayIndex == INDEX_NONE && (Target.Platform == SP_PCD3D_SM5 || Target.Platform == SP_DINGO || BindDesc.BindCount == 1) )
				{
					D3D11_SHADER_INPUT_BIND_DESC* TextureDesc = ShaderMap.Find(ExpectedTextureName);

					if( TextureDesc )
					{
						Output.ParameterMap.AddParameterAllocation( *BaseTextureName, 0, TextureDesc->BindPoint, 1, BindDesc.BindPoint );
					}
					else
					{
						//warnf(TEXT("Could not find matching texture for Sampler [%s] in shader"), *SamplerName);
						Output.ParameterMap.AddParameterAllocation( *SamplerName, 0, USHRT_MAX, BindDesc.BindCount, BindDesc.BindPoint );
					}
				}
				// ignore all but the FIRST element of the array that we find
				else
				{
					// Find the corresponding Base texture
					D3D11_SHADER_INPUT_BIND_DESC* TextureDesc = ShaderMap.Find(ExpectedTextureName);

					// Count how big the array is
					const FString BaseName = StripBrackets(BindDesc.Name);

					INT ArrayCount = BindDesc.BindCount;
					if (Target.Platform == SP_PCD3D_SM5 || Target.Platform == SP_DINGO)
					{
						ArrayCount = 1;
						while( TRUE )
						{
							const FString TestName = FString::Printf(TEXT("%s[%i]"), *BaseName, ArrayCount );
							D3D11_SHADER_INPUT_BIND_DESC* TestDesc = ShaderMap.Find(TestName);

							if( TestDesc )
							{
								++ArrayCount;
							}
							else
							{
								break;
							}
						}

						// This will skip over the next array items since we handled it with our array
						ResourceIndex += ArrayCount - 1;
					}

					Output.ParameterMap.AddParameterAllocation( *BaseTextureName, 0, TextureDesc->BindPoint, ArrayCount, BindDesc.BindPoint );
				}
//@zombie_ps4_end
			}
		}

		// Set the number of instructions.
		Output.NumInstructions = ShaderDesc.InstructionCount;

		// Reflector is a com interface, so it needs to be released.
		Reflector->Release();

		// Dump out our debug data if we have it
		if (Target.Platform == SP_DINGO)
		{
			if (GShaderCompilingThreadManager->IsDumpingShaderPDBs())
			{
				TRefCountPtr<ID3DBlob> DebugInfo;
				D3DGetDebugInfo_TargetWrapper(Target, Output.Code.GetData(), Output.Code.Num(), DebugInfo.GetInitReference());
				if (DebugInfo)
				{
					TArray<BYTE> DebugInfoArray(DebugInfo->GetBufferSize());
					appMemcpy(DebugInfoArray.GetTypedData(), DebugInfo->GetBufferPointer(), DebugInfo->GetBufferSize());
					appSaveArrayToFile(DebugInfoArray, *Output.DebugOutputFileName);
				}
			}

			// we are done with reflection, so strip out any extra data
			TRefCountPtr<ID3DBlob> StrippedShader;
			D3DStripShader_TargetWrapper(Target, Output.Code.GetData(),Output.Code.Num(), D3DCOMPILER_STRIP_REFLECTION_DATA|D3DCOMPILER_STRIP_DEBUG_INFO|D3DCOMPILER_STRIP_TEST_BLOBS, StrippedShader.GetInitReference());
			UINT NumShaderBytes = StrippedShader->GetBufferSize();
			Output.Code.Empty(NumShaderBytes);
			Output.Code.Add(NumShaderBytes);
			appMemcpy(&Output.Code(0),StrippedShader->GetBufferPointer(),NumShaderBytes);
		}

		// Pass the target through to the output.
		Output.Target = Target;
	}
	return bShaderCompileSucceeded;
#endif
}

/** Reads the worker outputs and forwards them to FinishCompilingD3D11Shader. */
UBOOL D3D11FinishCompilingShaderThroughWorker(
	FShaderTarget Target,
	INT& CurrentPosition,
	const TArray<BYTE>& WorkerOutput,
	FShaderCompilerOutput& Output)
{
	const BYTE D3D11ShaderCompileWorkerOutputVersion = 0;
	// Read the worker output in the same format that it was written
	BYTE ReadVersion;
	WorkerOutputReadValue(ReadVersion, CurrentPosition, WorkerOutput);
	check(ReadVersion == D3D11ShaderCompileWorkerOutputVersion);
	EWorkerJobType OutJobType;
	WorkerOutputReadValue(OutJobType, CurrentPosition, WorkerOutput);
#if DO_CHECK
	if( Target.Platform == SP_DINGO )
	{
		check( OutJobType == WJT_DingoShader );
	}
	else
	{
		check( OutJobType == WJT_D3D11Shader );
	}
#endif // DO_CHECK
	HRESULT CompileResult;
	WorkerOutputReadValue(CompileResult, CurrentPosition, WorkerOutput);
	UINT ByteCodeLength;
	WorkerOutputReadValue(ByteCodeLength, CurrentPosition, WorkerOutput);
	Output.Code.Empty(ByteCodeLength);
	Output.Code.Add(ByteCodeLength);
	if (ByteCodeLength > 0)
	{
		WorkerOutputReadMemory(&Output.Code(0), Output.Code.Num(), CurrentPosition, WorkerOutput);
	}
	UINT ErrorStringLength;
	WorkerOutputReadValue(ErrorStringLength, CurrentPosition, WorkerOutput);
	
	FString ErrorString;
	if (ErrorStringLength > 0)
	{
		ANSICHAR* ErrorBuffer = new ANSICHAR[ErrorStringLength + 1];
		WorkerOutputReadMemory(ErrorBuffer, ErrorStringLength, CurrentPosition, WorkerOutput);
		ErrorBuffer[ErrorStringLength] = 0;
		ErrorString = FString(ANSI_TO_TCHAR(ErrorBuffer));
		delete [] ErrorBuffer;
	}	

	UBOOL bShaderCompileSucceeded = TRUE;
	TArray<FString> FilteredErrors;
	if (FAILED(CompileResult))
	{
		// Copy the error text to the output.
		if (ErrorString.Len() > 0)
		{
			D3D11FilterShaderCompileWarnings(ErrorString, FilteredErrors);
		}
		else
		{
			FilteredErrors.AddItem(TEXT("Compile Failed without warnings!"));
		}

		bShaderCompileSucceeded = FALSE;
	}
	else
	{
		bShaderCompileSucceeded = TRUE;
	}
	return FinishCompilingD3D11Shader(bShaderCompileSucceeded, Target, FilteredErrors, Output);
}

/**
 * The D3D11/HLSL shader compiler.
 * If this is a multi threaded compile, this function merely enqueues a compilation job.
 */ 
UBOOL D3D11BeginCompileShader(
	INT JobId, 
	UINT ThreadId,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bDebugDump = FALSE,
	const TCHAR* ShaderSubDir = NULL
	)
{
	// ShaderSubDir must be valid if we are dumping debug shader data
	checkSlow(!bDebugDump || ShaderSubDir != NULL);
	// Must not be doing a multithreaded compile if we are dumping debug shader data
	checkSlow(!bDebugDump || !GShaderCompilingThreadManager->IsMultiThreadedCompile());

	// Translate the input environment's definitions to D3DXMACROs.
	TArray<D3D_SHADER_MACRO> Macros;
	for(TMap<FString,FString>::TConstIterator DefinitionIt(Environment.Definitions);DefinitionIt;++DefinitionIt)
	{
		FString Name = DefinitionIt.Key();
		FString Definition = DefinitionIt.Value();

		D3D_SHADER_MACRO* Macro = new(Macros) D3D_SHADER_MACRO;
		ANSICHAR* tName = new ANSICHAR[Name.Len() + 1];
		strncpy_s(tName,Name.Len() + 1,TCHAR_TO_ANSI(*Name),Name.Len() + 1);
		Macro->Name = tName;
		ANSICHAR* tDefinition = new ANSICHAR[Definition.Len() + 1];
		strncpy_s(tDefinition,Definition.Len() + 1,TCHAR_TO_ANSI(*Definition),Definition.Len() + 1);
		Macro->Definition = tDefinition;
	}

	// set the COMPILER type
	D3D_SHADER_MACRO* Macro = new(Macros) D3D_SHADER_MACRO;
#define COMPILER_NAME "COMPILER_HLSL"
	ANSICHAR* tName1 = new ANSICHAR[strlen(COMPILER_NAME) + 1];
	strcpy_s(tName1, strlen(COMPILER_NAME) + 1, COMPILER_NAME);
	Macro->Name = tName1;

	ANSICHAR* tDefinition1 = new ANSICHAR[2];
	strcpy_s(tDefinition1, 2, "1");
	Macro->Definition = tDefinition1;

	if (Target.Platform == SP_PCD3D_SM4)
	{
		// set the SM4_PROFILE definition
		static const char* ProfileName = "SM4_PROFILE";
		D3D_SHADER_MACRO* ProfileMacro = new(Macros) D3D_SHADER_MACRO;
		ProfileMacro->Name = appStrcpyANSI(new ANSICHAR[strlen(ProfileName) + 1],strlen(ProfileName) + 1,ProfileName);
		ProfileMacro->Definition = appStrcpyANSI(new ANSICHAR[2],2,"1");
	}
	else
	{
		// set the SM5_PROFILE definition
		static const char* ProfileName = "SM5_PROFILE";
		D3D_SHADER_MACRO* ProfileMacro = new(Macros) D3D_SHADER_MACRO;
		ProfileMacro->Name = appStrcpyANSI(new ANSICHAR[strlen(ProfileName) + 1],strlen(ProfileName) + 1,ProfileName);
		ProfileMacro->Definition = appStrcpyANSI(new ANSICHAR[2],2,"1");
	}

	// set SUPPORTS_DEPTH_TEXTURES
	D3D_SHADER_MACRO* MacroDepthSupport = new(Macros) D3D_SHADER_MACRO;
	ANSICHAR* tName2 = new ANSICHAR[strlen("SUPPORTS_DEPTH_TEXTURES") + 1];
	strcpy_s(tName2, strlen("SUPPORTS_DEPTH_TEXTURES") + 1, "SUPPORTS_DEPTH_TEXTURES");
	MacroDepthSupport->Name = tName2;

	ANSICHAR* tDefinition2 = new ANSICHAR[2];
	strcpy_s(tDefinition2, 2, "1");
	MacroDepthSupport->Definition = tDefinition2;

	DWORD CompileFlags = 0;

	// If we're compiling a Durango shader, set the Durango macro
	if( Target.Platform == SP_DINGO )
	{
		static const char* PlatformName = "DINGO";
		D3D_SHADER_MACRO* PlatformMacro = new(Macros) D3D_SHADER_MACRO;
		PlatformMacro->Name = appStrcpyANSI(new ANSICHAR[strlen(PlatformName) + 1],strlen(PlatformName) + 1,PlatformName);
		PlatformMacro->Definition = appStrcpyANSI(new ANSICHAR[2],2,"1");

		if (GShaderCompilingThreadManager->IsDumpingShaderPDBs())
		{
			CompileFlags |= D3DCOMPILE_DEBUG;

			struct FGuid
			{
				DWORD A,B,C,D;
			};

			FGuid ShaderGUID;
			// Save this shader's PDB's with a unique filename
			//@todo - error handling
			CoCreateGuid((GUID*)&ShaderGUID);

			char PDBPath[MAX_PATH] = {0};
			char QuotedPDBPath[MAX_PATH] = {0};
			char GUIDTemp[MAX_PATH] = {0};

			// have to hardcode this like so since we are using the d3d11 shader compiler which changes the pdb path
			FString ShaderPDBPath = FString(appBaseDir()) * TEXT("..") PATH_SEPARATOR TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders") PATH_SEPARATOR TEXT("PDBDump") PATH_SEPARATOR TEXT("Dingo") PATH_SEPARATOR;
			ShaderPDBPath = ShaderPDBPath.Replace(TEXT("\\"), TEXT("\\\\"));
			strcpy_s(PDBPath, TCHAR_TO_ANSI(*ShaderPDBPath));
			_ultoa_s(ShaderGUID.A, GUIDTemp, 10);
			strcat_s(PDBPath, GUIDTemp);
			_ultoa_s(ShaderGUID.B, GUIDTemp, 10);
			strcat_s(PDBPath, GUIDTemp);
			_ultoa_s(ShaderGUID.C, GUIDTemp, 10);
			strcat_s(PDBPath, GUIDTemp);
			_ultoa_s(ShaderGUID.D, GUIDTemp, 10);
			strcat_s(PDBPath, GUIDTemp);
			strcat_s(PDBPath, ".pdb");

			Output.DebugOutputFileName = PDBPath;

			QuotedPDBPath[0] = '\"';
			strcat_s(QuotedPDBPath, PDBPath);
			strcat_s(QuotedPDBPath, "\"");

			static const char* DebugDefintionName = "__XBOX_PDBFILENAME";
			D3D_SHADER_MACRO* DebugFileNameMacro = new(Macros) D3D_SHADER_MACRO;
			DebugFileNameMacro->Name = appStrcpyANSI(new ANSICHAR[strlen(DebugDefintionName) + 1], strlen(DebugDefintionName) + 1, DebugDefintionName);
			DebugFileNameMacro->Definition = appStrcpyANSI(new ANSICHAR[strlen(QuotedPDBPath) + 1], strlen(QuotedPDBPath) + 1, QuotedPDBPath);
		}
	}

	// @TODO - implement different material path to allow us to remove backwards compat flag on sm5 shaders
	//CompileFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;

	if (DEBUG_SHADERS) 
	{
		//add the debug flags
		CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	}
	else
	{
		CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
	}

	for(INT FlagIndex = 0;FlagIndex < Environment.CompilerFlags.Num();FlagIndex++)
	{
		//accumulate flags set by the shader
		CompileFlags |= TranslateCompilerFlagD3D11(Environment.CompilerFlags(FlagIndex));
	}

	UBOOL bShaderCompileSucceeded = FALSE;
	const TCHAR* ShaderPath = appShaderDir();
	const FString PreprocessorOutputDir = FString(ShaderPath) * ShaderPlatformToText((EShaderPlatform)Target.Platform) * FString(ShaderSubDir);

	TCHAR ShaderProfile[32];
	TArray<FString> FilteredErrors;
	{
		checkSlow(Target.Frequency == SF_Vertex ||
			Target.Frequency == SF_Pixel ||
			Target.Frequency == SF_Hull ||
			Target.Frequency == SF_Domain ||
			Target.Frequency == SF_Compute ||
			Target.Frequency == SF_Geometry);

		//set defines and profiles for the appropriate shader paths
		if (Target.Frequency == SF_Pixel)
		{
			appStrcpy(ShaderProfile, Target.Platform == SP_PCD3D_SM4 ? TEXT("ps_4_0") : TEXT("ps_5_0"));
		}
		else if(Target.Frequency == SF_Vertex)
		{
			appStrcpy(ShaderProfile, Target.Platform == SP_PCD3D_SM4 ? TEXT("vs_4_0") : TEXT("vs_5_0"));
		}
		else if(Target.Frequency == SF_Hull)
		{
			check(Target.Platform != SP_PCD3D_SM4);
			appStrcpy(ShaderProfile, TEXT("hs_5_0"));
		}
		else if(Target.Frequency == SF_Domain)
		{
			check(Target.Platform != SP_PCD3D_SM4);
			appStrcpy(ShaderProfile, TEXT("ds_5_0"));
		}
		else if(Target.Frequency == SF_Geometry)
		{
			appStrcpy(ShaderProfile, Target.Platform == SP_PCD3D_SM4 ? TEXT("gs_4_0") : TEXT("gs_5_0"));
		}
		else if(Target.Frequency == SF_Compute)
		{
			check(Target.Platform != SP_PCD3D_SM4);
			appStrcpy(ShaderProfile, TEXT("cs_5_0"));
		}
		else
		{
			return FALSE;
		}

		// Terminate the Macros list.
		D3D_SHADER_MACRO* TerminatorMacro = new(Macros) D3D_SHADER_MACRO;
		TerminatorMacro->Name = NULL;
		TerminatorMacro->Definition = NULL;

		// If this is a multi threaded compile, we must use the worker, otherwise compile the shader directly.
		if (GShaderCompilingThreadManager->IsMultiThreadedCompile())
		{
			D3D11BeginCompilingShaderThroughWorker(
				JobId,
				ThreadId,
				Target,
				SourceFilename,
				TCHAR_TO_ANSI(FunctionName),
				TCHAR_TO_ANSI(ShaderProfile),
				TCHAR_TO_ANSI(ShaderPath),
				CompileFlags,
				Environment,
				Macros
				);

			// The job has been enqueued, compilation results are not known yet
			bShaderCompileSucceeded = TRUE;
		}
		else
		{
			bShaderCompileSucceeded = D3D11CompileShaderThroughDll(
				Target,
				SourceFilename,
				FunctionName,
				ShaderProfile,
				CompileFlags,
				Environment,
				Output,
				Macros,
				FilteredErrors
				);

			bShaderCompileSucceeded = FinishCompilingD3D11Shader(bShaderCompileSucceeded, Target, FilteredErrors, Output);
		}
	}

	// If we are dumping out preprocessor data
	// @todo - also dump out shader data when compilation fails
	if (bDebugDump)
	{
		// just in case the preprocessed shader dir has not been created yet
		GFileManager->MakeDirectory( *PreprocessorOutputDir, true );

		// save out include files from the environment definitions
		// Note: Material.usf and VertexFactory.usf vary between materials/vertex factories
		// this is handled because fxc will search for the includes in the same directory as the main shader before searching the include path 
		// otherwise it would find a stale Material.usf and VertexFactory.usf left behind by other platforms
		for(TMap<FString,FString>::TConstIterator IncludeIt(Environment.IncludeFiles); IncludeIt; ++IncludeIt)
		{
			FString IncludePath = PreprocessorOutputDir * IncludeIt.Key();
			appSaveStringToFile(IncludeIt.Value(), *IncludePath);
		}

		const FString SaveFileName = PreprocessorOutputDir * SourceFilename;
		const FString SourceFile = LoadShaderSourceFile(SourceFilename);
		appSaveStringToFile(SourceFile, *(SaveFileName + TEXT(".usf")));

		//allow dumping the preprocessed shader
		FD3D11IncludeEnvironment IncludeEnvironment(Environment);
		D3D11PreProcessShader(Target, SourceFilename, SourceFile, Macros, IncludeEnvironment, *PreprocessorOutputDir);

		const FString AbsoluteShaderPath = FString(appBaseDir()) * PreprocessorOutputDir;
		const FString AbsoluteIncludePath = FString(appBaseDir()) * ShaderPath;
		// get the fxc command line
		FString FXCCommandline = D3D11CreateShaderCompileCommandLine(
			Target,
			AbsoluteShaderPath * SourceFilename + TEXT(".usf"), 
			AbsoluteIncludePath, 
			FunctionName, 
			ShaderProfile,
			&Macros(0),
			CompileFlags,
			FALSE);

		appSaveStringToFile(FXCCommandline, *(SaveFileName + TEXT(".bat")));

		FString PreprocessedFXCCommandline = D3D11CreateShaderCompileCommandLine(
			Target,
			FString(SourceFilename) + TEXT(".pre"), 
			AbsoluteIncludePath, 
			FunctionName, 
			ShaderProfile,
			&Macros(0),
			CompileFlags,
			TRUE);

		appSaveStringToFile(PreprocessedFXCCommandline, *(SaveFileName + TEXT("PRE.bat")));

		if (bDebugDump && !bShaderCompileSucceeded)
		{
			warnf( NAME_DevShaders, TEXT( "ASM not supported for DX10 as of March 2009 DirectX." ) );
		}
	}

	// Free temporary strings allocated for the macros.
	for(INT MacroIndex = 0;MacroIndex < Macros.Num();MacroIndex++)
	{
		delete [] Macros(MacroIndex).Name;
		delete [] Macros(MacroIndex).Definition;
	}

	return bShaderCompileSucceeded;
}

#endif