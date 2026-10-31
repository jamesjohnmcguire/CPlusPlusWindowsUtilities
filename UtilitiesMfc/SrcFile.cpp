/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "SrcFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSrcFile::CSrcFile()
{
	isUnicodeText = false;
	fileCodePage = -1;

	CStdioFile::CStdioFile( );
}

CSrcFile::CSrcFile(LPCTSTR sourceFileName, UINT openFlags)
	:CStdioFile(sourceFileName, ProcessFlags(sourceFileName, openFlags))
{
	fileCodePage = -1;

	CStdioFile::CStdioFile();
}

CSrcFile::~CSrcFile()
{

}

BOOL CSrcFile::Open()
{
	BOOL	returnResult = FALSE;

	if (!projectPath.IsEmpty())
	{
		//		UINT nOpenFlags =	CFile::typeText |
		UINT openFlags = CFile::typeBinary |
			CFile::modeCreate |
			CFile::modeNoTruncate |
			CFile::modeReadWrite |
			CFile::shareDenyNone;

		// Process any Unicode stuff
		ProcessFlags(projectPath, openFlags);

		returnResult = CStdioFile::Open(projectPath,
			openFlags);
	}
	return returnResult;
}

BOOL CSrcFile::Open(
	LPCTSTR sourceFileName,
	UINT openFlags,
	CFileException* errorInfo /*=nullptr*/)
{
	// Process any Unicode stuff
	ProcessFlags(sourceFileName, openFlags);

	return CStdioFile::Open(sourceFileName, openFlags, errorInfo);
}

UINT CSrcFile::ProcessFlags(const CString& filePath, UINT& openFlags)
{
	isUnicodeText = false;

#ifdef _DEBUG
	// If we have writeUnicode we must have write or writeRead as well
	if (openFlags & modeWriteUnicode)
	{
		ASSERT(openFlags & CFile::modeWrite || openFlags & CFile::modeReadWrite);
	}
#endif

	BOOL create = openFlags & CFile::modeCreate;
	BOOL preserveContent = openFlags & CFile::modeNoTruncate;
	BOOL writeMode = openFlags & CFile::modeWrite;

	if ((!create || preserveContent) && !writeMode)
	{
		isUnicodeText = IsFileUnicode(filePath);

		// If it's Unicode, switch to binary mode
		if (isUnicodeText)
		{
			openFlags |= CFile::typeBinary;
		}
	}

	fileOpenFlags = openFlags;

	return openFlags;
}

size_t	CSrcFile::Read(LPCTSTR rString, UINT count)
{
	CString	cstring = rString;

	return Read(cstring, count);
}

size_t CSrcFile::Read(CString& rString, UINT count)
{
	const int	maxLineCharacters = 4096;
	BOOL		readData = FALSE;
	int			textLength = 0;
	int			characterCount = 0;
	size_t		charactersRead = -1;

	LPTSTR		lineBuffer;
	wchar_t*	unicodeString = nullptr;
	char	*	multiByteString = nullptr;

	try
	{
		// If at position 0, discard byte-order mark before reading
		if (!m_pStream || (GetPosition() == 0 && isUnicodeText))
		{
			wchar_t	cDummy;
#if SRC_FILE_BOM_READ_ALTERNATIVE
			Read(&cDummy, sizeof(_TCHAR));
#endif
			CFile::Read(&cDummy, sizeof(wchar_t));
		}

		if (isUnicodeText)
		{
			// Do standard stuff - Unicode to Unicode. Seems to work OK.
			LPTSTR	textBuffer = rString.GetBufferSetLength(count);
			charactersRead = CFile::Read(textBuffer, count);
			rString.ReleaseBuffer();
		}
		else
		{
			unicodeString = new wchar_t[count];
			multiByteString = new char[count];

			// Initialise to something safe
			memset(unicodeString, 0, sizeof(wchar_t) * count);
			memset(multiByteString, 0, sizeof(char) * count);

			// Read the string
			charactersRead = fread(multiByteString, 1, count, m_pStream);

			if (charactersRead > 0)
			{
				readData = TRUE;

				// Convert multibyte to Unicode, using the specified code page
				characterCount = GetUnicodeStringFromMultiByteString(multiByteString, unicodeString, count, fileCodePage);

				if (characterCount > 0)
				{
#if SRC_FILE_UNICODE_STRING_CAST
					rString = (CString)unicodeString;
#endif
					rString = multiByteString;
				}
			}
		}

		// Then remove end-of-line character if in Unicode text mode
		if (readData)
		{
			// Copied from FileTxt.cpp but adapted to Unicode and then adapted for end-of-line being just '\r'. 
			textLength = rString.GetLength();
			if (textLength > 1 && rString.Mid(textLength - 2) == NEWLINE)
			{
				rString.GetBufferSetLength(textLength - 2);
			}
			else
			{
				lineBuffer = rString.GetBuffer(0);
				if (textLength != 0 && (lineBuffer[textLength - 1] == _T('\r') || lineBuffer[textLength - 1] == _T('\n')))
				{
					rString.GetBufferSetLength(textLength - 1);
				}
			}
		}
	}
	// Ensure we always delete in case of exception
	catch (CException *ex)
	{
#if SRC_FILE_UNUSED_COUNT
		int count;
#endif
		CString cError;
		LPTSTR errorText = cError.GetBuffer();
		_tprintf(_T("Exception: %s\r\n"), ex->GetErrorMessage(errorText, 3));
		cError.ReleaseBuffer();
		if (unicodeString)	delete[] unicodeString;

		if (multiByteString) delete[] multiByteString;

		throw;
	}

	if (unicodeString)		delete[] unicodeString;

	if (multiByteString)		delete[] multiByteString;

	return charactersRead;
}

