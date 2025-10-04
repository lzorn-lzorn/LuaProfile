#include <memory>
#include <string>
#include <type_traits>
#include <utility>
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lua.hpp"
}

namespace LuaBenchmark {
using LuaCFunctionTp = int (* )(lua_State* L);
static inline std::string ErrorInfo = "";
template<typename Ty>
concept ValidLuaCompatibleType = requires {
	std::is_arithmetic_v<std::decay_t<Ty>> ||
	std::is_same_v<std::decay_t<Ty>, bool> ||
	std::is_same_v<std::decay_t<Ty>, const char*> ||
	std::is_same_v<std::decay_t<Ty>, std::string>;
};
/**
* @brief: 将可变参数打包成 tuple, 如果是 ValidLuaCompatibleType 的支持参数则忽略 
*/
template<typename... Args>
constexpr auto TransformLuaFundamentalArgs(Args&&... args) {
	return std::tuple_cat(
		[&args](){
			if constexpr (ValidLuaCompatibleType<Args>) {
				return std::make_tuple(std::forward<Args>(args));
			} else {
				return std::tuple<>();
			}
		}()...
	);
}
template <typename CObjTp, typename ...MemObjTps>
CObjTp * CreateLuaUserdata (lua_State* L, const char* metatableName, MemObjTps&&... objs){
	/* 申请 userdata 的内存 */
	void* userdata = lua_newuserdata(L, sizeof(CObjTp));

	if (userdata == nullptr) {
		ErrorInfo.append("内存不足, 创建 Lua userdata 失败");
		return nullptr;
	}

	std::construct_at(static_cast<CObjTp*>(userdata), std::forward<MemObjTps>(objs)...);
	/* 设置 metatable */
	luaL_getmetatable(L, metatableName);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1); // 移除栈顶的 nil
		/* metatable 不存在, 创建一个新的 metatable */
		luaL_newmetatable(L, metatableName);
		/* 设置 __gc 元方法 */
		lua_pushcfunction(L, [&](lua_State* L)->int{
			CObjTp* obj = static_cast<CObjTp*>(luaL_checkudata(L, 1, metatableName));
			if (obj){
				/* 显式调用析构函数 */
				obj->~CObjTp();
			}
			return 0;
		});
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
	return userdata;
}
/** 
* @brief: 注册自定义的全局函数到 Lua 虚拟机中
* @param L: Lua 虚拟机指针
* @param name: Lua 中的函数名
* @param func: C++ 中的函数指针, 其类型必须是 lua_CFunction 也即
* 		int (*)(lua_State* L)
* @param args: 可变参数, 用于给注册的函数传递参数, 所有 Args 都必须是fundamental type
*       即 C语言就有的基础类型
*/
template <typename... Args>
static inline void RegisterLuaGFunction(lua_State* L, const char* name, LuaCFunctionTp func, Args... args){

}
}



