#include "pch.h"
#include "../Utilities/Utilities.h"
#include "../Utilities/Utf8.h"

TEST(Utf8Tests, RecognizesBomAtStart)
{
	const std::uint8_t exact[] = {0xEF, 0xBB, 0xBF};
	const std::uint8_t followedByText[] = {0xEF, 0xBB, 0xBF, 'A'};

	EXPECT_TRUE(HasUtf8Bom(exact, sizeof(exact)));
	EXPECT_TRUE(HasUtf8Bom(followedByText, sizeof(followedByText)));
}

TEST(Utf8Tests, RejectsOtherPrefixes)
{
	const std::uint8_t prefixed[] = {'A', 0xEF, 0xBB, 0xBF};
	const std::uint8_t wrongThirdByte[] = {0xEF, 0xBB, 0xBE};

	EXPECT_FALSE(HasUtf8Bom(prefixed, sizeof(prefixed)));
	EXPECT_FALSE(HasUtf8Bom(wrongThirdByte, sizeof(wrongThirdByte)));
}

TEST(Utf8Tests, RejectsShortAndMissingInput)
{
	const std::uint8_t bom[] = {0xEF, 0xBB, 0xBF};

	EXPECT_FALSE(HasUtf8Bom(bom, 0));
	EXPECT_FALSE(HasUtf8Bom(bom, 1));
	EXPECT_FALSE(HasUtf8Bom(bom, 2));
	EXPECT_FALSE(HasUtf8Bom(nullptr, 0));
	EXPECT_FALSE(HasUtf8Bom(nullptr, 3));
}
