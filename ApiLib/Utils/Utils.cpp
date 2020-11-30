#include <string>
#include <windows.h>
#include <time.h>
#include "Utils.h"

std::string GetCurrentDate()
{
	char buf[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	sprintf_s(buf, 256, "%04d-%02d-%02d",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay);
	return buf;
}

__time64_t CalcTimestamp(const std::string& timestr)
{
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0, milli = 0;

	if (sscanf_s(timestr.c_str(), "%d-%d-%d %d:%d:%d",
		&year, &month, &day, &hour, &minute, &second) < 3)
		return 0;

	year -= 1900;

	tm tmElements;
	__time64_t time_stamp;

	tmElements.tm_year = year;
	tmElements.tm_mon = month - 1;
	tmElements.tm_mday = day;
	tmElements.tm_hour = hour;
	tmElements.tm_min = minute;
	tmElements.tm_sec = second;
	tmElements.tm_isdst = 0;

	tmElements.tm_yday = -1;
	tmElements.tm_wday = -1;

	time_stamp = _mktime64(&tmElements);

	return time_stamp;
}

SYSTEMTIME GetSystemTime(__time64_t timestamp)
{
	struct tm tm = { 0 };
	gmtime_s(&tm, &timestamp);

	SYSTEMTIME systime = { 0 };
	systime.wYear = tm.tm_year + 1900;
	systime.wMonth = tm.tm_mon + 1;
	systime.wDay = tm.tm_mday;
	systime.wHour = tm.tm_hour;
	systime.wMinute = tm.tm_min;
	systime.wSecond = tm.tm_sec;
	systime.wMilliseconds = timestamp % 1000;
	systime.wDayOfWeek = tm.tm_wday;
	return systime;
}

__time64_t CalcTimestampMilli(SYSTEMTIME systime)
{
	struct tm tm;
	__time64_t time_stamp;

	tm.tm_year = systime.wYear - 1900;
	tm.tm_mon = systime.wMonth - 1;
	tm.tm_mday = systime.wDay;
	tm.tm_hour = systime.wHour;
	tm.tm_min = systime.wMinute;
	tm.tm_sec = systime.wSecond;
	tm.tm_isdst = 0;
	tm.tm_yday = -1;
	tm.tm_wday = -1;
	time_stamp = _mktime64(&tm) * 1000 + systime.wMilliseconds;

	return time_stamp;
}

SYSTEMTIME AddTime(const SYSTEMTIME& systime, int field, int num)
{
	__time64_t timestamp = CalcTimestampMilli(systime);
	switch (field)
	{
	case 0: // add date
		timestamp += num * 24 * 60 * 60 * 1000;
		break;

	case 1: // add hour
		timestamp += num * 60 * 60 * 1000;
		break;

	case 2: // add minute
		timestamp += num * 60 * 1000;
		break;

	case 3: // add second
		timestamp += num * 1000;
		break;

	default:
		return systime;
	}
}

std::string GetTempPath(const std::string &filepath)
{
	char buffer[MAX_PATH + 1];
	::GetTempPathA(MAX_PATH, buffer);
	std::string str = buffer;
	return str.substr(0, str.find_last_of('\\') + 1) + filepath;
}

char* safe_strcpy(char* dst, const char* src, unsigned int max_length)
{
	size_t len = strlen(src);
	strncpy_s(dst, max_length, src, len > max_length ? max_length : len);
	dst[max_length - 1] = 0;
	return dst;
}

int CompareDouble(double val1, double val2, int precision /*= 6*/)
{
	double dPrecision = (double)1 / (double)pow(10.0, precision);

	if (((val1 - dPrecision) < val2) && ((val1 + dPrecision) > val2))
	{
		return 0;
	}
	else if (val1 > val2)
	{
		return 1;
	}

	return -1;
}

