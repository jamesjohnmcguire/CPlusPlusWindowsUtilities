/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2025 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// using namespace std;

//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "pch.h"
//#include "..\Common\CommandLine\CommandLine.h"
//#include "..\Common\Inc\PCKey.h"
#include "SrcFile.h"
#include "Localize.h"

///////////////////////////////////////////////////////////////////////
// definitions
///////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////
const int	STATE_DEFAULT = 0;
const int	STATE_PHP = 1;
const int	STATE_TEXT = 2;

///////////////////////////////////////////////////////////////////////
// globals
///////////////////////////////////////////////////////////////////////
// hack until oo
bool	g_bIsUnicodeFile = false;
//TCHAR*	g_psTestArgv1;
TCHAR	g_szDebug[128];
TCHAR	g_szBufferTmp[128];
TCHAR	g_szVarNumber[10];
ULONG	g_uSectionCount;
ULONG	g_uVarCount;

///////////////////////////////////////////////////////////////////////
// Support function prototypes
///////////////////////////////////////////////////////////////////////
PTSTR	AtoiX(int nIndex);
CString&	GetBaseFileName(LPCTSTR pszFileName, CString& cBaseFileName);
TCHAR	getNextNonWhiteSpaceChar(PTSTR pszStream);
bool	IsHelp(int nCntParams, TCHAR** asParameters);
bool	IsJavaScript(LPCTSTR pszStream);
bool	IsLegitemate(PTSTR pszStream);
PTSTR	MakeNewVarName(LPCTSTR pszOrigVarName, int nIndex);
PTSTR	MakeNewVarNameEx(int nIndex);
UINT	NewLineCount(LPCTSTR pBuffer);
bool	ShowHelp(void);
bool	WriteSqlCommand(CSrcFile& cFile, UINT nSectionId, LPCTSTR pszVar, LPCTSTR pszValue);
bool	WriteSqlSection(CSrcFile& cSqlFile, LPCTSTR pszSection);
bool	WriteSqlSectionBegin(CSrcFile& cFile);

///////////////////////////////////////////////////////////////////////
// functions
///////////////////////////////////////////////////////////////////////

PTSTR	AtoiX(int nIndex)
{
	_itot_s(nIndex, g_szVarNumber, 10);

	return g_szVarNumber;
}


///////////////////////////////////////////////////////////////////////
// GetBaseFileName
///////////////////////////////////////////////////////////////////////
CString&	GetBaseFileName(LPCTSTR pszFileName, CString& cBaseFileName)
{
	CString	cszFilePath(pszFileName);
	CString	cszFileName;

	int intDelimiterIndex = cszFilePath.GetLength() - cszFilePath.ReverseFind(_TEXT('\\'));

	cszFileName = cszFilePath.Right(intDelimiterIndex - 1);

	intDelimiterIndex = cszFileName.ReverseFind(_TEXT('.'));
	cBaseFileName = cszFileName.Left(intDelimiterIndex);

	return cBaseFileName;
}

///////////////////////////////////////////////////////////////////////
// GetNextCharacterState
///////////////////////////////////////////////////////////////////////
int	GetNextCharacterState(
	int		nCurrentState,
	TCHAR	cNextNonSpaceChar,
	CString	csNextCharStream)
{
	int	nNextState = nCurrentState;

	switch (cNextNonSpaceChar)
	{
	case '<':
	{
		// default or html tag
		nNextState = STATE_DEFAULT;
		break;
	}
	case '{':
	{
		nNextState = STATE_PHP;
		break;
	}
	default:
	{
		bool	bRet = IsJavaScript(csNextCharStream);

		if (true == bRet)
			// it will change after the next '>'
			nNextState = STATE_DEFAULT;
		else
		{
			nNextState = STATE_TEXT;
		}
		break;
	}
	}

	return nNextState;
}

