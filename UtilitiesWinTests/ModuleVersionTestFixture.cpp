#include "../UtilitiesWin/ModuleVersion.h"

extern "C" HRESULT CALLBACK DllGetVersion(DLLVERSIONINFO* versionInfo)
{
	if (versionInfo == nullptr)
	{
		return E_POINTER;
	}

	if (versionInfo->cbSize != sizeof(DLLVERSIONINFO))
	{
		return E_INVALIDARG;
	}

	versionInfo->dwMajorVersion = 3;
	versionInfo->dwMinorVersion = 7;
	versionInfo->dwBuildNumber = 11;
	versionInfo->dwPlatformID = DLLVER_PLATFORM_NT;
	return S_OK;
}
