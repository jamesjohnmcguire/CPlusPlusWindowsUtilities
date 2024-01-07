/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2024 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "SrcFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*static*/ const UINT CSrcFile::modeWriteUnicode = 0x20000; // Add this flag to write in Unicode

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSrcFile::CSrcFile()
{
	bIsUnicodeText = false;
	nFileCodePage = -1;

	CStdioFile::CStdioFile( );
}

CSrcFile::CSrcFile(LPCTSTR lpszFileName, UINT nOpenFlags)
	:CStdioFile(lpszFileName, ProcessFlags(lpszFileName, nOpenFlags))
{
	nFileCodePage = -1;

	CStdioFile::CStdioFile();
}

CSrcFile::~CSrcFile()
{

}

BOOL CSrcFile::Open()
{
	BOOL	bReturn = FALSE;

	if (!cszPathName.IsEmpty())
	{
		//		UINT nOpenFlags =	CFile::typeText |
		UINT nOpenFlags = CFile::typeBinary |
			CFile::modeCreate |
			CFile::modeNoTruncate |
			CFile::modeReadWrite |
			CFile::shareDenyNone;

		// Process any Unicode stuff
		ProcessFlags(cszPathName, nOpenFlags);

		bReturn = CStdioFile::Open(cszPathName,
			nOpenFlags);
	}
	return bReturn;
}

BOOL CSrcFile::Open(
	LPCTSTR lpszFileName,
	UINT nOpenFlags,
	CFileException* pError /*=NULL*/)
{
	// Process any Unicode stuff
	ProcessFlags(lpszFileName, nOpenFlags);

	return CStdioFile::Open(lpszFileName, nOpenFlags, pError);
}

UINT CSrcFile::ProcessFlags(const CString& sFilePath, UINT& nOpenFlags)
{
	bIsUnicodeText = false;

	// If we have writeUnicode we must have write or writeRead as well
#ifdef _DEBUG
	if (nOpenFlags & CSrcFile::modeWriteUnicode)
	{
		ASSERT(nOpenFlags & CFile::modeWrite || nOpenFlags & CFile::modeReadWrite);
	}
#endif

	// If reading in text mode and not creating... ; fixed by Dennis Jeryd 6/8/03
//	BOOL bTypeText =  nOpenFlags & CFile::typeText;
	BOOL bCreate = nOpenFlags & CFile::modeCreate;
	BOOL bNoTruncate = nOpenFlags & CFile::modeNoTruncate;
	BOOL bModeWrite = nOpenFlags & CFile::modeWrite;

	//	if (bTypeText && (!bCreate || bNoTruncate) && !bModeWrite)
	if ((!bCreate || bNoTruncate) && !bModeWrite)
	{
		bIsUnicodeText = IsFileUnicode(sFilePath);

		// If it's Unicode, switch to binary mode
		if (bIsUnicodeText)
		{
			//nOpenFlags ^= CFile::typeText;
			nOpenFlags |= CFile::typeBinary;
		}
	}

	nFlags = nOpenFlags;

	return nOpenFlags;
}

size_t	CSrcFile::Read(LPCTSTR rString, UINT nCount)
{
	CString	cstring = rString;

	return Read(cstring, nCount);
}