BOOL CSrcFile::ReadString(CString& rString)
{
	const int	maxLineCharacters = 4096;
	BOOL			readData = FALSE;
	LPTSTR		lineBuffer;
	int			textLength = 0;
	wchar_t*		unicodeString = nullptr;
	char	*		multiByteString = nullptr;
	int			characterCount = 0;

	try
	{
		// If at position 0, discard byte-order mark before reading
		if (!m_pStream || (GetPosition() == 0 && isUnicodeText))
		{
			wchar_t	cDummy;
#if SRC_FILE_BOM_READ_ALTERNATIVE
			Read(&cDummy, sizeof(_TCHAR));
#endif
			CFile::Read(&cDummy, sizeof(wchar_t));
		}

		if (isUnicodeText)
		{
			// Do standard stuff - Unicode to Unicode. Seems to work OK.
			readData = CStdioFile::ReadString(rString);
		}
		else
		{
			unicodeString = new wchar_t[maxLineCharacters];
			multiByteString = new char[maxLineCharacters];

			// Initialise to something safe
			memset(unicodeString, 0, sizeof(wchar_t) * maxLineCharacters);
			memset(multiByteString, 0, sizeof(char) * maxLineCharacters);

			// Read the string
			readData = (nullptr != fgets(multiByteString, maxLineCharacters, m_pStream));

			if (readData)
			{
				// Convert multibyte to Unicode, using the specified code page
				characterCount = GetUnicodeStringFromMultiByteString(multiByteString, unicodeString, maxLineCharacters, fileCodePage);

				if (characterCount > 0)
				{
					rString = (CString)unicodeString;
				}
			}
		}

		// Then remove end-of-line character if in Unicode text mode
		if (readData)
		{
			// Copied from FileTxt.cpp but adapted to Unicode and then adapted for end-of-line being just '\r'. 
			textLength = rString.GetLength();
			if (textLength > 1 && rString.Mid(textLength - 2) == NEWLINE)
			{
				rString.GetBufferSetLength(textLength - 2);
			}
			else
			{
				lineBuffer = rString.GetBuffer(0);
				if (textLength != 0 && (lineBuffer[textLength - 1] == _T('\r') || lineBuffer[textLength - 1] == _T('\n')))
				{
					rString.GetBufferSetLength(textLength - 1);
				}
			}
		}
	}
	// Ensure we always delete in case of exception
	catch (...)
	{
		if (unicodeString)	delete[] unicodeString;

		if (multiByteString) delete[] multiByteString;

		throw;
	}

	if (unicodeString)		delete[] unicodeString;

	if (multiByteString)		delete[] multiByteString;

	return readData;
}

void CSrcFile::SetCodePage(IN const UINT codePage)
{
	fileCodePage = (int)codePage;
}

void CSrcFile::SetClassName(LPCTSTR className)
{
	cszClassName  = className;
}

bool CSrcFile::SetLogPath(LPCTSTR path)
{
	bool	result	= false;

	if (path && *path != '\0')
	{
		projectPath = path;

		result	= true;
	}

	return result;
}

