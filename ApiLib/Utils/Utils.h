#pragma once

#include <windows.h>
#include <string>
#include <time.h>

namespace Utils {

	/************************************************************************/
	/* Date Time Functions                                                  */
	/************************************************************************/
	std::string GetCurrentDate();
	std::string GetCurrentDateTime();
	/*
	 * @param format
	 *     0: yyyy-MM-dd HH:mm:ss
	 *     1: yyyy-MM-dd
	 */
	__time64_t Str2Time64(const std::string& timestr, int format = 0);
	SYSTEMTIME Time64ToSystemTime(__time64_t timestamp);
	__time64_t SystemTime2Time64(const SYSTEMTIME& systime);
	__time64_t GetTimestamp();
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
	void CreateDirectory(const char* path);

	int CompareDouble(double val1, double val2, int precision = 6);

	/************************************************************************/
	/* String Functions                                                     */
	/************************************************************************/

	char* safe_strcpy(char* dst, const char* src, unsigned int max_length);
	wchar_t* safe_wstrcpy(wchar_t* dst, const wchar_t* src, unsigned int max_length);

	std::string replace(std::string& str, const std::string& from, const std::string& to);
	std::string replace_all(std::string& str, const std::string& from, const std::string& to);

	std::string ConvertUnicode2Multibyte(const wchar_t* str);
	std::wstring ConvertMultibyte2Unicode(const char* str);

	/************************************************************************/
	/* File Functions                                                       */
	/************************************************************************/
	bool ExistFile(const std::string& path);
	bool SaveFile(const std::string& path, const char* data, bool append = true);

	/************************************************************************/
	/* Windows Handler Functions                                            */
	/************************************************************************/
	void CloseHandleSafely(HANDLE& event_handle);
}