size_t CSrcFile::Read(CString& rString, UINT nCount)
{
	const int	nMAX_LINE_CHARS = 4096;
	BOOL		bReadData = FALSE;
	int			nLen = 0;
	int			nChars = 0;
	size_t		nCharsRead = -1;

	LPTSTR		lpsz;
	wchar_t*	pszUnicodeString = NULL;
	char	*	pszMultiByteString = NULL;

	try
	{
		// If at position 0, discard byte-order mark before reading
		if (!m_pStream || (GetPosition() == 0 && bIsUnicodeText))
		{
			wchar_t	cDummy;
			//		Read(&cDummy, sizeof(_TCHAR));
			CFile::Read(&cDummy, sizeof(wchar_t));
		}

		// If compiled for Unicode
#ifdef _UNICODE
		if (bIsUnicodeText)
		{
			// Do standard stuff - Unicode to Unicode. Seems to work OK.
			LPTSTR	pszBuffer = rString.GetBufferSetLength(nCount);
			nCharsRead = CFile::Read(pszBuffer, nCount);
			rString.ReleaseBuffer();
		}
		else
		{
			pszUnicodeString = new wchar_t[nCount];
			pszMultiByteString = new char[nCount];

			// Initialise to something safe
			memset(pszUnicodeString, 0, sizeof(wchar_t) * nCount);
			memset(pszMultiByteString, 0, sizeof(char) * nCount);

			// Read the string
			nCharsRead = fread(pszMultiByteString, 1, nCount, m_pStream);

			if (nCharsRead > 0)
			{
				bReadData = TRUE;

				// Convert multibyte to Unicode, using the specified code page
				nChars = GetUnicodeStringFromMultiByteString(pszMultiByteString, pszUnicodeString, nCount, nFileCodePage);

				if (nChars > 0)
				{
					//					rString = (CString)pszUnicodeString;
					rString = pszMultiByteString;
				}
			}
		}
#else

		if (!bIsUnicodeText)
		{
			// Do standard stuff -- read ANSI in ANSI
			bReadData = CStdioFile::ReadString(rString);

			// Get the current code page
			UINT nLocaleCodePage = GetCurrentLocaleCodePage();

			// If we got it OK...
			if (nLocaleCodePage > 0)
			{
				// if file code page does not match the system code page, we need to do a double conversion!
				if (nLocaleCodePage != (UINT)nFileCodePage)
				{
					int nStringBufferChars = rString.GetLength() + 1;

					pszUnicodeString = new wchar_t[nStringBufferChars];

					// Initialise to something safe
					memset(pszUnicodeString, 0, sizeof(wchar_t) * nStringBufferChars);

					// Convert to Unicode using the file code page
					nChars = GetUnicodeStringFromMultiByteString(rString, pszUnicodeString, nStringBufferChars, nFileCodePage);

					// Convert back to multibyte using the system code page
					// (This doesn't really confer huge advantages except to avoid "mangling" of non-convertible special
					// characters. So, if a file in the E.European code page is displayed on a system using the 
					// western European code page, special accented characters which the system cannot display will be
					// replaced by the default character (a hash or something), rather than being incorrectly mapped to
					// other, western European accented characters).
					if (nChars > 0)
					{
						// Calculate how much we need for the MB buffer (it might be larger)
						nStringBufferChars = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nLocaleCodePage);
						pszMultiByteString = new char[nStringBufferChars];

						nChars = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nStringBufferChars, nLocaleCodePage);
						rString = (CString)pszMultiByteString;
					}
				}
			}
		}
		else
		{
			pszUnicodeString = new wchar_t[nMAX_LINE_CHARS];

			// Initialise to something safe
			memset(pszUnicodeString, 0, sizeof(wchar_t) * nMAX_LINE_CHARS);

			// Read as Unicode, convert to ANSI

			// Bug fix by Dennis Jeryd 06/07/2003: initialise bReadData
			bReadData = (NULL != fgetws(pszUnicodeString, nMAX_LINE_CHARS, m_pStream));

			if (bReadData)
			{
				// Calculate how much we need for the multibyte string
				int nRequiredMBBuffer = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nFileCodePage);
				pszMultiByteString = new char[nRequiredMBBuffer];

				nChars = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nRequiredMBBuffer, nFileCodePage);

				if (nChars > 0)
				{
					rString = (CString)pszMultiByteString;
				}
			}

		}
#endif

		// Then remove end-of-line character if in Unicode text mode
		if (bReadData)
		{
			// Copied from FileTxt.cpp but adapted to Unicode and then adapted for end-of-line being just '\r'. 
			nLen = rString.GetLength();
			if (nLen > 1 && rString.Mid(nLen - 2) == NEWLINE)
			{
				rString.GetBufferSetLength(nLen - 2);
			}
			else
			{
				lpsz = rString.GetBuffer(0);
				if (nLen != 0 && (lpsz[nLen - 1] == _T('\r') || lpsz[nLen - 1] == _T('\n')))
				{
					rString.GetBufferSetLength(nLen - 1);
				}
			}
		}
	}
	// Ensure we always delete in case of exception
	catch (CException *ex)
	{
		//int nCount;
		CString cError;
		LPTSTR pszError = cError.GetBuffer();
		_tprintf(_T("Exception: %s\r\n"), ex->GetErrorMessage(pszError, 3));
		cError.ReleaseBuffer();
		if (pszUnicodeString)	delete[] pszUnicodeString;

		if (pszMultiByteString) delete[] pszMultiByteString;

		throw;
	}

	if (pszUnicodeString)		delete[] pszUnicodeString;

	if (pszMultiByteString)		delete[] pszMultiByteString;

	return nCharsRead;
}

