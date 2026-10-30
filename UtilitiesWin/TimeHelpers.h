#pragma once

#include "UtilitiesWin.h"

#define DATE_FORMAT "%02d/%02d %d %02d:%02d"

DllExport void GetDateFromYYMMDDHHMMSS(
	char* str,
	char* formattedDate,
	size_t len);

extern "C" DllExport bool GetDateFromTimeLocal(
	std::time_t value,
	char* buffer,
	size_t len);
extern "C" DllExport bool GetDateFromTimeUtc(
	std::time_t value,
	char* buffer,
	size_t len);