///////////////////////////////////////////////////////////////////////
// getNextNonWhiteSpaceChar
///////////////////////////////////////////////////////////////////////
TCHAR	getNextNonWhiteSpaceChar(PTSTR pszStream)
{
	while ((UINT)*pszStream < 0x21)
		pszStream++;
	return *pszStream;
}


int	GetNextNonWhiteSpaceChar(CString& cszBuffer)
{
	int	i = -1;

	if (!cszBuffer.IsEmpty())
	{
		i = 1;

		while ((cszBuffer.GetAt(i) < 0x21) && (i < cszBuffer.GetLength()))
			i++;
	}
	return i;
}

///////////////////////////////////////////////////////////////////////
// IsHelp
///////////////////////////////////////////////////////////////////////
// returns whether the command line indicated the user wanted to see help or if there were problems
bool IsHelp(int nCntParams, TCHAR** asParameters)
{
	bool	bIsHelp = false;

	if (nCntParams == 0)
	{
		bIsHelp = true;
	}
	else
	{
		for (int i = 1; i <= nCntParams; i++)
		{
			if (_tcsncicmp(asParameters[i], _T("-?"), 2) == 0)
			{
				bIsHelp = true;
				break;
			}

			if (_tcsncicmp(asParameters[i], _T("/?"), 2) == 0)
			{
				bIsHelp = true;
				break;
			}

			if (_tcsncicmp(asParameters[i], _T("?"), 2) == 0)
			{
				bIsHelp = true;
				break;
			}
			if (_tcsncicmp(asParameters[i], _T("-h"), 2) == 0)
			{
				bIsHelp = true;
				break;
			}

			if (_tcsncicmp(asParameters[i], _T("/h"), 2) == 0)
			{
				bIsHelp = true;
				break;
			}

			if (_tcsncicmp(asParameters[i], _T("help"), 4) == 0)
			{
				bIsHelp = true;
				break;
			}
		}
	}
	return bIsHelp;
}

///////////////////////////////////////////////////////////////////////
// IsJavaScript
// this needs to be improved!
///////////////////////////////////////////////////////////////////////
bool IsJavaScript(LPCTSTR pszStream)
{
	bool	bRet = false;

	while ((UINT)*pszStream < 0x21)
		pszStream++;

	// if it matches 'function' let's assume it's javascript.
	int	nRes = _tcsnicmp(pszStream, _T("function"), 8);

	if (0 == nRes)
		bRet = true;

	return bRet;
}

///////////////////////////////////////////////////////////////////////
// IsLegitemate
///////////////////////////////////////////////////////////////////////
bool	IsLegitemate(PTSTR pszStream)
{
	bool	bRet = false;

	while ('\0' != *pszStream)
	{
		if (((UINT)*pszStream > 0x40) && ((UINT)*pszStream < 0x5B) ||
			((UINT)*pszStream > 0x60) && ((UINT)*pszStream < 0x7B))
		{
			bRet = true;
			break;
		}
		pszStream++;
	}
	return bRet;
}


///////////////////////////////////////////////////////////////////////
// IsUnicodeFile
///////////////////////////////////////////////////////////////////////
bool	IsUnicodeFile(CSrcFile& cSrcFile)
{
	bool	bUnicode = false;

	if (cSrcFile.IsFileUnicodeText())
		bUnicode = true;

	return  bUnicode;
}

///////////////////////////////////////////////////////////////////////
// LocalizeIt
///////////////////////////////////////////////////////////////////////
int LocalizeIt(TCHAR* pszPath)
{
	int nRetCode = EXIT_FAILURE;

	CStringArray	afilePaths;
	//		CString	cszFileTypes	= cszDir + _T("\\*.tpl");
	//		CString	cszFileTypes	= _T("\\");
	CString	cszFileTypes = pszPath;
	UINT	uFileCount = 0;
	CFileFind finder;

	g_uVarCount = 0;
	g_uSectionCount = 63;

	//		WriteSqlSectionBegin( CSrcFile& cFile )

	BOOL found = finder.FindFile(cszFileTypes);
	while (found)
	{
		found = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			afilePaths.Add(finder.GetFilePath());
			uFileCount++;
			g_uSectionCount++;
			ProcessFile((LPCTSTR)finder.GetFilePath());
		}
	}
	nRetCode = EXIT_SUCCESS;

	return nRetCode;
}

