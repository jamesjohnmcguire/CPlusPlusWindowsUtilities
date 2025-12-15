/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2025 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common.h"

class TextEncoding
{
	public:
		/// Gets a MultiByte string from a Unicode string.
		/// @param input Unicode input string
		/// @param encoding Encoding name for conversion (default: "UTF-8")
		/// return Multi-byte string, or std::nullopt on error
		static std::optional<std::string> GetMultiByteTextFromUnicode(
			const std::wstring& input,
			const std::string& encoding = "UTF-8");

		/// Gets a Unicode string from a MultiByte string.
		/// @param input Multi-byte input string
		/// @param encoding Encoding name for conversion (default: "UTF-8")
		/// @return Unicode string, or std::nullopt on error
		static std::optional<std::wstring> GetUnicodeTextFromMultiByte(
			const std::string& input,
			const std::string& encoding = "UTF-8");

private:
		static int GetCodePageFromName(const std::string& encoding) noexcept;
};

DllExport bool HasUtf8Bom(const uint8_t* data, size_t length);
