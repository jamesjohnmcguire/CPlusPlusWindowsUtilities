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
	LPCTSTR ModulePath) :
	resourceModule(NULL)
{
	modulePath = ModulePath;
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
	UINT	ResourceId)
{
	std::wstring resourceString;

	if (NULL == resourceModule)
	{
		resourceModule = GetResourceModule();
	}

	if (NULL != resourceModule)
	{
		resourceString = LoadStringResource(resourceModule, ResourceId);
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
	int TitleId,
	int	StringId)
{
	std::wstring Title = GetString(TitleId);
	std::wstring Message = GetString(StringId);
	LPCWSTR pTitle = Title.c_str();
	LPCWSTR pMessage = Message.c_str();
	int ReturnCode = MessageBox(GetActiveWindow(), pMessage, pTitle, MB_OK);

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessageError
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessageError(
	int TitleId,
	LPCTSTR	Message,
	int		StringId)
{
	std::wstring Title = GetString(TitleId);
	std::wstring ErrorMessage = GetString(StringId);
	LPCWSTR pTitle = Title.c_str();

	std::wstring CompleteMessage = std::wstring(Message) + L": " + ErrorMessage;
	LPCWSTR pMessage = CompleteMessage.c_str();

	int ReturnCode = MessageBox(GetActiveWindow(), pMessage, pTitle, MB_OK);

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessage
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessageYesNo(
	int TitleId,
	int	StringId)
{
	std::wstring Title = GetString(TitleId);
	std::wstring Message = GetString(StringId);
	LPCWSTR pTitle = Title.c_str();
	LPCWSTR pMessage = Message.c_str();

	int ReturnCode	= MessageBox(GetActiveWindow(), pMessage, pTitle, MB_YESNO);

	return ReturnCode;
}