///////////////////////////////////////////////////////////////////////
// MakeNewVarName
///////////////////////////////////////////////////////////////////////
PTSTR	MakeNewVarName(LPCTSTR pszOrigVarName, int nIndex)
{
	CString		cTmp(pszOrigVarName);

	_itot_s(nIndex, g_szVarNumber, 10);

	_tcscpy_s(g_szBufferTmp, pszOrigVarName);
	_tcscat_s(g_szBufferTmp, g_szVarNumber);
	//	cTmp =+ &g_szVarNumber;
	//	PTSTR	pszTmp	= cTmp.GetBuffer( 64 );
	//	cTmp.ReleaseBuffer
	return g_szBufferTmp;
}

///////////////////////////////////////////////////////////////////////
// MakeNewVarNameEx
///////////////////////////////////////////////////////////////////////
PTSTR	MakeNewVarNameEx(int nIndex)
{
	_itot_s(nIndex, g_szVarNumber, 10);

	_tcscpy_s(g_szBufferTmp, _T("Var"));
	_tcscat_s(g_szBufferTmp, g_szVarNumber);
	return g_szBufferTmp;
}

///////////////////////////////////////////////////////////////////////
// NewLineCount
///////////////////////////////////////////////////////////////////////
UINT	NewLineCount(LPCTSTR pBuffer)
{
	UINT	uCount = 0;
	int		nPos = 0;
	CString	cszBuffer = pBuffer;

	nPos = cszBuffer.Find(_T('\n'), nPos);
	while (-1 != nPos)
	{
		uCount++;
		nPos = cszBuffer.Find('\n', nPos + 1);
	}

	return	uCount;
}

///////////////////////////////////////////////////////////////////////
// ProcessDefaultState
///////////////////////////////////////////////////////////////////////
/*
bool	ProcessDefaultState( LPCTSTR sInputStream, LPCTSTR sOutputStream)
{
	bool	bRet	= false;

	if (cCurrentChar == '>')
	{
		nState	= GetNextCharacterState( nCurrentState, cNextNonSpaceChar);
								switch (cNextNonSpaceChar)
								{
									case '<':
									{
										nState = 0;
										break;
									}
									case '{':
									{
										nState = 1;
										break;
									}
									default:
									{
										bool	bRet	= IsJavaScript( csNextCharStream );

										if (true == bRet)
											// it will change after the next '>'
											nState = 0;
										else
										{
											bBegin = true;

											// were live now
											nState = 2;
											nVarIndex	= 0;
											cVarName = MakeNewVarName( cOrigVarName, nVarIncrement );
										}
										break;
									}
								}
							}
							// state stays the same
							pFileBufNew[k] = cCurrentChar;
							cNewFileBuffer += cCurrentChar;
							k++;
							break;
						}

	return bRet;
}
*/

