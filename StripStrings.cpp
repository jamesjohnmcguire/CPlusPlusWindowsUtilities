///////////////////////////////////////////////////////////////////////
// <FileName>
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "stdafx.h"

///////////////////////////////////////////////////////////////////////
// globals
///////////////////////////////////////////////////////////////////////
ULONG	g_uSectionCount;

///////////////////////////////////////////////////////////////////////
// Support function prototypes
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// functions
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// <FunctionName>
// options
///////////////////////////////////////////////////////////////////////
// StripStrings.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include "StripStrings.h"

ULONG	g_uVarCount;

#ifdef _DEBUG
	#define new DEBUG_NEW

//	// Comment out for real build
//	#define UNIT_TEST
#endif

#define UNIT_TEST

#include "SrcFile.h"

TCHAR	g_szVarNumber[10];
TCHAR	g_szBufferTmp[128];
TCHAR	g_szDebug[128];

PTSTR	MakeNewVarName( LPCTSTR pszOrigVarName, int nIndex )
{
	CString		cTmp( pszOrigVarName );
	
	_itot( nIndex, g_szVarNumber, 10);

	_tcscpy(  g_szBufferTmp, pszOrigVarName );
	_tcscat( g_szBufferTmp, g_szVarNumber );
//	cTmp =+ &g_szVarNumber;
//	PTSTR	pszTmp	= cTmp.GetBuffer( 64 );
//	cTmp.ReleaseBuffer
	return g_szBufferTmp;
}

PTSTR	MakeNewVarNameEx( int nIndex )
{
	_itot( nIndex, g_szVarNumber, 10);

	_tcscpy(  g_szBufferTmp, _T("Var") );
	_tcscat( g_szBufferTmp, g_szVarNumber );
	return g_szBufferTmp;
}

PTSTR	AtoiX( int nIndex )
{
	_itot( nIndex, g_szVarNumber, 10);

	return g_szVarNumber;
}

// this needs to be improved!
bool IsJavaScript(PTSTR pszStream )
{
	bool	bRet	= false;

	while ((UINT)*pszStream < 0x21)
		pszStream++;

	// if it matches 'function' let's assume it's javascript.
	int	nRes	= _tcsnicmp( pszStream, _T("function"), 8);

	if (0 == nRes)
		bRet	= true;	

	return bRet;
}

