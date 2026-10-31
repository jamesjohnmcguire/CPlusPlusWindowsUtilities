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
	resourceModule(nullptr)
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
	if (nullptr == resourceModule)
	{
		if (nullptr != modulePath)
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

	if (nullptr == resourceModule)
	{
		resourceModule = GetResourceModule();
	}

	if (nullptr != resourceModule)
	{
		resourceString = LoadStringResource(resourceModule, resourceId);
	}

	return resourceString;
}

std::wstring Resource::LoadStringResource(HINSTANCE moduleHandle, UINT resourceId)
{
	wchar_t buffer[256];
	int length = ::LoadString(moduleHandle, resourceId, buffer, _countof(buffer));

	if (length > 0) {
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
	LPCWSTR titleText = title.c_str();
	LPCWSTR messageText = message.c_str();
	int returnCode = MessageBox(GetActiveWindow(), messageText, titleText, MB_OK);

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
	LPCWSTR titleText = title.c_str();

	std::wstring completeMessage = std::wstring(message) + L": " + errorMessage;
	LPCWSTR messageText = completeMessage.c_str();

	int returnCode = MessageBox(GetActiveWindow(), messageText, titleText, MB_OK);

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
	LPCWSTR titleText = title.c_str();
	LPCWSTR messageText = message.c_str();

	int returnCode	= MessageBox(GetActiveWindow(), messageText, titleText, MB_YESNO);

	return returnCode;
}