///////////////////////////////////////////////////////////////////////
// ProcessFile
///////////////////////////////////////////////////////////////////////
bool	ProcessFile(LPCTSTR pszFileName)
{
	bool	bRet = true;
	bool	bBegin = false;

	BOOL	bErrorCheck = FALSE;
	ULONGLONG	nLength;				// in bytes or chars?
	int		nState = 0;			// 0 = in tag
									// 1 = in smarty tag
									// 2 = not in tag (in string)
	UINT	nVarIncrement = 1;	// the number of vars
	UINT	nVarIndex = 0;
	UINT k = 0;

	CSrcFile	cConfFile;
	CSrcFile	cSqlFile;
	CString	cszFilePath(pszFileName);
	CString	cszFileName;
	CString	cOrigVarName;
	CString	cVarName;
	CString cVarBuffer;
	CString cBaseFileName;
	CString cNewFileBuffer;
	CString cOriginalFileBuffer;

	CString	cszSectionName = GetBaseFileName(pszFileName, cBaseFileName);

	//	cOrigVarName	+= _T(":::Var");
	cOrigVarName = _T("Var");

	CString	cConfFileName = cszFilePath + _T(".conf");
	CString	cSqlFileName = _T("UpdateLanguage.sql");

	cConfFile.SetSrcFileName(cConfFileName);
	cSqlFile.SetSrcFileName(cSqlFileName);

	nLength = ReadFileIntoBuffer(pszFileName, cOriginalFileBuffer);

	// if we can't open these files, no point in going on
	if (nLength > 0)
	{
		PTSTR		pFileBufNew = NULL;
		PTSTR		pVarBuf = NULL;

		bErrorCheck = cConfFile.Open();
		const CString& projectPath = cConfFile.GetProjectPath();

		if (bRet == FALSE)
		{
			_tprintf(_T("error opening file: %s\r\n"), projectPath);
		}
		else
		{
			bErrorCheck = cSqlFile.Open();

			if (bRet == FALSE)
			{
				_tprintf(_T("error opening file: %s\r\n"), projectPath);
			}
			else
			{
				// Way overkill, in fact, it should probably be shorter then original
				size_t	nNewFileLength = (size_t)(nLength + 1) * sizeof(TCHAR) * 2;
				cSqlFile.SeekToEnd();

				pFileBufNew = (PTSTR)malloc(nNewFileLength);
				pVarBuf = (PTSTR)malloc(((size_t)nLength + 1) * sizeof(TCHAR));

				memset(pFileBufNew, 0, nNewFileLength);
				memset(pVarBuf, 0, ((size_t)nLength + 1) * sizeof(TCHAR));
				UINT	uNewLines = NewLineCount(cOriginalFileBuffer);

				nLength = nLength - uNewLines;
				//	int nFind	= cszSectionName.Find( _T("va_") );

				//	if (nFind != -1)
				//		cszSectionName = cszSectionName.Right( cszSectionName.GetLength( ) - 3);

				WriteSqlSection(cSqlFile, cszSectionName);

				// for length of file
				// for each character
				// do the following

				for (size_t i = 0; i < nLength; i++)
				{
					// determine how much is remaining for our input buffer
					size_t	nLengthRemainingBuffer;
					int	nNextNonSpaceChar;

					CString	csNextCharStream;
					TCHAR	cCurrentChar;
					TCHAR	cNextChar;
					TCHAR	cNextNonSpaceChar;

					// determine how much is remaining for our input buffer
					nLengthRemainingBuffer = cOriginalFileBuffer.GetLength() - i;

					// get that stream
					csNextCharStream = cOriginalFileBuffer.Right(nLengthRemainingBuffer);

					nNextNonSpaceChar = GetNextNonWhiteSpaceChar(csNextCharStream);

					cCurrentChar = cOriginalFileBuffer.GetAt(i);
					cNextChar = cOriginalFileBuffer.GetAt(i + 1);
					cNextNonSpaceChar = cOriginalFileBuffer.GetAt(i + nNextNonSpaceChar);

					//_tcsncpy( g_szDebug, &pFileBufOld[i], 120 );
					//g_szDebug[121] = '\0';
					switch (nState)
					{
						// in a html tag
					case STATE_DEFAULT:
					{
						if (cCurrentChar == '>')
						{
							nState = GetNextCharacterState(STATE_DEFAULT,
								cNextNonSpaceChar,
								csNextCharStream);

							if (nState == STATE_TEXT)
							{
								// were live now
								bBegin = true;
								nVarIndex = 0;
								cVarName = MakeNewVarName(cOrigVarName, nVarIncrement);
							}
						}
						// state stays the same
						pFileBufNew[k] = cCurrentChar;
						cNewFileBuffer += cCurrentChar;
						k++;
						break;
					}

					// in a smarty tag
					case STATE_PHP:
					{
						if (cCurrentChar == '}')
						{
							nState = GetNextCharacterState(STATE_PHP,
								cNextNonSpaceChar,
								csNextCharStream);

							if (nState == STATE_TEXT)
							{
								// were live now
								bBegin = true;
								nVarIndex = 0;
								cVarName = MakeNewVarName(cOrigVarName, nVarIncrement);
							}
						}
						// state stays the same
						pFileBufNew[k] = cCurrentChar;
						k++;
						break;
					}

					// Already in getvar state
					case 2:
					{
						if (('<' == cCurrentChar) || ('{' == cCurrentChar))
						{
							// ok, finish and close the var capturing
							nState = 0;

							pVarBuf[nVarIndex] = '\0';

							bool bOk = IsLegitemate(pVarBuf);
							size_t	uSize = nNewFileLength - k - 100;

							CString cVarValue = pVarBuf;

							cVarValue.Trim();
							if (true == bOk)
							{
								// write the string into the conf file
			//						CString cszLine	= cVarName + _T(":::") + pVarBuf;
								CString cszLine = cVarName + _T(":::") + cVarValue;

								cszLine += _T("\n");
								cConfFile.WriteString(cszLine);

								cVarBuffer = _T("{#") + cVarName + _T("#}");

								// write the variable into the new file
								LPCTSTR	pszBuf = (LPCTSTR)cVarBuffer;
								UINT	uStringLen = (UINT)_tcsclen(pszBuf);
								if (uStringLen > 0)
								{
									errno_t nErrorCode = _tcscpy_s(&pFileBufNew[k], uStringLen + 1, pszBuf);
									k += cVarBuffer.GetLength();
								}
								//
								PTSTR	pszVarName = MakeNewVarNameEx(nVarIncrement);

								cVarValue.Replace(_T("'"), _T("\\'"));
								WriteSqlCommand(cSqlFile, g_uSectionCount, pszVarName, cVarValue);

								nVarIncrement++;

								// also write this current character
								pFileBufNew[k] = cCurrentChar;
								k++;
							}
							else
							{
								UINT	uStringLen = (UINT)_tcsclen(pVarBuf);
								if (uStringLen > 0)
								{
									// ignore the string and leave in file as-is
									errno_t nErrorCode = _tcscpy_s(&pFileBufNew[k], uSize, (LPCTSTR)pVarBuf);
									k += uStringLen;
								}
								// also write this current character
								pFileBufNew[k] = cCurrentChar;
								k++;
							}
						}
						else
						{
							// if we are beginning, basically skip over an leading whitespace
							if (true == bBegin)
							{
								// if it's an alpha char
								if (((UINT)cCurrentChar > 0x40) && ((UINT)cCurrentChar < 0x5B) ||
									((UINT)cCurrentChar > 0x60) && ((UINT)cCurrentChar < 0x7B))
								{
									bBegin = false;
									// state stays the same
									pVarBuf[nVarIndex] = cCurrentChar;
									nVarIndex++;
								}
								else
								{
									// state stays the same
									pFileBufNew[k] = cCurrentChar;
									k++;
								}
							}
							else
							{
								// state stays the same
								pVarBuf[nVarIndex] = cCurrentChar;
								nVarIndex++;
							}
						}
						break;
					}
					}
					// read until close tag
					//
				}
				pFileBufNew[k] = '\0';

				nLength = _tcsclen(pFileBufNew);
				//		cSrcFile.Close( );
				cSqlFile.Close();

				CSrcFile	cNewFile;
				CString	cszNewFile = cszFilePath + _T(".new");
				cNewFile.SetSrcFileName(cszNewFile);
				cNewFile.SetUnicode(g_bIsUnicodeFile);
				cNewFile.Open();
				cNewFile.Write(pFileBufNew, k);
				cNewFile.Close();

				free((void *)pFileBufNew);
				free((void *)pVarBuf);
			}
			cConfFile.Close();
		}
	}

	return bRet;
}

