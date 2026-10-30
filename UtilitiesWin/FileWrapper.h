/////////////////////////////////////////////////////////////////////////////
// FileWrapper.h - Class Declaration
//
// Encapsulation class for reading and writing files.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "UtilitiesWin.h"
#include <string>

/////////////////////////////////////////////////////////////////////////////
// FileWrapper Class Definition
/////////////////////////////////////////////////////////////////////////////
class DllExport FileWrapper
{
	public:
		// Properties

		// Methods
			FileWrapper(
				LPCTSTR sourceFileName);
			FileWrapper(
				LPCSTR sourceFileName);
			~FileWrapper(void);

			bool Append(
				BYTE*	contents,
				DWORD	contentsLength);
			bool Create(
				BYTE*	contents,
				DWORD	contentsLength);
			BYTE* Read(
				DWORD*	contentsLength);

private:
		// Properties
			std::wstring fileName;

		// Methods
			char* ReadText(void);
			TCHAR* ReadUnicodeText(void);
};
