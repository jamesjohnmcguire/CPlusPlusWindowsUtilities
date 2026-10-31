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
	HKEY	rootKeyHandle,
	LPCTSTR	registryKeyPath,
	LPCTSTR	subKeyName,
	bool	onlyIfEmpty)
{
	LONG	registryResult	= -1;
	HKEY	keyHandle;

	registryResult = RegOpenKeyEx(	rootKeyHandle,
		registryKeyPath,
		0,
		KEY_READ,
		&keyHandle);

	if (ERROR_SUCCESS == registryResult)
	{
		if (onlyIfEmpty)
			registryResult	= RegDeleteKey(	keyHandle,
			subKeyName );
		else
			registryResult	= SHDeleteKey(	keyHandle,
			subKeyName );

		RegCloseKey( keyHandle );
	}

	return registryResult;
}

/////////////////////////////////////////////////////////////////////////////
// GetDwordValue
/////////////////////////////////////////////////////////////////////////////
DWORD Registry::GetDwordValue(HKEY keyRoot, LPCTSTR keyPath,
	LPCTSTR keyVariable)
{
	LONG	successCode	= -1;
	HKEY	keyHandle;
	DWORD	retunValue	= 0;
	DWORD	bufferSize	= sizeof(DWORD);
	successCode = RegOpenKeyEx(keyRoot, keyPath, 0, KEY_READ, &keyHandle);

	if (ERROR_SUCCESS == successCode)
	{
		successCode = RegQueryValueEx(keyHandle, keyVariable, 0, nullptr,
			(LPBYTE)&retunValue, &bufferSize);

		RegCloseKey( keyHandle );
	}

	return successCode;
}

/////////////////////////////////////////////////////////////////////////////
// GetSubKeyNames
/////////////////////////////////////////////////////////////////////////////
TCHAR** Registry::GetSubKeyNames(
	HKEY keyRoot, const TCHAR* regKey, DWORD* keyCount)
{
	LONG	registryResult = -1;
	HKEY	keyHandle;
	DWORD	keyValuesCount = 0;
	TCHAR**	keyNames = nullptr;

	registryResult = RegOpenKeyEx(keyRoot, regKey, 0, KEY_READ, &keyHandle);

	if (ERROR_SUCCESS == registryResult)
	{
		registryResult = RegQueryInfoKey(keyHandle, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			&keyValuesCount, nullptr, nullptr, nullptr, nullptr);

		if ((ERROR_SUCCESS == registryResult) && (keyValuesCount > 0))
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
					registryResult = RegEnumValue(keyHandle,
						loopIndex,
						keyName,
						&keyNameLength,
						nullptr,
						nullptr,
						nullptr,
						nullptr);

					if (ERROR_SUCCESS == registryResult)
					{
						keyNames[loopIndex] = GetStringCopy(keyName);
					}
				}
			}

			*keyCount = keyValuesCount;
		}
		RegCloseKey( keyHandle );
	}

	return keyNames;
}

