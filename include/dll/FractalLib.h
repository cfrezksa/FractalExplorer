#pragma once

#ifdef FRACTALLIB_EXPORTS
#define FRACTALLIB_API __declspec(dllexport)
#else
#define FRACTALLIB_API __declspec(dllimport)
#endif

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


/*
// This class is exported from the FractalLib.dll
class FRACTALLIB_API CFractalLib {
public:
	CFractalLib(void);
	// TODO: add your methods here.
};

extern FRACTALLIB_API int nFractalLib;

FRACTALLIB_API int fnFractalLib(void);
*/
