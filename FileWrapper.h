/////////////////////////////////////////////////////////////////////////////
// FileWrapper.h - Class Declaration
//
// Encapsulation class for reading and writing files.
//
// Created: 2008-07-09
//
// Copyright ©  2008-2010 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "../Include/Common.h"

/////////////////////////////////////////////////////////////////////////////
// FileWrapper Class Definition
/////////////////////////////////////////////////////////////////////////////
class DllExport FileWrapper
{
	public:
		// Properties

		// Methods
			FileWrapper(
				LPCTSTR FileName);
			FileWrapper(
				LPCSTR FileName);
			~FileWrapper(void);

			bool Append(
				BYTE*	Contents,
				DWORD	ContentsLength);
			bool Create(
				BYTE*	Contents,
				DWORD	ContentsLength);
			BYTE* Read(
				DWORD*	ContentsLength);

private:
		// Properties
			LPCTSTR	m_FileName;

		// Methods
			char* ReadText(void);
			TCHAR* ReadUnicodeText(void);
};
