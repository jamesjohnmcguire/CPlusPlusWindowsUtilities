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
		catch(const wchar_t* caughtException)
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
wchar_t* ConcatStrings(const wchar_t* firstString, const wchar_t* secondString)
{
	wchar_t*	concatString = nullptr;

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
				const UINT	concatStringLength = (UINT)wcslen(firstString) +
					(UINT)wcslen(secondString) +
					1;

				concatString = new wchar_t[concatStringLength];

				wcscpy_s(concatString, concatStringLength, firstString);
				wcscat_s(concatString, concatStringLength, secondString);
			}
		}
		catch (const wchar_t* caughtException)
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
		catch(const wchar_t* caughtException)
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
wchar_t* __cdecl ConcatStringsV(const wchar_t* firstString, const wchar_t* secondString, ...)
{
	wchar_t* newString = nullptr;
	va_list	arguments;

	va_start(arguments, secondString);

	wchar_t* oldString = newString = ConcatStrings(firstString, secondString);

	wchar_t* nextArg = va_arg(arguments, wchar_t*);

	while (nullptr != nextArg)
	{
		newString = ConcatStrings(oldString, nextArg);

		delete oldString;
		oldString = newString;

		nextArg = va_arg(arguments, wchar_t*);
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
	const BOOL	usedDefaultCharacter	= FALSE;
	int		charactersWritten = 0;
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
			charactersWritten = WideCharToMultiByte(codePage,
				flags,
				unicodeString,
				-1,
				multiByteString,
				multiByteBufferSize,
				nullptr,
				nullptr);

			// If no chars were written and the buffer is not 0, error!
			if (charactersWritten == 0 && multiByteBufferSize > 0)
			{
				wchar_t	errorMessage[256];
				swprintf_s(errorMessage, L"\r\nError in WideCharToMultiByte: %d\r\n", GetLastError());
				OutputDebugString(errorMessage);
				multiByteString[0]	= '\0';
			}
		}

		// Now fix nCharsWritten
		if (charactersWritten > 0)
		{
			charactersWritten--;
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
		OutputDebugString(L"Controlled exception in WideCharToMultiByte!\n");
	}

	return multiByteString;
}

