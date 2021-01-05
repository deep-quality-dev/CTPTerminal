#pragma once

#include <string.h>
#include "log4z.h"

namespace Utils {

	void LogInitialize(const char* path = NULL);

	void Log(const std::string& data, bool to_console = true, int log_level = ENUM_LOG_LEVEL::LOG_LEVEL_INFO);
	void Log(const std::wstring& data, bool to_console = true, int log_level = ENUM_LOG_LEVEL::LOG_LEVEL_INFO);
}