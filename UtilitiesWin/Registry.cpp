/////////////////////////////////////////////////////////////////////////////
// Registry.h
//
// Represents the  Windows registry.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#define USE_CRTDBG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <shlwapi.h>
#include "Registry.h"
#include "Utils.h"

/////////////////////////////////////////////////////////////////////////////
// Constructors / Destructors
/////////////////////////////////////////////////////////////////////////////
Registry::Registry(void)
{
}

Registry::~Registry(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// Delete
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::Delete(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszSubKey,
	bool	bOnlyIfEmpty)
{
	LONG	lRet	= -1;
	HKEY	hKey;

	lRet = RegOpenKeyEx(	hKeyRoot,
		pszRegKeyPath,
		0,
		KEY_READ,
		&hKey);

	if (ERROR_SUCCESS == lRet)
	{
		if (bOnlyIfEmpty)
			lRet	= RegDeleteKey(	hKey,
			pszSubKey );
		else
			lRet	= SHDeleteKey(	hKey,
			pszSubKey );

		RegCloseKey( hKey );
	}

	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// GetDwordValue
/////////////////////////////////////////////////////////////////////////////
DWORD Registry::GetDwordValue(HKEY keyRoot, LPCTSTR keyPath,
	LPCTSTR keyVariable)
{
	LONG	successCode	= -1;
	HKEY	hKey;
	DWORD	retunValue	= 0;
	DWORD	bufferSize	= sizeof(DWORD);
	successCode = RegOpenKeyEx(keyRoot, keyPath, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == successCode)
	{
		successCode = RegQueryValueEx(hKey, keyVariable, 0, nullptr,
			(LPBYTE)&retunValue, &bufferSize);

		RegCloseKey( hKey );
	}

	return successCode;
}

/////////////////////////////////////////////////////////////////////////////
// GetSubKeyNames
/////////////////////////////////////////////////////////////////////////////
TCHAR** Registry::GetSubKeyNames(
	HKEY keyRoot, const TCHAR* regKey, DWORD* keyCount)
{
	LONG	lRet = -1;
	HKEY	hKey;
	DWORD	keyValuesCount = 0;
	TCHAR**	keyNames = nullptr;

	lRet = RegOpenKeyEx(keyRoot, regKey, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegQueryInfoKey(hKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			&keyValuesCount, nullptr, nullptr, nullptr, nullptr);

		if ((ERROR_SUCCESS == lRet) && (keyValuesCount > 0))
		{
			keyNames = new TCHAR*[keyValuesCount];

			if (nullptr != keyNames)
			{
				TCHAR	keyName[MAX_KEY_LENGTH];
				DWORD	keyNameLength = MAX_KEY_LENGTH;

				for (DWORD loopIndex=0; loopIndex < keyValuesCount; loopIndex++)
				{
					keyName[0] = '\0';
					keyNameLength = MAX_KEY_LENGTH;
					lRet = RegEnumValue(hKey,
						loopIndex,
						keyName,
						&keyNameLength,
						nullptr,
						nullptr,
						nullptr,
						nullptr);

					if (ERROR_SUCCESS == lRet)
					{
						keyNames[loopIndex] = GetStringCopy(keyName);
					}
				}
			}

			*keyCount = keyValuesCount;
		}
		RegCloseKey( hKey );
	}

	return keyNames;
}

/////////////////////////////////////////////////////////////////////////////
// GetSubKeysCount
/////////////////////////////////////////////////////////////////////////////
DWORD Registry::GetSubKeysCount(HKEY keyRoot, LPCTSTR regKey)
{
	LONG	lRet	= -1;
	HKEY	hKey;
	DWORD	keyValuesCount	= 0;

	lRet = RegOpenKeyEx(keyRoot, regKey, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		lRet	= RegQueryInfoKey(	hKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			&keyValuesCount, nullptr, nullptr, nullptr, nullptr);

		if (ERROR_SUCCESS != lRet)
		{
			keyValuesCount = 0;
		}

		RegCloseKey( hKey );
	}

	return keyValuesCount;
}

/////////////////////////////////////////////////////////////////////////////
// GetValue
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	Registry::GetValue(
	HKEY keyRoot,
	LPCTSTR regKeyPath,
	LPCTSTR regKeyVar)
{
	LONG	returnCode	= -1;
	HKEY	hKey;
	TCHAR*	stringValue	= nullptr;
	DWORD	bufferSize	= 0;
	returnCode = RegOpenKeyEx(	keyRoot,
		regKeyPath,
		0,
		KEY_READ,
		&hKey);

	if (ERROR_SUCCESS == returnCode)
	{
		returnCode	= RegQueryValueEx(	hKey,
			regKeyVar,
			0,
			nullptr,
			nullptr,
			&bufferSize);

		if (ERROR_SUCCESS == returnCode)
		{
			stringValue = new TCHAR[bufferSize+1];

			returnCode	= RegQueryValueEx(	hKey,
				regKeyVar,
				0,
				nullptr,
				(LPBYTE)stringValue,
				&bufferSize);
		}

		RegCloseKey( hKey );
	}

	return stringValue;
}

/////////////////////////////////////////////////////////////////////////////
// GetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::GetValue(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszRegKeyVar,
	LPTSTR	pszValue,
	DWORD	dwValueBufSize)
{
	LONG	lRet	= -1;
	HKEY	hKey;

	if (pszValue)
	{
		lRet = RegOpenKeyEx(	hKeyRoot,
			pszRegKeyPath,
			0,
			KEY_READ,
			&hKey);

		if (ERROR_SUCCESS == lRet)
		{
			lRet	= RegQueryValueEx(	hKey,
				pszRegKeyVar,
				0,
				nullptr,
				(LPBYTE)pszValue,
				&dwValueBufSize );

			RegCloseKey( hKey );
		}
	}

	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// SetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::SetValue(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszRegKeyVar,
	LPCTSTR	pszValue)
{
	LONG	lRet	= -1;
	HKEY	hKey;
	DWORD	dwKeyOut;

	if (pszValue)
	{
		lRet = RegCreateKeyEx(	hKeyRoot,
			pszRegKeyPath,
			0,
			nullptr,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			nullptr,
			&hKey,
			&dwKeyOut);

#if REGISTRY_OPEN_EXISTING_KEY_ALTERNATIVE
		lRet = RegOpenKeyEx(	hKeyRoot,
		pszRegKeyPath,
		0,
		KEY_READ | KEY_SET_VALUE,
		&hKey);
#endif
		if (ERROR_SUCCESS == lRet)
		{
			lRet	= RegSetValueEx(	hKey,
				pszRegKeyVar,
				0,
				REG_SZ,
				(LPBYTE)pszValue,
				(DWORD)(_tcslen(pszValue) + 1) * sizeof(TCHAR));

#ifdef _DEBUG
			if (ERROR_SUCCESS != lRet)
			{
				TCHAR message[128];
				_stprintf_s(message,
					_T("Registry::SetValue: RegSetValueEx failed (%ld)\n"),
					lRet);
				OutputDebugString(message);
			}
#endif
			RegCloseKey( hKey );
		}
#ifdef _DEBUG
		else
		{
			TCHAR message[128];
			_stprintf_s(message,
				_T("Registry::SetValue: RegCreateKeyEx failed (%ld)\n"),
				lRet);
			OutputDebugString(message);
		}
#endif
	}

	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// SetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::SetValue(
	HKEY keyRoot,
	LPCTSTR regKeyPath,
	LPCTSTR regKeyVar,
	DWORD value)
{
	LONG	lRet	= -1;
	HKEY	hKey;
	DWORD	dwKeyOut;

	lRet = RegCreateKeyEx(	keyRoot,
		regKeyPath,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		nullptr,
		&hKey,
		&dwKeyOut);

	if (ERROR_SUCCESS == lRet)
	{
		lRet	= RegSetValueEx(hKey,
			regKeyVar,
			0,
			REG_DWORD,
			(LPBYTE)&value,
			(DWORD)sizeof(DWORD));

		RegCloseKey( hKey );
	}

	return lRet;
}
