///////////////////////////////////////////////////////////////////////
// Utils.h
//
// Public header file for the Utils DLL
// Utils contains commonly used functions.
//
// Created: 2008-07-09
//
// Copyright ©  2008-2010 by James John McGuire
// All rights reserved.
///////////////////////////////////////////////////////////////////////
#pragma once

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "../Include/Common.h"

///////////////////////////////////////////////////////////////////////
// defines
///////////////////////////////////////////////////////////////////////
const ULONG CodePageShiftJis		= 932;		// ANSI/OEM - Japanese, Shift-JIS 
const ULONG CodePageJis				= 50220;	// ISO 2022 Japanese with no halfwidth Katakana 
const ULONG CodePage50221			= 50221;	// ISO 2022 Japanese with halfwidth Katakana 
const ULONG CodePage50222			= 50222;	// ISO 2022 Japanese JIS X 0201-1989
const ULONG CodePageEucOfficial		= 51932;	// EUC - Japanese - doesn't work, at least on XP
const ULONG CodePageEucUndocumented	= 20932;	// EUC - Japanese - undocumented, works on, at least, XP
const ULONG CodePageUsAscii			= 20127;	// US-ASCII (7-bit) 
const ULONG CodePageUtf8			= 65001;	// Unicode UTF-8 

///////////////////////////////////////////////////////////////////////
// APIs
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// File related utils
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetBaseFileName(
	LPCTSTR	FileName);

///////////////////////////////////////////////////////////////////////
// GetFileNameTempPath
//
// Given a filename, will return that filename appended to the temp path
///////////////////////////////////////////////////////////////////////
DllExport LPCTSTR
GetFileNameTempPath(
	LPCTSTR	FileName);

DllExport TCHAR*
GetInstallPath(void);

DllExport TCHAR*
GetUserDataPath(
	TCHAR*	FileName);

DllExport TCHAR*
GetTempFilePathName();

DllExport LPCTSTR
GetWindowsDllPath(
	TCHAR*	FileName);

///////////////////////////////////////////////////////////////////////
// Language related utils
///////////////////////////////////////////////////////////////////////
DllExport WORD
GetLanguageId();

DllExport void
SetLanguageId(
	WORD	LangId,
	HMODULE ResourceModule);

DllExport HMODULE
GetResourceModule();

///////////////////////////////////////////////////////////////////////
// String related utils
///////////////////////////////////////////////////////////////////////
DllExport BYTE*
ConcatStreams(
	BYTE*	FirstStream,
	DWORD	FirstStreamLength,
	BYTE*	SecondStream,
	DWORD	SecondStreamLength);

DllExport void 
GetDateFromYYMMDDHHMMSS(
	char *str, 
	TCHAR* formattedDate, 
	size_t len);

DllExport void 
GetDateFromTime_t(
    time_t* time_t, 
	TCHAR* formattedDate, 
	size_t len);

DllExport TCHAR*
ConcatStrings(
	LPCTSTR	FirstString,
	LPCTSTR	SecondString);

DllExport char*
ConcatStringsA(
	const char*	FirstString,
	const char*	SecondString);
DllExport TCHAR* __cdecl
ConcatStringsV(
	LPCTSTR	FirstString,
	LPCTSTR	SecondString,
	...);
DllExport char* __cdecl
ConcatStringsVA(
	const char*	FirstString,
	const char*	SecondString,
	...);

///////////////////////////////////////////////////////////////////////
// Language related string functions
///////////////////////////////////////////////////////////////////////
DllExport char*
GetMultiByteString(
	LPCWSTR szUnicodeString);
DllExport char*
GetMultiByteString(
	ULONG	CodePage,
	LPCWSTR szUnicodeString);
DllExport char*
GetMultiByteStringFromUnicodeString(
	LPCWSTR szUnicodeString,
	ULONG nCodePage);

DllExport wchar_t*
GetUnicodeString(
	LPCSTR	MultiByteString);
DllExport wchar_t*
GetUnicodeString(
	ULONG	CodePage,
	LPCSTR	MultiByteString);
DllExport int
GetUnicodeStringFromMultiByteString(
	LPCSTR		szMultiByteString,
	wchar_t*	szUnicodeString,
	int			nUnicodeBufferSize,
	ULONG		nCodePage);

///////////////////////////////////////////////////////////////////////
// GetString
//
// Gets an localized string from the resource table
//
// delete after use.
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetString(
	UINT	ResourceId);

///////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetStringCopy(
	LPCTSTR	SourceString);

DllExport char*
GetStringCopyA(
	const char*	SourceString);

DllExport TCHAR*
GetStringCopyAmount(
	LPCTSTR	SourceString,
	size_t	Amount);

DllExport char*
GetStringCopyAmountA(
	const char*	SourceString,
	size_t		Amount);

DllExport TCHAR*
StringSprintfInt(
	TCHAR*	OriginalString,
	int	Number);

DllExport bool
IsUtf8BomMark(
	BYTE ByteInQuestion);

///////////////////////////////////////////////////////////////////////
// Test related utils
///////////////////////////////////////////////////////////////////////
DllExport void
ShowTestResult(
	TCHAR * TestName,
	long TestResult);

DllExport void
ShowConditionResult(
	TCHAR * TestName,
	long TestResult,
	long ExpectedResult);
DllExport void
ShowConditionResultNot(
	TCHAR * TestName,
	long TestResult,
	long ExpectedResult);
DllExport void
ShowConditionResultNot(
	TCHAR*	TestName,
	void*	TestResult,
	void*	ExpectedResult);

///////////////////////////////////////////////////////////////////////
// Misc functions
///////////////////////////////////////////////////////////////////////
DllExport bool
IsEmailValid(
	const TCHAR* Address);

DllExport int
ShowMessageStr(
	LPCTSTR Message,
	UINT uType = MB_OK);

DllExport int
ShowMessage(
	int	StringId);

DllExport int
ShowMessageError(
	LPCTSTR	Message,
	int		StringId);

DllExport int
ShowMessageYesNo(
	int	StringId);

DllExport bool
GetLastErrorInfo(
	HMODULE	ModuleHandle);

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
	UINT	ResourceId);

///////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
///////////////////////////////////////////////////////////////////////
DllExport TCHAR*
GetStringCopy(
	TCHAR*	SourceString);

LONG DelRegKey(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszSubKey,
	bool	bOnlyIfEmpty);

LONG GetRegKeyValue(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszRegKeyVar,
	LPTSTR	pszValue,
	DWORD	dwValueBufSize);

bool	SetDebug(
	int	nOption);

LONG SetRegKeyValue(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszRegKeyVar,
	LPTSTR	pszValue);
