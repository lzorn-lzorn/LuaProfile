#include <iostream>
#include <benchmark/benchmark.h>
#include <chrono>
#include <limits>
#include <thread>
#include <string>
#include <optional>
#include "lua.hpp" // LuaJIT 头文件
#include "LuaVM.hpp"
#include "Tools.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// int main(int argc, char** argv) {
//     auto luaCodePath = GetLuaCodePath("main").value_or("Not Found");
//     std::cout << "Main Log: Lua code path: " << luaCodePath << std::endl;
//     auto ret = LuaBenchmark::RunLuaScript(GetLuaCodePath("main").value());
//     if (ret.bSuccess) {
//         std::cout << "Main Log: Lua script executed successfully, result: " << ret.luaResult << std::endl;
//     } else {
//         std::cerr << "Main Log: Lua script execution failed: " << ret.ErrorMessage << std::endl;
//     }
//     return 0;
// }
static void BM_RunLuaScript(benchmark::State& state) {
    // 预先获取路径
    std::string scriptPath = GetLuaCodePath("main").value();
    std::cout << "====== Benchmark Info ======" << std::endl;
    std::cout << "Script path: " << scriptPath << std::endl;
    
    // 收集统计信息
    struct Stats {
        size_t successful_runs = 0;
        double total_result = 0.0;
        double min_result = std::numeric_limits<double>::max();
        double max_result = std::numeric_limits<double>::lowest();
        std::chrono::nanoseconds fastest_run = std::chrono::nanoseconds::max();
        std::chrono::nanoseconds slowest_run = std::chrono::nanoseconds::min();
        std::chrono::nanoseconds total_time = std::chrono::nanoseconds(0);
    } stats;
    
    // 预热运行，输出初始信息
    {
        auto prerun_result = LuaBenchmark::RunLuaScript(scriptPath);
        std::cout << "Pre-run status: " << (prerun_result.bSuccess ? "Success" : "Failed") << std::endl;
        if (!prerun_result.bSuccess) {
            std::cout << "Pre-run error: " << prerun_result.ErrorMessage << std::endl;
        } else {
            std::cout << "Pre-run result: " << prerun_result.luaResult << std::endl;
        }
    }
    
    std::cout << "\n====== Starting Benchmark ======" << std::endl;
    
    // 性能测试循环
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 被测量的代码
        auto result = LuaBenchmark::RunLuaScript(scriptPath);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        
        // 防止编译器优化掉结果
        benchmark::DoNotOptimize(result);
        
        // 收集统计信息
        if (result.bSuccess) {
            stats.successful_runs++;
            stats.total_result += result.luaResult;
            stats.min_result = std::min(stats.min_result, result.luaResult);
            stats.max_result = std::max(stats.max_result, result.luaResult);
            
            // 记录时间信息
            stats.total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
            stats.fastest_run = std::min(stats.fastest_run, 
                                       std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
            stats.slowest_run = std::max(stats.slowest_run, 
                                       std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
        } else {
            state.SkipWithError(result.ErrorMessage.c_str());
            break;
        }
    }
    
    // 打印详细统计信息
    if (stats.successful_runs > 0) {
        std::cout << "\n====== Benchmark Results ======" << std::endl;
        std::cout << "Total successful runs: " << stats.successful_runs << std::endl;
        std::cout << "Average Lua result: " << (stats.total_result / stats.successful_runs) << std::endl;
        std::cout << "Min Lua result: " << stats.min_result << std::endl;
        std::cout << "Max Lua result: " << stats.max_result << std::endl;
        std::cout << "Fastest run: " << 
            std::chrono::duration_cast<std::chrono::microseconds>(stats.fastest_run).count() << " µs" << std::endl;
        std::cout << "Slowest run: " << 
            std::chrono::duration_cast<std::chrono::microseconds>(stats.slowest_run).count() << " µs" << std::endl;
        std::cout << "Average run time: " << 
            std::chrono::duration_cast<std::chrono::microseconds>(stats.total_time).count() / 
            static_cast<double>(stats.successful_runs) << " µs" << std::endl;
    }
    
    // 添加自定义计数器
    state.counters["SuccessfulRuns"] = benchmark::Counter(stats.successful_runs);
    state.counters["AvgResult"] = benchmark::Counter(
        stats.successful_runs > 0 ? stats.total_result / stats.successful_runs : 0);
    state.counters["MinResult"] = benchmark::Counter(
        stats.successful_runs > 0 ? stats.min_result : 0);
    state.counters["MaxResult"] = benchmark::Counter(
        stats.successful_runs > 0 ? stats.max_result : 0);
    
    // 执行时间计数器 (微秒)
    state.counters["FastestRun_us"] = benchmark::Counter(
        std::chrono::duration_cast<std::chrono::microseconds>(stats.fastest_run).count());
    state.counters["SlowestRun_us"] = benchmark::Counter(
        std::chrono::duration_cast<std::chrono::microseconds>(stats.slowest_run).count());
    
    // 可以添加系统信息
    std::cout << "\n====== System Info ======" << std::endl;
    std::cout << "CPU cores: " << std::thread::hardware_concurrency() << std::endl;
    
    #ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        std::cout << "Total physical memory: " << 
            memInfo.ullTotalPhys / (1024 * 1024) << " MB" << std::endl;
        std::cout << "Available memory: " << 
            memInfo.ullAvailPhys / (1024 * 1024) << " MB" << std::endl;
    }
    #endif
    
    std::cout << "====== End of Benchmark ======\n" << std::endl;
}

// 使用不同配置注册基准测试
BENCHMARK(BM_RunLuaScript)
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10)  // 固定迭代次数，便于观察每次结果
    ->Repetitions(3)  // 重复整个测试3次
    ->DisplayAggregatesOnly(false);  // 显示每次迭代的详细信息

int main(int argc, char** argv) {
    // 添加 JSON 输出参数
    const char* json_args[] = {
        argv[0],
        "--benchmark_format=json",
        "--benchmark_out=lua_benchmark_results.json"
    };
    
    int json_argc = sizeof(json_args) / sizeof(json_args[0]);
    
    // 初始化并运行基准测试
    ::benchmark::Initialize(&json_argc, const_cast<char**>(json_args));
    ::benchmark::RunSpecifiedBenchmarks();
    
    std::cout << "Benchmark results have been saved to lua_benchmark_results.json" << std::endl;
    
    return 0;
}