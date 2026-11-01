/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

///////////////////////////////////////////////////////////////////////
// Support function prototypes
///////////////////////////////////////////////////////////////////////
int	GetNextCharacterState(
	int		currentState,
	wchar_t	cNextNonSpaceChar,
	CString	csNextCharStream);
int		GetNextNonWhiteSpaceChar(CString& cszBuffer);
DllExport bool	IsHelp(int parameterCount, wchar_t** asParameters);
DllExport int		LocalizeIt(wchar_t* path);
int		MainInit(void);
bool	ProcessFile(const wchar_t* fileName);
ULONGLONG	ReadFileIntoBuffer(const wchar_t* fileName, CString& cszBuffer);
#if LOCALIZE_UNICODE_DETECTION_UNIMPLEMENTED
bool	IsUnicodeFile(CSrcFile& cSrcFile);
#endif

extern wchar_t*	g_psTestArgv1;
extern ULONG	g_uSectionCount;

class Localize
{
public:
	Localize();
	~Localize();

	bool IsUnicodeFile();
};

