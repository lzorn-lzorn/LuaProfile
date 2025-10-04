
#pragma once
#include <filesystem>
#include <iostream>
#include <format>
#include <fstream>

enum class Level : int {
	DEBUGINFO = 0,
	INFO,
	WARNING,
	ERROR
};
enum class Output {
	CONSOLE, /* 输出到控制台 */
	FILE,    /* 输出到文件 */
	BOTH     /* 同时输出到控制台和文件 */
};
/**
* @brief: 日志记录单例类
*/
class LoggerManager{
public:
	static LoggerManager& Instance(){
		static LoggerManager instance;
		return instance;
	}
	
public:
	void SetLogOutputPath(const std::filesystem::path& path){
		logFilePath = path;
	}
private:
	LoggerManager() = default;
	~LoggerManager() = default;
	LoggerManager(const LoggerManager&) = delete;
	LoggerManager& operator=(const LoggerManager&) = delete;

private:
	std::filesystem::path logFilePath{"./Log"};
};

struct Logger{
private:
	using LogFilePath = std::filesystem::path;
	LogFilePath logFilePath;
	std::string logStartMsg{""};
	std::string logEndMsg{""};
public:
	
	Logger(std::string prefix, std::string startmsg, std::string endmsg, LogFilePath path)
		: commonPrefix(prefix), logStartMsg(startmsg), logEndMsg(endmsg), logFilePath(path) {
		if (!logFilePath.empty()) {
			// 尝试创建或清空日志文件
			std::ofstream logFile(logFilePath, std::ios::trunc);
			if (logFile.is_open()) {
				logFile << logStartMsg << "\n";
				logFile.close();
			} else {
				std::cerr << "[ERROR] Unable to open log file: " << logFilePath << "\n";
			}
		}
	}
	Logger(const Logger&) = default;
	Logger& operator=(const Logger&) = default;
	Logger(Logger&&) = default;
	Logger& operator=(Logger&&) = default;

	~Logger(){
		if (!logFilePath.empty()) {
			// 尝试在日志文件末尾添加结束信息
			std::ofstream logFile(logFilePath, std::ios::app);
			if (logFile.is_open()) {
				logFile << logEndMsg << "\n";
				logFile.close();
			} else {
				std::cerr << "[ERROR] Unable to open log file: " << logFilePath << "\n";
			}
		}
	}

	void Log(Level level, const std::string& message, Output output=Output::BOTH){
		std::string levelStr;
		switch(level){
			case Level::DEBUGINFO: levelStr = "DEBUG"; break;
			case Level::INFO: levelStr = "INFO"; break;
			case Level::WARNING: levelStr = "WARNING"; break;
			case Level::ERROR: levelStr = "ERROR"; break;
			default: levelStr = "UNKNOWN"; break;
		}
		std::string logMessage = std::format("[{}] {}\n", levelStr, commonPrefix.append(message));
		
		if (output == Output::CONSOLE || output == Output::BOTH) {
			std::cout << logMessage;
		}
		if ((output == Output::FILE || output == Output::BOTH) && !logFilePath.empty()) {
			std::ofstream logFile(logFilePath, std::ios::app);
			if (logFile.is_open()) {
				logFile << logMessage;
				logFile.close();
			} else {
				std::cerr << "[ERROR] Unable to open log file: " << logFilePath << "\n";
			}
		}
	}
public:
	std::string commonPrefix{""};
};