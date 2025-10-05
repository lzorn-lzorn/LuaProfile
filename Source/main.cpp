#include <format>
#include <iostream>
#include <chrono>
#include <thread>
#include <type_traits>
#include "Logger.hpp"
//#include "LuaVM.hpp"
//using namespace LuaBenchmark;

void TestLog2() {
    std::cout << "\n=== Logger Test Start ===" << std::endl;
    
    try {

        std::filesystem::path path = "./Log/luavm_test.log";
        // 创建 Logger 实例
        Logger logger(
            "TestApp: ", 
            "=== Test Session Start ===", 
            "=== Test Session End ===", 
            path
        );
        
        std::cout << "Logger created successfully!" << std::endl;
        
        // 测试不同级别的日志
        logger.Log(Level::INFO, "Application initialization started", Output::BOTH);
        logger.Log(Level::INFO, "Loading configuration files", Output::BOTH);
        logger.Log(Level::WARNING, "Configuration file not found, using defaults", Output::BOTH);
        logger.Log(Level::INFO, "System ready", Output::BOTH);
        
        // 测试调试信息（只在 Debug 模式下显示）
        logger.Log(Level::DEBUGINFO, "Debug: Memory usage: 1024KB", Output::BOTH);
        
        // 模拟一些操作
        std::cout << "\n--- Simulating some operations ---" << std::endl;
        
        for (int i = 1; i <= 3; ++i) {
            logger.Log(Level::INFO, 
                      std::format("Processing item {}", i), 
                      Output::BOTH);
            
            // 模拟处理时间
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 测试错误日志
        logger.Log(Level::ERROR, "Simulated error for testing", Output::BOTH);
        
        // 测试只输出到文件
        logger.Log(Level::INFO, "This message only goes to file", Output::FILE);
        
        // 测试只输出到控制台
        logger.Log(Level::INFO, "This message only goes to console", Output::CONSOLE);
        
        std::cout << "\n--- Operations completed ---" << std::endl;
        logger.Log(Level::INFO, "All operations completed successfully", Output::BOTH);
        
        std::cout << "\nLogger test completed. Check the log file at:\n";
        
        // 验证日志文件内容
        if (std::filesystem::exists(path)) {
            std::cout << "\n=== Log File Content ===" << std::endl;
            std::ifstream logFile(path);
            std::string line;
            int lineNum = 1;
            while (std::getline(logFile, line) && lineNum <= 10) { // 只显示前10行
                std::cout << std::format("{:2}: {}", lineNum++, line) << std::endl;
            }
            if (lineNum > 10) {
                std::cout << "... (truncated)" << std::endl;
            }
            logFile.close();
            std::cout << "=========================" << std::endl;
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        std::cerr << "Path: " << e.path1() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Logger Test End ===" << std::endl;
}

void TestLog1() {
    std::cout << "\n=== Logger Test Start ===" << std::endl;
    
    // ✅ 先用绝对路径测试
    auto currentDir = std::filesystem::current_path();
    std::filesystem::path logDir;
    
    if (currentDir.filename() == "build") {
        logDir = currentDir.parent_path() / "Log";
    } else {
        logDir = currentDir / "Log";
    }
    
    auto logPath = logDir / "luavm_test.log";
    
    std::cout << "Current directory: " << currentDir << std::endl;
    std::cout << "Log directory: " << logDir << std::endl;
    std::cout << "Log file path: " << logPath << std::endl;
    std::cout << "Log directory exists: " << std::filesystem::exists(logDir) << std::endl;
    std::cout << "Log file exists: " << std::filesystem::exists(logPath) << std::endl;
    
    try {
        // 确保目录存在
        if (!std::filesystem::exists(logDir)) {
            std::cout << "Creating log directory..." << std::endl;
            std::filesystem::create_directories(logDir);
        }
        
        Logger logger(
            "TestApp: ", 
            "=== Test Session Start ===", 
            "=== Test Session End ===", 
            logPath
        );
        
        // ... 其余测试代码保持不变 ...
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void TestLuaVM() {
    std::cout << "Test LuaVM Start" << std::endl;
    // LuaVM vm("./Lua", "./Lua::main");
    std::cout << "Test LuaVM Over" << std::endl;
}

int main(){
    TestLuaVM();
    TestLog2();
    return 0;
}