BOOL CSrcFile::ReadString(CString& rString)
{
	const int	nMAX_LINE_CHARS = 4096;
	BOOL			bReadData = FALSE;
	LPTSTR		lpsz;
	int			nLen = 0;
	wchar_t*		pszUnicodeString = NULL;
	char	*		pszMultiByteString = NULL;
	int			nChars = 0;

	try
	{
		// If at position 0, discard byte-order mark before reading
		if (!m_pStream || (GetPosition() == 0 && bIsUnicodeText))
		{
			wchar_t	cDummy;
			//		Read(&cDummy, sizeof(_TCHAR));
			CFile::Read(&cDummy, sizeof(wchar_t));
		}

		// If compiled for Unicode
#ifdef _UNICODE
		if (bIsUnicodeText)
		{
			// Do standard stuff - Unicode to Unicode. Seems to work OK.
			bReadData = CStdioFile::ReadString(rString);
		}
		else
		{
			pszUnicodeString = new wchar_t[nMAX_LINE_CHARS];
			pszMultiByteString = new char[nMAX_LINE_CHARS];

			// Initialise to something safe
			memset(pszUnicodeString, 0, sizeof(wchar_t) * nMAX_LINE_CHARS);
			memset(pszMultiByteString, 0, sizeof(char) * nMAX_LINE_CHARS);

			// Read the string
			bReadData = (NULL != fgets(pszMultiByteString, nMAX_LINE_CHARS, m_pStream));

			if (bReadData)
			{
				// Convert multibyte to Unicode, using the specified code page
				nChars = GetUnicodeStringFromMultiByteString(pszMultiByteString, pszUnicodeString, nMAX_LINE_CHARS, nFileCodePage);

				if (nChars > 0)
				{
					rString = (CString)pszUnicodeString;
				}
			}
		}
#else

		if (!bIsUnicodeText)
		{
			// Do standard stuff -- read ANSI in ANSI
			bReadData = CStdioFile::ReadString(rString);

			// Get the current code page
			UINT nLocaleCodePage = GetCurrentLocaleCodePage();

			// If we got it OK...
			if (nLocaleCodePage > 0)
			{
				// if file code page does not match the system code page, we need to do a double conversion!
				if (nLocaleCodePage != (UINT)nFileCodePage)
				{
					int nStringBufferChars = rString.GetLength() + 1;

					pszUnicodeString = new wchar_t[nStringBufferChars];

					// Initialise to something safe
					memset(pszUnicodeString, 0, sizeof(wchar_t) * nStringBufferChars);

					// Convert to Unicode using the file code page
					nChars = GetUnicodeStringFromMultiByteString(rString, pszUnicodeString, nStringBufferChars, nFileCodePage);

					// Convert back to multibyte using the system code page
					// (This doesn't really confer huge advantages except to avoid "mangling" of non-convertible special
					// characters. So, if a file in the E.European code page is displayed on a system using the 
					// western European code page, special accented characters which the system cannot display will be
					// replaced by the default character (a hash or something), rather than being incorrectly mapped to
					// other, western European accented characters).
					if (nChars > 0)
					{
						// Calculate how much we need for the MB buffer (it might be larger)
						nStringBufferChars = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nLocaleCodePage);
						pszMultiByteString = new char[nStringBufferChars];

						nChars = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nStringBufferChars, nLocaleCodePage);
						rString = (CString)pszMultiByteString;
					}
				}
			}
		}
		else
		{
			pszUnicodeString = new wchar_t[nMAX_LINE_CHARS];

			// Initialise to something safe
			memset(pszUnicodeString, 0, sizeof(wchar_t) * nMAX_LINE_CHARS);

			// Read as Unicode, convert to ANSI

			// Bug fix by Dennis Jeryd 06/07/2003: initialise bReadData
			bReadData = (NULL != fgetws(pszUnicodeString, nMAX_LINE_CHARS, m_pStream));

			if (bReadData)
			{
				// Calculate how much we need for the multibyte string
				int nRequiredMBBuffer = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nFileCodePage);
				pszMultiByteString = new char[nRequiredMBBuffer];

				nChars = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nRequiredMBBuffer, nFileCodePage);

				if (nChars > 0)
				{
					rString = (CString)pszMultiByteString;
				}
			}

		}
#endif

		// Then remove end-of-line character if in Unicode text mode
		if (bReadData)
		{
			// Copied from FileTxt.cpp but adapted to Unicode and then adapted for end-of-line being just '\r'. 
			nLen = rString.GetLength();
			if (nLen > 1 && rString.Mid(nLen - 2) == NEWLINE)
			{
				rString.GetBufferSetLength(nLen - 2);
			}
			else
			{
				lpsz = rString.GetBuffer(0);
				if (nLen != 0 && (lpsz[nLen - 1] == _T('\r') || lpsz[nLen - 1] == _T('\n')))
				{
					rString.GetBufferSetLength(nLen - 1);
				}
			}
		}
	}
	// Ensure we always delete in case of exception
	catch (...)
	{
		if (pszUnicodeString)	delete[] pszUnicodeString;

		if (pszMultiByteString) delete[] pszMultiByteString;

		throw;
	}

	if (pszUnicodeString)		delete[] pszUnicodeString;

	if (pszMultiByteString)		delete[] pszMultiByteString;

	return bReadData;
}

