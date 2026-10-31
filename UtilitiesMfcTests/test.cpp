#include "pch.h"
#include "../UtilitiesWin/UtilitiesWin.h"
#include "../UtilitiesMfc/Localize.h"

TEST(LocalizeTests, RecognizesHelpArgument)
{
	TCHAR program[] = _T("tool");
	TCHAR help[] = _T("-h");
	TCHAR* arguments[] = {program, help};

	EXPECT_TRUE(IsHelp(2, arguments));
}

TEST(LocalizeTests, IgnoresOrdinaryArgument)
{
	TCHAR program[] = _T("tool");
	TCHAR file[] = _T("input.txt");
	TCHAR* arguments[] = {program, file};

	EXPECT_FALSE(IsHelp(2, arguments));
}

TEST(LocalizeTests, AcceptsProgramNameWithoutArguments)
{
	TCHAR program[] = _T("tool");
	TCHAR* arguments[] = {program};

	EXPECT_FALSE(IsHelp(1, arguments));
}
