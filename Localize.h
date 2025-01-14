/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2025 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "resource.h"

///////////////////////////////////////////////////////////////////////
// Support function prototypes
///////////////////////////////////////////////////////////////////////
int	GetNextCharacterState(
	int		nCurrentState,
	TCHAR	cNextNonSpaceChar,
	CString	csNextCharStream);
int		GetNextNonWhiteSpaceChar(CString& cszBuffer);
DllExport bool	IsHelp(int nCntParams, TCHAR** asParameters);
DllExport int		LocalizeIt(TCHAR* pszPath);
int		MainInit(void);
bool	ProcessFile(LPCTSTR pszFileName);
ULONGLONG	ReadFileIntoBuffer(LPCTSTR pszFileName, CString& cszBuffer);
//bool	IsUnicodeFile(CSrcFile& cSrcFile);

extern TCHAR*	g_psTestArgv1;
extern ULONG	g_uSectionCount;

class Localize
{
public:
	Localize();
	~Localize();

	bool IsUnicodeFile();
};

