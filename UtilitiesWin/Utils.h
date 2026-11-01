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
DllExport wchar_t*
GetBaseFileName(
	const wchar_t*	fileName);

/////////////////////////////////////////////////////////////////////////////
// GetFileNameTempPath
//
// Given a filename, will return that filename appended to the temp path
/////////////////////////////////////////////////////////////////////////////
DllExport const wchar_t*
GetFileNameTempPath(
	const wchar_t*	fileName);

DllExport wchar_t*
GetInstallPath(void);

DllExport wchar_t*
GetUserDataPath(
	wchar_t*	fileName);

DllExport wchar_t*
GetTempFilePathName(
	const wchar_t*	applicationName);

DllExport const wchar_t*
GetWindowsDllPath(
	wchar_t*	fileName);

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

DllExport wchar_t*
ConcatStrings(
	const wchar_t*	firstString,
	const wchar_t*	secondString);

// hide from InstallHelper
#ifndef INSTALLHELPER_EXPORTS

DllExport char*
ConcatStringsA(
	const char*	firstString,
	const char*	secondString);
DllExport wchar_t* __cdecl
ConcatStringsV(
	const wchar_t*	firstString,
	const wchar_t*	secondString,
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
	LPCWSTR unicodeString);
DllExport char*
GetMultiByteString(
	ULONG	codePage,
	LPCWSTR unicodeString);
DllExport char*
GetMultiByteStringFromUnicodeString(
	LPCWSTR unicodeString,
	ULONG codePage);

DllExport wchar_t*
GetUnicodeString(
	LPCSTR	multiByteString);
DllExport wchar_t*
GetUnicodeString(
	ULONG	codePage,
	LPCSTR	multiByteString);
DllExport int
GetUnicodeStringFromMultiByteString(
	LPCSTR		multiByteString,
	wchar_t*	unicodeString,
	int			unicodeBufferSize,
	ULONG		codePage);

#endif
// allow InstallHelper to use

/////////////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
DllExport wchar_t*
GetStringCopy(
	const wchar_t*	sourceString);

// hide from InstallHelper
#ifndef INSTALLHELPER_EXPORTS

DllExport char*
GetStringCopyA(
	const char*	sourceString);

DllExport wchar_t*
GetStringCopyAmount(
	const wchar_t*	sourceString,
	size_t	amount);

DllExport char*
GetStringCopyAmountA(
	const char*	sourceString,
	size_t		amount);

DllExport wchar_t*
StringSprintfInt(
	wchar_t*	originalString,
	int	number);

constexpr DllExport bool IsUtf8BomMark(
	BYTE byteInQuestion) noexcept;

/////////////////////////////////////////////////////////////////////////////
// Misc functions
/////////////////////////////////////////////////////////////////////////////
DllExport bool
IsEmailValid(
	const wchar_t* address);

DllExport bool
GetLastErrorInfo(
	HMODULE	moduleHandle);
#endif

int GetDigitsFromString(char* str, int index);
