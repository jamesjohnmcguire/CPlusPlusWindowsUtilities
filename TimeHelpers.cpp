/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <time.h>
#include "TimeHelpers.h"
#include "Utils.h"

void GetDateFromYYMMDDHHMMSS(char* str, TCHAR* formattedDate, size_t len)
{
	struct tm tm;
	int year = GetDigitsFromString(str, 0);
	if (year < 50)
	{
		year += 100;
	}

	tm.tm_year = year;
	tm.tm_mon = GetDigitsFromString(str, 2) - 1;
	tm.tm_mday = GetDigitsFromString(str, 4);
	tm.tm_hour = GetDigitsFromString(str, 6);
	tm.tm_min = GetDigitsFromString(str, 8);
	tm.tm_sec = 0;

	time_t t = str[strlen(str) - 1] == 'Z' ? _mkgmtime(&tm) : mktime(&tm);
	localtime_s(&tm, &t);

	swprintf_s(formattedDate, len, DATE_FORMAT, tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
}

void GetDateFromTime_t(time_t* time_t, TCHAR* formattedDate, size_t len)
{
	struct tm tm;
	gmtime_s(&tm, time_t);

	//wcsftime(formattedDate, len, _T("%m/%d %Y %H:%M"), &tm);
	swprintf_s(formattedDate, len, DATE_FORMAT, tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
}

#if !defined(_MSC_VER) && !defined(gmtime_s)
// Fallback for non-MSVC compilers (e.g., MinGW, GCC, Clang)
inline int gmtime_s(struct tm* _Tm, const time_t* _Time)
{
	struct tm* tmp = gmtime(_Time);
	if (!tmp) return 1;
	*_Tm = *tmp;
	return 0;
}
#endif
