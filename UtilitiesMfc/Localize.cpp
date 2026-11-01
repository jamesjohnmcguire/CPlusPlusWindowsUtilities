/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#if LEGACY_STD_NAMESPACE
using namespace std;
#endif

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
#if LOCALIZE_TEST_ARGUMENT
wchar_t*	g_psTestArgv1;
#endif
wchar_t	g_szDebug[128];
wchar_t	g_szBufferTmp[128];
wchar_t	g_szVarNumber[10];
ULONG	g_uSectionCount;
ULONG	g_uVarCount;

///////////////////////////////////////////////////////////////////////
// Support function prototypes
///////////////////////////////////////////////////////////////////////
wchar_t*	AtoiX(int index);
CString&	GetBaseFileName(const wchar_t* fileName, CString& cBaseFileName);
wchar_t	getNextNonWhiteSpaceChar(wchar_t* streamText);
bool	IsHelp(int parameterCount, wchar_t** asParameters);
bool	IsJavaScript(const wchar_t* streamText);
bool	IsLegitemate(wchar_t* streamText);
wchar_t*	MakeNewVarName(const wchar_t* originalVariableName, int index);
wchar_t*	MakeNewVarNameEx(int index);
UINT	NewLineCount(const wchar_t* bufferData);
bool	ShowHelp(void);
bool	WriteSqlCommand(CSrcFile& cFile, UINT sectionId, const wchar_t* variableText, const wchar_t* valueText);
bool	WriteSqlSection(CSrcFile& cSqlFile, const wchar_t* sectionName);
bool	WriteSqlSectionBegin(CSrcFile& cFile);

///////////////////////////////////////////////////////////////////////
// functions
///////////////////////////////////////////////////////////////////////

wchar_t*	AtoiX(int index)
{
	_itow_s(index, g_szVarNumber, 10);

	return g_szVarNumber;
}


///////////////////////////////////////////////////////////////////////
// GetBaseFileName
///////////////////////////////////////////////////////////////////////
CString&	GetBaseFileName(const wchar_t* fileName, CString& cBaseFileName)
{
	CString	cszFilePath(fileName);
	CString	cszFileName;

	int intDelimiterIndex = cszFilePath.GetLength() - cszFilePath.ReverseFind(L'\\');

	cszFileName = cszFilePath.Right(intDelimiterIndex - 1);

	intDelimiterIndex = cszFileName.ReverseFind(L'.');
	cBaseFileName = cszFileName.Left(intDelimiterIndex);

	return cBaseFileName;
}

///////////////////////////////////////////////////////////////////////
// GetNextCharacterState
///////////////////////////////////////////////////////////////////////
int	GetNextCharacterState(
	int		currentState,
	wchar_t	cNextNonSpaceChar,
	CString	csNextCharStream)
{
	int	nextState = currentState;

	switch (cNextNonSpaceChar)
	{
	case '<':
	{
		// default or html tag
		nextState = STATE_DEFAULT;
		break;
	}
	case '{':
	{
		nextState = STATE_PHP;
		break;
	}
	default:
	{
		bool	result = IsJavaScript(csNextCharStream);

		if (true == result)
			// it will change after the next '>'
			nextState = STATE_DEFAULT;
		else
		{
			nextState = STATE_TEXT;
		}
		break;
	}
	}

	return nextState;
}

