////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CModuleVersion provides an easy way to get version info
// for a module.(DLL or EXE).
//
#include "pch.h"
#include "ModuleVersion.h"

#include <utility>

CModuleVersion::CModuleVersion()
{
	// Reset VS_FIXEDFILEINFO to zeroes.
	*static_cast<VS_FIXEDFILEINFO*>(this) = {};
}

//////////////////
// Destroy: release owned version info.
//
CModuleVersion::~CModuleVersion() = default;

//////////////////
// Get file version info for a given module.
// Clear the previous result first, then store valid fixed and string data.
//
BOOL CModuleVersion::GetFileVersionInfo(const wchar_t* moduleName)
{
	versionInfoData.clear();
	translation = {};
	*static_cast<VS_FIXEDFILEINFO*>(this) = {};

	// get module handle
	wchar_t moduleFileName[_MAX_PATH];
	HMODULE moduleHandle = ::GetModuleHandle(moduleName);

	if (moduleHandle==nullptr && moduleName!=nullptr)
		return FALSE;

	// get module file name
	DWORD length = GetModuleFileName(moduleHandle, moduleFileName,
		sizeof(moduleFileName)/sizeof(moduleFileName[0]));

	if (length <= 0)
		return FALSE;

	// read file version info
	DWORD ignoredHandle; // will always be set to zero
	length = GetFileVersionInfoSize(moduleFileName, &ignoredHandle);

	if (length <= 0)
		return FALSE;

	std::vector<BYTE> loadedVersionInfo(length);

	if (!::GetFileVersionInfo(moduleFileName, 0, length,
		loadedVersionInfo.data()))
		return FALSE;

	LPVOID versionValue = nullptr;
	UINT valueLength = 0;

	if (!VerQueryValue(loadedVersionInfo.data(), L"\\", &versionValue,
		&valueLength))
		return FALSE;

	if (versionValue == nullptr)
		return FALSE;

	if (valueLength < sizeof(VS_FIXEDFILEINFO))
		return FALSE;

	const VS_FIXEDFILEINFO loadedFixedInfo =
		*static_cast<VS_FIXEDFILEINFO*>(versionValue);

	if (loadedFixedInfo.dwSignature != VS_FFI_SIGNATURE)
		return FALSE;

	// Get translation info
	TRANSLATION loadedTranslation{};
	versionValue = nullptr;
	valueLength = 0;
	const BOOL translationFound = VerQueryValue(loadedVersionInfo.data(),
		L"\\VarFileInfo\\Translation", &versionValue, &valueLength);

	if (translationFound && versionValue != nullptr)
	{
		if (valueLength >= sizeof(TRANSLATION))
		{
			loadedTranslation = *static_cast<TRANSLATION*>(versionValue);
		}
	}

	*static_cast<VS_FIXEDFILEINFO*>(this) = loadedFixedInfo;
	translation = loadedTranslation;
	versionInfoData = std::move(loadedVersionInfo);

	return TRUE;
}

//////////////////
// Get string file info.
// Key name is something like "CompanyName".
// Returns the value as a std::wstring.
//
std::wstring CModuleVersion::GetValue(const wchar_t* keyName)
{
	std::wstring result;

	if (!versionInfoData.empty())
	{
		// To get a string value must pass query in the form
		//
		//    "\StringFileInfo\<langID><codepage>\keyname"
		//
		// where <lang-codepage> is the languageID concatenated with the
		// code page, in hex. Wow.
		std::wstring langCodePage = std::format(L"{:04x}{:04x}",
			translation.langID,
			translation.charset);

		std::wstring query = std::format(L"\\StringFileInfo\\{}\\{}",
			langCodePage,
			keyName);

		const wchar_t* value;
		UINT valueLength;

		bool returnCode = VerQueryValue(
			versionInfoData.data(),
			query.c_str(),
			(LPVOID*)&value,
			&valueLength);

		if (returnCode == true)
		{

			result = value;
		}
	}

	return result;
}

// typedef for DllGetVersion proc
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

/////////////////
// Get DLL Version by calling the DLL's DllGetVersion proc.
// Release the loaded library whether the export exists or not.
//
BOOL CModuleVersion::DllGetVersion(
	const wchar_t* moduleName, DLLVERSIONINFO& versionInfo)
{
	BOOL result = FALSE;

	HINSTANCE moduleHandle = LoadLibrary(moduleName);

	if (moduleHandle != nullptr)
	{
		// Must use GetProcAddress because the DLL might not implement 
		// DllGetVersion. Depending upon the DLL, the lack of implementation of
		// the function may be a version marker in itself.
		DLLGETVERSIONPROC getDllVersion =
			(DLLGETVERSIONPROC)GetProcAddress(moduleHandle, "DllGetVersion");

		if (getDllVersion != nullptr)
		{
			memset(&versionInfo, 0, sizeof(versionInfo));
			versionInfo.cbSize = sizeof(versionInfo);

			HRESULT longResult = (*getDllVersion)(&versionInfo);
			result = SUCCEEDED(longResult);
		}

		FreeLibrary(moduleHandle);
	}

	return result;
}
