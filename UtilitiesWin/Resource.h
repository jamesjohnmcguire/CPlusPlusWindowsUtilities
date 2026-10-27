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
#include "Common.h"
#include <string>

class DllExport Resource
{
	public:
		Resource(
			LPCTSTR ModulePath);
		~Resource(void);

		HMODULE GetResourceModule();
		std::wstring GetString(
			UINT	ResourceId);
		std::wstring LoadStringResource(
			HINSTANCE hInstance,
			UINT resourceId);
		int ShowMessageString(
			LPCTSTR Message,
			UINT uType = MB_OK);
		int ShowMessage(
			int TitleId,
			int	StringId);
		int ShowMessageError(
			int TitleId,
			LPCTSTR	Message,
			int		StringId);
		int ShowMessageYesNo(
			int TitleId,
			int	StringId);

	private:
		LPCTSTR m_ModulePath;
		HMODULE m_ResourceModule;
};