///////////////////////////////////////////////////////////////////////
// getNextNonWhiteSpaceChar
///////////////////////////////////////////////////////////////////////
wchar_t	getNextNonWhiteSpaceChar(wchar_t* streamText)
{
	while ((UINT)*streamText < 0x21)
		streamText++;
	return *streamText;
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
bool IsHelp(int parameterCount, wchar_t** asParameters)
{
	bool	isHelp = false;

	if (parameterCount == 0)
	{
		isHelp = true;
	}
	else
	{
		for (int i = 1; i < parameterCount; i++)
		{
			if (_wcsnicmp(asParameters[i], L"-?", 2) == 0)
			{
				isHelp = true;
				break;
			}

			if (_wcsnicmp(asParameters[i], L"/?", 2) == 0)
			{
				isHelp = true;
				break;
			}

			if (_wcsnicmp(asParameters[i], L"?", 2) == 0)
			{
				isHelp = true;
				break;
			}
			if (_wcsnicmp(asParameters[i], L"-h", 2) == 0)
			{
				isHelp = true;
				break;
			}

			if (_wcsnicmp(asParameters[i], L"/h", 2) == 0)
			{
				isHelp = true;
				break;
			}

			if (_wcsnicmp(asParameters[i], L"help", 4) == 0)
			{
				isHelp = true;
				break;
			}
		}
	}
	return isHelp;
}

///////////////////////////////////////////////////////////////////////
// IsJavaScript
// this needs to be improved!
///////////////////////////////////////////////////////////////////////
bool IsJavaScript(const wchar_t* streamText)
{
	bool	result = false;

	while ((UINT)*streamText < 0x21)
		streamText++;

	// if it matches 'function' let's assume it's javascript.
	int	resultCode = _wcsnicmp(streamText, L"function", 8);

	if (0 == resultCode)
		result = true;

	return result;
}

///////////////////////////////////////////////////////////////////////
// IsLegitemate
///////////////////////////////////////////////////////////////////////
bool	IsLegitemate(wchar_t* streamText)
{
	bool	result = false;

	while ('\0' != *streamText)
	{
		if (((UINT)*streamText > 0x40) && ((UINT)*streamText < 0x5B) ||
			((UINT)*streamText > 0x60) && ((UINT)*streamText < 0x7B))
		{
			result = true;
			break;
		}
		streamText++;
	}
	return result;
}


///////////////////////////////////////////////////////////////////////
// IsUnicodeFile
///////////////////////////////////////////////////////////////////////
bool	IsUnicodeFile(CSrcFile& cSrcFile)
{
	bool	useUnicode = false;

	if (cSrcFile.IsFileUnicodeText())
		useUnicode = true;

	return  useUnicode;
}

///////////////////////////////////////////////////////////////////////
// LocalizeIt
///////////////////////////////////////////////////////////////////////
int LocalizeIt(wchar_t* path)
{
	int returnCode = EXIT_FAILURE;

	CStringArray	afilePaths;
#if LOCALIZE_FILE_PATTERN_ALTERNATIVES
		CString	cszFileTypes	= cszDir + L"\\*.tpl";
		CString	cszFileTypes	= L"\\";
#endif
	CString	cszFileTypes = path;
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
			ProcessFile((const wchar_t*)finder.GetFilePath());
		}
	}
	returnCode = EXIT_SUCCESS;

	return returnCode;
}

///////////////////////////////////////////////////////////////////////
// MakeNewVarName
///////////////////////////////////////////////////////////////////////
wchar_t*	MakeNewVarName(const wchar_t* originalVariableName, int index)
{
	CString		cTmp(originalVariableName);

	_itow_s(index, g_szVarNumber, 10);

	wcscpy_s(g_szBufferTmp, originalVariableName);
	wcscat_s(g_szBufferTmp, g_szVarNumber);
#if LOCALIZE_VAR_NAME_BUFFER_INCOMPLETE
	cTmp =+ &g_szVarNumber;
	wchar_t*	temporaryText	= cTmp.GetBuffer( 64 );
	cTmp.ReleaseBuffer
#endif
	return g_szBufferTmp;
}

///////////////////////////////////////////////////////////////////////
// MakeNewVarNameEx
///////////////////////////////////////////////////////////////////////
wchar_t*	MakeNewVarNameEx(int index)
{
	_itow_s(index, g_szVarNumber, 10);

	wcscpy_s(g_szBufferTmp, L"Var");
	wcscat_s(g_szBufferTmp, g_szVarNumber);
	return g_szBufferTmp;
}

///////////////////////////////////////////////////////////////////////
// NewLineCount
///////////////////////////////////////////////////////////////////////
UINT	NewLineCount(const wchar_t* bufferData)
{
	UINT	uCount = 0;
	int		position = 0;
	CString	cszBuffer = bufferData;

	position = cszBuffer.Find(L'\n', position);
	while (-1 != position)
	{
		uCount++;
		position = cszBuffer.Find('\n', position + 1);
	}

	return	uCount;
}

