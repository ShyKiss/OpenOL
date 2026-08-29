// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string.h>

#include <windows.h>
#include <ole2.h>      // CoInitialize, CoCreateInstance, VARIANT, BSTR helpers
#include <wininet.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shldisp.h>   // IShellDispatch, Folder, FolderItems

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