char*
	GetMultiByteString(
	const wchar_t* unicodeString)
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
	const wchar_t* unicodeString)
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
wchar_t*
	GetStringCopyAmount(
	const wchar_t*	sourceString,
	size_t	amount)
{
	wchar_t* stringCopy = nullptr;

	if (nullptr != sourceString)
	{
		const BOOL badString	= IsBadStringPtr(sourceString,(UINT_PTR)-1);

		if (FALSE == badString)
		{
			const size_t	sourceStringLength = (wcslen(sourceString) + 1) * sizeof(wchar_t);

			stringCopy = new wchar_t[amount+1];

			if (nullptr != stringCopy)
			{
				wcsncpy_s(	stringCopy,
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
wchar_t*
	GetBaseFileName(
	const wchar_t*	fileName)
{
	wchar_t*	baseFileName	= nullptr;

	if (nullptr != fileName)
	{
		const wchar_t*	found	= wcsrchr(fileName, L'\\');

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
wchar_t*
	GetUserDataPath(
	wchar_t*	fileName)
{
	wchar_t*	dataFilePath	= nullptr;
	wchar_t*	userDataPath	= new wchar_t[MAX_PATH];

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
			wchar_t*	tempUserDataPath = ConcatStrings(userDataPath, L"\\");
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
const wchar_t*
	GetWindowsDllPath(
	wchar_t*	fileName)
{
	wchar_t*	dataFilePath	= nullptr;
	wchar_t*	userDataPath	= new wchar_t[MAX_PATH];

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
			wchar_t*	tempUserDataPath = ConcatStrings(userDataPath, L"\\");
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
const wchar_t*	GetFileNameTempPath(
	const wchar_t*	fileName)
{
	wchar_t*	fileNameTempPath		= nullptr;
	const wchar_t*	friendlyFileNameBase	= GetBaseFileName(fileName);

	if (nullptr != friendlyFileNameBase)
	{
		wchar_t	tempPath[MAX_PATH]	= {0};

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
	LPCSTR		multiByteString,
	wchar_t*	unicodeString,
	int			unicodeBufferSize,
	ULONG		codePage)
{
	const bool	succeeded = true;
	int		charactersWritten = 0;

	if (unicodeString && multiByteString)
	{
		// If no code page specified, take default for system
		if (codePage == -1)
		{
			codePage = GetACP();
		}

		DWORD flags = MB_PRECOMPOSED;

		const bool useFlags = AllowConversionFlags(codePage);

		if (false == useFlags)
		{
			flags = 0;
		}

		try
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)unicodeString, '\0', sizeof(wchar_t) *
				unicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			for (int codePageIndex = 0;;) {
				charactersWritten = MultiByteToWideChar(codePage,
					flags,
					multiByteString,
					-1,
					unicodeString,
					unicodeBufferSize + 1);

				if (charactersWritten > 0 || alternativeCodePages[codePageIndex] == 0)
					break;

				//If there a problem converting to a wide char with the given code page.
				codePage = alternativeCodePages[codePageIndex++];
			}
		}
		catch (const std::exception& ex)
		{
			std::string errorMsg = ex.what();
			std::wstring wideErrorMessage(errorMsg.begin(), errorMsg.end());
			OutputDebugString(wideErrorMessage.c_str());
		}
		catch (...)
		{
			OutputDebugString(L"Unknown exception occurred\n");
		}
	}

	// Now fix nCharsWritten
	if (charactersWritten > 0)
	{
		charactersWritten--;
	}
	else
	{
		GetLastErrorInfo(nullptr);
	}

	return charactersWritten;
}

char* GetUtfMultiByteString(const wchar_t* unicodeString)
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
	const wchar_t* title,
	const wchar_t* message,
	UINT uType)
{
	const int returnCode	= MessageBox(GetActiveWindow(), message, title, uType);

	return returnCode;
}

wchar_t*	StringSprintfInt(
	wchar_t*	originalString,
	int	number)
{
	wchar_t* stringCopy = nullptr;

	if (originalString != nullptr)
	{
		const size_t	sourceStringLength = (wcslen(originalString) + 1) * sizeof(wchar_t) + 16;

		stringCopy = new wchar_t[sourceStringLength];

		if (nullptr != stringCopy)
		{
			swprintf_s(stringCopy, sourceStringLength, originalString, number);
		}
	}

	return stringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetTempFilePathName
/////////////////////////////////////////////////////////////////////////////
wchar_t*
	GetTempFilePathName(
	const wchar_t*	applicationName)
{
	wchar_t*	tempFileName	= new wchar_t[MAX_PATH];

	if (nullptr != tempFileName)
	{
		wchar_t	tempPath[MAX_PATH] = {0};

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
	const bool	result		= false;

	LPVOID	messageBuffer = nullptr;
	const DWORD	errorCode		= GetLastError();
	DWORD	formattedLength;
	DWORD	formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
#if FORMAT_MESSAGE_BUFFER_CAST
	wchar_t* buffer = reinterpret_cast<wchar_t*>(&messageBuffer);
#endif
	wchar_t* buffer = nullptr;

	if (nullptr != moduleHandle)
	{
		formatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	formattedLength	= FormatMessage(formatFlags,
		moduleHandle,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&buffer,
		0,
		nullptr);

	if (nullptr != buffer)
	{
		OutputDebugString(buffer);
#if MFC_TRACE_LOGGING
		TRACE(L"Error: %s\r\n", buffer);
#endif
		wprintf(L"\tError: %s", buffer);
		LocalFree(messageBuffer);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// ShowTestResult
/////////////////////////////////////////////////////////////////////////////
void
	ShowTestResult(
	wchar_t * testName,
	long testResult)
{
	if (testResult > 0)
	{
		wprintf(L"**%s: succeeded\r\n", testName);
	}
	else
	{
		wprintf(L"--%s: failed\r\n", testName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowConditionResult
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResult(
	wchar_t * testName,
	long testResult,
	long expectedResult)
{
	if (testResult == expectedResult)
	{
		wprintf(L"%s: succeeded\r\n", testName);
	}
	else
	{
		wprintf(L"%s: failed\r\n", testName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowConditionResultNot
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResultNot(
	wchar_t * testName,
	long testResult,
	long expectedResult)
{
	if (testResult != expectedResult)
	{
		wprintf(L"%s: succeeded\r\n", testName);
	}
	else
	{
		wprintf(L"%s: failed\r\n", testName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowTestResultNot
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResultNot(
	wchar_t*	testName,
	void*	testResult,
	void*	expectedResult)
{
	if (testResult != expectedResult)
	{
		wprintf(L"%s: succeeded\r\n", testName);
	}
	else
	{
		wprintf(L"%s: failed\r\n", testName);
	}
}

#endif

/////////////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
wchar_t* GetStringCopy(const wchar_t* sourceString)
{
	wchar_t* stringCopy = nullptr;

	if (sourceString != nullptr)
	{
		const size_t	sourceStringLength = (wcslen(sourceString) + 1) * sizeof(wchar_t);

		stringCopy = new wchar_t[sourceStringLength];

		if (nullptr != stringCopy)
		{
			wcscpy_s(stringCopy, sourceStringLength, sourceString);
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
