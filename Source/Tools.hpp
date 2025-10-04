#pragma once
#include <iostream>
#include <filesystem>
#include <optional>
#include <string>
#include <ctime>
#include <chrono>
#include <format>
#include <type_traits>
#define INFO std::cout
#define ERROR std::cerr
#ifdef USELOG
    #define LOG(ostream, fmt, ...) \
        ostream << std::format(fmt, ##__VA_ARGS__) << "\n"

    #define GETLOG(ostream, fmt, ...) \
        ([&]() { \
            std::string msg = std::format(fmt, ##__VA_ARGS__); \
            ostream << msg << "\n"; \
            return msg; \
        })()
#else
    #define LOG(ostream, fmt, ...) (void)0
    #define GETLOG(ostream, fmt, ...) (void)0
#endif 

inline static std::string GetTimeString(std::chrono::high_resolution_clock::time_point time) {
    // 获取与 system_clock 相同的 epoch 的 time_point
    auto sys_tp = std::chrono::system_clock::time_point(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            time.time_since_epoch()
        )
    );


/** 
 * @brief 这里会出现静态分析报错的情况:
 *      std::chrono::zoned_time 和 std::chrono::current_zone()
 *      不存在于 std::chrono
 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#if __has_include(<chrono>) && defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
    // C++20 时区功能
    std::chrono::zoned_time zt{std::chrono::current_zone(), sys_tp};
    return std::format("{:%H:%M:%S}", zt);
#else
    // 回退到传统方法  
    auto time_t_val = std::chrono::system_clock::to_time_t(sys_tp);
    auto local_time = *std::localtime(&time_t_val);
    return std::format("{:02d}:{:02d}:{:02d}", 
                      local_time.tm_hour, 
                      local_time.tm_min, 
                      local_time.tm_sec);
#endif
#pragma clang diagnostic pop
}




// 判断是否是标准智能指针
template <typename T>
struct IsUniquePtr : std::false_type {};
template <typename T, typename D>
struct IsUniquePtr<std::unique_ptr<T, D>> : std::true_type {};

template <typename T>
struct IsSharedPtr : std::false_type {};
template <typename T>
struct IsSharedPtr<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct IsWeakPtr : std::false_type {};
template <typename T>
struct IsWeakPtr<std::weak_ptr<T>> : std::true_type {};
template <typename T>
struct IsSmartPtr : std::integral_constant<bool,
    IsUniquePtr<T>::value ||
    IsSharedPtr<T>::value ||
    IsWeakPtr<T>::value>
{};

template <typename Pointer>
constexpr bool IsPointerVal = 
    std::is_pointer<Pointer>::value || 
    IsSmartPtr<Pointer>::value;

inline static std::optional<std::filesystem::path> GetLuaWorkpace(){
    std::filesystem::path curPath = std::filesystem::current_path();
    std::filesystem::path luaPath {};
    std::cout << "Current path: " << curPath << std::endl;
    
    // 先尝试在当前目录下查找
    luaPath = curPath / "Lua";
    if (std::filesystem::exists(luaPath) && std::filesystem::is_directory(luaPath)) {
        std::cout << "GetLuaWorkpace Log: Found Lua directory at: " << luaPath << std::endl;
        return std::make_optional(luaPath);
    }
    
    // 再尝试在父目录下查找
    luaPath = curPath.parent_path() / "Lua";
    if (std::filesystem::exists(luaPath) && std::filesystem::is_directory(luaPath)) {
        std::cout << "GetLuaWorkpace Log: Found Lua directory at: " << luaPath << std::endl;
        return std::make_optional(luaPath);
    }
    
    std::cout << "GetLuaWorkpace Log: Lua directory not found!" << std::endl;
    return std::nullopt;  // 如果都找不到，返回空
}
inline static std::optional<std::string> GetLuaCodePath(const std::string name){
    if (!GetLuaWorkpace().has_value()) {
        return std::nullopt;
    }
    std::filesystem::path luaPath = GetLuaWorkpace().value() / (name + ".lua");
    if (!std::filesystem::exists(luaPath)) {
        return std::nullopt;
    }
    return std::make_optional(std::move(luaPath.string()));
}

inline static std::optional<std::filesystem::path> GetPathPreix(std::optional<std::filesystem::path> path){
	if (!path.has_value()) {
		return std::nullopt;
	}
	return std::make_optional(path.value().parent_path());
}

inline static bool CheckPath(std::filesystem::path path){
    bool ret {true};
    if (!std::filesystem::is_directory(path) || 
        !std::filesystem::exists(path)
    ) [[unlikely]] {
        ret = false;
    }

    return ret;
}