void CSrcFile::SetCodePage(IN const UINT nCodePage)
{
	nFileCodePage = (int)nCodePage;
}

void CSrcFile::SetClassName(LPCTSTR pszClassName)
{
	cszClassName  = pszClassName;
}

bool CSrcFile::SetLogPath(LPCTSTR pszPath)
{
	bool	bRet	= false;

	if (pszPath && *pszPath != '\0')
	{
		cszPath   = pszPath;

		bRet	= true;
	}

	return bRet;
}

bool CSrcFile::SetSrcFileName( LPCTSTR pszName /* = NULL */)
{
	bool	bRet	= false;

	if (NULL != pszName)
	{
		cszPathName = pszName;
		SetFilePath( pszName );	
		bRet	= true;
	}
	else
	{
		if (cszPath.IsEmpty())
			SetDefaultPath();

		int intDelimiterIndex = cszPathName.GetLength() - cszPathName.ReverseFind(_TEXT('\\'));

		cszPathName.Right(intDelimiterIndex);

		intDelimiterIndex = cszPathName.ReverseFind(_TEXT('.'));

		cszPathName.Left(intDelimiterIndex);
		cszPathName += _T(".log");
	}

	return bRet;
}

void	CSrcFile::SetUnicode(bool bUnicode)
{
	if (bUnicode == true)
	{
		bIsUnicodeText = true;

		nFlags = nFlags | CSrcFile::modeWriteUnicode;
	}
	else
	{
		bIsUnicodeText = false;

		nFlags = nFlags ^ CSrcFile::modeWriteUnicode;
	}
}

// --------------------------------------------------------------------------------------------
//
//	CSrcFile::IsFileUnicode()
//
// --------------------------------------------------------------------------------------------
// Returns:    bool
// Parameters: const CString& sFilePath
//
// Purpose:		Determines whether a file is Unicode by reading the first character and detecting
//					whether it's the Unicode byte marker.
// Notes:		None.
// Exceptions:	None.
//
/*static*/ bool CSrcFile::IsFileUnicode(const CString& sFilePath)
{
	CFile				file;
	bool				bIsUnicode = false;
	wchar_t			cFirstChar;
	CFileException	exFile;

	// Open file in binary mode and read first character
	if (file.Open(sFilePath, CFile::typeBinary | CFile::modeRead, &exFile))
	{
		// If byte is Unicode byte-order marker, let's say it's Unicode
		if (file.Read(&cFirstChar, sizeof(wchar_t)) > 0 && cFirstChar == (wchar_t)UNICODE_BOM)
		{
			bIsUnicode = true;
		}

		file.Close();
	}
	else
	{
		// Handle error here if you like
	}

	return bIsUnicode;
}

bool CSrcFile::GetLastErrorInfo(void)
{
	bool	bRet = false;

	LPVOID	pMsgBuffer;
	DWORD	dwErr = GetLastError();
	DWORD	dwRet;

	dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&pMsgBuffer),
		0,
		NULL);

	if (pMsgBuffer)
	{
		OutputDebugString(reinterpret_cast<LPTSTR>(pMsgBuffer));
		_tprintf(_T("Error: %s\r\n"), reinterpret_cast<LPTSTR>(pMsgBuffer));
		LocalFree(pMsgBuffer);
	}

	return bRet;
}

unsigned long CSrcFile::GetCharCount()
{
	int				nCharSize;
	ULONGLONG		nByteCount = 0;
	unsigned long	nCharCount = 0;

	if (m_pStream)
	{
		// Get size of chars in file
		nCharSize = bIsUnicodeText ? sizeof(wchar_t) : sizeof(char);

		// If Unicode, remove byte order mark from count
		nByteCount = GetLength();

		if (bIsUnicodeText)
		{
			nByteCount = nByteCount - sizeof(wchar_t);
		}

		// Calc chars
		nCharCount = ((size_t)nByteCount / nCharSize);
	}

	return nCharCount;
}