bool	IsLegitemate( PTSTR pszStream )
{
	bool	bRet	= false;
	
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

TCHAR	getNextNonWhiteSpaceChar( PTSTR pszStream )
{
	while ((UINT)*pszStream < 0x21)
		pszStream++;
	return *pszStream;
}
bool	WriteSqlSectionBegin( CSrcFile& cFile )
{
	bool bRet	= true;

	CString	cSql	= _T("DROP TABLE language_section;\n");
	cFile.WriteString( cSql );	

	cSql	= _T("CREATE TABLE `language_section`\n");
	cFile.WriteString( cSql );	
	cSql	= _T("(\n");
	cFile.WriteString( cSql );	
	cSql	= _T("  `id` int(4) unsigned NOT NULL auto_increment,\n");
	cFile.WriteString( cSql );	
	cSql	= _T("  `Section` varchar(64) NOT NULL default '',\n");
	cFile.WriteString( cSql );	
	cSql	= _T("  PRIMARY KEY (id) \n");
	cFile.WriteString( cSql );	
	cSql	= _T(") TYPE=MyISAM;\n");
	cFile.WriteString( cSql );	
	
	return bRet;
}

bool	WriteSqlSection( CSrcFile& cSqlFile, LPCTSTR pszSection )
{
	bool bRet	= true;

	CString	cSql	= _T("INSERT INTO language_section(Section) values ('");
	cSql	+= pszSection;
	cSql	+= _T("');\n");

	cSqlFile.WriteString( cSql );	
	
	return bRet;
}
UINT	NewLineCount( LPCTSTR pBuffer )
{
	UINT	uCount = 0;
	int		nPos	= 0;	
	CString	cszBuffer = pBuffer;

	nPos	= 	cszBuffer.Find( '\n', nPos );
	while (-1 != nPos)
	{
		uCount++;
		nPos	= 	cszBuffer.Find( '\n', nPos+1 );
	}
	
	return	uCount;
}
// In theory, we should know the id, since it is in sync with our var count, section count
bool	WriteSqlCommand( CSrcFile& cFile, UINT nSectionId, LPCTSTR pszVar, LPCTSTR pszValue )
{
	bool bRet	= true;

	CString	cszSectionId	= AtoiX( nSectionId);

	// first insert into var
	
	CString	cSql	= _T("INSERT INTO string( Var, id_section) values ('");
	cSql	+= pszVar;
	cSql	+= _T("', '");
	cSql	+= cszSectionId;
	cSql	+= _T("');\n");
	cFile.WriteString( cSql );	

	g_uVarCount++;
	PTSTR	pszVarCount		= AtoiX( g_uVarCount );
	
	cSql	= _T("INSERT INTO language_string( id_lang, id_var, string) values ('1', '");
	cSql	+= pszVarCount;
	cSql	+= _T("', '");
	cSql	+= pszValue;
	cSql	+= _T("');\n");
	cFile.WriteString( cSql );	

	cSql	= _T("INSERT INTO language_string( id_lang, id_var, string) values ('2', '");
	cSql	+= pszVarCount;
	cSql	+= _T("', '");
	cSql	+= pszValue;
	cSql	+= _T("');\n");
	cFile.WriteString( cSql );	

	return bRet;
}

bool	ProcessFile( LPCTSTR pszFileName )
{
	bool bRet	= true;
	bool bBegin	= false;

	CSrcFile	cSrcFile;
	CSrcFile	cConfFile;
	CSrcFile	cSqlFile;
	int			nState	= 0;	// 0 = in tag
								// 1 = in smarty tag
								// 2 = not in tag (in string)
	UINT	nVarIncrement	= 1;	// the number of vars
	UINT	nVarIndex		= 0;
	UINT k	= 0;

	CString	cszFilePath( pszFileName );
	CString	cszFileName;
	CString	cOrigVarName;
	CString	cVarName;
	CString cVarBuffer;
	
	int intDelimiterIndex = cszFilePath.GetLength( ) - cszFilePath.ReverseFind(_TEXT('\\'));
	
	cszFileName = cszFilePath.Right( intDelimiterIndex - 1 );

	intDelimiterIndex = cszFileName.ReverseFind(_TEXT('.'));
	cOrigVarName = cszFileName.Left( intDelimiterIndex  );
	CString	cszSectionName	= cszFileName.Left( intDelimiterIndex  );
//	cOrigVarName	+= _T(":::Var");
	cOrigVarName	= _T("Var");

	cSrcFile.SetSrcFileName( cszFilePath );
	CString	cConfFileName = cszFilePath + _T(".conf");
	CString	cSqlFileName = _T("UpdateLanguage.sql");

	cConfFile.SetSrcFileName( cConfFileName );
	cSqlFile.SetSrcFileName( cSqlFileName );

	cSrcFile.Open( );
	cConfFile.Open( );
	cSqlFile.Open( );
	cSqlFile.SeekToEnd( );

	ULONG nLength	= (ULONG)cSrcFile.GetLength( );
	PTSTR		pFileBufOld	= (PTSTR)malloc( (nLength + 1) * sizeof (TCHAR));
	PTSTR		pFileBufNew	= (PTSTR)malloc( (nLength + 1) * sizeof (TCHAR) * 2);
	PTSTR		pVarBuf		= (PTSTR)malloc( (nLength + 1) * sizeof (TCHAR));

	cSrcFile.Read( pFileBufOld, nLength );
	cSrcFile.Close( );

	UINT	uNewLines = NewLineCount( pFileBufOld );
	
	nLength  = nLength - uNewLines;
//	int nFind	= cszSectionName.Find( _T("va_") );

//	if (nFind != -1)
//		cszSectionName = cszSectionName.Right( cszSectionName.GetLength( ) - 3);
		
	WriteSqlSection( cSqlFile, cszSectionName );

	// for length of file
	// for each character
	// do the following
	
	for (ULONG i=0; i < nLength; i++)
	{
		_tcsncpy( g_szDebug, &pFileBufOld[i], 120 );
		g_szDebug[121] = '\0';
		switch (nState)
		{
			// in a html tag
			case 0:
			{
				if ( '>' == pFileBufOld[i])
				{
					TCHAR	cNextChar = getNextNonWhiteSpaceChar( &pFileBufOld[i+1] );
					
					switch (cNextChar)
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
							bool	bRet	= IsJavaScript( &pFileBufOld[i+1] );
							
							if (true == bRet)
								// it will change after the next '>'
								nState = 0;
							else
							{
								bBegin = true;

								if (cNextChar != pFileBufOld[i+1])
								{
									// we have non alpha chars to get past
								}									
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
				pFileBufNew[k] = pFileBufOld[i];
				k++;
				break;
			}
			// in a smarty tag
			case 1:
			{
				if ( '}' == pFileBufOld[i])
				{
					TCHAR	cNextChar = getNextNonWhiteSpaceChar( &pFileBufOld[i+1] );
					
					switch (cNextChar)
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
							bool	bRet	= IsJavaScript( &pFileBufOld[i+1] );
							
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
				pFileBufNew[k] = pFileBufOld[i];
				k++;
				break;
			}
			// Already in getvar state
			case 2:
			{
				if (( '<' == pFileBufOld[i]) || ( '{' == pFileBufOld[i]))
				{
					// ok, finish and close the var capturing
					nState = 0;

					pVarBuf[nVarIndex] = '\0';

					bool bOk	= IsLegitemate( pVarBuf );

					CString cVarValue = pVarBuf;
					
					cVarValue.Trim( );
					if (true == bOk)
					{
					// write the string into the conf file
//						CString cszLine	= cVarName + _T(":::") + pVarBuf;
						CString cszLine	= cVarName + _T(":::") + cVarValue;

						cszLine += _T("\n");
						cConfFile.WriteString( cszLine );
						
						cVarBuffer = _T("{#") + cVarName + _T("#}");

						// write the variable into the new file
						_tcscpy(&pFileBufNew[k], (LPCTSTR)cVarBuffer);
						k += cVarBuffer.GetLength( );

						//
						PTSTR	pszVarName	= MakeNewVarNameEx( nVarIncrement ) ;
						
						cVarValue.Replace(_T("'"), _T("\\'"));
						WriteSqlCommand( cSqlFile, g_uSectionCount, pszVarName, cVarValue );

						nVarIncrement++;
						
						// also write this current character
						pFileBufNew[k] = pFileBufOld[i];
						k++;
					}
					else
					{
						// ignore the string and leave in file as-is
						_tcscpy(&pFileBufNew[k], (LPCTSTR)pVarBuf);
						UINT	uStringLen	= (UINT)_tcsclen( pVarBuf );
						k += uStringLen;

						// also write this current character
						pFileBufNew[k] = pFileBufOld[i];
						k++;
					}
				}
				else
				{
					// if we are beginning, basically skip over an leading whitespace
					if (true == bBegin)
					{
						// if it's an alpha char
						if (((UINT)pFileBufOld[i] > 0x40) && ((UINT)pFileBufOld[i] < 0x5B) ||
							((UINT)pFileBufOld[i] > 0x60) && ((UINT)pFileBufOld[i] < 0x7B))
						{
							bBegin = false;
							// state stays the same
							pVarBuf[nVarIndex] = pFileBufOld[i];
							nVarIndex++;
						}
						else
						{
							// state stays the same
							pFileBufNew[k] = pFileBufOld[i];
							k++;
						}
					}
					else
					{
						// state stays the same
						pVarBuf[nVarIndex] = pFileBufOld[i];
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

	nLength	= _tcsclen( pFileBufNew );
//		cSrcFile.Close( );
	cConfFile.Close( );	
	cSqlFile.Close( );

	CSrcFile	cNewFile;
	CString	cszNewFile = cszFilePath + _T(".new");
	cNewFile.SetSrcFileName( cszNewFile );
	cNewFile.Open( );
	cNewFile.Write( pFileBufNew, k );
	cNewFile.Close( );

	free( (void *)pFileBufNew );
	free( (void *)pFileBufOld );
	free( (void *)pVarBuf );

	return bRet;
}

int MainInit()
{
	int nRetCode = EXIT_FAILURE;
	return nRetCode;
}

void StripStrings(TCHAR* files)
{
	CStringArray	afilePaths;
	//		CString	cszFileTypes	= m_cszDir + _T("\\*.tpl");
	//		CString	cszFileTypes	= _T("\\");
	CString	cszFileTypes = files;
	UINT	uFileCount = 0;
	// Get founder count via find first...next of *.dbx
	CFileFind finder;

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
}
