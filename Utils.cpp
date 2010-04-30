///////////////////////////////////////////////////////////////////////
// Utils.cpp
//
// Utils contains commonly used functions.
//
// Created: 2008-07-09
//
// Copyright ©  2008-2010 by James John McGuire
// All rights reserved.
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
//#include <shlwapi.h>
#include <shlobj.h>
#include "Utils.h"
#include "Registry.h"
//#include "version.h"
//#include "Common.h"
#include "FileWrapper.h"

///////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////
#ifdef BUILD_JAPAN
static const WORD g_LanguageId = LANG_JAPANESE;
#else
static const WORD g_LanguageId = LANG_ENGLISH;
#endif

///////////////////////////////////////////////////////////////////////
// globals
///////////////////////////////////////////////////////////////////////
UINT g_ZenDiagnostics;

///////////////////////////////////////////////////////////////////////
// Support function prototypes
///////////////////////////////////////////////////////////////////////
bool
AllowConversionFlags(
	ULONG CodePage);

///////////////////////////////////////////////////////////////////////
// functions
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// GetMultiByteStringFromUnicodeString()
//
// Returns:		int			number of characters written. 0 means error
// Parameters:
//	wchar_t *	szUnicodeString			(IN)	Unicode input string
//	char*		szMultiByteString		(OUT)	Multibyte output string
//	int			nMultiByteBufferSize	(IN)	Multibyte buffer size (chars)
//	UINT		nCodePage				(IN)	Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a MultiByte string from a Unicode string
// Exceptions:	None.
///////////////////////////////////////////////////////////////////////
DllExport char *
GetMultiByteStringFromUnicodeString(
	LPCWSTR	UnicodeString,
	ULONG	CodePage)
{
	BOOL	bUsedDefChar	= FALSE;
	int		nCharsWritten = 0;
	char*	MultiByteString = NULL;
	try
	{
		if (UnicodeString)
		{
			// If no code page specified, take default for system
			if (-1 == CodePage)
			{
				CodePage = GetACP();
			}

			int MultiByteBufferSize = WideCharToMultiByte( CodePage,
															0,
															UnicodeString,
															-1,
															NULL,
															0,
															NULL,
															NULL);

			MultiByteString = new char[MultiByteBufferSize];

			// Zero out buffer first
			memset((void*)MultiByteString, '\0', MultiByteBufferSize);

			DWORD Flags	= WC_COMPOSITECHECK | WC_SEPCHARS;

			bool UseFlags	= AllowConversionFlags(CodePage);

			if (false == UseFlags)
			{
				Flags	= 0;
			}

			// If writing to UTF8, flags, default char and boolean flag must be NULL
			nCharsWritten = WideCharToMultiByte(CodePage,
												Flags,
												UnicodeString,
												-1,
												MultiByteString,
												MultiByteBufferSize,
												NULL,
												NULL);

			// If no chars were written and the buffer is not 0, error!
			if (nCharsWritten == 0 && MultiByteBufferSize > 0)
			{
				TCHAR	ErrorMessage[256];
				_stprintf_s(ErrorMessage, _T("\r\nError in WideCharToMultiByte: %d\r\n"), GetLastError());
				ATLTRACE(ErrorMessage);
				MultiByteString[0]	= '\0';
			}
		}

		// Now fix nCharsWritten 
		if (nCharsWritten > 0)
		{
			nCharsWritten--;
		}
		else
		{
			//MultiByteString[0]	= '\0';
		}
	}
	catch(...) 
	{
		ATLTRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
	}
	
	return MultiByteString;
}

///////////////////////////////////////////////////////////////////////
// ConcatStreams
// Caller is responsible for 'delete'ing the return stream.
///////////////////////////////////////////////////////////////////////
DllExport BYTE*
ConcatStreams(
	BYTE*	FirstStream,
	DWORD	FirstStreamLength,
	BYTE*	SecondStream,
	DWORD	SecondStreamLength)
{
	BYTE*	ConcatStream	= NULL;

	// if both null, nothing to do
	if ((NULL != FirstStream) || (NULL != SecondStream))
	{
		try
		{
			DWORD	ConcatStreamLength = FirstStreamLength + SecondStreamLength;

			ConcatStream = new BYTE[ConcatStreamLength];

			errno_t ErrorCode = memcpy_s(ConcatStream, ConcatStreamLength, FirstStream, FirstStreamLength);
			if ((ErrorCode == 0) || (ErrorCode == EINVAL))
			{
				BYTE* SecondPart = ConcatStream + FirstStreamLength;

				ErrorCode = memcpy_s(SecondPart,
									ConcatStreamLength-FirstStreamLength,
									SecondStream,
									SecondStreamLength);

				if ((ErrorCode != 0) && (ErrorCode != EINVAL))
				{
					SetLastError(ErrorCode);
					throw;
				}
			}
			else
			{
				SetLastError(ErrorCode);
				throw;
			}
		}
		catch(TCHAR* Exception)
		{
			// log
			OutputDebugString(Exception);
			throw;
		}
	}
	return ConcatStream;
}

