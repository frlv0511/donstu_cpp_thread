#include "threadfuncs.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <future>
#include <ios>
#include <mutex>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

void runNormalThreads() {
    ThreadArgs args[COUNT_THREADS];
    for (int i = 0; i < COUNT_THREADS; ++i) {
        std::ostringstream tag;
        tag << "T" << i;
        std::ostringstream msg;
        msg << "привет от потока " << i;
        args[i].id = i;
        args[i].tag = tag.str();
        args[i].message = msg.str();
    }

    std::vector<std::thread> threads;
    std::vector<std::thread::id> ids(COUNT_THREADS);
    threads.reserve(COUNT_THREADS);
    for (int i = 0; i < COUNT_THREADS; ++i) {
        threads.emplace_back(funcThread, args[i]);
        ids[static_cast<std::size_t>(i)] = threads.back().get_id();
    }

    std::ostringstream cmp;
    cmp << std::boolalpha << "main: сравнение std::thread::id через == : ids[0]==ids[1] -> "
        << (ids[0] == ids[1]) << ", ids[0]==ids[0] -> " << (ids[0] == ids[0]);
    g_logger.writeLine(cmp.str());

    if (std::getenv("NO_JOIN") != nullptr) {
        g_logger.writeLine("main: NO_JOIN=1, join пропущен намеренно (ожидается std::terminate)");
        return;
    }

    for (auto& t : threads) {
        t.join();
    }
}

void runCounterDemo() {
    constexpr int perThreadIters = 100000;
    int plainCounter = 0;
    std::atomic<int> atomicCounter{0};

    auto worker = [&]() {
        for (int i = 0; i < perThreadIters; ++i) {
            ++plainCounter;
            ++atomicCounter;
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(COUNT_THREADS);
    for (int i = 0; i < COUNT_THREADS; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::ostringstream oss;
    oss << "main: counter(plain, без защиты) = " << plainCounter
        << ", counter(atomic) = " << atomicCounter.load()
        << ", ожидалось = " << (perThreadIters * COUNT_THREADS);
    g_logger.writeLine(oss.str());
}

void runPromiseDemo() {
    ThreadArgs args;
    args.id = 100;
    args.tag = "PROM";
    args.message = "поток с promise/future";

    std::promise<std::string> prom;
    std::future<std::string> fut = prom.get_future();
    std::thread t(funcThreadPromise, args, std::move(prom));
    std::string result = fut.get();
    t.join();

    g_logger.writeLine("main: получено через future: " + result);
}

void runProducerConsumerDemo() {
    std::mutex m;
    std::condition_variable cv;
    int value = 0;
    bool ready = false;
    bool done = false;
    constexpr int kTransfers = 10;

    std::thread producer([&]() {
        for (int i = 1; i <= kTransfers; ++i) {
            {
                std::unique_lock<std::mutex> lock(m);
                cv.wait(lock, [&] { return !ready; });
                value = i;
                ready = true;
            }
            cv.notify_one();
        }
        {
            std::lock_guard<std::mutex> lock(m);
            done = true;
        }
        cv.notify_one();
    });

    std::thread consumer([&]() {
        int received = 0;
        while (true) {
            int v = 0;
            {
                std::unique_lock<std::mutex> lock(m);
                cv.wait(lock, [&] { return ready || done; });
                if (!ready && done) {
                    break;
                }
                v = value;
                ready = false;
            }
            cv.notify_one();
            ++received;
            std::ostringstream oss;
            oss << "[producer-consumer] потребитель получил значение " << v;
            g_logger.writeLine(oss.str());
        }
        std::ostringstream oss;
        oss << "[producer-consumer] потребитель завершил работу, получено " << received << " значений";
        g_logger.writeLine(oss.str());
    });

    producer.join();
    consumer.join();
}

void runLogOnlyTiming() {
    int iters = 100000;
    if (const char* v = std::getenv("ITER")) {
        iters = std::atoi(v);
    }
    auto worker = [&](int id) {
        for (int i = 0; i < iters; ++i) {
            std::ostringstream oss;
            oss << "T" << id << " iter " << i;
            g_logger.writeLine(oss.str());
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(COUNT_THREADS);
    for (int i = 0; i < COUNT_THREADS; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
}

}  // namespace

int main() {
    if (std::getenv("ONLY_LOG") != nullptr) {
        runLogOnlyTiming();
        return 0;
    }

    std::ostringstream oss;
    oss << "main: pid = " << getpid() << ", ppid = " << getppid();
    g_logger.writeLine(oss.str());

    runNormalThreads();

    if (std::getenv("NO_JOIN") != nullptr) {
        return 0;
    }

    runCounterDemo();
    runPromiseDemo();
    runProducerConsumerDemo();

    return 0;
}
