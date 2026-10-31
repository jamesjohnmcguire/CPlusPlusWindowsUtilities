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

CModuleVersion::CModuleVersion()
{
	versionInfoData = nullptr;				// raw version info data
}

//////////////////
// Destroy: delete version info
//
CModuleVersion::~CModuleVersion()
{
	delete [] versionInfoData;
}

//////////////////
// Get file version info for a given module
// Allocates storage for all info, fills "this" with
// VS_FIXEDFILEINFO, and sets codepage.
//
BOOL CModuleVersion::GetFileVersionInfo(LPCTSTR moduleName)
{
	translation.charset = 1252;		// default = ANSI code page
	memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

	// get module handle
	TCHAR moduleFileName[_MAX_PATH];
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

	versionInfoData = new BYTE[length]; // allocate version info
	if (!::GetFileVersionInfo(moduleFileName, 0, length, versionInfoData))
		return FALSE;

	LPVOID versionValue;
	UINT valueLength;
	if (!VerQueryValue(versionInfoData, _T("\\"), &versionValue, &valueLength))
		return FALSE;

	// copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
	*(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)versionValue;

	// Get translation info
	if (VerQueryValue(versionInfoData,
		_T("\\VarFileInfo\\Translation"), &versionValue, &valueLength) && valueLength >= 4) {
		translation = *(TRANSLATION*)versionValue;
	}

	return dwSignature == VS_FFI_SIGNATURE;
}

//////////////////
// Get string file info.
// Key name is something like "CompanyName".
// returns the value as a CString.
//
std::wstring CModuleVersion::GetValue(LPCTSTR keyName)
{
	std::wstring result;

	if (versionInfoData)
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

		LPCTSTR value;
		UINT valueLength;

		bool returnCode = VerQueryValue(
			versionInfoData,
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
// Get DLL Version by calling DLL's DllGetVersion proc
//
BOOL CModuleVersion::DllGetVersion(LPCTSTR moduleName, DLLVERSIONINFO& versionInfo)
{
	HINSTANCE moduleHandle = LoadLibrary(moduleName);
	if (!moduleHandle)
		return FALSE;

	// Must use GetProcAddress because the DLL might not implement 
	// DllGetVersion. Depending upon the DLL, the lack of implementation of the 
	// function may be a version marker in itself.
	//
	DLLGETVERSIONPROC getDllVersion =
		(DLLGETVERSIONPROC)GetProcAddress(moduleHandle, "DllGetVersion");

	if (!getDllVersion)
		return FALSE;

	memset(&versionInfo, 0, sizeof(versionInfo));			 // clear
	versionInfo.cbSize = sizeof(versionInfo);				 // set size for Windows

	return SUCCEEDED((*getDllVersion)(&versionInfo));
}
