// SrcFile.h: interface for the CSrcFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SrcFile_H__
#define __SrcFile_H__

//#include "c:/data/tech/projects/ZenWare/common/utils/utils.h"

class CSrcFile : public CStdioFile  
{
public:
	bool SetSrcFileName(LPCTSTR pszName = NULL);
	bool SetLogPath( LPCTSTR pszPath);
	void SetClassName( LPCTSTR pszClassName);
//	HRESULT Write( LPCTSTR pszString, bool bIgnore = false);
//	HRESULT Write( CString& cszString, bool bIgnore = false);
	CSrcFile();
	virtual ~CSrcFile();

	BOOL Open();

protected:
	bool SetDefaultPath();
	CString m_cszPath;
	CString m_cszPathName;
	CString m_cszBaseFileName;
	CString m_cszClassName;
};

#endif // __SrcFile_H__
