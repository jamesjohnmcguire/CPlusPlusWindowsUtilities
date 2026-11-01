#include "pch.h"
#include "../UtilitiesWin/UtilitiesWin.h"
#include "../UtilitiesMfc/Localize.h"

TEST(LocalizeTests, RecognizesHelpArgument)
{
	wchar_t program[] = L"tool";
	wchar_t help[] = L"-h";
	wchar_t* arguments[] = {program, help};

	EXPECT_TRUE(IsHelp(2, arguments));
}

TEST(LocalizeTests, IgnoresOrdinaryArgument)
{
	wchar_t program[] = L"tool";
	wchar_t file[] = L"input.txt";
	wchar_t* arguments[] = {program, file};

	EXPECT_FALSE(IsHelp(2, arguments));
}

TEST(LocalizeTests, AcceptsProgramNameWithoutArguments)
{
	wchar_t program[] = L"tool";
	wchar_t* arguments[] = {program};

	EXPECT_FALSE(IsHelp(1, arguments));
}
