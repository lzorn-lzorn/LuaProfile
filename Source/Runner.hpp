
#include <string>
#include <functional>
#include <unordered_map>
#include "LuaVM.hpp"
namespace LuaBenchmark {

class Runner{
	template <typename ...Args>
	using runner = std::function<void(std::string, Args...)>;
public:
	static Runner& Instance(){
		static Runner instance;
		return instance;
	}

	Runner(const Runner&) = delete;
	Runner& operator=(const Runner&) = delete;

	/*
	 * - file = "path::funcname"
	 * - func = "path::funcname" arg1 arg2 ...
	 * - "--start": 新建一个Lua虚拟机
	 * - "--list": 查当前注册的所有lua虚拟机
	 * - "--close id=1": 关闭指定id为1的lua虚拟机
	 * - "--over": 关闭所有lua虚拟机
	 * - "--help": 帮助信息
 	*/
	template<typename ...Args>
	void parse(std::string argc, Args... args){
		if (argc == ""){

		}
	}

public:
	
private:
	Runner() = default;

};

}