ULONGLONG	ReadFileIntoBuffer(LPCTSTR pszFileName, CString& cszBuffer)
{
	BOOL		bErrorCheck = FALSE;
	ULONGLONG	uFileLength = 0;
	ULONGLONG	uRead = 0;
	CSrcFile	cSrcFile;

	cSrcFile.SetSrcFileName(pszFileName);

	bErrorCheck = cSrcFile.Open();

	g_bIsUnicodeFile = cSrcFile.IsFileUnicodeText();

	// if we can't open these files, no point in going on
	if (bErrorCheck == FALSE)
	{
		_tprintf(_T("error opening file: %s\r\n"), pszFileName);
	}
	else
	{
		uFileLength = cSrcFile.GetLength();

		if (uFileLength > 0)
		{
			uRead = cSrcFile.Read(cszBuffer, (UINT)uFileLength);
			cSrcFile.Close();
		}
	}
	return uRead;
}

///////////////////////////////////////////////////////////////////////
// WriteSqlCommand
///////////////////////////////////////////////////////////////////////
// In theory, we should know the id, since it is in sync with our var count, section count
bool	WriteSqlCommand(CSrcFile& cFile, UINT nSectionId, LPCTSTR pszVar, LPCTSTR pszValue)
{
	bool bRet = true;

	CString	cszSectionId = AtoiX(nSectionId);

	// first insert into var

	CString	cSql = _T("INSERT INTO string( Var, id_section) values ('");
	cSql += pszVar;
	cSql += _T("', '");
	cSql += cszSectionId;
	cSql += _T("');\n");
	cFile.WriteString(cSql);

	g_uVarCount++;
	PTSTR	pszVarCount = AtoiX(g_uVarCount);

	cSql = _T("INSERT INTO language_string( id_lang, id_var, string) values ('1', '");
	cSql += pszVarCount;
	cSql += _T("', '");
	cSql += pszValue;
	cSql += _T("');\n");
	cFile.WriteString(cSql);

	cSql = _T("INSERT INTO language_string( id_lang, id_var, string) values ('2', '");
	cSql += pszVarCount;
	cSql += _T("', '");
	cSql += pszValue;
	cSql += _T("');\n");
	cFile.WriteString(cSql);

	return bRet;
}

