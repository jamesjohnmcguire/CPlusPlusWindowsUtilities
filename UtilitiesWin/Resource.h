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
			LPCTSTR sourceModulePath);
		~Resource(void);

		HMODULE GetResourceModule();
		std::wstring GetString(
			UINT	resourceId);
		std::wstring LoadStringResource(
			HINSTANCE moduleHandle,
			UINT resourceId);
		int ShowMessageString(
			LPCTSTR message,
			UINT messageType = MB_OK);
		int ShowMessage(
			int titleId,
			int	stringId);
		int ShowMessageError(
			int titleId,
			LPCTSTR	message,
			int		stringId);
		int ShowMessageYesNo(
			int titleId,
			int	stringId);

	private:
		LPCTSTR modulePath;
		HMODULE resourceModule;
};
