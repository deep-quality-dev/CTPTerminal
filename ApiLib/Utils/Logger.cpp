#include "Logger.h"
#include "Utils/Utils.h"
#include <iostream>
#include "log4cplus/logger.h"

void Utils::Log(const std::string& data, bool to_console, int log_level /*= log4cplus::INFO_LOG_LEVEL*/)
{
	if (to_console) {
		std::cout << GetCurrentDateTime() << " [INFO] " << data << std::endl;
	}
	if (log_level >= log4cplus::INFO_LOG_LEVEL)
		LOG4CPLUS_INFO(log4cplus::Logger::getInstance(L"Test1"), ConvertMultibyte2Unicode(data.c_str()));
}

void Utils::Log(const std::wstring& data, bool to_console, int log_level /*= log4cplus::INFO_LOG_LEVEL*/)
{
	if (to_console) {
		std::cout << GetCurrentDateTime() << " [INFO] " << ConvertUnicode2Multibyte(data.c_str()).c_str() << std::endl;
	}
	if (log_level >= log4cplus::INFO_LOG_LEVEL)
		LOG4CPLUS_INFO(log4cplus::Logger::getInstance(L"Test1"), data.c_str());
}
