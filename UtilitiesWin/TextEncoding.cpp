/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TextEncoding.h"

std::optional<std::string> TextEncoding::GetMultiByteTextFromUnicode(
	const std::wstring& input,
	const std::string& encoding)
{
	std::optional<std::string> result;

	if (input.empty())
	{
		result = std::string();
	}
	else
	{
		int codePage = GetCodePageFromName(encoding);

		// Get required buffer size
		const wchar_t* inputPointer = input.c_str();
		int length = static_cast<int>(input.length());

		int sizeNeeded = WideCharToMultiByte(
			codePage,
			0,
			inputPointer,
			length,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		if (sizeNeeded <= 0)
		{
			result = std::nullopt;
		}
		else
		{
			std::string output(sizeNeeded, '\0');
			char* outputPointer = output.data();

			int written = WideCharToMultiByte(
				codePage,
				0,
				inputPointer,
				length,
				outputPointer,
				sizeNeeded,
				nullptr,
				nullptr
			);

			if (written <= 0)
			{
				result = std::nullopt;
			}
			else
			{
				result = output;
			}
		}
	}

	return result;
}

std::optional<std::wstring> TextEncoding::GetUnicodeTextFromMultiByte(
	const std::string& input,
	const std::string& encoding)
{
	std::optional<std::wstring> result;

	if (input.empty())
	{
		result = std::wstring();
	}
	else
	{
		int codePage = GetCodePageFromName(encoding);
		DWORD flags = (codePage == CP_UTF8) ? 0 : MB_PRECOMPOSED;

		// Get required buffer size
		const char* inputPointer = input.c_str();
		int length = static_cast<int>(input.length());

		int sizeNeeded = MultiByteToWideChar(
			codePage,
			flags,
			inputPointer,
			length,
			nullptr,
			0
		);

		if (sizeNeeded <= 0)
		{
			result = std::nullopt;
		}
		else
		{
			std::wstring output(sizeNeeded, L'\0');
			wchar_t* outputPointer = output.data();

			int written = MultiByteToWideChar(
				codePage,
				flags,
				inputPointer,
				length,
				outputPointer,
				sizeNeeded
			);

			if (written <= 0)
			{
				result = std::nullopt;
			}
			else
			{
				result = output;
			}
		}
	}

	return result;
}

int TextEncoding::GetCodePageFromName(const std::string& encoding) noexcept
{
	int codePage = -1;

	if (encoding == "UTF-8")
	{
		codePage = CP_UTF8;
	}
	else if (encoding == "UTF-7")
	{
		codePage = CP_UTF7;
	}
	else if (encoding == "ASCII" || encoding == "US-ASCII")
	{
		codePage = 20127;
	}
	else if (encoding == "ISO-8859-1" || encoding == "Latin1")
	{
		codePage = 28591;
	}
	else
	{
		// System default
		codePage = CP_ACP;
	}

	return codePage;
}
#include <cstdint>  // for uint8_t or BYTE if defined

// Returns true if the data starts with the UTF-8 BOM (EF BB BF)
// Assumes data has at least 3 bytes available; otherwise returns false.
bool HasUtf8Bom(const uint8_t* data, size_t length)
{
	bool result = false;

	if (length > 2 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
	{
		result = true;
	}

	return result;
}