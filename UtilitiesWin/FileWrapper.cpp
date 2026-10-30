/////////////////////////////////////////////////////////////////////////////
// FileWrapper.cpp - Class Implementation
//
// Encapsulation class for reading and writing files.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "FileWrapper.h"
#include <memory>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "utils.h"

#if defined _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

/////////////////////////////////////////////////////////////////////////////
// Constructors / Destructors
// Store an owned filename so the caller's buffer can change or be released.
/////////////////////////////////////////////////////////////////////////////
FileWrapper::FileWrapper(
	LPCTSTR sourceFileName)
{
	if (sourceFileName != nullptr)
	{
		fileName = sourceFileName;
	}
}

FileWrapper::FileWrapper(
	LPCSTR sourceFileName)
{
	std::unique_ptr<wchar_t[]> converted(GetUnicodeString(sourceFileName));
	if (converted != nullptr)
	{
		fileName = converted.get();
	}
}

FileWrapper::~FileWrapper(void) = default;

/////////////////////////////////////////////////////////////////////////////
// Append
/////////////////////////////////////////////////////////////////////////////
bool
	FileWrapper::Append(
	BYTE*	contents,
	DWORD	contentsLength)
{
	bool	returnCode	= false;

	if (!fileName.empty() && (NULL != contents))
	{
		HANDLE	fileHandle	= CreateFile(fileName.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_ALWAYS,
			0,
			0);

		if (INVALID_HANDLE_VALUE != fileHandle)
		{
			DWORD	bytesRead	= 0;

			SetFilePointer(fileHandle, 0, NULL, FILE_END);

			BOOL	resultCode	= WriteFile(fileHandle, contents, contentsLength, &bytesRead, NULL);

			CloseHandle(fileHandle);

			if (FALSE != resultCode)
			{
				returnCode	= true;
			}
		}
	}

	return returnCode;
}

/////////////////////////////////////////////////////////////////////////////
// Create
/////////////////////////////////////////////////////////////////////////////
bool
	FileWrapper::Create(
	BYTE*	contents,
	DWORD	contentsLength)
{
	bool	returnCode	= false;

	if (!fileName.empty() && (NULL != contents))
	{
		HANDLE	fileHandle	= CreateFile(fileName.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS,
			0,
			0);

		if (INVALID_HANDLE_VALUE != fileHandle)
		{
			DWORD	bytesRead	= 0;

			BOOL	resultCode	= WriteFile(fileHandle, contents, contentsLength, &bytesRead, NULL);

			CloseHandle(fileHandle);

			if (FALSE != resultCode)
			{
				returnCode	= true;
			}
		}
	}

	return returnCode;
}

/////////////////////////////////////////////////////////////////////////////
// Read
/////////////////////////////////////////////////////////////////////////////
BYTE*
	FileWrapper::Read(
	DWORD*	contentsLength)
{
	BYTE*	inputContents = NULL;

	if (!fileName.empty())
	{
		HANDLE	fileHandle	= CreateFile(fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != fileHandle)
		{
			DWORD	contentsSize	= GetFileSize(fileHandle, NULL) + 2;

			if (0 < contentsSize )
			{
				inputContents = new BYTE[contentsSize];

				if (NULL != inputContents)
				{
					DWORD	bytesRead	= 0;

					SecureZeroMemory(inputContents, contentsSize);

					BOOL	resultCode	= ReadFile(fileHandle, inputContents, contentsSize, &bytesRead, NULL) ;

					CloseHandle(fileHandle);

					if (FALSE != resultCode)
					{
						*contentsLength	= contentsSize;
					}
				}
			}
		}
	}

	return inputContents;
}

/////////////////////////////////////////////////////////////////////////////
// ReadText
/////////////////////////////////////////////////////////////////////////////
char*
	FileWrapper::ReadText(void)
{
	char*	inputContents = NULL;

	if (!fileName.empty())
	{
		HANDLE	fileHandle	= CreateFile(fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != fileHandle)
		{
			DWORD	contentsSize	= GetFileSize(fileHandle, NULL) + 1;

			if (0 < contentsSize )
			{
				inputContents = new char[contentsSize];

				if (NULL != inputContents)
				{
					DWORD	bytesRead	= 0;

					SecureZeroMemory(inputContents, contentsSize);

					BOOL result = ReadFile(fileHandle, inputContents, contentsSize, &bytesRead, NULL) ;

					if (result == FALSE)
					{
					}

					CloseHandle(fileHandle);
				}
			}
		}
	}

	return inputContents;
}

/////////////////////////////////////////////////////////////////////////////
// ReadUnicodeText
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	FileWrapper::ReadUnicodeText(void)
{
	TCHAR*	inputContents = NULL;

	if (!fileName.empty())
	{
		HANDLE	fileHandle	= CreateFile(fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != fileHandle)
		{
			DWORD	contentsSize	= GetFileSize(fileHandle, NULL) / sizeof(TCHAR) + 2;

			if (0 < contentsSize )
			{
				inputContents = new TCHAR[contentsSize];

				if (NULL != inputContents)
				{
					DWORD	bytesRead	= 0;

					SecureZeroMemory(inputContents, contentsSize);

					BOOL result = ReadFile(fileHandle, inputContents, contentsSize, &bytesRead, NULL) ;

					if (result == FALSE)
					{
					}

					CloseHandle(fileHandle);
				}
			}
		}
	}

	return inputContents;
}