// Get the current userLs code page
UINT CSrcFile::GetCurrentLocaleCodePage()
{
	_TCHAR	szLocalCodePage[10];
	UINT		nLocaleCodePage = 0;
	int		nLocaleChars = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, szLocalCodePage, 10);

	// If OK
	if (nLocaleChars > 0)
	{
		nLocaleCodePage = (UINT)_ttoi(szLocalCodePage);
		ASSERT(nLocaleCodePage > 0);
	}
	else
	{
		ASSERT(false);
	}

	// O means either: no ANSI code page (Unicode-only locale?) or failed to get locale
	// In the case of Unicode-only locales, what do multibyte apps do? Answers on a postcard.
	return nLocaleCodePage;
}

// --------------------------------------------------------------------------------------------
//
//	CSrcFile::GetUnicodeStringFromMultiByteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - num. of chars written (0 means error)
// Parameters: char *		szMultiByteString		(IN)		Multi-byte input string
//					wchar_t*		szUnicodeString		(OUT)		Unicode outputstring
//					int			nUnicodeBufferSize	(IN)		Size of Unicode output buffer in chars(IN)
//					UINT			nCodePage				(IN)		Code page used to perform conversion
//																			Default = -1 (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
// Notes:		None.
// Exceptions:	None.
//
int CSrcFile::GetUnicodeStringFromMultiByteString(IN LPCSTR szMultiByteString, OUT wchar_t* szUnicodeString, IN int nUnicodeBufferSize, IN int nCodePage)
{
	bool		bOK = true;
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		try
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			nCharsWritten = MultiByteToWideChar((UINT)nCodePage,
				(nCodePage == CP_UTF8 ? 0 : MB_PRECOMPOSED), // Flags
				szMultiByteString,
				-1,
				szUnicodeString,
				0);
			//													nUnicodeBufferSize+1);

			if (nCharsWritten < 1)
				GetLastErrorInfo();

		}
		catch (CException *ex)
		{
			GetLastErrorInfo();

			//int nCount;
			CString cError;
			LPTSTR pszError = cError.GetBuffer();
			_tprintf(_T("Exception: %s\r\n"), ex->GetErrorMessage(pszError, 3));
			cError.ReleaseBuffer();
			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}

	ASSERT(nCharsWritten > 0);
	return nCharsWritten;
}

// --------------------------------------------------------------------------------------------
//
//	CSrcFile::GetMultiByteStringFromUnicodeString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - number of characters written. 0 means error
// Parameters: wchar_t *	szUnicodeString			(IN)	Unicode input string
//					char*			szMultiByteString			(OUT)	Multibyte output string
//					int			nMultiByteBufferSize		(IN)	Multibyte buffer size (chars)
//					UINT			nCodePage					(IN)	Code page used to perform conversion
//																			Default = -1 (Get local code page).
//
// Purpose:		Gets a MultiByte string from a Unicode string
// Notes:		Added fix by Andy Goodwin: make buffer into int.
// Exceptions:	None.
//
int CSrcFile::GetMultiByteStringFromUnicodeString(wchar_t * szUnicodeString, char* szMultiByteString,
	int nMultiByteBufferSize, int nCodePage)
{
	BOOL		bUsedDefChar = FALSE;
	int		nCharsWritten = 0;

	// Fix by Andy Goodwin: don't do anything if buffer is 0
	if (nMultiByteBufferSize > 0)
	{
		if (szUnicodeString && szMultiByteString)
		{
			// Zero out buffer first
			memset((void*)szMultiByteString, '\0', nMultiByteBufferSize);

			// If no code page specified, take default for system
			if (nCodePage == -1)
			{
				nCodePage = GetACP();
			}

			try
			{
				// If writing to UTF8, flags, default char and boolean flag must be NULL
				nCharsWritten = WideCharToMultiByte((UINT)nCodePage,
					(nCodePage == CP_UTF8 ? 0 : WC_COMPOSITECHECK | WC_SEPCHARS), // Flags
					szUnicodeString, -1,
					szMultiByteString,
					nMultiByteBufferSize,
					(nCodePage == CP_UTF8 ? NULL : DEFAULT_UNICODE_FILLER_CHAR),	// Filler char
					(nCodePage == CP_UTF8 ? NULL : &bUsedDefChar));						// Did we use filler char?

				// If no chars were written and the buffer is not 0, error!
				if (nCharsWritten == 0 && nMultiByteBufferSize > 0)
				{
					TRACE1("Error in WideCharToMultiByte: %d\n", ::GetLastError());
				}
			}
			catch (...)
			{
				TRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
			}
		}
	}

	// Now fix nCharsWritten 
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}

	return nCharsWritten;
}

