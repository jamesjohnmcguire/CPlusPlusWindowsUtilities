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
	TCHAR	cNextNonSpaceChar,
	CString	csNextCharStream);
int		GetNextNonWhiteSpaceChar(CString& cszBuffer);
DllExport bool	IsHelp(int parameterCount, TCHAR** asParameters);
DllExport int		LocalizeIt(TCHAR* path);
int		MainInit(void);
bool	ProcessFile(LPCTSTR fileName);
ULONGLONG	ReadFileIntoBuffer(LPCTSTR fileName, CString& cszBuffer);
#if LOCALIZE_UNICODE_DETECTION_UNIMPLEMENTED
bool	IsUnicodeFile(CSrcFile& cSrcFile);
#endif

extern TCHAR*	g_psTestArgv1;
extern ULONG	g_uSectionCount;

class Localize
{
public:
	Localize();
	~Localize();

	bool IsUnicodeFile();
};

