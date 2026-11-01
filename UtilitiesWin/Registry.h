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
				const wchar_t*	registryKeyPath,
				const wchar_t*	subKeyName,
				bool	onlyIfEmpty);
			DWORD GetDwordValue(
				HKEY	keyRoot,
				const wchar_t*	keyPath,
				const wchar_t*	keyVariable);
			wchar_t** GetSubKeyNames(
				HKEY keyRoot,
				const wchar_t* regKey,
				DWORD*	keyCount);
			DWORD GetSubKeysCount(
				HKEY keyRoot,
				const wchar_t* regKey);
			wchar_t* GetValue(
				HKEY	keyRoot,
				const wchar_t*	regKeyPath,
				const wchar_t*	regKeyVar);
			LONG GetValue(
				HKEY	rootKeyHandle,
				const wchar_t*	registryKeyPath,
				const wchar_t*	registryValueName,
				wchar_t*	registryValue,
				DWORD	valueBufferSize);
			LONG SetValue(
				HKEY	rootKeyHandle,
				const wchar_t*	registryKeyPath,
				const wchar_t*	registryValueName,
				const wchar_t*	registryValue);
			LONG SetValue(
				HKEY	keyRoot,
				const wchar_t*	regKeyPath,
				const wchar_t*	regKeyVar,
				DWORD	value);
};
