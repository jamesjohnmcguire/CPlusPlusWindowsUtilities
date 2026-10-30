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
	LPCTSTR FileName)
{
	if (FileName != nullptr)
	{
		fileName = FileName;
	}
}

FileWrapper::FileWrapper(
	LPCSTR FileName)
{
	std::unique_ptr<wchar_t[]> converted(GetUnicodeString(FileName));
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
	BYTE*	Contents,
	DWORD	ContentsLength)
{
	bool	ReturnCode	= false;

	if (!fileName.empty() && (NULL != Contents))
	{
		HANDLE	FileHandle	= CreateFile(fileName.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_ALWAYS,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	BytesRead	= 0;

			SetFilePointer(FileHandle, 0, NULL, FILE_END);

			BOOL	ResultCode	= WriteFile(FileHandle, Contents, ContentsLength, &BytesRead, NULL);

			CloseHandle(FileHandle);

			if (FALSE != ResultCode)
			{
				ReturnCode	= true;
			}
		}
	}

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// Create
/////////////////////////////////////////////////////////////////////////////
bool
	FileWrapper::Create(
	BYTE*	Contents,
	DWORD	ContentsLength)
{
	bool	ReturnCode	= false;

	if (!fileName.empty() && (NULL != Contents))
	{
		HANDLE	FileHandle	= CreateFile(fileName.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	BytesRead	= 0;

			BOOL	ResultCode	= WriteFile(FileHandle, Contents, ContentsLength, &BytesRead, NULL);

			CloseHandle(FileHandle);

			if (FALSE != ResultCode)
			{
				ReturnCode	= true;
			}
		}
	}

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// Read
/////////////////////////////////////////////////////////////////////////////
BYTE*
	FileWrapper::Read(
	DWORD*	ContentsLength)
{
	BYTE*	InputContents = NULL;

	if (!fileName.empty())
	{
		HANDLE	FileHandle	= CreateFile(fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	ContentsSize	= GetFileSize(FileHandle, NULL) + 2;

			if (0 < ContentsSize )
			{
				InputContents = new BYTE[ContentsSize];

				if (NULL != InputContents)
				{
					DWORD	BytesRead	= 0;

					SecureZeroMemory(InputContents, ContentsSize);

					BOOL	ResultCode	= ReadFile(FileHandle, InputContents, ContentsSize, &BytesRead, NULL) ;

					CloseHandle(FileHandle);

					if (FALSE != ResultCode)
					{
						*ContentsLength	= ContentsSize;
					}
				}
			}
		}
	}

	return InputContents;
}

/////////////////////////////////////////////////////////////////////////////
// ReadText
/////////////////////////////////////////////////////////////////////////////
char*
	FileWrapper::ReadText(void)
{
	char*	InputContents = NULL;

	if (!fileName.empty())
	{
		HANDLE	FileHandle	= CreateFile(fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	ContentsSize	= GetFileSize(FileHandle, NULL) + 1;

			if (0 < ContentsSize )
			{
				InputContents = new char[ContentsSize];

				if (NULL != InputContents)
				{
					DWORD	BytesRead	= 0;

					SecureZeroMemory(InputContents, ContentsSize);

					BOOL result = ReadFile(FileHandle, InputContents, ContentsSize, &BytesRead, NULL) ;

					if (result == FALSE)
					{
					}

					CloseHandle(FileHandle);
				}
			}
		}
	}

	return InputContents;
}

/////////////////////////////////////////////////////////////////////////////
// ReadUnicodeText
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	FileWrapper::ReadUnicodeText(void)
{
	TCHAR*	InputContents = NULL;

	if (!fileName.empty())
	{
		HANDLE	FileHandle	= CreateFile(fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	ContentsSize	= GetFileSize(FileHandle, NULL) / sizeof(TCHAR) + 2;

			if (0 < ContentsSize )
			{
				InputContents = new TCHAR[ContentsSize];

				if (NULL != InputContents)
				{
					DWORD	BytesRead	= 0;

					SecureZeroMemory(InputContents, ContentsSize);

					BOOL result = ReadFile(FileHandle, InputContents, ContentsSize, &BytesRead, NULL) ;

					if (result == FALSE)
					{
					}

					CloseHandle(FileHandle);
				}
			}
		}
	}

	return InputContents;
}
