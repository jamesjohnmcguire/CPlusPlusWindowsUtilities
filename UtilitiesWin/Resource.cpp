/////////////////////////////////////////////////////////////////////////////
// Resource.cpp
//
// Represents a resource module.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Resource.h"
#include "Utils.h"

Resource::Resource(
	LPCTSTR sourceModulePath) :
	resourceModule(NULL)
{
	modulePath = sourceModulePath;
}

Resource::~Resource(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// GetResourceModule
/////////////////////////////////////////////////////////////////////////////
HMODULE
Resource::GetResourceModule()
{
	if (NULL == resourceModule)
	{
		if (NULL != modulePath)
		{
			resourceModule = LoadLibrary( modulePath );
		}
	}

	return resourceModule;
}

/////////////////////////////////////////////////////////////////////////////
// GetString
//
// Gets an localized string from the resource table
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
std::wstring
Resource::GetString(
	UINT	resourceId)
{
	std::wstring resourceString;

	if (NULL == resourceModule)
	{
		resourceModule = GetResourceModule();
	}

	if (NULL != resourceModule)
	{
		resourceString = LoadStringResource(resourceModule, resourceId);
	}

	return resourceString;
}

std::wstring Resource::LoadStringResource(HINSTANCE hInstance, UINT resourceId)
{
	wchar_t buffer[256];
	int len = ::LoadString(hInstance, resourceId, buffer, _countof(buffer));

	if (len > 0) {
		return std::wstring(buffer);
	}
	return L"";  // or throw an exception
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessage
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessage(
	int titleId,
	int	stringId)
{
	std::wstring title = GetString(titleId);
	std::wstring message = GetString(stringId);
	LPCWSTR pTitle = title.c_str();
	LPCWSTR pMessage = message.c_str();
	int returnCode = MessageBox(GetActiveWindow(), pMessage, pTitle, MB_OK);

	return returnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessageError
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessageError(
	int titleId,
	LPCTSTR	message,
	int		stringId)
{
	std::wstring title = GetString(titleId);
	std::wstring errorMessage = GetString(stringId);
	LPCWSTR pTitle = title.c_str();

	std::wstring completeMessage = std::wstring(message) + L": " + errorMessage;
	LPCWSTR pMessage = completeMessage.c_str();

	int returnCode = MessageBox(GetActiveWindow(), pMessage, pTitle, MB_OK);

	return returnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessage
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessageYesNo(
	int titleId,
	int	stringId)
{
	std::wstring title = GetString(titleId);
	std::wstring message = GetString(stringId);
	LPCWSTR pTitle = title.c_str();
	LPCWSTR pMessage = message.c_str();

	int returnCode	= MessageBox(GetActiveWindow(), pMessage, pTitle, MB_YESNO);

	return returnCode;
}