/////////////////////////////////////////////////////////////////////////////
// GetSubKeysCount
/////////////////////////////////////////////////////////////////////////////
DWORD Registry::GetSubKeysCount(HKEY keyRoot, LPCTSTR regKey)
{
	LONG	registryResult	= -1;
	HKEY	keyHandle;
	DWORD	keyValuesCount	= 0;

	registryResult = RegOpenKeyEx(keyRoot, regKey, 0, KEY_READ, &keyHandle);

	if (ERROR_SUCCESS == registryResult)
	{
		registryResult	= RegQueryInfoKey(	keyHandle, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			&keyValuesCount, nullptr, nullptr, nullptr, nullptr);

		if (ERROR_SUCCESS != registryResult)
		{
			keyValuesCount = 0;
		}

		RegCloseKey( keyHandle );
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
	HKEY	keyHandle;
	TCHAR*	stringValue	= nullptr;
	DWORD	bufferSize	= 0;
	returnCode = RegOpenKeyEx(	keyRoot,
		regKeyPath,
		0,
		KEY_READ,
		&keyHandle);

	if (ERROR_SUCCESS == returnCode)
	{
		returnCode	= RegQueryValueEx(	keyHandle,
			regKeyVar,
			0,
			nullptr,
			nullptr,
			&bufferSize);

		if (ERROR_SUCCESS == returnCode)
		{
			stringValue = new TCHAR[bufferSize+1];

			returnCode	= RegQueryValueEx(	keyHandle,
				regKeyVar,
				0,
				nullptr,
				(LPBYTE)stringValue,
				&bufferSize);
		}

		RegCloseKey( keyHandle );
	}

	return stringValue;
}

/////////////////////////////////////////////////////////////////////////////
// GetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::GetValue(
	HKEY	rootKeyHandle,
	LPCTSTR	registryKeyPath,
	LPCTSTR	registryValueName,
	LPTSTR	registryValue,
	DWORD	valueBufferSize)
{
	LONG	registryResult	= -1;
	HKEY	keyHandle;

	if (registryValue)
	{
		registryResult = RegOpenKeyEx(	rootKeyHandle,
			registryKeyPath,
			0,
			KEY_READ,
			&keyHandle);

		if (ERROR_SUCCESS == registryResult)
		{
			registryResult	= RegQueryValueEx(	keyHandle,
				registryValueName,
				0,
				nullptr,
				(LPBYTE)registryValue,
				&valueBufferSize );

			RegCloseKey( keyHandle );
		}
	}

	return registryResult;
}

/////////////////////////////////////////////////////////////////////////////
// SetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::SetValue(
	HKEY	rootKeyHandle,
	LPCTSTR	registryKeyPath,
	LPCTSTR	registryValueName,
	LPCTSTR	registryValue)
{
	LONG	registryResult	= -1;
	HKEY	keyHandle;
	DWORD	keyDisposition;

	if (registryValue)
	{
		registryResult = RegCreateKeyEx(	rootKeyHandle,
			registryKeyPath,
			0,
			nullptr,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			nullptr,
			&keyHandle,
			&keyDisposition);

#if REGISTRY_OPEN_EXISTING_KEY_ALTERNATIVE
		registryResult = RegOpenKeyEx(	rootKeyHandle,
		registryKeyPath,
		0,
		KEY_READ | KEY_SET_VALUE,
		&keyHandle);
#endif
		if (ERROR_SUCCESS == registryResult)
		{
			registryResult	= RegSetValueEx(	keyHandle,
				registryValueName,
				0,
				REG_SZ,
				(LPBYTE)registryValue,
				(DWORD)(_tcslen(registryValue) + 1) * sizeof(TCHAR));

#ifdef _DEBUG
			if (ERROR_SUCCESS != registryResult)
			{
				TCHAR message[128];
				_stprintf_s(message,
					_T("Registry::SetValue: RegSetValueEx failed (%ld)\n"),
					registryResult);
				OutputDebugString(message);
			}
#endif
			RegCloseKey( keyHandle );
		}
#ifdef _DEBUG
		else
		{
			TCHAR message[128];
			_stprintf_s(message,
				_T("Registry::SetValue: RegCreateKeyEx failed (%ld)\n"),
				registryResult);
			OutputDebugString(message);
		}
#endif
	}

	return registryResult;
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
	LONG	registryResult	= -1;
	HKEY	keyHandle;
	DWORD	keyDisposition;

	registryResult = RegCreateKeyEx(	keyRoot,
		regKeyPath,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		nullptr,
		&keyHandle,
		&keyDisposition);

	if (ERROR_SUCCESS == registryResult)
	{
		registryResult	= RegSetValueEx(keyHandle,
			regKeyVar,
			0,
			REG_DWORD,
			(LPBYTE)&value,
			(DWORD)sizeof(DWORD));

		RegCloseKey( keyHandle );
	}

	return registryResult;
}
