#include "Logger.h"
#include "Utils.h"
#include "log4z.h"
#include <iostream>

using namespace zsummer::log4z;

static const char* g_log_path = "./log2";
static bool g_initialized = false;

void Utils::LogInitialize(const char* path)
{
	if (g_initialized) {
		return;
	}

	//start log4z
	ILog4zManager::getRef().setLoggerPath(LOG4Z_MAIN_LOGGER_ID, path == NULL ? g_log_path : path);
	ILog4zManager::getRef().start();
	ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_TRACE);

	g_initialized = true;
}

void Utils::Log(const std::string& data, bool to_console /*= false*/, int log_level /*= ENUM_LOG_LEVEL::LOG_LEVEL_INFO*/)
{
	LogInitialize();

	if (to_console) {
		std::cout << GetCurrentDateTime() << " [INFO] " << data << std::endl;
	}
 	if (log_level >= ENUM_LOG_LEVEL::LOG_LEVEL_INFO)
 		LOGI(data);
}

void Utils::Log(const std::wstring& data, bool to_console /*= false*/, int log_level /*= ENUM_LOG_LEVEL::LOG_LEVEL_INFO*/)
{
	LogInitialize();

	if (to_console) {
		std::cout << GetCurrentDateTime() << " [INFO] " << ConvertUnicode2Multibyte(data.c_str()).c_str() << std::endl;
	}
 	if (log_level >= ENUM_LOG_LEVEL::LOG_LEVEL_INFO)
 		LOGI(ConvertUnicode2Multibyte(data.c_str()));
}
