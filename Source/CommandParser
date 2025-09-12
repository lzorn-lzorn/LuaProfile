#include <iostream>
#include <benchmark/benchmark.h>
#include <chrono>
#include <limits>
#include <thread>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "lua.hpp" // LuaJIT 头文件
#include "LuaVM.hpp"
#include "Tools.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

void CheckCommandNum(const std::vector<std::string>& args, int expected) {
    if (args.size() != expected) {
        std::string info = "";
        if (expected == 0){
            info = std::format("{} 不接受参数, 请使用 --help 查看帮助信息", args[1]);
        }else{
            info = std::format("{} 并不支持 {} 个参数, 请使用 --help 查看帮助信息", args[1], args.size() - 2);
        }
        throw std::invalid_argument(info);
    }
}

struct Command{
    std::string name;
    /* Command 自己的参数*/
    std::vector<std::string> args;
    /* Command 子命令对应的参数 */
    std::unordered_map<std::string, std::vector<std::string>> argMap;
};
auto ParseCommand(const std::vector<std::string>& args){
    Command cmd;
    cmd.name = args[1];
    int i=2;
    int len = args.size();
    while(i < len){
        if (args[i].rfind("-", 0) == 0){ // 以 - 开头的参数
            /* 2. 然后压入子命令即对应的参数 */
            std::string key = args[i];
            std::vector<std::string> values; 
            // 收集后续的非 - 开头的参数作为该键的值
            while (i + 1 < len && args[i + 1].rfind("-", 0) != 0) {
                values.push_back(args[++i]);
            }
            cmd.argMap[key] = values;
        } else {
            /* 1. 首先压入自己的命令 */
            cmd.args.push_back(args[i]);
            i++;
        }
    }
    return cmd;
}

void CommandParser(const std::vector<std::string>& args){
    for (auto const it: args) {
        std::cout << it << " ";
    }
    std::cout << "\n";
    if (args[1] == "--help") {
        CheckCommandNum(args, 0);
        // 显示帮助信息
        std::cout << "Help Say Hello\n";
    } else if (args[1] == "--version") {
        CheckCommandNum(args, 0);
        // 显示版本信息
        std::cout << "Version Say Hello\n";
    } else if (args[1] == "--list") {
        CheckCommandNum(args, 0);
        // 列出所有Lua虚拟机
        std::cout << "List Say Hello\n";
    } else if (args[1] == "--create") {
        // 创建Lua执行环境
    } else if (args[1] == "--destroy") {
        // 销毁Lua执行环境
    } else if (args[1] == "--run") {
        // 运行Lua脚本
    } else {
        std::cout << "Unknown command: " << args[1] << std::endl;
    }
}

int entry(int argc, char** argv) {
    std::cout << "Argc:"  << argc << std::endl;
    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
        // std::cout << std::format("Argv[{}]: {}\n", i, argv[i]);
    } 
/*
 * --help: 显示帮助信息并退出
 * --version: 显示版本信息并退出
 * --list: 列出当前所有注册的Lua虚拟机
 * --create: 创建一个lua执行环境
 *    -m <name>: 指定lua虚拟机名称
 *    -w <path>: 指定lua工作目录(根目录)
 *    -s <script>: 指定lua脚本文件
 *    -p <path>: 指定检测报告输出路径
 * --destroy: 销毁一个lua执行环境
 *    -m <name>: 指定lua虚拟机名称
 * --run: 运行Lua脚本
 *    <name>: 指定lua虚拟机名称
 *    -a  <args>: 传递给Lua脚本的参数
 *    -af <file>: 指定文件作为参数传递给Lua脚本
 *    -r <num>: 指定脚本运行次数，默认1次
 */
    for (auto const it: args) {
        std::cout << it << " ";
    }
    std::cout << "\n";
    try {
        CommandParser(args);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
  
    return 0;
}