///////////////////////////////////////////////////////////////////////
// ConcatStrings
// Caller is responsible for 'delete'ing the string.
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
ConcatStrings(
	LPCTSTR	FirstString,
	LPCTSTR	SecondString)
{
	TCHAR*	ConcatString		= NULL;
	try
	{
		UINT	ConcatStringLength = (UINT)_tcslen(FirstString) +
											(UINT)_tcslen(SecondString) +
											1;

		ConcatString = new TCHAR[ConcatStringLength];

		_tcscpy_s(ConcatString, ConcatStringLength, FirstString);
		_tcscat_s(ConcatString, ConcatStringLength, SecondString);
	}
	catch(TCHAR* Exception)
	{
		// log
		OutputDebugString(Exception);
		throw;
	}

	return ConcatString;
}

///////////////////////////////////////////////////////////////////////
// ConcatStringsA
//
// Ascii version
//
// Caller is responsible for 'delete'ing the string.
///////////////////////////////////////////////////////////////////////
DllExport char*
ConcatStringsA(
	char*	FirstString,
	char*	SecondString)
{
	char*	ConcatString		= NULL;
	try
	{
		UINT	ConcatStringLength = (UINT)strlen(FirstString) +
											(UINT)strlen(SecondString) +
											1;

		ConcatString = new char[ConcatStringLength];

		strcpy_s(ConcatString, ConcatStringLength, FirstString);
		strcat_s(ConcatString, ConcatStringLength, SecondString);
	}
	catch(TCHAR* Exception)
	{
		// log
		OutputDebugString(Exception);
		throw;
	}

	return ConcatString;
}

///////////////////////////////////////////////////////////////////////
// ConcatStringsV
///////////////////////////////////////////////////////////////////////
TCHAR* __cdecl
ConcatStringsV(
	LPCTSTR	FirstString,
	LPCTSTR	SecondString,
	...)
{
	TCHAR*	NewString	= NULL;
	va_list	Arguments;

	va_start( Arguments, SecondString );

	TCHAR*	OldString	= NewString = ConcatStrings(FirstString, SecondString);

	TCHAR*	NextArg	= va_arg( Arguments, TCHAR*);

	while (NULL != NextArg)
	{
		NewString	= ConcatStrings(OldString, NextArg);

		delete OldString;
		OldString	= NewString;

		NextArg	= va_arg( Arguments, TCHAR*);
	}

	va_end( Arguments );

	return	NewString;
}

///////////////////////////////////////////////////////////////////////
// GetString
//
// Gets an localized string from the resource table
//
// delete after use.
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetString(
	HMODULE	Module,
	UINT	ResourceId)
{
	CString ResourceCstring;
	TCHAR*	ResourceString = NULL;

	BOOL ResultCode = ResourceCstring.LoadString(Module, ResourceId, g_LanguageId);
	ResultCode = ResourceCstring.LoadString(Module, ResourceId);

	if (TRUE == ResultCode)
	{
		int ResourceSize = (ResourceCstring.GetLength() + 1) * sizeof(TCHAR);

		ResourceString = new TCHAR[ResourceSize];

		if (NULL != ResourceString)
		{
			_tcscpy_s(ResourceString, ResourceSize, ResourceCstring);
		}
	}

	return ResourceString;
}

///////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetStringCopy(
	TCHAR*	SourceString)
{
	size_t	SourceStringLength = (_tcslen(SourceString) + 1) * sizeof(TCHAR);

	TCHAR* StringCopy =  new TCHAR[SourceStringLength];

	if (NULL != StringCopy)
	{
		_tcscpy_s(StringCopy, SourceStringLength, SourceString);
	}

	return StringCopy;
}

///////////////////////////////////////////////////////////////////////
// AllowConversionFlags
//
// Some code pages don't support the converion flags
///////////////////////////////////////////////////////////////////////
bool
AllowConversionFlags(
	int CodePage)
{
	bool ReturnCode = true;

	switch(CodePage)
	{
		case 50220:
		case 50221:
		case 50222:
		case 50225:
		case 50227:
		case 50229:
		case 52936:
		case 54936:
		case 57002:
		case 57003:
		case 57004:
		case 57005:
		case 57006:
		case 57007:
		case 57008:
		case 57009:
		case 57010:
		case 57011:
		case 65000:
		case 65001:
		case 42:
		{
			ReturnCode = false;
			break;
		}
		default:
		{
			ReturnCode = true;
			break;
		}
	}

	return ReturnCode;
}