///////////////////////////////////////////////////////////////////////
// ProcessDefaultState
///////////////////////////////////////////////////////////////////////
#if LOCALIZE_DEFAULT_STATE_INCOMPLETE
bool	ProcessDefaultState( const wchar_t* sInputStream, const wchar_t* sOutputStream)
{
	bool	result	= false;

	if (cCurrentChar == '>')
	{
		state	= GetNextCharacterState( currentState, cNextNonSpaceChar);
								switch (cNextNonSpaceChar)
								{
									case '<':
									{
										state = 0;
										break;
									}
									case '{':
									{
										state = 1;
										break;
									}
									default:
									{
										bool	result	= IsJavaScript( csNextCharStream );

										if (true == result)
											// it will change after the next '>'
											state = 0;
										else
										{
											begin = true;

											// were live now
											state = 2;
											variableIndex	= 0;
											cVarName = MakeNewVarName( cOrigVarName, variableIncrement );
										}
										break;
									}
								}
							}
							// state stays the same
							newFileBuffer[k] = cCurrentChar;
							cNewFileBuffer += cCurrentChar;
							k++;
							break;
						}

	return result;
}
#endif

///////////////////////////////////////////////////////////////////////
// ProcessFile
///////////////////////////////////////////////////////////////////////
bool	ProcessFile(const wchar_t* fileName)
{
	bool	result = true;
	bool	begin = false;

	BOOL	checkErrors = FALSE;
	ULONGLONG	length;				// in bytes or chars?
	int		state = 0;			// 0 = in tag
									// 1 = in smarty tag
									// 2 = not in tag (in string)
	UINT	variableIncrement = 1;	// the number of vars
	UINT	variableIndex = 0;
	UINT k = 0;

	CSrcFile	cConfFile;
	CSrcFile	cSqlFile;
	CString	cszFilePath(fileName);
	CString	cszFileName;
	CString	cOrigVarName;
	CString	cVarName;
	CString cVarBuffer;
	CString cBaseFileName;
	CString cNewFileBuffer;
	CString cOriginalFileBuffer;

	CString	cszSectionName = GetBaseFileName(fileName, cBaseFileName);

#if LOCALIZE_ORIGINAL_VARIABLE_SUFFIX
	cOrigVarName	+= L":::Var";
#endif
	cOrigVarName = L"Var";

	CString	cConfFileName = cszFilePath + L".conf";
	CString	cSqlFileName = L"UpdateLanguage.sql";

	cConfFile.SetSrcFileName(cConfFileName);
	cSqlFile.SetSrcFileName(cSqlFileName);

	length = ReadFileIntoBuffer(fileName, cOriginalFileBuffer);

	// if we can't open these files, no point in going on
	if (length > 0)
	{
		wchar_t*		newFileBuffer = nullptr;
		wchar_t*		variableBuffer = nullptr;

		checkErrors = cConfFile.Open();
		const CString& projectPath = cConfFile.GetProjectPath();

		if (result == FALSE)
		{
			wprintf(L"error opening file: %s\r\n", projectPath);
		}
		else
		{
			checkErrors = cSqlFile.Open();

			if (result == FALSE)
			{
				wprintf(L"error opening file: %s\r\n", projectPath);
			}
			else
			{
				// Way overkill, in fact, it should probably be shorter then original
				size_t	newFileLength = (size_t)(length + 1) * sizeof(wchar_t) * 2;
				cSqlFile.SeekToEnd();

				newFileBuffer = (wchar_t*)malloc(newFileLength);
				variableBuffer = (wchar_t*)malloc(((size_t)length + 1) * sizeof(wchar_t));

				memset(newFileBuffer, 0, newFileLength);
				memset(variableBuffer, 0, ((size_t)length + 1) * sizeof(wchar_t));
				UINT	uNewLines = NewLineCount(cOriginalFileBuffer);

				length = length - uNewLines;
#if LOCALIZE_SECTION_PREFIX
				int foundIndex	= cszSectionName.Find( L"va_" );

				if (foundIndex != -1)
					cszSectionName = cszSectionName.Right( cszSectionName.GetLength( ) - 3);
#endif

				WriteSqlSection(cSqlFile, cszSectionName);

				// for length of file
				// for each character
				// do the following

				for (size_t i = 0; i < length; i++)
				{
					// determine how much is remaining for our input buffer
					size_t	remainingBufferLength;
					int	nextNonSpaceCharacter;

					CString	csNextCharStream;
					wchar_t	cCurrentChar;
					wchar_t	cNextChar;
					wchar_t	cNextNonSpaceChar;

					// determine how much is remaining for our input buffer
					remainingBufferLength = cOriginalFileBuffer.GetLength() - i;

					// get that stream
					csNextCharStream = cOriginalFileBuffer.Right(remainingBufferLength);

					nextNonSpaceCharacter = GetNextNonWhiteSpaceChar(csNextCharStream);

					cCurrentChar = cOriginalFileBuffer.GetAt(i);
					cNextChar = cOriginalFileBuffer.GetAt(i + 1);
					cNextNonSpaceChar = cOriginalFileBuffer.GetAt(i + nextNonSpaceCharacter);

#if LOCALIZE_DEBUG_BUFFER
					wcsncpy( g_szDebug, &previousFileBuffer[i], 120 );
					g_szDebug[121] = '\0';
#endif
					switch (state)
					{
						// in a html tag
					case STATE_DEFAULT:
					{
						if (cCurrentChar == '>')
						{
							state = GetNextCharacterState(STATE_DEFAULT,
								cNextNonSpaceChar,
								csNextCharStream);

							if (state == STATE_TEXT)
							{
								// were live now
								begin = true;
								variableIndex = 0;
								cVarName = MakeNewVarName(cOrigVarName, variableIncrement);
							}
						}
						// state stays the same
						newFileBuffer[k] = cCurrentChar;
						cNewFileBuffer += cCurrentChar;
						k++;
						break;
					}

					// in a smarty tag
					case STATE_PHP:
					{
						if (cCurrentChar == '}')
						{
							state = GetNextCharacterState(STATE_PHP,
								cNextNonSpaceChar,
								csNextCharStream);

							if (state == STATE_TEXT)
							{
								// were live now
								begin = true;
								variableIndex = 0;
								cVarName = MakeNewVarName(cOrigVarName, variableIncrement);
							}
						}
						// state stays the same
						newFileBuffer[k] = cCurrentChar;
						k++;
						break;
					}

					// Already in getvar state
					case 2:
					{
						if (('<' == cCurrentChar) || ('{' == cCurrentChar))
						{
							// ok, finish and close the var capturing
							state = 0;

							variableBuffer[variableIndex] = '\0';

							bool succeeded = IsLegitemate(variableBuffer);
							size_t	uSize = newFileLength - k - 100;

							CString cVarValue = variableBuffer;

							cVarValue.Trim();
							if (true == succeeded)
							{
								// write the string into the conf file
#if LOCALIZE_VARIABLE_LINE
						CString cszLine	= cVarName + L":::" + variableBuffer;
#endif
								CString cszLine = cVarName + L":::" + cVarValue;

								cszLine += L"\n";
								cConfFile.WriteString(cszLine);

								cVarBuffer = L"{#" + cVarName + L"#}";

								// write the variable into the new file
								const wchar_t*	temporaryBuffer = (const wchar_t*)cVarBuffer;
								UINT	uStringLen = (UINT)wcslen(temporaryBuffer);
								if (uStringLen > 0)
								{
									errno_t errorCode = wcscpy_s(&newFileBuffer[k], uStringLen + 1, temporaryBuffer);
									k += cVarBuffer.GetLength();
								}
								//
								wchar_t*	variableName = MakeNewVarNameEx(variableIncrement);

								cVarValue.Replace(L"'", L"\\'");
								WriteSqlCommand(cSqlFile, g_uSectionCount, variableName, cVarValue);

								variableIncrement++;

								// also write this current character
								newFileBuffer[k] = cCurrentChar;
								k++;
							}
							else
							{
								UINT	uStringLen = (UINT)wcslen(variableBuffer);
								if (uStringLen > 0)
								{
									// ignore the string and leave in file as-is
									errno_t errorCode = wcscpy_s(&newFileBuffer[k], uSize, (const wchar_t*)variableBuffer);
									k += uStringLen;
								}
								// also write this current character
								newFileBuffer[k] = cCurrentChar;
								k++;
							}
						}
						else
						{
							// if we are beginning, basically skip over an leading whitespace
							if (true == begin)
							{
								// if it's an alpha char
								if (((UINT)cCurrentChar > 0x40) && ((UINT)cCurrentChar < 0x5B) ||
									((UINT)cCurrentChar > 0x60) && ((UINT)cCurrentChar < 0x7B))
								{
									begin = false;
									// state stays the same
									variableBuffer[variableIndex] = cCurrentChar;
									variableIndex++;
								}
								else
								{
									// state stays the same
									newFileBuffer[k] = cCurrentChar;
									k++;
								}
							}
							else
							{
								// state stays the same
								variableBuffer[variableIndex] = cCurrentChar;
								variableIndex++;
							}
						}
						break;
					}
					}
					// read until close tag
					//
				}
				newFileBuffer[k] = '\0';

				length = wcslen(newFileBuffer);
#if LOCALIZE_SOURCE_FILE_CLOSE
				cSrcFile.Close( );
#endif
				cSqlFile.Close();

				CSrcFile	cNewFile;
				CString	cszNewFile = cszFilePath + L".new";
				cNewFile.SetSrcFileName(cszNewFile);
				cNewFile.SetUnicode(g_bIsUnicodeFile);
				cNewFile.Open();
				cNewFile.Write(newFileBuffer, k);
				cNewFile.Close();

				free((void *)newFileBuffer);
				free((void *)variableBuffer);
			}
			cConfFile.Close();
		}
	}

	return result;
}

