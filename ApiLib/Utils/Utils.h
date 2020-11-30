#pragma once

#include <windows.h>
#include <string>
#include <time.h>

std::string GetCurrentDate();
std::string GetCurrentDateTime();
__time64_t CalcTimestamp(const std::string& timestr);
SYSTEMTIME GetSystemTime(__time64_t timestamp);
__time64_t CalcTimestampMilli(SYSTEMTIME systime);
// field: 0, add date
// field: 1, add hour
// field: 2, add minute
// field: 3, add second
SYSTEMTIME AddTime(const SYSTEMTIME& systime, int field, int num);

std::string GetTempPath(const std::string& filepath);
std::string GetRelativePath(const char* path);

char* safe_strcpy(char* dst, const char* src, unsigned int max_length);

int CompareDouble(double val1, double val2, int precision = 6);

std::string ConvertUnicode2Multibyte(const wchar_t* str);
std::wstring ConvertMultibyte2Unicode(const char* str);
