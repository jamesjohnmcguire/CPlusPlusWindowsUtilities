#pragma once

#include "Common.h"

#define DATE_FORMAT _T("%02d/%02d %d %02d:%02d")

DllExport void GetDateFromYYMMDDHHMMSS(
	char* str,
	TCHAR* formattedDate,
	size_t len);

DllExport void GetDateFromTime_t(
	time_t* time_t,
	TCHAR* formattedDate,
	size_t len);
