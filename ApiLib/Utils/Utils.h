#pragma once

#include <windows.h>
#include <string>
#include <time.h>

/************************************************************************/
/* Date Time Functions                                                  */
/************************************************************************/
std::string GetCurrentDate();
std::string GetCurrentDateTime();
__time64_t CalcTimestamp(const std::string& timestr);
SYSTEMTIME GetSystemTime(__time64_t timestamp);
__time64_t CalcTimestampMilli(SYSTEMTIME systime);
/*
 * @param format
 *    0: yyyy-MM-dd
 *    1: yyyy-MM-dd HH:mm:ss.fff
 */
std::string GetTimeString(__time64_t timestamp, int format = 1);
std::string GetTimeString(SYSTEMTIME systime, int format = 1);
/*
 * @param field
 *	  0: add date
 *	  1: add hour
 *	  2: add minute
 *	  3: add second
 */
SYSTEMTIME AddTime(const SYSTEMTIME& systime, int field, int num);

std::string GetTempPath(const std::string& filepath);
std::string GetRelativePath(const char* path);

int CompareDouble(double val1, double val2, int precision = 6);

/************************************************************************/
/* String Functions                                                     */
/************************************************************************/

char* safe_strcpy(char* dst, const char* src, unsigned int max_length);

std::string replace(std::string& str, const std::string& from, const std::string& to);
std::string replace_all(std::string& str, const std::string& from, const std::string& to);

std::string ConvertUnicode2Multibyte(const wchar_t* str);
std::wstring ConvertMultibyte2Unicode(const char* str);

/************************************************************************/
/* File Functions                                                       */
/************************************************************************/
bool ExistFile(const std::string& path);
bool SaveFile(const std::string& path, const char* data, bool append = true);