///////////////////////////////////////////////////////////////////////
// GetUnicodeStringFromMultiByteString
//
// Returns:		int			num. of chars written (0 means error)
// Parameters:
//	char *		szMultiByteString	(IN)		Multi-byte input string
//	wchar_t*	szUnicodeString		(OUT)		Unicode outputstring
//	int			nUnicodeBufferSize	(IN)		Size of Unicode output buffer in chars(IN)
//	UINT		nCodePage			(IN)		Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
///////////////////////////////////////////////////////////////////////
int
GetUnicodeStringFromMultiByteString(
	LPCSTR		szMultiByteString,
	wchar_t*	szUnicodeString,
	int			nUnicodeBufferSize,
	int			nCodePage)
{
	bool	bOK = true;
	int		nCharsWritten = 0;
		
	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		DWORD Flags	= MB_PRECOMPOSED;

		bool UseFlags	= AllowConversionFlags(nCodePage);

		if (false == UseFlags)
		{
			Flags = 0;
		}

		//try 
		//{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			nCharsWritten = MultiByteToWideChar((UINT)nCodePage,
												Flags,
												szMultiByteString,
												-1,
												szUnicodeString,
												nUnicodeBufferSize+1);
		//}
		//catch(CException *ex)
		//{
		//	CString cError;
		//	LPTSTR pszError = cError.GetBuffer();
		//	_tprintf(_T("Exception: %s\r\n"), ex->GetErrorMessage(pszError, 3));
		//	cError.ReleaseBuffer();
		//	TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		//}
	}

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}
	
	ASSERT(nCharsWritten > 0);
	return nCharsWritten;
}

///////////////////////////////////////////////////////////////////////
// GetUnicodeString
///////////////////////////////////////////////////////////////////////
wchar_t*
GetUnicodeString(
	LPCSTR	MultiByteString)
{
	wchar_t*	UnicodeString	= NULL;

	if (NULL != MultiByteString)
	{
		size_t	StringLength = strlen(MultiByteString) + 1;
		UnicodeString = new wchar_t[StringLength];
		GetUnicodeStringFromMultiByteString(MultiByteString, UnicodeString, (int)StringLength, -1);
	}

	return UnicodeString;
}

///////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
///////////////////////////////////////////////////////////////////////
TCHAR*
GetStringCopy(
	LPCTSTR	SourceString)
{
	TCHAR* StringCopy = NULL;

	if (NULL != SourceString)
	{
		BOOL BadString	= IsBadStringPtr(SourceString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = (_tcslen(SourceString) + 1) * sizeof(TCHAR);

			StringCopy = new TCHAR[SourceStringLength];

			if (NULL != StringCopy)
			{
				_tcscpy_s(StringCopy, SourceStringLength, SourceString);
			}
		}
	}

	return StringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// FindStrings
/////////////////////////////////////////////////////////////////////////////
TCHAR**
FindStrings(
	TCHAR*	FileName,
	TCHAR*	StringToFind,
	UINT*	LineNumbers)
{
	TCHAR**	FoundStrings	= NULL;
	DWORD	ContentsSize	= 0;

	FileWrapper	FileObject(FileName);
	TCHAR* FileContents = (TCHAR*)FileObject.Read(&ContentsSize);

	TCHAR*	FoundString	= _tcsstr(FileContents, StringToFind);

	if (NULL != FoundString)
	{

	}

	return FoundStrings;
}

///////////////////////////////////////////////////////////////////////
// AllowConversionFlags
//
// Some code pages don't support the converion flags
///////////////////////////////////////////////////////////////////////
bool
AllowConversionFlags(
	ULONG CodePage)
{
	bool ReturnCode = true;

	switch(CodePage)
	{
		case 50220:
		case 50221:
		case 50222:
		case 50225:
		case 50227:
		case 50229:
		case 52936:
		case 54936:
		case 57002:
		case 57003:
		case 57004:
		case 57005:
		case 57006:
		case 57007:
		case 57008:
		case 57009:
		case 57010:
		case 57011:
		case 65000:
		case 65001:
		case 42:
		{
			ReturnCode = false;
			break;
		}
		default:
		{
			ReturnCode = true;
			break;
		}
	}

	return ReturnCode;
}