//---------------------------------------------------------------------------------------------------
//
//	CSrcFile::GetRequiredMultiByteLengthForUnicodeString()
//
//---------------------------------------------------------------------------------------------------
// Returns:    int
// Parameters: wchar_t * szUnicodeString,int nCodePage=-1
//
// Purpose:		Obtains the multi-byte buffer size needed to accommodate a converted Unicode string.
//	Notes:		We can't assume that the buffer length is simply equal to the number of characters
//					because that wouldn't accommodate multibyte characters!
//
/*static*/ int CSrcFile::GetRequiredMultiByteLengthForUnicodeString(wchar_t * szUnicodeString, int nCodePage /*=-1*/)
{
	int nCharsNeeded = 0;

	try
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		// If writing to UTF8, flags, default char and boolean flag must be NULL
		nCharsNeeded = WideCharToMultiByte((UINT)nCodePage,
			(nCodePage == CP_UTF8 ? 0 : WC_COMPOSITECHECK | WC_SEPCHARS), // Flags
			szUnicodeString, -1,
			NULL,
			0,	// Calculate required buffer, please! 
			(nCodePage == CP_UTF8 ? NULL : DEFAULT_UNICODE_FILLER_CHAR),	// Filler char
			NULL);
	}
	catch (...)
	{
		TRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
	}

	return nCharsNeeded;
}

int	CSrcFile::Write(LPCTSTR pszString, int nCharsToWrite)
{
	wchar_t*	pszUnicodeString = NULL;
	char*		pszMultiByteString = NULL;
	int			nCharsWritten = -1;

	try
	{
		// If writing Unicode and at the start of the file, need to write byte mark
		if (nFlags & CSrcFile::modeWriteUnicode)
		{
			// If at position 0, write byte-order mark before writing anything else
			if (!m_pStream || GetPosition() == 0)
			{
				wchar_t cBOM = (wchar_t)UNICODE_BOM;
				CFile::Write(&cBOM, sizeof(wchar_t));
				nCharsWritten = 1;
			}
		}

		// If compiled in Unicode...
#ifdef _UNICODE

		// If writing Unicode, no conversion needed
		if (nFlags & CSrcFile::modeWriteUnicode)
		{
			// Write in byte mode
			CFile::Write(pszString, nCharsToWrite * sizeof(wchar_t));
			nCharsWritten = 1;
		}
		// Else if we don't want to write Unicode, need to convert
		else
		{
			int		nChars = nCharsToWrite + 1;				// Why plus 1? Because yes
//			int		nBufferSize = nChars * sizeof(char);	// leave space for multi-byte chars
			int		nCharsWritten = 0;
			int		nBufferSize = 0;

			pszUnicodeString = new wchar_t[nChars];

			// Copy string to Unicode buffer
			lstrcpy(pszUnicodeString, pszString);

			// Work out how much space we need for the multibyte conversion
			nBufferSize = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nFileCodePage);
			pszMultiByteString = new char[nBufferSize];

			// Get multibyte string
			nCharsWritten = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nBufferSize, nFileCodePage);

			if (nCharsWritten > 0)
			{
				// Do byte-mode write using actual chars written (fix by Howard J Oh)
	//			CFile::Write((const void*)pszMultiByteString, lstrlen(lpsz));
				CFile::Write((const void*)pszMultiByteString,
					nCharsWritten * sizeof(char));
			}
		}
		// Else if *not* compiled in Unicode
