#include "base/log.hpp"
#include <assert.h>
#include <ctime>
#include <iomanip>
#include <sstream>

void Log::logger(LogLevel log_level, const std::string& msg)
{
	std::string log_level_name = std::move(get_log_level_name(log_level));
	std::string log_time = std::move(get_time());
	std::string log_line = log_time + "-" + this->log_name + "-" + " [" + log_level_name + "] " + msg + "\n";
	int bytes = std::fwrite(log_line.c_str(), sizeof(char), log_line.size(), this->log_file.get());
	this->flush();
}

std::string Log::get_log_level_name(LogLevel log_level)
{
	std::string ret;
	switch (log_level)
	{
	case Log::LogLevel::DEBUG:
		ret = "DEBUG";
		break;
	case Log::LogLevel::INFO:
		ret = "INFO";
		break;
	case Log::LogLevel::WARN:
		ret = "WARN";
		break;
	case Log::LogLevel::ERR:
		ret = "ERROR";
		break;
	case Log::LogLevel::CRITICAL:
		ret = "CRITICAL";
		break;
	default:
		break;
	}
	return ret;
}

std::string Log::get_time()
{
	std::time_t now = std::time(nullptr);

	// 将时间转换为本地时间
	std::tm* local_time = std::localtime(&now);

	// 格式化时间为字符串
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);

	std::string ret(buffer);
	return ret;
}

void Log::debug(const std::string& msg)
{
	logger(LogLevel::DEBUG, msg);
}

void Log::info(const std::string& msg)
{
	logger(LogLevel::DEBUG, msg);
}

void Log::warn(const std::string& msg)
{
	logger(LogLevel::WARN, msg);
}

void Log::error(const std::string& msg)
{
	logger(LogLevel::ERR, msg);
}

void Log::critical(const std::string& msg)
{
	logger(LogLevel::CRITICAL, msg);
}

void Log::flush()
{
	fflush(this->log_file.get());
}

Log::Log(const std::string & log_name, const std::string & file_name)
{
	this->log_name = log_name;
	this->log_file =  std::shared_ptr<FILE>(fopen(file_name.c_str(), "a+"), fclose);
	assert(this->log_file != nullptr);
	this->log_level = LogLevel::DEBUG;
}

Log::Log()
{
}

std::map<std::string, Log> Log::logs;
Log* Log::get_logger(const std::string& log_name, const std::string& file_name)
{
	if (logs.find(log_name) == logs.end()) {
		Log logger(log_name, file_name);
		logs[log_name] = logger;
	}
	return &logs[log_name];
}

void Log::remove_logger(const std::string& log_name)
{
	auto it = logs.find(log_name);
	if (it != logs.end()) {
		logs.erase(it);
	}
}

Log::~Log()
{
}
