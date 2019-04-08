#include "stdafx.h"
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
	CStdioFile::CStdioFile( );
}

CSrcFile::~CSrcFile()
{

}

BOOL CSrcFile::Open()
{
#ifdef DEBUG
//    ShowWarning( _T("log file:") );
// TODO - FIX!
//    ShowWarning( (LPCTSTR)m_cszPathName );
#endif

    if(m_cszPathName.IsEmpty())
	{
		int intDelimiterIndex;

		m_cszPathName   = AfxGetAppName( ); 

		intDelimiterIndex = m_cszPathName.ReverseFind(_TEXT('\\'));
		m_cszPathName.Left(intDelimiterIndex);

        m_cszPathName   = _T("\\LofFile.log");
	}
//    else
//        m_cszPathName   = m_cszPath +  _T("\\LofFile.log");

    return CStdioFile::Open(    m_cszPathName,
                                CFile::modeCreate |
                                CFile::modeNoTruncate |
                                CFile::modeReadWrite |
                                CFile::shareDenyNone);
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

            if (!m_cszClassName.IsEmpty())
                cszOutput   = m_cszClassName + "::" + cszString;
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
void CSrcFile::SetClassName(LPCTSTR pszClassName)
{
    m_cszClassName  = pszClassName;
}

bool CSrcFile::SetLogPath(LPCTSTR pszPath)
{
	bool	bRet	= false;

	if (pszPath && *pszPath != '\0')
	{
		m_cszPath   = pszPath;

		bRet	= true;
	}

	return bRet;
}

bool CSrcFile::SetSrcFileName( LPCTSTR pszName /* = NULL */)
{
	bool	bRet	= false;

	if (NULL != pszName)
	{
		m_cszPathName = pszName;
		SetFilePath( pszName );	
		bRet	= true;
	}
	else
	{
		if (m_cszPath.IsEmpty())
			SetDefaultPath();

		int intDelimiterIndex = m_cszPathName.GetLength() - m_cszPathName.ReverseFind(_TEXT('\\'));

		m_cszPathName.Right(intDelimiterIndex);

		intDelimiterIndex = m_cszPathName.ReverseFind(_TEXT('.'));

		m_cszPathName.Left(intDelimiterIndex);
		m_cszPathName += _T(".log");
	}

	return bRet;
}

bool CSrcFile::SetDefaultPath()
{
	bool	bRet	= true;

	int		intDelimiterIndex;

	m_cszPath   = AfxGetAppName( ); 

	intDelimiterIndex = m_cszPath.ReverseFind(_TEXT('\\'));
	m_cszPath.Left(intDelimiterIndex);

	return bRet;
}