#else
		// If writing Unicode, need to convert
		if (nFlags & CSrcFile::modeWriteUnicode)
		{
			int		nChars = lstrlen(lpsz) + 1;	 // Why plus 1? Because yes
			int		nBufferSize = nChars * sizeof(wchar_t);
			int		nCharsWritten = 0;

			pszUnicodeString = new wchar_t[nChars];
			pszMultiByteString = new char[nChars];

			// Copy string to multibyte buffer
			lstrcpy(pszMultiByteString, lpsz);

			nCharsWritten = GetUnicodeStringFromMultiByteString(pszMultiByteString, pszUnicodeString, nChars, nFileCodePage);

			if (nCharsWritten > 0)
			{
				// Do byte-mode write using actual chars written (fix by Howard J Oh)
	//			CFile::Write(pszUnicodeString, lstrlen(lpsz) * sizeof(wchar_t));
				CFile::Write(pszUnicodeString, nCharsWritten * sizeof(wchar_t));
			}
			else
			{
				ASSERT(false);
			}

		}
		// Else if we don't want to write Unicode, no conversion needed, unless the code page differs
		else
		{
			//		// Do standard stuff
			//		CStdioFile::WriteString(lpsz);

					// Get the current code page
			UINT nLocaleCodePage = GetCurrentLocaleCodePage();

			// If we got it OK, and if file code page does not match the system code page, we need to do a double conversion!
			if (nLocaleCodePage > 0 && nLocaleCodePage != (UINT)nFileCodePage)
			{
				int	nChars = lstrlen(lpsz) + 1;	 // Why plus 1? Because yes

				pszUnicodeString = new wchar_t[nChars];

				// Initialise to something safe
				memset(pszUnicodeString, 0, sizeof(wchar_t) * nChars);

				// Convert to Unicode using the locale code page (the code page we are using in memory)
				nChars = GetUnicodeStringFromMultiByteString((LPCSTR)(const char*)lpsz, pszUnicodeString, nChars, nLocaleCodePage);

				// Convert back to multibyte using the file code page
				// (Note that you can't reliably read a non-Unicode file written in code page A on a system using a code page B,
				// modify the file and write it back using code page A, unless you disable all this double-conversion code.
				// In effect, you have to choose between a mangled character display and mangled file writing).
				if (nChars > 0)
				{
					// Calculate how much we need for the MB buffer (it might be larger)
					nChars = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nFileCodePage);

					pszMultiByteString = new char[nChars];
					memset(pszMultiByteString, 0, sizeof(char) * nChars);

					nChars = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nChars, nFileCodePage);

					// Do byte-mode write. This avoids annoying "interpretation" of \n's as \r\n
					CFile::Write((const void*)pszMultiByteString, nChars * sizeof(char));
				}
			}
			else
			{
				// Do byte-mode write. This avoids annoying "interpretation" of \n's as \r\n
				CFile::Write((const void*)lpsz, lstrlen(lpsz) * sizeof(char));
			}
		}

#endif
	}
	// Ensure we always clean up
	catch (...)
	{
		if (pszUnicodeString)
			delete[] pszUnicodeString;
		if (pszMultiByteString)
			delete[] pszMultiByteString;
		throw;
	}

	if (pszUnicodeString)
		delete[] pszUnicodeString;
	if (pszMultiByteString)
		delete[] pszMultiByteString;

	return nCharsWritten;
}

/*
HRESULT CSrcFile::Write(CString &cszString, bool bIgnore /*= FALSE *)
{
	if (!bIgnore)
	{
		CString cszOutput;

		if (Open( ))
		{
			SeekToEnd( );

			if (!cszClassName.IsEmpty())
				cszOutput   = cszClassName + "::" + cszString;
			else
				cszOutput   = cszString;

				WriteString( cszOutput );

			Close( );
		}
	}
	return TRUE;
}

HRESULT CSrcFile::Write(LPCTSTR pszString, bool bIgnore /* = FALSE *)
{
	CString cszString   = pszString;

	return Write( cszString, bIgnore );
}
*/

// --------------------------------------------------------------------------------------------
//
//	CSrcFile::WriteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    void
// Parameters: LPCTSTR lpsz
//
// Purpose:		Writes string to file either in Unicode or multibyte, depending on whether the caller specified the
//					CSrcFile::modeWriteUnicode flag. Override of base class function.
// Notes:		If writing in Unicode we need to:
//						a) Write the Byte-order-mark at the beginning of the file
//						b) Write all strings in byte-mode
//					-	If we were compiled in Unicode, we need to convert Unicode to multibyte if 
//						we want to write in multibyte
//					-	If we were compiled in multi-byte, we need to convert multibyte to Unicode if 
//						we want to write in Unicode.
// Exceptions:	None.
//
void CSrcFile::WriteString(LPCTSTR lpsz)
{
	wchar_t*	pszUnicodeString = NULL;
	char	*	pszMultiByteString = NULL;

	try
	{
		// If writing Unicode and at the start of the file, need to write byte mark
		if (nFlags & CSrcFile::modeWriteUnicode)
		{
			// If at position 0, write byte-order mark before writing anything else
			if (!m_pStream || GetPosition() == 0)
			{
				wchar_t cBOM = (wchar_t)UNICODE_BOM;
				CFile::Write(&cBOM, sizeof(wchar_t));
			}
		}

		// If compiled in Unicode...
#ifdef _UNICODE

		// If writing Unicode, no conversion needed
		if (nFlags & CSrcFile::modeWriteUnicode)
		{
			// Write in byte mode
			CFile::Write(lpsz, lstrlen(lpsz) * sizeof(wchar_t));
		}
		// Else if we don't want to write Unicode, need to convert
		else
		{
			int		nChars = lstrlen(lpsz) + 1;				// Why plus 1? Because yes
//			int		nBufferSize = nChars * sizeof(char);	// leave space for multi-byte chars
			int		nCharsWritten = 0;
			int		nBufferSize = 0;

			pszUnicodeString = new wchar_t[nChars];

			// Copy string to Unicode buffer
			lstrcpy(pszUnicodeString, lpsz);

			// Work out how much space we need for the multibyte conversion
			nBufferSize = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nFileCodePage);
			pszMultiByteString = new char[nBufferSize];

			// Get multibyte string
			nCharsWritten = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nBufferSize, nFileCodePage);

			if (nCharsWritten > 0)
			{
				// Do byte-mode write using actual chars written (fix by Howard J Oh)
	//			CFile::Write((const void*)pszMultiByteString, lstrlen(lpsz));
				CFile::Write((const void*)pszMultiByteString,
					nCharsWritten * sizeof(char));
			}
		}
		// Else if *not* compiled in Unicode
