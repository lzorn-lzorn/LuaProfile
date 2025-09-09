
#pragma once
#include "lauxlib.h"
#include "lua.h"
#include "lua.hpp"
#include <benchmark/benchmark.h>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <regex>
#include <string>
#include <type_traits>
#include <optional>
#include <iostream>
#include <vector>

#include  "Tools.hpp"
namespace LuaBenchmark {
inline static bool CheckLuaFunction(const std::filesystem::path& modulePath, const std::string& funcname){
	if (modulePath.empty() || !std::filesystem::exists(modulePath)){
		LOG(Error, "Invalid module path: {}", modulePath.string());
		return false;
	}
	// 通过简单的词法分析解析源码文件是否存在该函数
	try{
		std::ifstream  file(modulePath);
		if (!file){
			LOG(Error, "CheckLuaFunction Failed to open file: {}", modulePath.string());
            return false;
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		// 全局函数
		std::regex patternGlobalFunc("function\\s+" + funcname + "\\s*\\(");
		// 局部函数
		std::regex patternLocalFunc("local\\s+function\\s+" + funcname + "\\s*\\(");
		// 表分配函数 moduleName.funcname = funcname(...)
		std::regex patternTableFunc("\\w+\\." + funcname + "\\s*=\\s*function\\s*\\(");
		// 变量分配函数
		std::regex patternVarFunc("\\b" + funcname + "\\s*=\\s*function\\s*\\(");
        // 返回表中的函数: return { funcname = function(...) 或 funcname = function(...)
        std::regex patternReturnTable("return\\s*\\{[^\\}]*\\b" + funcname + "\\s*=\\s*function\\s*\\(");

        // 检查是否匹配任一模式
        if (std::regex_search(content, patternGlobalFunc) ||
            std::regex_search(content, patternLocalFunc) ||
            std::regex_search(content, patternTableFunc) ||
            std::regex_search(content, patternVarFunc) ||
            std::regex_search(content, patternReturnTable)) {
            LOG(Info, "Found function '{}' in module: {}", funcname, modulePath.string());
            return true;
        }

        LOG(Error, "Function '{}' not found in module: {}", funcname, modulePath.string());
        return false;

	}catch(const std::exception& e){
		LOG(Error, "Exception while parsing Lua file: {}", e.what());
        return false;
	}


}



class LuaVM{
	struct LuaVMDeleter{
		void operator()(lua_State* L) const {
			if(L) lua_close(L);
		}
	};
	struct LuaResult{
		bool bSuccess {false};
		std::string msgError {};
		std::optional<double> luaResult {std::nullopt};
		std::optional<std::string> luaLog {std::nullopt};
		LuaResult() 
			: bSuccess(false), msgError(""), luaResult(std::nullopt), luaLog(std::nullopt) {}
		LuaResult(bool ret, std::string error, 
			std::optional<double> luaReturn=std::nullopt, 
			std::optional<std::string> log=std::nullopt)
			: bSuccess(ret), msgError(error), luaResult(luaReturn), luaLog(log) {}

		operator bool() const{
			return bSuccess;
		}
	};
	struct LuaProfileReport{
		LuaResult result {};
	};
	struct LuaEntry{
		std::string luaFileName{""};
		std::string luaFuncName{""};
	};

	using LuaWorkspace = std::filesystem::path;
	using LuaVMInstancePtr = std::unique_ptr<lua_State, LuaVMDeleter>;
	using LuaResultPtr = LuaResult*;

public:
	/* 
	 * @function: 构造函数
	 * @param path: Lua 脚本工作空间 (目录路径)
	 * @param funcname: Lua 执行的入口函数, 
	 * @ 	格式: "<模块名>::<函数名>" (不用.lua 后缀名)
	*/
	LuaVM(std::filesystem::path pathWorkspace, const std::string& funcname) {
		InitLuaVMContext();
		InitWorkSpace(pathWorkspace);
		InitEntryFunction(funcname);

	}
	~LuaVM() = default;

public:
	LuaProfileReport Run() {
		LuaProfileReport report {};

		return report;
	}
private:
	LuaResult InitLuaVMContext(){	
		lua_State* L = luaL_newstate();
		PushLog("Init Lua VM Context:");	
		if (!L){
			LuaResult ret {};
			ret.bSuccess = false;
			ret.msgError = "Failed to create Lua VM; lua_State* L == nullptr"; 
			PushLog(&ret, true);
			return ret;
		}
		luaVMContext = LuaVMInstancePtr(L);
		/* 导入lua库 */
		luaL_openlibs(luaVMContext.get());

		LuaResult ret {};
		ret.bSuccess = true;
		ret.msgError = "Create Lua VM success"; 
		PushLog(&ret);
		return ret;
	}
	LuaResult InitWorkSpace(std::filesystem::path path) {
		PushLog("Init Lua Work Space:");
		if (!CheckPath(path))[[unlikely]]{
			LuaResult ret = LuaResult(
				false,
				std::format("Directory Error, path = {}", path.string())
			);
			PushLog(&ret, true);
			return ret;
		}
		if (!luaVMContext){
			LuaResult ret = LuaResult(
				false,
				"Lua VM Context Error! LuaVM::luaVMContext not exists"
			);
			PushLog(&ret, true);
			return ret;
		}
		auto luaVMptr = luaVMContext.get(); 
		workspace = path;

		
		lua_getglobal(luaVMptr, "package");
		if(!lua_istable(luaVMptr, -1)){
			LuaResult ret = LuaResult(
				false,
				"Lua package table not found"
			);
			PushLog(&ret, true);
			return ret;
		}

		/* 设置 lua 的工作目录 */
		lua_getfield(luaVMptr, -1, "path");
		const char* current_path = lua_tostring(luaVMptr, -1);
		std::string root_path = current_path ? current_path : "";
		LOG(Info, "current path is {}", root_path);
		lua_pop(luaVMptr, 1);

		std::string package_path = root_path + ";" + // 单文件模块
			workspace.string() + "/?.lua;"    + // 无扩展名模块
			workspace.string() + "/?;"        + // 子目录模块
			workspace.string() + "/?/?.lua;"  + // 子目录模块
			workspace.string() + "/?/?;"      + // 子目录中无扩展名模块
			workspace.string() + "/?.luac";     // 预编译模块

		LOG(Info, "Lua package.path is {}", package_path);

		lua_pushstring(luaVMptr, package_path.c_str());
		lua_setfield(luaVMptr, -2, "path");
		
		// 设置C模块搜索路径
		lua_getfield(luaVMptr, -1, "cpath");
		const char* current_cpath = lua_tostring(luaVMptr, -1);
		std::string root_cpath = current_cpath ? current_cpath : "";
		lua_pop(luaVMptr, 1);

		std::string clib_path = root_cpath + ";" + 
			workspace.string() + "/?.dlll"     + // Windows
			workspace.string() + "/?.so"       + // Linux
			workspace.string() + "/loadall.dll"; // 通用加载器

		LOG(Info, "Lua package.path is {}", clib_path);

		lua_pushstring(luaVMptr, clib_path.c_str());
		lua_setfield(luaVMptr, -2, "cpath");

		// 模块加载器
		const char* luaLoader = R"(
			function loadmodule(name)
				local module = require(name)
				if module then
					_G[name] = module
				end
				return module
			end
			-- 设置工作目录
			__WORKDIR = ...
		)";

		if (luaL_dostring(luaVMptr, luaLoader) != LUA_OK){
			LOG(Error, "Failed to set up Lua Helper functions: {}", lua_tostring(luaVMptr, -1));
		}

		// 设置工作目录为全局变量
		lua_pushstring(luaVMptr, workspace.string().c_str());
		lua_setglobal(luaVMptr, "__WORKDIR");

		lua_pop(luaVMptr, 1);
		LuaResult ret = LuaResult(
			true,
			"Success! Path Check Pass"
		);
		PushLog(&ret);
		return ret;
	}

	LuaResult InitEntryFunction(const std::string& funcname){
		if (!luaVMContext) {
			LuaResult ret(false, "Lua VM Context is not initialized");
			PushLog(&ret, true);
			return ret;
		}

		luaEntryFunction = funcname;
		LuaEntry entry = GetLuaEntry();
		
		if (entry.luaFileName.empty()) {
			LuaResult ret(false, "Invalid entry function format. Expected 'module::function'");
			PushLog(&ret, true);
			return ret;
		}

		std::filesystem::path modulePath = FindLuaModule(entry.luaFileName);

		if(modulePath.empty()){
			LuaResult ret(false, 
            std::format("Module '{}' not found in workspace", entry.luaFileName));
			PushLog(&ret, true);
			return ret;
		}
		
		    // 检查函数是否存在
		if (!CheckLuaFunction(modulePath, entry.luaFuncName)) {
			LuaResult ret(false, 
				std::format("Function '{}' not found in module '{}'", 
						entry.luaFuncName, entry.luaFileName));
			PushLog(&ret, true);
			return ret;
		}
		
		LuaResult ret(true, 
			std::format("Entry function '{}::{}' found at {}", 
					entry.luaFileName, entry.luaFuncName, modulePath.string()));
		PushLog(&ret);
		return ret;
	}
private:

	std::filesystem::path FindModule(const std::string&  moduleName) const {
		if(!CheckPath(workspace)){
			LOG(Error, "FindLuaModule Failed, workspace is invalid, module name = {}", moduleName);
        	return "";
		}

		std::string normalizedName = moduleName;
		std::replace(normalizedName.begin(), normalizedName.end(), '.', '/');

		std::vector<std::filesystem::path> possiblePaths = {
			workspace / (normalizedName + ".lua"),
			workspace / normalizedName
		};
		// 先找可能存的预设目录
		for(const auto&path : possiblePaths){
			if(std::filesystem::exists(path) &&
			   std::filesystem::is_regular_file(path)){
				LOG(Info, "Find Lua Module: {} at {}", moduleName, path.string());
				return path;
			}
		}
		// 找不到在递归搜索
		try{
			for(const auto& entry: std::filesystem::recursive_directory_iterator(workspace)){
				if (!std::filesystem::is_regular_file(entry)){
					continue;
				}
				std::string filename = entry.path().stem().string();
				size_t last = normalizedName.find_last_of('/');
				std::string basename = (last !=  std::string::npos)
					? normalizedName.substr(last+1)
					: normalizedName;

				if (filename == basename){
					LOG(Info, "Found module via recursive search: {}", entry.path().string());
					return entry.path();
				}
			}
		}catch (...){
			LOG(Error, "Error occurred during directory traversal for module '{}'", moduleName);
			return "";
		}
		// 未找到模块
		LOG(Error, "Module '{}' not found in workspace", moduleName);
		return "";
	}

	void PushLog(LuaResult* ret, bool bIsChild=false, std::string extra="", int tabnum=1){
		if (ret) {
			PushLog(ret->msgError, bIsChild, extra, tabnum);
		}
	}

	void PushLog(std::string log="", bool bIsChild=false, std::string extra="", int tabnum=1){
		if (log != "") {
			if (bIsChild) {
				int loop = tabnum < 1 ? 1 : tabnum;
				for(int i=1; i<loop; ++i)
					luaVMlog += "\t";
			}
			luaVMlog += std::format("LuaVM Log: {}\n", log);
			if (extra != ""){
				luaVMlog += "\t" + extra + "\n";	
			}
		}
	}

	LuaEntry GetLuaEntry() const {
		if(luaEntryFunction == "")
			return {"", ""};

		uint32_t index = luaEntryFunction.find_first_of("::");
		std::string luaFileName = luaEntryFunction.substr(0, index);
		std::string luaFuncName = luaEntryFunction.substr(index + 2);
		return {luaFileName, luaFuncName};
	}

	std::filesystem::path FindLuaModule(const std::string& moduleName){
		if (!CheckPath(workspace)){
			std::string error = std::format("FindLuaModule Failed, module name = {}", moduleName);
			LuaResult ret = LuaResult(false, error);
			return "";
		}
		

	}
private:
	LuaProfileReport report{};
	LuaWorkspace workspace {};
	LuaVMInstancePtr luaVMContext {nullptr};
	std::string luaVMlog {""};
	std::string luaEntryFunction {""};
};


struct LuaVMResult {
	bool bSuccess {false};
	std::string ErrorMessage {};
	double luaResult {0.0};
};
/*
  * @function: 运行 Lua 脚本并返回结果
  * @param path: Lua 脚本路径
  * @note:  该脚本必须有东西可以执行 
 */
inline static LuaVMResult RunLuaScript(std::optional<std::string> path) {
	std::cout << "RunLuaScript: " << (path.has_value() ? path.value() : "null") << std::endl;
	LuaVMResult result;
	result.bSuccess = false; // 初始化为失败状态
    result.luaResult = 0.0;  // 初始化数值
	result.ErrorMessage = "";
	if (!path.has_value() || !GetLuaWorkpace().has_value()) {
		result.ErrorMessage = "RunLuaScript Log:  Lua script path is invalid";
		return result;
	}
    lua_State* L = luaL_newstate();

    if (!L) {
        printf("RunLuaScript Log:  Failed to create Lua state\n");
        result.ErrorMessage = "RunLuaScript Log:  Failed to create Lua state";
        return result;
    }
	/* 导入 Lua 库函数 */
    luaL_openlibs(L);

    std::string luaWorkspace = GetLuaWorkpace().value().string();

	/* 设置 lua 的工作目录 */
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	const char* current_path = lua_tostring(L, -1);
	lua_pop(L, 1);
	/* 添加 lua 的搜索路径 */
	std::string new_path = std::string(current_path) + ";" + luaWorkspace + "/?.lua";
	lua_pushstring(L, new_path.c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	std::cout << "RunLuaScript Log:  Set package.path to include: " << luaWorkspace << std::endl;


    if (luaL_dofile(L, path.value().c_str()) == LUA_OK) {
        if (lua_isnumber(L, -1)) {
            double val = lua_tonumber(L, -1);
            printf("LuaJIT result: %f\n", val);
            result.luaResult = val;
            result.bSuccess = true;
        } else {
            printf("RunLuaScript Log:  LuaJIT did not return a number.\n");
            result.ErrorMessage = "Lua script did not return a number";
        }
    } else {
        const char* error = lua_tostring(L, -1);
        printf("RunLuaScript Log:  LuaJIT error: %s\n", error);
        result.ErrorMessage = error ? error : "Unknown Lua error";
    }
    
    lua_close(L);
    return result;
}

template <typename ... Args>
inline static LuaVMResult RunLuaFunction(
	const std::string path,     /* 文件路径 */
	const std::string funcname, /* 函数名 */
	Args... args   		        /* 函数参数 */
){
	LuaVMResult result;
	result.bSuccess = false; // 初始化为失败状态
	result.luaResult = 0.0;  // 初始化数值
	
	lua_State* L = luaL_newstate();
	if (!L) {
		printf("Failed to create Lua state\n");
		result.ErrorMessage = "Failed to create Lua state";
		return result;
	}
	
	luaL_openlibs(L);
	
	if (luaL_dofile(L, path.c_str()) == LUA_OK) {
		lua_getglobal(L, funcname.c_str());
		if (lua_isfunction(L, -1)) {
			// 将函数参数压栈 
			(lua_pushnumber(L, args), ...);
			
			if (lua_pcall(L, sizeof...(args), 1, 0) == LUA_OK) {
				if (lua_isnumber(L, -1)) {
					double val = lua_tonumber(L, -1);
					printf("LuaJIT function result: %f\n", val);
					result.luaResult = val;
					result.bSuccess = true;
				} else {
					printf("LuaJIT function did not return a number.\n");
					result.ErrorMessage = "Lua function did not return a number";
				}
			} else {
				const char* error = lua_tostring(L, -1);
				printf("LuaJIT function call error: %s\n", error);
				result.ErrorMessage = error ? error : "Unknown Lua function call error";
			}
		} else {
			printf("Function %s not found in Lua script.\n", funcname.c_str());
			result.ErrorMessage = "Function not found in Lua script";
		}
	} else {
		const char* error = lua_tostring(L, -1);
		printf("LuaJIT error: %s\n", error);
		result.ErrorMessage = error ? error : "Unknown Lua error";
	}
	
	lua_close(L);
	return result;

}

} // namespace LuaBenchmark