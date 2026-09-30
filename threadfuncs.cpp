#include "threadfuncs.h"

#include <chrono>
#include <cstdlib>
#include <sstream>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

Logger g_logger("output.log");

Logger::Logger(const std::string& filename)
    : file_(filename, std::ios::out | std::ios::trunc) {
    if (!file_) {
        throw std::runtime_error("Не удалось открыть лог-файл: " + filename);
    }
}

bool Logger::writeLine(const std::string& msg) {
    if (std::getenv("NO_LOG_MUTEX") != nullptr) {
        file_ << msg << "\n";
        file_.flush();
        return static_cast<bool>(file_);
    }
    std::lock_guard<std::mutex> lock(mutex_);
    file_ << msg << "\n";
    file_.flush();
    return static_cast<bool>(file_);
}

std::uint64_t getThreadID() {
    return static_cast<std::uint64_t>(syscall(SYS_gettid));
}

namespace {

int sleepMs() {
    if (const char* v = std::getenv("SLEEP_MS")) {
        return std::atoi(v);
    }
    return 100;
}

int iterCount() {
    if (const char* v = std::getenv("ITER")) {
        return std::atoi(v);
    }
    return COUNT_ITERATIONS;
}

}  // namespace

void funcThread(const ThreadArgs& args) {
    const int iters = iterCount();
    const int delay = sleepMs();
    for (int i = 0; i < iters; ++i) {
        std::ostringstream oss;
        oss << "[tag = " << args.tag << "] pid = " << getpid()
            << " ppid = " << getppid() << " tid = " << getThreadID()
            << " std::thread::id = " << std::this_thread::get_id()
            << " iter = " << i << " msg = " << args.message;
        g_logger.writeLine(oss.str());
        if (delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }
}

void funcThreadPromise(ThreadArgs args, std::promise<std::string> prom) {
    const int iters = iterCount();
    for (int i = 0; i < iters; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::ostringstream oss;
    oss << "поток " << args.tag << " отработал " << iters << " итераций";
    prom.set_value(oss.str());
}
