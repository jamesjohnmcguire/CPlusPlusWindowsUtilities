/////////////////////////////////////////////////////////////////////////////
// Utils.cpp
//
// String and other common functions.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#ifndef INSTALLHELPER_EXPORTS
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>
#include <stdlib.h>
#include <shlobj.h>
#include "Utils.h"
#include "Registry.h"

#if defined _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#ifdef INSTALLHELPER_EXPORTS
#include <tchar.h>
#else
/////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////

#ifdef BUILD_JAPAN
static WORD g_LanguageId = LANG_JAPANESE;
#else
static WORD g_LanguageId = LANG_ENGLISH;
#endif

static HMODULE g_ResourceModule	= nullptr;

static ULONG alternativeCodePages[] = {CodePageUtf8, CodePageUsAscii, 0};

/////////////////////////////////////////////////////////////////////////////
// AllowConversionFlags
//
// Some code pages don't support the conversion flags
/////////////////////////////////////////////////////////////////////////////
bool
AllowConversionFlags(
	ULONG codePage)
{
	bool returnCode = true;

	switch (codePage)
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
		returnCode = false;
		break;
	}
	default:
	{
		returnCode = true;
		break;
	}
	}

	return returnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStreams
// Caller is responsible for 'delete'ing the return stream.
/////////////////////////////////////////////////////////////////////////////
BYTE* ConcatStreams(BYTE* firstStream, DWORD firstStreamLength,
	BYTE* secondStream, DWORD secondStreamLength)
{
	BYTE* concatStream	= nullptr;

	// if both null, nothing to do
	if ((nullptr != firstStream) || (nullptr != secondStream))
	{
		try
		{
			// even though we shouldn't assume these are strings, we want to insure
			// the buffer is zero-terminated.
			// there are some situations, where a memory blob is cast as a string.
			const DWORD	concatStreamLength =
				firstStreamLength + secondStreamLength + 2;

			concatStream = new BYTE[concatStreamLength];

			SecureZeroMemory(concatStream, concatStreamLength);
			errno_t errorCode = memcpy_s(concatStream, concatStreamLength,
				firstStream, firstStreamLength);
			if ((errorCode == 0) || (errorCode == EINVAL))
			{
				BYTE* secondPart = concatStream + firstStreamLength;

				errorCode = memcpy_s(secondPart,
					concatStreamLength - firstStreamLength,
					secondStream,
					secondStreamLength);

				if ((errorCode != 0) && (errorCode != EINVAL))
				{
					SetLastError(errorCode);
					throw;
				}
			}
			else
			{
				SetLastError(errorCode);
				throw;
			}
		}
		catch(const TCHAR* caughtException)
		{
			// log
			OutputDebugString(caughtException);
			throw;
		}
	}

	return concatStream;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStrings
// Caller is responsible for 'deleting the string.
/////////////////////////////////////////////////////////////////////////////
TCHAR* ConcatStrings(LPCTSTR firstString, LPCTSTR secondString)
{
	TCHAR*	concatString = nullptr;

	if ((nullptr != firstString) || (nullptr != secondString))
	{
		try
		{
			if (nullptr == firstString)
			{
				concatString = GetStringCopy(secondString);
			}
			else if (nullptr == secondString)
			{
				concatString = GetStringCopy(firstString);
			}
			else
			{
				const UINT	concatStringLength = (UINT)_tcslen(firstString) +
					(UINT)_tcslen(secondString) +
					1;

				concatString = new TCHAR[concatStringLength];

				_tcscpy_s(concatString, concatStringLength, firstString);
				_tcscat_s(concatString, concatStringLength, secondString);
			}
		}
		catch (const TCHAR* caughtException)
		{
			// log
			OutputDebugString(caughtException);
			throw;
		}
	}
	return concatString;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStringsA
//
// Ascii version
//
// Caller is responsible for deleting the string.
/////////////////////////////////////////////////////////////////////////////
char* ConcatStringsA(const char* firstString, const char* secondString)
{
	char*	concatString		= nullptr;

	if ((nullptr != firstString) || (nullptr != secondString))
	{
		try
		{
			if (nullptr == firstString)
			{
				concatString = GetStringCopyA(secondString);
			}
			else if (nullptr == secondString)
			{
				concatString = GetStringCopyA(firstString);
			}
			else
			{
				const UINT	concatStringLength = (UINT)strlen(firstString) +
					(UINT)strlen(secondString) +
					1;

				concatString = new char[concatStringLength];

				strcpy_s(concatString, concatStringLength, firstString);
				strcat_s(concatString, concatStringLength, secondString);
			}
		}
		catch(const TCHAR* caughtException)
		{
			// log
			OutputDebugString(caughtException);
			throw;
		}
	}

	return concatString;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStringsVA
//
// The last parameter passed needs to be nullptr
//
// Caller is responsible for deleting the string.
/////////////////////////////////////////////////////////////////////////////
TCHAR* __cdecl ConcatStringsV(LPCTSTR firstString, LPCTSTR secondString, ...)
{
	TCHAR* newString = nullptr;
	va_list	arguments;

	va_start(arguments, secondString);

	TCHAR* oldString = newString = ConcatStrings(firstString, secondString);

	TCHAR* nextArg = va_arg(arguments, TCHAR*);

	while (nullptr != nextArg)
	{
		newString = ConcatStrings(oldString, nextArg);

		delete oldString;
		oldString = newString;

		nextArg = va_arg(arguments, TCHAR*);
	}

	va_end(arguments);

	return newString;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStringsVA
//
// Ascii version
// The last parameter passed needs to be nullptr
//
// Caller is responsible for deleting the string.
/////////////////////////////////////////////////////////////////////////////
char* __cdecl ConcatStringsVA(const char* firstString,
	const char* secondString, ...)
{
	char*	newString	= nullptr;
	va_list	arguments;

	va_start( arguments, secondString );

	const char*	oldString	= newString = ConcatStringsA(firstString, secondString);

	const char*	nextArg	= va_arg( arguments, const char*);

	while (nullptr != nextArg)
	{
		newString	= ConcatStringsA(oldString, nextArg);

		delete oldString;
		oldString	= newString;

		nextArg	= va_arg( arguments, const char*);
	}

	va_end( arguments );

	return	newString;
}

/////////////////////////////////////////////////////////////////////////////
// GetMultiByteStringFromUnicodeString()
//
// Returns:		int			number of characters written. 0 means error
// Parameters:
//	wchar_t *	szUnicodeString			(IN)	Unicode input string
//	char*		szMultiByteString		(OUT)	Multibyte output string
//	int			nMultiByteBufferSize	(IN)	Multibyte buffer size (chars)
//	ULONG		nCodePage				(IN)	Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a MultiByte string from a Unicode string
// Exceptions:	None.
/////////////////////////////////////////////////////////////////////////////
DllExport char *
	GetMultiByteStringFromUnicodeString(
	LPCWSTR	unicodeString,
	ULONG	codePage)
{
	const BOOL	bUsedDefChar	= FALSE;
	int		nCharsWritten = 0;
	char*	multiByteString = nullptr;
	try
	{
		if (unicodeString)
		{
			// If no code page specified, take default for system
			if (-1 == codePage)
			{
				codePage = GetACP();
			}

			const int multiByteBufferSize = WideCharToMultiByte( codePage,
				0,
				unicodeString,
				-1,
				nullptr,
				0,
				nullptr,
				nullptr);

			multiByteString = new char[multiByteBufferSize];

			// Zero out buffer first
			memset((void*)multiByteString, '\0', multiByteBufferSize);

			DWORD flags	= WC_COMPOSITECHECK | WC_SEPCHARS;

			const bool useFlags	= AllowConversionFlags(codePage);

			if (false == useFlags)
			{
				flags	= 0;
			}

			// If writing to UTF8, flags, default char and boolean flag must be nullptr
			nCharsWritten = WideCharToMultiByte(codePage,
				flags,
				unicodeString,
				-1,
				multiByteString,
				multiByteBufferSize,
				nullptr,
				nullptr);

			// If no chars were written and the buffer is not 0, error!
			if (nCharsWritten == 0 && multiByteBufferSize > 0)
			{
				TCHAR	errorMessage[256];
				_stprintf_s(errorMessage, _T("\r\nError in WideCharToMultiByte: %d\r\n"), GetLastError());
				OutputDebugString(errorMessage);
				multiByteString[0]	= '\0';
			}
		}

		// Now fix nCharsWritten
		if (nCharsWritten > 0)
		{
			nCharsWritten--;
		}
		else
		{
#if EMPTY_MULTIBYTE_STRING_TERMINATOR
			multiByteString[0]	= '\0';
#endif
		}
	}
	catch(...)
	{
		OutputDebugString(_T("Controlled exception in WideCharToMultiByte!\n"));
	}

	return multiByteString;
}

char*
	GetMultiByteString(
	LPCTSTR unicodeString)
{
	// if this is not a Unicode build, just use the string itself.
	// this function is then essentially a NOP, but its stamped all over the place.
#ifndef UNICODE
	return unicodeString;
#else
	char* multiByteString	= nullptr;

	if (nullptr != unicodeString)
	{
		multiByteString = GetMultiByteStringFromUnicodeString(	unicodeString,
			-1);
	}

	return multiByteString;
#endif
}

char*
	GetMultiByteString(
	ULONG	codePage,
	LPCTSTR unicodeString)
{
	// if this is not a Unicode build, just use the string itself.
	// this function is then essentially a NOP, but its stamped all over the place.
#ifndef UNICODE
	return unicodeString;
#else
	char* multiByteString	= nullptr;

	if (nullptr != unicodeString) {
		multiByteString = GetMultiByteStringFromUnicodeString(	unicodeString,
			codePage);
	}

	return multiByteString;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// GetStringCopyAmount
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetStringCopyAmount(
	LPCTSTR	sourceString,
	size_t	amount)
{
	TCHAR* stringCopy = nullptr;

	if (nullptr != sourceString)
	{
		const BOOL badString	= IsBadStringPtr(sourceString,(UINT_PTR)-1);

		if (FALSE == badString)
		{
			const size_t	sourceStringLength = (_tcslen(sourceString) + 1) * sizeof(TCHAR);

			stringCopy = new TCHAR[amount+1];

			if (nullptr != stringCopy)
			{
				_tcsncpy_s(	stringCopy,
					amount+1,
					sourceString,
					amount);
			}
		}
	}

	return stringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetStringCopyAmountA
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
char*
	GetStringCopyAmountA(
	const char*	sourceString,
	size_t		amount)
{
	char* stringCopy = nullptr;

	if (sourceString != nullptr)
	{
		const size_t	sourceStringLength = (strlen(sourceString) + 1);

		stringCopy = new char[amount+1];

		if (nullptr != stringCopy)
		{
			strncpy_s(	stringCopy,
				amount+1,
				sourceString,
				amount);
		}
	}

	return stringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// IsUtf8BomMark
/////////////////////////////////////////////////////////////////////////////
constexpr bool IsUtf8BomMark(BYTE byteInQuestion) noexcept
{
	bool	utf8BomMark	= false;

	if (0xEF == byteInQuestion)
	{
		utf8BomMark	= true;
	}

	return utf8BomMark;
}

/////////////////////////////////////////////////////////////////////////////
// GetBaseFileName
//
// delete returned buffer after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetBaseFileName(
	LPCTSTR	fileName)
{
	TCHAR*	baseFileName	= nullptr;

	if (nullptr != fileName)
	{
		LPCTSTR	found	= _tcsrchr(fileName, _T('\\'));

		if (nullptr != found)
		{
			// get past the slash
			found++;

			baseFileName	= GetStringCopy(found);
		}
		else
		{
			baseFileName	= GetStringCopy(fileName);
		}
	}

	return baseFileName;
}

/////////////////////////////////////////////////////////////////////////////
// GetUserDataFilePath
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetUserDataPath(
	TCHAR*	fileName)
{
	TCHAR*	dataFilePath	= nullptr;
	TCHAR*	userDataPath	= new TCHAR[MAX_PATH];

	const HRESULT	resultCode	= SHGetFolderPath(	nullptr,
		CSIDL_PERSONAL|CSIDL_FLAG_CREATE,
		nullptr,
		SHGFP_TYPE_CURRENT,
		userDataPath);

	if(FAILED(resultCode))
	{
	}
	else
	{
		if (fileName[0] != '\\')
		{
			TCHAR*	tempUserDataPath = ConcatStrings(userDataPath, _T("\\"));
			delete[] userDataPath;
			userDataPath	= tempUserDataPath;
		}

		dataFilePath	= ConcatStrings(userDataPath, fileName);

		if (nullptr != userDataPath)
		{
			delete[] userDataPath;
		}
	}

	return dataFilePath;
}

/////////////////////////////////////////////////////////////////////////////
// GetWindowsDllPath
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
LPCTSTR
	GetWindowsDllPath(
	TCHAR*	fileName)
{
	TCHAR*	dataFilePath	= nullptr;
	TCHAR*	userDataPath	= new TCHAR[MAX_PATH];

	const HRESULT	resultCode	= SHGetFolderPath(	nullptr,
		CSIDL_WINDOWS|CSIDL_FLAG_CREATE,
		nullptr,
		SHGFP_TYPE_CURRENT,
		userDataPath);

	if(FAILED(resultCode))
	{
	}
	else
	{
		if (fileName[0] != '\\')
		{
			TCHAR*	tempUserDataPath = ConcatStrings(userDataPath, _T("\\"));
			delete[] userDataPath;
			userDataPath	= tempUserDataPath;
		}

		dataFilePath	= ConcatStrings(userDataPath, fileName);

		if (nullptr != userDataPath)
		{
			delete[] userDataPath;
		}
	}

	return dataFilePath;
}

/////////////////////////////////////////////////////////////////////////////
// GetFileNameTempPath
/////////////////////////////////////////////////////////////////////////////
LPCTSTR	GetFileNameTempPath(
	LPCTSTR	fileName)
{
	TCHAR*	fileNameTempPath		= nullptr;
	const TCHAR*	friendlyFileNameBase	= GetBaseFileName(fileName);

	if (nullptr != friendlyFileNameBase)
	{
		TCHAR	tempPath[MAX_PATH]	= {0};

		GetTempPath(MAX_PATH, tempPath);

		fileNameTempPath = ConcatStrings(tempPath, friendlyFileNameBase);
	}

	return fileNameTempPath;
}

char*
GetStringCopyA(
	const char*	sourceString)
{
	char* stringCopy = nullptr;

	if (sourceString != nullptr)
	{
		const size_t	sourceStringLength = strlen(sourceString) + 1;

		stringCopy = new char[sourceStringLength];

		if (nullptr != stringCopy)
		{
			strcpy_s(stringCopy, sourceStringLength, sourceString);
		}
	}

	return stringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetUnicodeString
/////////////////////////////////////////////////////////////////////////////
wchar_t*
GetUnicodeString(
	LPCSTR	multiByteString)
{
	wchar_t*	unicodeString = nullptr;

	if (nullptr != multiByteString)
	{
		const size_t	stringLength = strlen(multiByteString) + 1;
		unicodeString = new wchar_t[stringLength];
		GetUnicodeStringFromMultiByteString(multiByteString, unicodeString, (int)stringLength, -1);
	}

	return unicodeString;
}

/////////////////////////////////////////////////////////////////////////////
// GetUnicodeString
/////////////////////////////////////////////////////////////////////////////
wchar_t*
GetUnicodeString(
	ULONG	codePage,
	LPCSTR	multiByteString)
{
	wchar_t*	unicodeString = nullptr;

	if (nullptr != multiByteString)
	{
		const size_t	stringLength = strlen(multiByteString) + 1;
		unicodeString = new wchar_t[stringLength];
		GetUnicodeStringFromMultiByteString(multiByteString, unicodeString, (int)stringLength, codePage);
	}

	return unicodeString;
}

/////////////////////////////////////////////////////////////////////////////
// GetUnicodeStringFromMultiByteString
//
// Returns:		int			num. of chars written (0 means error)
// Parameters:
//	char *		szMultiByteString	(IN)		Multi-byte input string
//	wchar_t*	szUnicodeString		(OUT)		Unicode outputstring
//	int			nUnicodeBufferSize	(IN)		Size of Unicode output buffer in chars(IN)
//	ULONG		nCodePage			(IN)		Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
/////////////////////////////////////////////////////////////////////////////
int GetUnicodeStringFromMultiByteString(
	LPCSTR		szMultiByteString,
	wchar_t*	szUnicodeString,
	int			nUnicodeBufferSize,
	ULONG		nCodePage)
{
	const bool	bOK = true;
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		DWORD flags = MB_PRECOMPOSED;

		const bool useFlags = AllowConversionFlags(nCodePage);

		if (false == useFlags)
		{
			flags = 0;
		}

		try
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			for (int codePageIndex = 0;;) {
				nCharsWritten = MultiByteToWideChar(nCodePage,
					flags,
					szMultiByteString,
					-1,
					szUnicodeString,
					nUnicodeBufferSize + 1);

				if (nCharsWritten > 0 || alternativeCodePages[codePageIndex] == 0)
					break;

				//If there a problem converting to a wide char with the given code page.
				nCodePage = alternativeCodePages[codePageIndex++];
			}
		}
		catch (const std::exception& ex)
		{
			std::string errorMsg = ex.what();
			std::wstring wErrorMsg(errorMsg.begin(), errorMsg.end());
			OutputDebugString(wErrorMsg.c_str());
		}
		catch (...)
		{
			OutputDebugString(L"Unknown exception occurred\n");
		}
	}

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}
	else
	{
		GetLastErrorInfo(nullptr);
	}

	return nCharsWritten;
}

char* GetUtfMultiByteString(LPCTSTR unicodeString)
{
	// if this is not a Unicode build, just use the string itself.
	// this function is then essentially a NOP, but its stamped all over the place.
#ifndef UNICODE
	return unicodeString;
#else
	char* multiByteString = nullptr;

	if (nullptr != unicodeString)
	{
		multiByteString = GetMultiByteStringFromUnicodeString(unicodeString,
			CP_UTF8);
	}

	return multiByteString;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessageString
/////////////////////////////////////////////////////////////////////////////
int
	ShowMessageString(
	LPCTSTR title,
	LPCTSTR message,
	UINT uType)
{
	const int returnCode	= MessageBox(GetActiveWindow(), message, title, uType);

	return returnCode;
}

TCHAR*	StringSprintfInt(
	TCHAR*	originalString,
	int	number)
{
	TCHAR* stringCopy = nullptr;

	if (originalString != nullptr)
	{
		const size_t	sourceStringLength = (_tcslen(originalString) + 1) * sizeof(TCHAR) + 16;

		stringCopy = new TCHAR[sourceStringLength];

		if (nullptr != stringCopy)
		{
			_stprintf_s(stringCopy, sourceStringLength, originalString, number);
		}
	}

	return stringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetTempFilePathName
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetTempFilePathName(
	LPCTSTR	applicationName)
{
	TCHAR*	tempFileName	= new TCHAR[MAX_PATH];

	if (nullptr != tempFileName)
	{
		TCHAR	tempPath[MAX_PATH] = {0};

		GetTempPath(MAX_PATH, tempPath);

		GetTempFileName(tempPath, applicationName, 0, tempFileName);
	}

	return tempFileName;
}

/////////////////////////////////////////////////////////////////////////////
// GetLastErrorInfo
/////////////////////////////////////////////////////////////////////////////
bool
	GetLastErrorInfo(
	HMODULE	moduleHandle)
{
	const bool	bRet		= false;

	LPVOID	pMsgBuffer = nullptr;
	const DWORD	dwErr		= GetLastError();
	DWORD	dwRet;
	DWORD	formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
#if FORMAT_MESSAGE_BUFFER_CAST
	LPTSTR buffer = reinterpret_cast<LPTSTR>(&pMsgBuffer);
#endif
	LPTSTR buffer = nullptr;

	if (nullptr != moduleHandle)
	{
		formatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	dwRet	= FormatMessage(formatFlags,
		moduleHandle,
		dwErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&buffer,
		0,
		nullptr);

	if (nullptr != buffer)
	{
		OutputDebugString(buffer);
#if MFC_TRACE_LOGGING
		TRACE(_T("Error: %s\r\n"), buffer);
#endif
		_tprintf(_T("\tError: %s"), buffer);
		LocalFree(pMsgBuffer);
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// ShowTestResult
/////////////////////////////////////////////////////////////////////////////
void
	ShowTestResult(
	TCHAR * testName,
	long testResult)
{
	if (testResult > 0)
	{
		_tprintf(_T("**%s: succeeded\r\n"), testName);
	}
	else
	{
		_tprintf(_T("--%s: failed\r\n"), testName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowConditionResult
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResult(
	TCHAR * testName,
	long testResult,
	long expectedResult)
{
	if (testResult == expectedResult)
	{
		_tprintf(_T("%s: succeeded\r\n"), testName);
	}
	else
	{
		_tprintf(_T("%s: failed\r\n"), testName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowConditionResultNot
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResultNot(
	TCHAR * testName,
	long testResult,
	long expectedResult)
{
	if (testResult != expectedResult)
	{
		_tprintf(_T("%s: succeeded\r\n"), testName);
	}
	else
	{
		_tprintf(_T("%s: failed\r\n"), testName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowTestResultNot
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResultNot(
	TCHAR*	testName,
	void*	testResult,
	void*	expectedResult)
{
	if (testResult != expectedResult)
	{
		_tprintf(_T("%s: succeeded\r\n"), testName);
	}
	else
	{
		_tprintf(_T("%s: failed\r\n"), testName);
	}
}

#endif

/////////////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR* GetStringCopy(LPCTSTR sourceString)
{
	TCHAR* stringCopy = nullptr;

	if (sourceString != nullptr)
	{
		const size_t	sourceStringLength = (_tcslen(sourceString) + 1) * sizeof(TCHAR);

		stringCopy = new TCHAR[sourceStringLength];

		if (nullptr != stringCopy)
		{
			_tcscpy_s(stringCopy, sourceStringLength, sourceString);
		}
	}

	return stringCopy;
}

bool IsEmailValid(const char* address)
{
	int				count = 0;
	const char* c;
	const char* domain;
	const char*	rfc822Specials = "()<>@,;:\\\"[]";

	// first we validate the length of the name and domain (name@domain)
	// name must be no longer than 64 chars and domain must be no longer than 255 chars
	const char* t = strchr(address, '@');

	if (!t)
		return false;
	size_t len = (size_t)(t - address);
	if (len > 64)
		return false;
	len = strlen(t);
	if (len > 256)
		return false;

	// then we validate the name portion (name@domain)
	for (c = address; *c; c++)
	{
		if (*c == '\"' &&
			(c == address || *(c - 1) == '.' || *(c - 1) == '\"'))
		{
			while (*++c)
			{
				if (*c == '\"')
					break;
				if (*c == '\\' && (*++c == ' '))
					continue;
				if (*c <= ' ' || *c >= 127)
					return false;
			}
			if (!*c++)
				return false;
			if (*c == '@')
				break;
			if (*c != '.')
				return false;
			continue;
		}
		if (*c == '@')
			break;
		if (*c <= ' ' || *c >= 127)
			return false;
		if (strchr(rfc822Specials, *c))
			return false;
	}
	if (c == address || *(c - 1) == '.')
		return false;

	// then we validate the domain portion (name@domain)
	if (!*(domain = ++c))
		return false;
	do
	{
		if (*c == '.')
		{
			if (c == domain || *(c - 1) == '.')
				return false;
			count++;
		}
		if (*c <= ' ' || *c >= 127)
			return false;
		if (strchr(rfc822Specials, *c))
			return false;
	}
	while (*++c);

	return true;
}

int GetDigitsFromString(char *str, int index)
{
	return (str[index] - '0') * 10 + (str[index + 1] - '0');
}
