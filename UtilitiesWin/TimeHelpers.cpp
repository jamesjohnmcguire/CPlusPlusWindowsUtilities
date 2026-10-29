/////////////////////////////////////////////////////////////////////////////
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TimeHelpers.h"
#include "Utils.h"

std::string GetDateFromTimeLocalInternal(std::time_t value);
std::string GetDateFromTimeUtcInternal(std::time_t value);

void GetDateFromYYMMDDHHMMSS(char* str, char* formattedDate, size_t len)
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

	sprintf_s(
		formattedDate,
		len,
		DATE_FORMAT,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_year + 1900,
		tm.tm_hour,
		tm.tm_min);
}

bool GetDateFromTimeLocal(
	std::time_t value,
	char* buffer,
	size_t len)
{
	try
	{
		auto s = GetDateFromTimeLocalInternal(value);

		if (s.size() + 1 > len)
			return false;

		std::memcpy(buffer, s.c_str(), s.size() + 1);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool GetDateFromTimeUtc(
	std::time_t value,
	char* buffer,
	size_t len)
{
	try
	{
		auto s = GetDateFromTimeUtcInternal(value);

		if (s.size() + 1 > len)
			return false;

		std::memcpy(buffer, s.c_str(), s.size() + 1);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

std::string GetDateFromTimeLocalInternal(std::time_t value)
{
	using namespace std::chrono;

	const auto tp = system_clock::from_time_t(value);
	auto timeRaw = floor<minutes>(tp);

	std::string formattedTime = std::format("{:%m/%d %Y %H:%M}", timeRaw);

	return formattedTime;
}

std::string GetDateFromTimeUtcInternal(std::time_t value)
{
	using namespace std::chrono;

	const auto tp = system_clock::from_time_t(value);
	auto utc = utc_clock::from_sys(tp);
	auto timeRaw = floor<minutes>(utc);

	std::string formattedTime = std::format("{:%m/%d %Y %H:%M}", timeRaw);

	return formattedTime;
}