void CSrcFile::SetProjectPath(const CString& path)
{
	projectPath = path;
}

bool CSrcFile::SetSrcFileName( LPCTSTR name /* = nullptr */)
{
	bool	result	= false;

	if (nullptr != name)
	{
		projectPath = name;
		SetFilePath( name );
		result	= true;
	}
	else
	{
		if (projectPath.IsEmpty())
		{
			SetDefaultPath();
		}

		int intDelimiterIndex = projectPath.GetLength() - projectPath.ReverseFind(_TEXT('\\'));

		projectPath.Right(intDelimiterIndex);

		intDelimiterIndex = projectPath.ReverseFind(_TEXT('.'));

		projectPath.Left(intDelimiterIndex);
		projectPath += _T(".log");
	}

	return result;
}

void	CSrcFile::SetUnicode(bool useUnicode)
{
	if (useUnicode == true)
	{
		isUnicodeText = true;

		fileOpenFlags = fileOpenFlags | CSrcFile::modeWriteUnicode;
	}
	else
	{
		isUnicodeText = false;

		fileOpenFlags = fileOpenFlags ^ CSrcFile::modeWriteUnicode;
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
	bool				isUnicode = false;
	wchar_t			cFirstChar;
	CFileException	exFile;

	// Open file in binary mode and read first character
	if (file.Open(sFilePath, CFile::typeBinary | CFile::modeRead, &exFile))
	{
		// If byte is Unicode byte-order marker, let's say it's Unicode
		if (file.Read(&cFirstChar, sizeof(wchar_t)) > 0 && cFirstChar == (wchar_t)UNICODE_BOM)
		{
			isUnicode = true;
		}

		file.Close();
	}
	else
	{
		// Handle error here if you like
	}

	return isUnicode;
}

bool CSrcFile::GetLastErrorInfo(void)
{
	bool	result = false;

	LPVOID	messageBuffer;
	DWORD	errorCode = GetLastError();
	DWORD	formattedLength;

	formattedLength = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&messageBuffer),
		0,
		nullptr);

	if (messageBuffer)
	{
		OutputDebugString(reinterpret_cast<LPTSTR>(messageBuffer));
		_tprintf(_T("Error: %s\r\n"), reinterpret_cast<LPTSTR>(messageBuffer));
		LocalFree(messageBuffer);
	}

	return result;
}

unsigned long CSrcFile::GetCharCount()
{
	int				characterSize;
	ULONGLONG		byteCount = 0;
	unsigned long	characterCount = 0;

	if (m_pStream)
	{
		// Get size of chars in file
		characterSize = isUnicodeText ? sizeof(wchar_t) : sizeof(char);

		// If Unicode, remove byte order mark from count
		byteCount = GetLength();

		if (isUnicodeText)
		{
			byteCount = byteCount - sizeof(wchar_t);
		}

		// Calc chars
		characterCount = ((size_t)byteCount / characterSize);
	}

	return characterCount;
}

// Get the current userLs code page
UINT CSrcFile::GetCurrentLocaleCodePage()
{
	_TCHAR	localCodePageText[10];
	UINT		localeCodePage = 0;
	int		localeCharacterCount = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, localCodePageText, 10);

	// If OK
	if (localeCharacterCount > 0)
	{
		localeCodePage = (UINT)_ttoi(localCodePageText);
		ASSERT(localeCodePage > 0);
	}
	else
	{
		ASSERT(false);
	}

	// O means either: no ANSI code page (Unicode-only locale?) or failed to get locale
	// In the case of Unicode-only locales, what do multibyte apps do? Answers on a postcard.
	return localeCodePage;
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
int CSrcFile::GetUnicodeStringFromMultiByteString(
	LPCSTR multiByteString,
	wchar_t* unicodeString,
	int unicodeBufferSize,
	int codePage)
{
	bool		succeeded = true;
	int		charactersWritten = 0;

	if (unicodeString && multiByteString)
	{
		// If no code page specified, take default for system
		if (codePage == -1)
		{
			codePage = GetACP();
		}

		try
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)unicodeString, '\0', sizeof(wchar_t) *
				unicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			charactersWritten = MultiByteToWideChar((UINT)codePage,
				(codePage == CP_UTF8 ? 0 : MB_PRECOMPOSED), // Flags
				multiByteString,
				-1,
				unicodeString,
				0);
#if SRC_FILE_OUTPUT_SIZE_ARGUMENT
			unicodeBufferSize+1);
