
#pragma once
#include <filesystem>
#include <iostream>
#include <format>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

enum class Level : int {
	DEBUGINFO = 0,
	MARK,
	INFO,
	WARNING,
	ERROR
};
enum class Output {
	CONSOLE, /* 输出到控制台 */
	FILE,    /* 输出到文件 */
	BOTH     /* 同时输出到控制台和文件 */
};


struct Logger{
private:
	using LogFilePath = std::filesystem::path;
	LogFilePath logFilePath;
	std::string logStartMsg{""};
	std::string logEndMsg{""};

public:
	std::string commonPrefix{""};
public:
	
	Logger(std::string prefix, std::string startmsg, std::string endmsg, LogFilePath path)
		: commonPrefix(prefix), logStartMsg(startmsg), logEndMsg(endmsg), logFilePath(path) {
		if (logFilePath.empty()) {
			const std::string errMsg = "Path is empty \n";
			std::cerr << errMsg;
			throw std::runtime_error(errMsg);
		}
    
		try {
			if (!std::filesystem::exists(logFilePath)){
				auto parentPath = logFilePath.parent_path();
				if (!parentPath.empty()){
					if (!std::filesystem::exists(parentPath)) try {
						std::filesystem::create_directories(parentPath);
					}catch(std::filesystem::filesystem_error& exception){
						/* 创建文件目录失败 */
						std::cerr << exception.what() << "\n";
						throw exception; 
					}
				}
				// 创建空文件
				std::ofstream ofs(logFilePath, std::ios::out);
				if (!ofs) {
					const std::string curAbsolutePath = std::filesystem::absolute(logFilePath).string();
					throw std::runtime_error(
						std::format("Create Log File Failed: {}", curAbsolutePath)
					);
				}
				ofs.close();
			}
			if (!logStartMsg.empty()){
				OutputMessage(Position::START, Level::MARK, logStartMsg, Output::BOTH);
			}
			
		}catch(const std::runtime_error& runtime_error){
			std::cerr << runtime_error.what() << "\n";
			throw runtime_error;
		}
	}
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger(Logger&& other) = delete;
	Logger& operator=(Logger&& other) = delete;

	~Logger(){
		try {
			if (!logEndMsg.empty()) [[unlikely]] {
				OutputMessage(Position::END, Level::MARK, logEndMsg, Output::BOTH);
			}
		}catch (const std::runtime_error& e){
			std::cerr << e.what() << "\n";
		}
	}

	void Log(Level level, const std::string& message, Output output=Output::BOTH){
		OutputMessage(Position::MIDDLE, level, message, output);
	}
private:
	enum class Position {
		START,
		MIDDLE,
		END
	};
	void OutputMessage(Position pos, Level level, const std::string& message, Output output=Output::BOTH){ 
		std::string realMsg;
		if (pos == Position::START){
			realMsg = logStartMsg;
		} else if (pos == Position::END){
			realMsg = logEndMsg;
		} else {
			if (level == Level::INFO){
				realMsg = "\t[INFO]: " + commonPrefix + message + "\n";
			}else if (level == Level::WARNING){
				realMsg = "\t[WARNING]: " + commonPrefix + message + "\n";
			}else if (level == Level::ERROR){
				realMsg = "\t[ERROR]: " + commonPrefix + message + "\n";
			}else if (level == Level::DEBUGINFO){
			#ifndef NDEBUG
				realMsg = "\t[DEBUG]: " + commonPrefix + message + "\n";
			#else
				return;
			#endif
			}else {
				realMsg = "\t[UNKNOWN]: " + message;
			
			}
		}
		if (realMsg.empty()) return;
		if (output == Output::CONSOLE || output == Output::BOTH) {
			std::cout << realMsg;
		}
		if (output == Output::FILE || output == Output::BOTH){
			WriteMessageToFile(realMsg);
		}

	}

	void WriteMessageToFile(std::string msg){
		if (logFilePath.empty()) [[unlikely]] {
			throw std::runtime_error(
				std::format("Log File Path: {} is empty", logFilePath.string())
			);
		}
		if (msg.empty()) [[unlikely]] {
			return;
		}
		std::ofstream logFile(logFilePath, std::ios::app); 
		if (logFile.is_open()) {
			logFile << msg << "\n";
			logFile.close();
		} else {
			throw std::runtime_error(
				std::format("Open Log File Failed: {}", logFilePath.string())
			);
		}
		
	}

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
	/**
	 * @brief 往loggerIdx 号日志中写入日志
	 * @param loggerId 日志编号
	 * @param level 日志等级
	 * @param message 日志内容
	 * @param output 日志输出位置
	*/
	void Log(uint32_t loggerIdx, Level level, const std::string& message, Output output=Output::BOTH){
		if (loggerIdx >= loggers.size()) [[unlikely]]{
			std::cerr << "[ERROR] Logger index out of range: " << loggerIdx << "\n";
			return;
		}
		loggers[loggerIdx].Log(level, message, output);

	}
private:
	LoggerManager(std::string path="./Log") : logFilePath(path){
		
	}
	~LoggerManager() = default;
	LoggerManager(const LoggerManager&) = delete;
	LoggerManager& operator=(const LoggerManager&) = delete;

private:
	std::filesystem::path logFilePath{""};
	std::vector<Logger> loggers;
};

