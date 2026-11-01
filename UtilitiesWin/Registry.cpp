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
	const wchar_t*	registryKeyPath,
	const wchar_t*	subKeyName,
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
DWORD Registry::GetDwordValue(HKEY keyRoot, const wchar_t* keyPath,
	const wchar_t* keyVariable)
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
wchar_t** Registry::GetSubKeyNames(
	HKEY keyRoot, const wchar_t* regKey, DWORD* keyCount)
{
	LONG	registryResult = -1;
	HKEY	keyHandle;
	DWORD	keyValuesCount = 0;
	wchar_t**	keyNames = nullptr;

	registryResult = RegOpenKeyEx(keyRoot, regKey, 0, KEY_READ, &keyHandle);

	if (ERROR_SUCCESS == registryResult)
	{
		registryResult = RegQueryInfoKey(keyHandle, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			&keyValuesCount, nullptr, nullptr, nullptr, nullptr);

		if ((ERROR_SUCCESS == registryResult) && (keyValuesCount > 0))
		{
			keyNames = new wchar_t*[keyValuesCount];

			if (nullptr != keyNames)
			{
				wchar_t	keyName[MAX_KEY_LENGTH];
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
DWORD Registry::GetSubKeysCount(HKEY keyRoot, const wchar_t* regKey)
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
wchar_t*
	Registry::GetValue(
	HKEY keyRoot,
	const wchar_t* regKeyPath,
	const wchar_t* regKeyVar)
{
	LONG	returnCode	= -1;
	HKEY	keyHandle;
	wchar_t*	stringValue	= nullptr;
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
			stringValue = new wchar_t[bufferSize+1];

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
	const wchar_t*	registryKeyPath,
	const wchar_t*	registryValueName,
	wchar_t*	registryValue,
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
	const wchar_t*	registryKeyPath,
	const wchar_t*	registryValueName,
	const wchar_t*	registryValue)
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
				(DWORD)(wcslen(registryValue) + 1) * sizeof(wchar_t));

#ifdef _DEBUG
			if (ERROR_SUCCESS != registryResult)
			{
				wchar_t message[128];
				swprintf_s(message,
					L"Registry::SetValue: RegSetValueEx failed (%ld)\n",
					registryResult);
				OutputDebugString(message);
			}
#endif
			RegCloseKey( keyHandle );
		}
#ifdef _DEBUG
		else
		{
			wchar_t message[128];
			swprintf_s(message,
				L"Registry::SetValue: RegCreateKeyEx failed (%ld)\n",
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
	const wchar_t* regKeyPath,
	const wchar_t* regKeyVar,
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
