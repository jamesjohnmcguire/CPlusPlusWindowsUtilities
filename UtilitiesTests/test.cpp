#include "pch.h"
#include "../Utilities/Utilities.h"
#include "../Utilities/Utf8.h"

TEST(Utf8Tests, RecognizesBomAtStart)
{
	const std::uint8_t exact[] = {0xEF, 0xBB, 0xBF};
	const std::uint8_t followedByText[] = {0xEF, 0xBB, 0xBF, 'A'};

	constexpr std::size_t size1 = sizeof(exact);
	constexpr std::size_t size2 = sizeof(followedByText);
	bool result1 = HasUtf8Bom(exact, size1);
	bool result2 = HasUtf8Bom(followedByText, size2);

	EXPECT_TRUE(result1);
	EXPECT_TRUE(result2);
}

TEST(Utf8Tests, RejectsOtherPrefixes)
{
	const std::uint8_t prefixed[] = {'A', 0xEF, 0xBB, 0xBF};
	const std::uint8_t wrongThirdByte[] = {0xEF, 0xBB, 0xBE};

	constexpr std::size_t size1 = sizeof(prefixed);
	constexpr std::size_t size2 = sizeof(wrongThirdByte);
	bool result1 = HasUtf8Bom(prefixed, size1);
	bool result2 = HasUtf8Bom(wrongThirdByte, size2);

	EXPECT_FALSE(result1);
	EXPECT_FALSE(result2);
}

TEST(Utf8Tests, RejectsShortAndMissingInput)
{
	const std::uint8_t bom[] = {0xEF, 0xBB, 0xBF};

	bool result1 = HasUtf8Bom(bom, 0);
	bool result2 = HasUtf8Bom(bom, 1);
	bool result3 = HasUtf8Bom(bom, 2);
	bool result4 = HasUtf8Bom(nullptr, 0);
	bool result5 = HasUtf8Bom(nullptr, 3);

	EXPECT_FALSE(result1);
	EXPECT_FALSE(result2);
	EXPECT_FALSE(result3);
	EXPECT_FALSE(result4);
	EXPECT_FALSE(result5);
}
