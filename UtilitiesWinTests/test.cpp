#include "pch.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "../UtilitiesWin/FileWrapper.h"
#include "../UtilitiesWin/TextEncoding.h"

TEST(TextEncodingTests, ConvertsUnicodeToUtf8AndBack)
{
	const std::wstring unicode = L"Copyright \u00A9 \u65E5";
	const std::string utf8 = "Copyright \xC2\xA9 \xE6\x97\xA5";

	EXPECT_EQ(TextEncoding::GetMultiByteTextFromUnicode(unicode), utf8);
	EXPECT_EQ(TextEncoding::GetUnicodeTextFromMultiByte(utf8), unicode);
}

TEST(TextEncodingTests, ConvertsEmptyStrings)
{
	EXPECT_EQ(TextEncoding::GetMultiByteTextFromUnicode(L""), "");
	EXPECT_EQ(TextEncoding::GetUnicodeTextFromMultiByte(""), L"");
}

TEST(TextEncodingTests, RecognizesUtf8BomOnlyAtStart)
{
	const uint8_t bom[] = {0xEF, 0xBB, 0xBF, 0x41};
	const uint8_t other[] = {0x41, 0xEF, 0xBB, 0xBF};

	EXPECT_TRUE(HasUtf8Bom(bom, sizeof(bom)));
	EXPECT_FALSE(HasUtf8Bom(bom, 2));
	EXPECT_FALSE(HasUtf8Bom(other, sizeof(other)));
	EXPECT_FALSE(HasUtf8Bom(nullptr, 0));
}

TEST(FileWrapperTests, OwnsFileName)
{
	wchar_t tempDirectory[MAX_PATH]{};
	const DWORD directoryLength = GetTempPathW(MAX_PATH, tempDirectory);
	ASSERT_GT(directoryLength, 0u);
	ASSERT_LT(directoryLength, static_cast<DWORD>(MAX_PATH));

	wchar_t fileName[MAX_PATH]{};
	ASSERT_NE(GetTempFileNameW(tempDirectory, L"fwt", 0, fileName), 0u);
	struct DeleteTemporaryFile
	{
		const wchar_t* path;
		~DeleteTemporaryFile() { DeleteFileW(path); }
	} cleanup{fileName};

	std::wstring inputPath = fileName;
	FileWrapper file(inputPath.c_str());
	FileWrapper copy(file);
	inputPath.assign(L"changed");

	BYTE expected[] = {'O', 'K'};
	ASSERT_TRUE(copy.Create(expected, sizeof(expected)));

	std::ifstream stream(std::filesystem::path(fileName), std::ios::binary);
	ASSERT_TRUE(stream.is_open());
	char actual[2]{};
	stream.read(actual, sizeof(actual));
	EXPECT_EQ(stream.gcount(), 2);
	EXPECT_EQ(std::string(actual, sizeof(actual)), "OK");
}

TEST(FileWrapperTests, RejectsUnusableFileName)
{
	BYTE contents[] = {'O', 'K'};
	FileWrapper empty(L"");
	FileWrapper missing(static_cast<LPCTSTR>(nullptr));

	EXPECT_FALSE(empty.Create(contents, sizeof(contents)));
	EXPECT_FALSE(missing.Create(contents, sizeof(contents)));
}
