#include "Utils.h"
#include <fstream>

std::string Utils::GetCurrentDate()
{
	char buf[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	sprintf_s(buf, 256, "%04d-%02d-%02d",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay);
	return buf;
}

std::string Utils::GetCurrentDateTime()
{
	char buf[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	sprintf_s(buf, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	return buf;
}

__time64_t Utils::Str2Time64(const std::string& timestr, int format/* = 0*/)
{
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0, milli = 0;

	switch (format) {
	case 0:
		if (sscanf_s(timestr.c_str(), "%d-%d-%d %d:%d:%d",
			&year, &month, &day, &hour, &minute, &second) < 3)
			return 0;
		break;

	case 1:
		hour = 0;
		minute = 0;
		second = 0;
		if (sscanf_s(timestr.c_str(), "%d-%d-%d",
			&year, &month, &day) < 3)
			return 0;
		break;
		
	default:
		return 0;
	}
	

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

	return time_stamp * 1000;
}

SYSTEMTIME Utils::Time64ToSystemTime(__time64_t timestamp)
{
	__time64_t local = timestamp / 1000;
	struct tm tm = { 0 };
	localtime_s(&tm, &local);

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

__time64_t Utils::SystemTime2Time64(const SYSTEMTIME& systime)
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

__time64_t Utils::GetTimestamp()
{
	SYSTEMTIME sysTime = { 0 };
	GetLocalTime(&sysTime);
	return SystemTime2Time64(sysTime);
}

std::string Utils::GetTimeString(__time64_t timestamp, int format /*= 1*/)
{
	SYSTEMTIME systime = Time64ToSystemTime(timestamp);
	return GetTimeString(systime, format);
}

std::string Utils::GetTimeString(SYSTEMTIME systime, int format /*= 1*/)
{
	char buf[256];
	switch (format) {
	case 0:
		sprintf_s(buf, 256, "%04d-%02d-%02d",
			systime.wYear, systime.wMonth, systime.wDay);
		break;
	default:
		sprintf_s(buf, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
		break;
	}
	return buf;
}

SYSTEMTIME Utils::AddTime(const SYSTEMTIME& systime, int field, int num)
{
	__time64_t timestamp = SystemTime2Time64(systime);
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
	return Time64ToSystemTime(timestamp);
}

std::string Utils::GetTempPath(const std::string &filepath)
{
	char buffer[MAX_PATH + 1];
	::GetTempPathA(MAX_PATH, buffer);
	std::string str = buffer;
	return str.substr(0, str.find_last_of('\\') + 1) + filepath;
}

std::string Utils::GetRelativePath(const char* path)
{
	char szFull[MAX_PATH], szDrv[_MAX_DRIVE], szPath[MAX_PATH];
	GetModuleFileNameA(NULL, szFull, MAX_PATH);
	_splitpath_s(szFull, szDrv, _MAX_DRIVE, szPath, MAX_PATH, NULL, 0, NULL, 0);
	_makepath_s(szFull, MAX_PATH, szDrv, szPath, NULL, NULL);
	
	return std::string(szFull) + path;
}

int Utils::CompareDouble(double val1, double val2, int precision /*= 6*/)
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

char* Utils::safe_strcpy(char* dst, const char* src, unsigned int max_length)
{
	size_t len = strlen(src);
	strncpy_s(dst, max_length, src, len > max_length ? max_length : len);
	dst[max_length - 1] = 0;
	return dst;
}

wchar_t* Utils::safe_wstrcpy(wchar_t* dst, const wchar_t* src, unsigned int max_length)
{
	size_t len = wcslen(src);
	wcsncpy_s(dst, max_length, src, len > max_length ? max_length : len);
	dst[max_length - 1] = 0;
	return dst;
}

std::string Utils::replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return str;
}

std::string Utils::replace_all(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return false;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return str;
}

std::string Utils::ConvertUnicode2Multibyte(const wchar_t* str)
{
	if (!str) {
		return "";
	}

	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	char* pMultiByteString = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str, -1, pMultiByteString, len, NULL, NULL);

	std::string result;
	result.append(pMultiByteString);

	delete[]pMultiByteString;
	pMultiByteString = NULL;

	return result;
}

std::wstring Utils::ConvertMultibyte2Unicode(const char* str)
{
	if (!str) {
		return L"";
	}

	int nBufferLength = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	wchar_t* pUnicodeString = new wchar_t[nBufferLength];
	MultiByteToWideChar(CP_ACP, 0, str, -1, pUnicodeString, nBufferLength);

	std::wstring wstr = pUnicodeString;

	delete[]pUnicodeString;
	pUnicodeString = NULL;

	return wstr;
}

bool Utils::ExistFile(const std::string& path)
{
	std::ifstream f(path.c_str());
	bool exist = f.good();
	f.close();
	return exist;
}

bool Utils::SaveFile(const std::string& path, const char* data, bool append /*= true*/)
{
	std::ofstream fout;

	if (append) {
		if (ExistFile(path)) {
			fout.open(path, std::ofstream::out | std::ofstream::app);
		}
		else {
			fout.open(path);
		}
	}
	else {
		fout.open(path);
	}

	if (fout.good()) {
		fout.clear();
		fout << data;
		fout.close();
		return true;
	}
	return false;
}

void Utils::CloseHandleSafely(HANDLE& event_handle)
{
	if (event_handle) {
		::CloseHandle(event_handle);
		event_handle = NULL;
	}
}

