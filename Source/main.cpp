#include <format>
#include <iostream>
#include "Logger.hpp"
//#include "LuaVM.hpp"
//using namespace LuaBenchmark;

void TestLog() {
    std::cout << "Test LuaVM Start" << std::endl;
    
    // 创建一个 Logger 实例进行测试
    Logger logger("LuaVM_Test: ", "=== LuaVM Test Start ===", "=== LuaVM Test End ===", "./Log/luavm_test.log");
    logger.Log(Level::INFO, "Creating LuaVM instance", Output::BOTH);
    
    try {
        logger.Log(Level::INFO, "LuaVM created successfully", Output::BOTH);
    } catch (const std::exception& e) {
        logger.Log(Level::ERROR, std::format("LuaVM creation failed: {}", e.what()), Output::BOTH);
    }
    
    std::cout << "Test LuaVM Over" << std::endl;
}

void TestLuaVM() {
    std::cout << "Test LuaVM Start" << std::endl;
    // LuaVM vm("./Lua", "./Lua::main");
    std::cout << "Test LuaVM Over" << std::endl;
}

int main(){
    TestLuaVM();
    TestLog();
    return 0;
}