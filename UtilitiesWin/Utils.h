/////////////////////////////////////////////////////////////////////////////
// Utils.h
//
// Public header file for the Utils DLL
// Utils contains commonly used functions.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "UtilitiesWin.h"

/////////////////////////////////////////////////////////////////////////////
// defines
/////////////////////////////////////////////////////////////////////////////
const ULONG CodePageShiftJis		= 932;		// ANSI/OEM - Japanese, Shift-JIS
const ULONG CodePageJis				= 50220;	// ISO 2022 Japanese with no halfwidth Katakana
const ULONG CodePage50221			= 50221;	// ISO 2022 Japanese with halfwidth Katakana
const ULONG CodePage50222			= 50222;	// ISO 2022 Japanese JIS X 0201-1989
const ULONG CodePageEucOfficial		= 51932;	// EUC - Japanese - doesn't work, at least on XP
const ULONG CodePageEucUndocumented	= 20932;	// EUC - Japanese - undocumented, works on, at least, XP
const ULONG CodePageUsAscii			= 20127;	// US-ASCII (7-bit)
const ULONG CodePageUtf8			= 65001;	// Unicode UTF-8

/////////////////////////////////////////////////////////////////////////////
// APIs
/////////////////////////////////////////////////////////////////////////////

// hide from InstallHelper
#ifndef INSTALLHELPER_EXPORTS

/////////////////////////////////////////////////////////////////////////////
// File related utils
/////////////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetBaseFileName(
	LPCTSTR	fileName);

/////////////////////////////////////////////////////////////////////////////
// GetFileNameTempPath
//
// Given a filename, will return that filename appended to the temp path
/////////////////////////////////////////////////////////////////////////////
DllExport LPCTSTR
GetFileNameTempPath(
	LPCTSTR	fileName);

DllExport TCHAR*
GetInstallPath(void);

DllExport TCHAR*
GetUserDataPath(
	TCHAR*	fileName);

DllExport TCHAR*
GetTempFilePathName(
	LPCTSTR	applicationName);

DllExport LPCTSTR
GetWindowsDllPath(
	TCHAR*	fileName);

/////////////////////////////////////////////////////////////////////////////
// Language related utils
/////////////////////////////////////////////////////////////////////////////
DllExport WORD
GetLanguageId();

DllExport void
SetLanguageId(
	WORD	langId,
	HMODULE resourceModule);

/////////////////////////////////////////////////////////////////////////////
// String related utils
/////////////////////////////////////////////////////////////////////////////
DllExport BYTE*
ConcatStreams(
	BYTE*	firstStream,
	DWORD	firstStreamLength,
	BYTE*	secondStream,
	DWORD	secondStreamLength);
#endif
// allow InstallHelper to use

DllExport TCHAR*
ConcatStrings(
	LPCTSTR	firstString,
	LPCTSTR	secondString);

// hide from InstallHelper
#ifndef INSTALLHELPER_EXPORTS

DllExport char*
ConcatStringsA(
	const char*	firstString,
	const char*	secondString);
DllExport TCHAR* __cdecl
ConcatStringsV(
	LPCTSTR	firstString,
	LPCTSTR	secondString,
	...);
DllExport char* __cdecl
ConcatStringsVA(
	const char*	firstString,
	const char*	secondString,
	...);

/////////////////////////////////////////////////////////////////////////////
// Language related string functions
/////////////////////////////////////////////////////////////////////////////
DllExport char*
GetMultiByteString(
	LPCWSTR szUnicodeString);
DllExport char*
GetMultiByteString(
	ULONG	codePage,
	LPCWSTR szUnicodeString);
DllExport char*
GetMultiByteStringFromUnicodeString(
	LPCWSTR szUnicodeString,
	ULONG nCodePage);

DllExport wchar_t*
GetUnicodeString(
	LPCSTR	multiByteString);
DllExport wchar_t*
GetUnicodeString(
	ULONG	codePage,
	LPCSTR	multiByteString);
DllExport int
GetUnicodeStringFromMultiByteString(
	LPCSTR		szMultiByteString,
	wchar_t*	szUnicodeString,
	int			nUnicodeBufferSize,
	ULONG		nCodePage);

#endif
// allow InstallHelper to use

/////////////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetStringCopy(
	LPCTSTR	sourceString);

// hide from InstallHelper
#ifndef INSTALLHELPER_EXPORTS

DllExport char*
GetStringCopyA(
	const char*	sourceString);

DllExport TCHAR*
GetStringCopyAmount(
	LPCTSTR	sourceString,
	size_t	amount);

DllExport char*
GetStringCopyAmountA(
	const char*	sourceString,
	size_t		amount);

DllExport TCHAR*
StringSprintfInt(
	TCHAR*	originalString,
	int	number);

constexpr DllExport bool IsUtf8BomMark(
	BYTE byteInQuestion) noexcept;

/////////////////////////////////////////////////////////////////////////////
// Test related utils
/////////////////////////////////////////////////////////////////////////////
DllExport void
ShowTestResult(
	TCHAR * testName,
	long testResult);

DllExport void
ShowConditionResult(
	TCHAR * testName,
	long testResult,
	long expectedResult);
DllExport void
ShowConditionResultNot(
	TCHAR * testName,
	long testResult,
	long expectedResult);
DllExport void
ShowConditionResultNot(
	TCHAR*	testName,
	void*	testResult,
	void*	expectedResult);

/////////////////////////////////////////////////////////////////////////////
// Misc functions
/////////////////////////////////////////////////////////////////////////////
DllExport bool
IsEmailValid(
	const TCHAR* address);

DllExport bool
GetLastErrorInfo(
	HMODULE	moduleHandle);
#endif

int GetDigitsFromString(char* str, int index);
