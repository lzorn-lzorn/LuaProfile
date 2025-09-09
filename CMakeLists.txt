cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXPORT_COMPILE_COMMANDS ON)

set(LuaPath ${CMAKE_CURRENT_SOURCE_DIR}/Lua)
set(CppPath ${CMAKE_CURRENT_SOURCE_DIR}/Source)
set(INSTALL_BIN ${CMAKE_CURRENT_SOURCE_DIR}/Bin)
set(INSTALL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/Lib)
set(INSTALL_INC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set(INSTALL_LUA ${CMAKE_CURRENT_SOURCE_DIR}/Lua)

project(LuaJITBenchmark
	LANGUAGES CXX C
	VERSION 0.1.0
)

file(GLOB_RECURSE src
	PUBLIC
	${CppPath}/*.cpp
)

file(GLOB_RECURSE include
	PUBLIC
	${CppPath}/*.h
	${CppPath}/*.hpp
)

add_executable(${PROJECT_NAME} ${src})
target_include_directories(${PROJECT_NAME} PRIVATE
	${include}
	${CMAKE_CURRENT_SOURCE_DIR}/benchmark/include
	${CMAKE_CURRENT_SOURCE_DIR}/LuaJIT/src
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest tests")
set(BENCHMARK_USE_BUNDLED_GTEST OFF CACHE BOOL "Disable bundled Google Test")
add_subdirectory(benchmark)

add_subdirectory(LuaJIT)
target_link_libraries(${PROJECT_NAME} PRIVATE libluajit benchmark::benchmark)

# 安装设置
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${INSTALL_BIN}
	LIBRARY DESTINATION ${INSTALL_LIB}
	ARCHIVE DESTINATION ${INSTALL_LIB}
)

# 安装头文件
install(FILES ${include}
	DESTINATION ${INSTALL_INC}
)

# 安装 benchmark 头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/include/
	DESTINATION ${INSTALL_INC}
	FILES_MATCHING PATTERN "*.h"
)

# 安装 LuaJIT 头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LuaJIT/src/
	DESTINATION ${INSTALL_INC}
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 安装 Lua 源码文件到 lua/ 目录
file(GLOB_RECURSE lua_scripts ${LuaPath}/*.lua)
install(FILES ${lua_scripts}
	DESTINATION ${INSTALL_LUA}
)
# 保持 Lua 脚本的目录结构
install(DIRECTORY ${LuaPath}/
	DESTINATION ${INSTALL_LUA}
	FILES_MATCHING PATTERN "*.lua"
)

# 在 CMakeLists.txt 末尾添加
include(CPack)

# 设置基本打包信息
set(CPACK_PACKAGE_NAME "LuaJITBenchmark")
set(CPACK_PACKAGE_VENDOR "LuaJITBenchmark")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LuaJIT Benchmark Tool")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LuaJITBenchmark")

# Windows 特定设置
if(WIN32)
	set(CPACK_GENERATOR "ZIP;NSIS") # ZIP压缩包和NSIS安装程序
else()
	set(CPACK_GENERATOR "TGZ") # Linux/Mac用tar.gz
endif()