#else
		// If writing Unicode, need to convert
		if (nFlags & CSrcFile::modeWriteUnicode)
		{
			int		nChars = lstrlen(lpsz) + 1;	 // Why plus 1? Because yes
			int		nBufferSize = nChars * sizeof(wchar_t);
			int		nCharsWritten = 0;

			pszUnicodeString = new wchar_t[nChars];
			pszMultiByteString = new char[nChars];

			// Copy string to multibyte buffer
			lstrcpy(pszMultiByteString, lpsz);

			nCharsWritten = GetUnicodeStringFromMultiByteString(pszMultiByteString, pszUnicodeString, nChars, nFileCodePage);

			if (nCharsWritten > 0)
			{
				// Do byte-mode write using actual chars written (fix by Howard J Oh)
	//			CFile::Write(pszUnicodeString, lstrlen(lpsz) * sizeof(wchar_t));
				CFile::Write(pszUnicodeString, nCharsWritten * sizeof(wchar_t));
			}
			else
			{
				ASSERT(false);
			}

		}
		// Else if we don't want to write Unicode, no conversion needed, unless the code page differs
		else
		{
			//		// Do standard stuff
			//		CStdioFile::WriteString(lpsz);

					// Get the current code page
			UINT nLocaleCodePage = GetCurrentLocaleCodePage();

			// If we got it OK, and if file code page does not match the system code page, we need to do a double conversion!
			if (nLocaleCodePage > 0 && nLocaleCodePage != (UINT)nFileCodePage)
			{
				int	nChars = lstrlen(lpsz) + 1;	 // Why plus 1? Because yes

				pszUnicodeString = new wchar_t[nChars];

				// Initialise to something safe
				memset(pszUnicodeString, 0, sizeof(wchar_t) * nChars);

				// Convert to Unicode using the locale code page (the code page we are using in memory)
				nChars = GetUnicodeStringFromMultiByteString((LPCSTR)(const char*)lpsz, pszUnicodeString, nChars, nLocaleCodePage);

				// Convert back to multibyte using the file code page
				// (Note that you can't reliably read a non-Unicode file written in code page A on a system using a code page B,
				// modify the file and write it back using code page A, unless you disable all this double-conversion code.
				// In effect, you have to choose between a mangled character display and mangled file writing).
				if (nChars > 0)
				{
					// Calculate how much we need for the MB buffer (it might be larger)
					nChars = GetRequiredMultiByteLengthForUnicodeString(pszUnicodeString, nFileCodePage);

					pszMultiByteString = new char[nChars];
					memset(pszMultiByteString, 0, sizeof(char) * nChars);

					nChars = GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nChars, nFileCodePage);

					// Do byte-mode write. This avoids annoying "interpretation" of \n's as \r\n
					CFile::Write((const void*)pszMultiByteString, nChars * sizeof(char));
				}
			}
			else
			{
				// Do byte-mode write. This avoids annoying "interpretation" of \n's as \r\n
				CFile::Write((const void*)lpsz, lstrlen(lpsz) * sizeof(char));
			}
		}

#endif
	}
	// Ensure we always clean up
	catch (...)
	{
		if (pszUnicodeString)	delete[] pszUnicodeString;
		if (pszMultiByteString)	delete[] pszMultiByteString;
		throw;
	}

	if (pszUnicodeString)	delete[] pszUnicodeString;
	if (pszMultiByteString)	delete[] pszMultiByteString;
}

bool CSrcFile::SetDefaultPath()
{
	bool	bRet = true;

	int		intDelimiterIndex;

	cszPath = AfxGetAppName();

	intDelimiterIndex = cszPath.ReverseFind(_TEXT('\\'));
	cszPath.Left(intDelimiterIndex);

	return bRet;
}
