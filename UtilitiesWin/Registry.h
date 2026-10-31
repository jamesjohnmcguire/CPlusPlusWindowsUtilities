/////////////////////////////////////////////////////////////////////////////
// Registry.h
//
// Represents the  Windows registry.
//
// Copyright © 2007 - 2025 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "UtilitiesWin.h"

#define MAX_KEY_LENGTH 255

/////////////////////////////////////////////////////////////////////////////
// Registry Class Definition
/////////////////////////////////////////////////////////////////////////////
class DllExport Registry
{
	public:
		// Properties

		// Methods
			Registry(void);
			~Registry(void);

			LONG Delete(
				HKEY	rootKeyHandle,
				LPCTSTR	registryKeyPath,
				LPCTSTR	subKeyName,
				bool	onlyIfEmpty);
			DWORD GetDwordValue(
				HKEY	keyRoot,
				LPCTSTR	keyPath,
				LPCTSTR	keyVariable);
			TCHAR** GetSubKeyNames(
				HKEY keyRoot,
				const TCHAR* regKey,
				DWORD*	keyCount);
			DWORD GetSubKeysCount(
				HKEY keyRoot,
				LPCTSTR regKey);
			TCHAR* GetValue(
				HKEY	keyRoot,
				LPCTSTR	regKeyPath,
				LPCTSTR	regKeyVar);
			LONG GetValue(
				HKEY	rootKeyHandle,
				LPCTSTR	registryKeyPath,
				LPCTSTR	registryValueName,
				LPTSTR	registryValue,
				DWORD	valueBufferSize);
			LONG SetValue(
				HKEY	rootKeyHandle,
				LPCTSTR	registryKeyPath,
				LPCTSTR	registryValueName,
				LPCTSTR	registryValue);
			LONG SetValue(
				HKEY	keyRoot,
				LPCTSTR	regKeyPath,
				LPCTSTR	regKeyVar,
				DWORD	value);
};