///////////////////////////////////////////////////////////////////////
// WriteSqlSection
///////////////////////////////////////////////////////////////////////
bool	WriteSqlSection(CSrcFile& cSqlFile, LPCTSTR pszSection)
{
	bool bRet = true;

	CString	cSql = _T("INSERT INTO language_section(Section) values ('");
	cSql += pszSection;
	cSql += _T("');\n");

	cSqlFile.WriteString(cSql);

	return bRet;
}

///////////////////////////////////////////////////////////////////////
// WriteSqlSectionBegin
///////////////////////////////////////////////////////////////////////
bool	WriteSqlSectionBegin(CSrcFile& cFile)
{
	bool bRet = true;

	CString	cSql = _T("DROP TABLE language_section;\n");
	cFile.WriteString(cSql);

	cSql = _T("CREATE TABLE `language_section`\n");
	cFile.WriteString(cSql);
	cSql = _T("(\n");
	cFile.WriteString(cSql);
	cSql = _T("  `id` int(4) unsigned NOT NULL auto_increment,\n");
	cFile.WriteString(cSql);
	cSql = _T("  `Section` varchar(64) NOT NULL default '',\n");
	cFile.WriteString(cSql);
	cSql = _T("  PRIMARY KEY (id) \n");
	cFile.WriteString(cSql);
	cSql = _T(") TYPE=MyISAM;\n");
	cFile.WriteString(cSql);

	return bRet;
}

Localize::Localize()
{
}


Localize::~Localize()
{
}
