/////////////////////////////////////////////////////////////////////////////
// Resource.h
//
// Represents a resource module.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "UtilitiesWin.h"
#include <string>

class DllExport Resource
{
	public:
		Resource(
			const wchar_t* sourceModulePath);
		~Resource(void);

		HMODULE GetResourceModule();
		std::wstring GetString(
			UINT	resourceId);
		std::wstring LoadStringResource(
			HINSTANCE moduleHandle,
			UINT resourceId);
		int ShowMessageString(
			const wchar_t* message,
			UINT messageType = MB_OK);
		int ShowMessage(
			int titleId,
			int	stringId);
		int ShowMessageError(
			int titleId,
			const wchar_t*	message,
			int		stringId);
		int ShowMessageYesNo(
			int titleId,
			int	stringId);

	private:
		const wchar_t* modulePath;
		HMODULE resourceModule;
};
