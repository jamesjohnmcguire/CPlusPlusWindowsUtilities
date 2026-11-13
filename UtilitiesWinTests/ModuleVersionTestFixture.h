#pragma once

#include "../UtilitiesWin/ModuleVersion.h"

#ifdef MODULEVERSIONTESTFIXTURE_EXPORTS
	#define MODULEVERSIONTESTFIXTURE_API __declspec(dllexport)
#else
	#define MODULEVERSIONTESTFIXTURE_API __declspec(dllimport)
#endif

extern "C" MODULEVERSIONTESTFIXTURE_API HRESULT CALLBACK DllGetVersion(
	DLLVERSIONINFO* versionInfo);