#endif

			if (charactersWritten < 1)
				GetLastErrorInfo();

		}
		catch (CException *ex)
		{
			GetLastErrorInfo();

#if SRC_FILE_UNUSED_COUNT
			int count;
#endif
			CString cError;
			LPTSTR errorText = cError.GetBuffer();
			_tprintf(_T("Exception: %s\r\n"), ex->GetErrorMessage(errorText, 3));
			cError.ReleaseBuffer();
			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}

	// Now fix nCharsWritten
	if (charactersWritten > 0)
	{
		charactersWritten--;
	}

	ASSERT(charactersWritten > 0);
	return charactersWritten;
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
int CSrcFile::GetMultiByteStringFromUnicodeString(wchar_t * unicodeString, char* multiByteString,
	int multiByteBufferSize, int codePage)
{
	BOOL		usedDefaultCharacter = FALSE;
	int		charactersWritten = 0;

	// Fix by Andy Goodwin: don't do anything if buffer is 0
	if (multiByteBufferSize > 0)
	{
		if (unicodeString && multiByteString)
		{
			// Zero out buffer first
			memset((void*)multiByteString, '\0', multiByteBufferSize);

			// If no code page specified, take default for system
			if (codePage == -1)
			{
				codePage = GetACP();
			}

			try
			{
				// If writing to UTF8, flags, default char and boolean flag must be nullptr
				charactersWritten = WideCharToMultiByte((UINT)codePage,
					(codePage == CP_UTF8 ? 0 : WC_COMPOSITECHECK | WC_SEPCHARS), // Flags
					unicodeString, -1,
					multiByteString,
					multiByteBufferSize,
					(codePage == CP_UTF8 ? nullptr : DEFAULT_UNICODE_FILLER_CHAR),	// Filler char
					(codePage == CP_UTF8 ? nullptr : &usedDefaultCharacter));						// Did we use filler char?

				// If no chars were written and the buffer is not 0, error!
				if (charactersWritten == 0 && multiByteBufferSize > 0)
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
	if (charactersWritten > 0)
	{
		charactersWritten--;
	}

	return charactersWritten;
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
/*static*/ int CSrcFile::GetRequiredMultiByteLengthForUnicodeString(wchar_t * unicodeString, int codePage /*=-1*/)
{
	int requiredCharacterCount = 0;

	try
	{
		// If no code page specified, take default for system
		if (codePage == -1)
		{
			codePage = GetACP();
		}

		// If writing to UTF8, flags, default char and boolean flag must be nullptr
		requiredCharacterCount = WideCharToMultiByte((UINT)codePage,
			(codePage == CP_UTF8 ? 0 : WC_COMPOSITECHECK | WC_SEPCHARS), // Flags
			unicodeString, -1,
			nullptr,
			0,	// Calculate required buffer, please! 
			(codePage == CP_UTF8 ? nullptr : DEFAULT_UNICODE_FILLER_CHAR),	// Filler char
			nullptr);
	}
	catch (...)
	{
		TRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
	}

	return requiredCharacterCount;
}

int	CSrcFile::Write(LPCTSTR text, int charactersToWrite)
{
	wchar_t*	unicodeString = nullptr;
	char*		multiByteString = nullptr;
	int			charactersWritten = -1;

	try
	{
		// If writing Unicode and at the start of the file, need to write byte mark
		if (fileOpenFlags & CSrcFile::modeWriteUnicode)
		{
			// If at position 0, write byte-order mark before writing anything else
			if (!m_pStream || GetPosition() == 0)
			{
				wchar_t cBOM = (wchar_t)UNICODE_BOM;
				CFile::Write(&cBOM, sizeof(wchar_t));
				charactersWritten = 1;
			}
		}

		// If writing Unicode, no conversion needed
		if (fileOpenFlags & CSrcFile::modeWriteUnicode)
		{
			// Write in byte mode
			CFile::Write(text, charactersToWrite * sizeof(wchar_t));
			charactersWritten = 1;
		}
		// Else if we don't want to write Unicode, need to convert
		else
		{
			int		characterCount = charactersToWrite + 1;				// Why plus 1? Because yes
#if SRC_FILE_BUFFER_SIZE_ALTERNATIVE
int		bufferSize = characterCount * sizeof(char);	// leave space for multi-byte chars
#endif
			int		charactersWritten = 0;
			int		bufferSize = 0;

			unicodeString = new wchar_t[characterCount];

			// Copy string to Unicode buffer
			lstrcpy(unicodeString, text);

			// Work out how much space we need for the multibyte conversion
			bufferSize = GetRequiredMultiByteLengthForUnicodeString(unicodeString, fileCodePage);
			multiByteString = new char[bufferSize];

			// Get multibyte string
			charactersWritten = GetMultiByteStringFromUnicodeString(unicodeString, multiByteString, bufferSize, fileCodePage);

			if (charactersWritten > 0)
			{
				// Do byte-mode write using actual chars written (fix by Howard J Oh)
#if SRC_FILE_MULTIBYTE_WRITE_ALTERNATIVE
	CFile::Write((const void*)multiByteString, lstrlen(text));
#endif
				CFile::Write((const void*)multiByteString,
					charactersWritten * sizeof(char));
			}
		}
	}
	// Ensure we always clean up
	catch (...)
	{
		if (unicodeString)
			delete[] unicodeString;
		if (multiByteString)
			delete[] multiByteString;
		throw;
	}

	if (unicodeString)
		delete[] unicodeString;
	if (multiByteString)
		delete[] multiByteString;

	return charactersWritten;
}

#if LEGACY_SRC_FILE_WRITE
HRESULT CSrcFile::Write(CString &cszString, bool ignore /* = FALSE */)
{
	if (!ignore)
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

HRESULT CSrcFile::Write(LPCTSTR text, bool ignore /* = FALSE */)
{
	CString cszString   = text;

	return Write( cszString, ignore );
}
#endif

// --------------------------------------------------------------------------------------------
//
//	CSrcFile::WriteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    void
// Parameters: LPCTSTR text
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
void CSrcFile::WriteString(LPCTSTR text)
{
	wchar_t*	unicodeString = nullptr;
	char	*	multiByteString = nullptr;

	try
	{
		// If writing Unicode and at the start of the file, need to write byte mark
		if (fileOpenFlags & CSrcFile::modeWriteUnicode)
		{
			// If at position 0, write byte-order mark before writing anything else
			if (!m_pStream || GetPosition() == 0)
			{
				wchar_t cBOM = (wchar_t)UNICODE_BOM;
				CFile::Write(&cBOM, sizeof(wchar_t));
			}
		}

		// If writing Unicode, no conversion needed
		if (fileOpenFlags & CSrcFile::modeWriteUnicode)
		{
			// Write in byte mode
			CFile::Write(text, lstrlen(text) * sizeof(wchar_t));
		}
		// Else if we don't want to write Unicode, need to convert
		else
		{
			int		characterCount = lstrlen(text) + 1;				// Why plus 1? Because yes
#if SRC_FILE_BUFFER_SIZE_ALTERNATIVE
int		bufferSize = characterCount * sizeof(char);	// leave space for multi-byte chars
#endif
			int		charactersWritten = 0;
			int		bufferSize = 0;

			unicodeString = new wchar_t[characterCount];

			// Copy string to Unicode buffer
			lstrcpy(unicodeString, text);

			// Work out how much space we need for the multibyte conversion
			bufferSize = GetRequiredMultiByteLengthForUnicodeString(unicodeString, fileCodePage);
			multiByteString = new char[bufferSize];

			// Get multibyte string
			charactersWritten = GetMultiByteStringFromUnicodeString(unicodeString, multiByteString, bufferSize, fileCodePage);

			if (charactersWritten > 0)
			{
				// Do byte-mode write using actual chars written (fix by Howard J Oh)
#if SRC_FILE_MULTIBYTE_WRITE_ALTERNATIVE
	CFile::Write((const void*)multiByteString, lstrlen(text));
#endif
				CFile::Write((const void*)multiByteString,
					charactersWritten * sizeof(char));
			}
		}
	}
	// Ensure we always clean up
	catch (...)
	{
		if (unicodeString)	delete[] unicodeString;
		if (multiByteString)	delete[] multiByteString;
		throw;
	}

	if (unicodeString)	delete[] unicodeString;
	if (multiByteString)	delete[] multiByteString;
}

bool CSrcFile::SetDefaultPath()
{
	const bool result = true;

	projectPath = AfxGetAppName();

	int delimiterIndex = projectPath.ReverseFind(_TEXT('\\'));
	projectPath.Left(delimiterIndex);

	return result;
}