ULONGLONG	ReadFileIntoBuffer(const wchar_t* fileName, CString& cszBuffer)
{
	BOOL		checkErrors = FALSE;
	ULONGLONG	uFileLength = 0;
	ULONGLONG	uRead = 0;
	CSrcFile	cSrcFile;

	cSrcFile.SetSrcFileName(fileName);

	checkErrors = cSrcFile.Open();

	g_bIsUnicodeFile = cSrcFile.IsFileUnicodeText();

	// if we can't open these files, no point in going on
	if (checkErrors == FALSE)
	{
		wprintf(L"error opening file: %s\r\n", fileName);
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
bool	WriteSqlCommand(CSrcFile& cFile, UINT sectionId, const wchar_t* variableText, const wchar_t* valueText)
{
	bool result = true;

	CString	cszSectionId = AtoiX(sectionId);

	// first insert into var

	CString	cSql = L"INSERT INTO string( Var, id_section) values ('";
	cSql += variableText;
	cSql += L"', '";
	cSql += cszSectionId;
	cSql += L"');\n";
	cFile.WriteString(cSql);

	g_uVarCount++;
	wchar_t*	variableCountText = AtoiX(g_uVarCount);

	cSql = L"INSERT INTO language_string( id_lang, id_var, string) values ('1', '";
	cSql += variableCountText;
	cSql += L"', '";
	cSql += valueText;
	cSql += L"');\n";
	cFile.WriteString(cSql);

	cSql = L"INSERT INTO language_string( id_lang, id_var, string) values ('2', '";
	cSql += variableCountText;
	cSql += L"', '";
	cSql += valueText;
	cSql += L"');\n";
	cFile.WriteString(cSql);

	return result;
}

///////////////////////////////////////////////////////////////////////
// WriteSqlSection
///////////////////////////////////////////////////////////////////////
bool	WriteSqlSection(CSrcFile& cSqlFile, const wchar_t* sectionName)
{
	bool result = true;

	CString	cSql = L"INSERT INTO language_section(Section) values ('";
	cSql += sectionName;
	cSql += L"');\n";

	cSqlFile.WriteString(cSql);

	return result;
}

///////////////////////////////////////////////////////////////////////
// WriteSqlSectionBegin
///////////////////////////////////////////////////////////////////////
bool	WriteSqlSectionBegin(CSrcFile& cFile)
{
	bool result = true;

	CString	cSql = L"DROP TABLE language_section;\n";
	cFile.WriteString(cSql);

	cSql = L"CREATE TABLE `language_section`\n";
	cFile.WriteString(cSql);
	cSql = L"(\n";
	cFile.WriteString(cSql);
	cSql = L"  `id` int(4) unsigned NOT NULL auto_increment,\n";
	cFile.WriteString(cSql);
	cSql = L"  `Section` varchar(64) NOT NULL default '',\n";
	cFile.WriteString(cSql);
	cSql = L"  PRIMARY KEY (id) \n";
	cFile.WriteString(cSql);
	cSql = L") TYPE=MyISAM;\n";
	cFile.WriteString(cSql);

	return result;
}

Localize::Localize()
{
}


Localize::~Localize()
{
}
