//////////////////////////////////////////////////////////////////////
// SrcFile.h: interface for the CSrcFile class.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
//////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "UtilitiesWin.h"

///////////////////////////////////////////////////////////////////////
// definitions
///////////////////////////////////////////////////////////////////////
// Unicode "byte order mark" which goes at start of file
#define	UNICODE_BOM 0xFEFF
// New line characters
#define	NEWLINE _T("\r\n")
// Filler char used when no conversion from Unicode to local code page
// is possible
#define	DEFAULT_UNICODE_FILLER_CHAR "#"

///////////////////////////////////////////////////////////////////////
// Class prototypes
///////////////////////////////////////////////////////////////////////

class CSrcFile : public CStdioFile  
{
	// Members
	public:
		// Additional flag to allow Unicode text writing
		static const UINT modeWriteUnicode = 0x20000;

	protected:
		bool isUnicodeText;
		UINT fileOpenFlags;
		int fileCodePage;
		CString cszBaseFileName;
		CString cszClassName;

	private:
		CString projectPath;

	// Methods
	public:
		CSrcFile();
		CSrcFile(LPCTSTR sourceFileName, UINT openFlags);
		virtual ~CSrcFile();

		unsigned long GetCharCount();
		static UINT	GetCurrentLocaleCodePage();
		static bool GetLastErrorInfo(void);
		const CString& GetProjectPath() const { return projectPath; }

		///////////////////////////////////////////////////////////////////////
		// GetMultiByteStringFromUnicodeString()
		// 
		// Gets a MultiByte string from a Unicode string.
		// 
		// Parameters: unicodeString       Unicode input string
		//             multiByteString     Multibyte output string
		//             multiByteBufferSize Multibyte buffer size
		//             codePage            Code page used to
		//                                            perform conversion
		//                                            Default = -1
		//                                            (local code page)
		// Exceptions:	None.
		// Returns:   int - number of characters written. 0 means error
		///////////////////////////////////////////////////////////////////////
		static int GetMultiByteStringFromUnicodeString(
			wchar_t* unicodeString,
			char* multiByteString,
			int multiByteBufferSize,
			int codePage = -1);

		///////////////////////////////////////////////////////////////////////
		// GetRequiredMultiByteLengthForUnicodeString()
		//
		// Obtains the multi-byte buffer size needed to accommodate a converted
		// Unicode string.
		//
		// Notes
		// We can't assume that the buffer length is simply equal to the number
		// of characters because that wouldn't accommodate multibyte characters!
		///////////////////////////////////////////////////////////////////////
		// Parameters: unicodeString, codePage = -1
		// Returns:    int
		///////////////////////////////////////////////////////////////////////
		static int GetRequiredMultiByteLengthForUnicodeString(
			wchar_t* unicodeString,
			int codePage = -1);

		/// Gets a Unicode string from a MultiByte string.
		/// @param multiByteString Multi-byte input string
		/// @param unicodeString Unicode output string (output parameter)
		/// @param unicodeBufferSize Size of Unicode output buffer in characters
		/// @param codePage Code page for conversion (-1 = local code page)
		/// @return Number of characters written, or 0 on error
		static int GetUnicodeStringFromMultiByteString(
			LPCSTR multiByteString,
			wchar_t* unicodeString,
			int unicodeBufferSize,
			int codePage = -1
		);

		///////////////////////////////////////////////////////////////////////
		// IsFileUnicode()
		//
		// Determines whether a file is Unicode by reading the first character
		// and detecting whether it's the Unicode byte marker.
		///////////////////////////////////////////////////////////////////////
		// Parameters: const CString& sFilePath
		// Exceptions:	None.
		// Returns:    bool
		///////////////////////////////////////////////////////////////////////
		static bool IsFileUnicode(const CString& sFilePath);
		bool IsFileUnicodeText() const noexcept { return isUnicodeText; }
		BOOL Open();
		virtual BOOL Open(LPCTSTR sourceFileName, UINT openFlags, CFileException* errorInfo = nullptr);
		virtual size_t Read(CString& rString, UINT count);
		virtual size_t Read(LPCTSTR rString, UINT count);
		virtual BOOL ReadString(CString& rString);
		void SetClassName(LPCTSTR className);
		void SetCodePage(IN const UINT codePage);
		bool SetLogPath( LPCTSTR path);
		void SetProjectPath(const CString& path);
		bool SetSrcFileName(LPCTSTR name = nullptr);
		void SetUnicode(bool useUnicode);
		virtual int Write(LPCTSTR text, int bytesToWrite);
		virtual void	WriteString(LPCTSTR text);

	protected:
		UINT ProcessFlags(const CString& sFilePath, UINT& openFlags);
		bool SetDefaultPath();
};
