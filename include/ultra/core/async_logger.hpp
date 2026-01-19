#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdarg>
#include "lockfree/spsc_queue.hpp"
#include "compiler.hpp"
#include "time/rdtsc_clock.hpp"

namespace ultra {

/**
 * High-Performance Asynchronous Logger (Non-blocking)
 * Uses SPSC Queue to pass log messages to a dedicated thread.
 */
class AsyncLogger {
public:
    static constexpr size_t QUEUE_SIZE = 65536; // 64K messages
    static constexpr size_t MAX_MSG_LEN = 128;

    struct LogEntry {
        uint64_t tsc;
        char msg[MAX_MSG_LEN];
    };

    static AsyncLogger& instance() {
        static AsyncLogger instance;
        return instance;
    }

    void start(const std::string& filename) {
        if (running_) return;
        file_.open(filename, std::ios::out | std::ios::app);
        if (!file_.is_open()) return;

        running_ = true;
        flush_thread_ = std::thread(&AsyncLogger::flush_loop, this);
    }

    void stop() {
        running_ = false;
        if (flush_thread_.joinable()) {
            flush_thread_.join();
        }
        if (file_.is_open()) {
            file_.close();
        }
    }

    ULTRA_ALWAYS_INLINE void log(const char* fmt, ...) {
        LogEntry entry;
        entry.tsc = RDTSCClock::now();
        
        va_list args;
        va_start(args, fmt);
        vsnprintf(entry.msg, MAX_MSG_LEN, fmt, args);
        va_end(args);

        // Try to push to queue. If full, we drop (low-latency choice)
        // or we could block (high-reliability choice). 
        // For HFT, dropping is better than stalling the critical path.
        queue_.push(entry);
    }

private:
    AsyncLogger() = default;
    ~AsyncLogger() { stop(); }

    void flush_loop() {
        LogEntry entry;
        while (running_ || !queue_.empty()) {
            if (queue_.pop(entry)) {
                file_ << entry.tsc << " " << entry.msg << "\n";
            } else {
                // No logs, relax the CPU
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        file_.flush();
    }

    std::atomic<bool> running_{false};
    std::ofstream file_;
    std::thread flush_thread_;
    SPSCQueue<LogEntry, QUEUE_SIZE> queue_;
};

// Macro for easier usage
#define ULTRA_LOG(fmt, ...) ultra::AsyncLogger::instance().log(fmt, ##__VA_ARGS__)

} // namespace ultra
