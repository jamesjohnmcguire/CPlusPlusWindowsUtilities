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
#include "Common.h"

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
		int nFileCodePage;
		CString cszBaseFileName;
		CString cszClassName;

	private:
		CString projectPath;

	// Methods
	public:
		CSrcFile();
		CSrcFile(LPCTSTR lpszFileName, UINT nOpenFlags);
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
		// Parameters: wchar_t * szUnicodeString      Unicode input string
		//             char*     szMultiByteString    Multibyte output string
		//             int       nMultiByteBufferSize Multibyte buffer size
		//             int       nCodePage            Code page used to
		//                                            perform conversion
		//                                            Default = -1
		//                                            (local code page)
		// Exceptions:	None.
		// Returns:   int - number of characters written. 0 means error
		///////////////////////////////////////////////////////////////////////
		static int GetMultiByteStringFromUnicodeString(
			wchar_t* szUnicodeString,
			char* szMultiByteString,
			int nMultiByteBufferSize,
			int nCodePage = -1);

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
		// Parameters: wchar_t * szUnicodeString,int nCodePage=-1
		// Returns:    int
		///////////////////////////////////////////////////////////////////////
		static int GetRequiredMultiByteLengthForUnicodeString(
			wchar_t* szUnicodeString,
			int nCodePage = -1);

		/// Gets a Unicode string from a MultiByte string.
		/// @param szMultiByteString Multi-byte input string
		/// @param szUnicodeString Unicode output string (output parameter)
		/// @param nUnicodeBufferSize Size of Unicode output buffer in characters
		/// @param nCodePage Code page for conversion (-1 = local code page)
		/// @return Number of characters written, or 0 on error
		static int GetUnicodeStringFromMultiByteString(
			LPCSTR szMultiByteString,
			wchar_t* szUnicodeString,
			int nUnicodeBufferSize,
			int nCodePage = -1
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
		virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL);
		virtual size_t Read(CString& rString, UINT nCount);
		virtual size_t Read(LPCTSTR rString, UINT nCount);
		virtual BOOL ReadString(CString& rString);
		void SetClassName(LPCTSTR pszClassName);
		void SetCodePage(IN const UINT nCodePage);
		bool SetLogPath( LPCTSTR pszPath);
		void SetProjectPath(const CString& path);
		bool SetSrcFileName(LPCTSTR pszName = NULL);
		void SetUnicode(bool bUnicode);
		virtual int Write(LPCTSTR pszString, int nBytesToWrite);
		virtual void	WriteString(LPCTSTR lpsz);

	protected:
		UINT ProcessFlags(const CString& sFilePath, UINT& nOpenFlags);
		bool SetDefaultPath();
};
