#pragma once

#include <string.h>
#include "log4cplus/loglevel.h"

namespace Utils {

	void Log(const std::string& data, bool to_console, int log_level = log4cplus::INFO_LOG_LEVEL);
	void Log(const std::wstring& data, bool to_console, int log_level = log4cplus::INFO_LOG_LEVEL);
}