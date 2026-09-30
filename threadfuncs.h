#pragma once

#include <cstdint>
#include <future>
#include <fstream>
#include <mutex>
#include <string>

inline constexpr int COUNT_THREADS = 4;
inline constexpr int COUNT_ITERATIONS = 3;

struct ThreadArgs {
    int id;
    std::string tag;
    std::string message;
};

class Logger {
public:
    explicit Logger(const std::string& filename);
    bool writeLine(const std::string& msg);

private:
    std::ofstream file_;
    std::mutex mutex_;
};

extern Logger g_logger;

std::uint64_t getThreadID();

void funcThread(const ThreadArgs& args);

void funcThreadPromise(ThreadArgs args, std::promise<std::string> prom);
