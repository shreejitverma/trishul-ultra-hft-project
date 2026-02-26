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
    static constexpr size_t MAX_MSG_LEN = 128; ///< const int variable representing MAX_MSG_LEN.

    struct LogEntry {
        uint64_t tsc; ///< int variable representing tsc.
        char msg[MAX_MSG_LEN]; ///< int variable representing msg.
    };

    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

                        /**
                         * @brief Auto-generated description for instance.
                         * @return AsyncLogger & value.
                         */
    static AsyncLogger& instance() {
        static AsyncLogger instance;
        return instance;
    }

         /**
          * @brief Auto-generated description for start.
          * @param filename Parameter description.
          * @param json_mode Whether to output logs in JSON format.
          */
    void start(const std::string& filename, bool json_mode = false) {
        if (running_) return;
        json_mode_ = json_mode;
        file_.open(filename, std::ios::out | std::ios::app);
        if (!file_.is_open()) return;

        running_ = true;
        flush_thread_ = std::thread(&AsyncLogger::flush_loop, this);
    }

         /**
          * @brief Auto-generated description for stop.
          */
    void stop() {
        running_ = false;
        if (flush_thread_.joinable()) {
            flush_thread_.join();
        }
        if (file_.is_open()) {
            file_.close();
        }
    }

                             /**
                              * @brief Standard logging function.
                              * @param level Log level.
                              * @param fmt Format string.
                              */
    ULTRA_ALWAYS_INLINE void log(LogLevel level, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_internal(level, 0, nullptr, 0, fmt, args);
        va_end(args);
    }

    /**
     * @brief Flow-aware logging function.
     */
    ULTRA_ALWAYS_INLINE void log_flow(LogLevel level, const char* func, uint64_t duration_ns, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_internal(level, flow_depth_, func, duration_ns, fmt, args);
        va_end(args);
    }

    static void enter_flow(const char* func) { 
        flow_depth_++; 
        if (call_stack_.size() < MAX_STACK_DEPTH) call_stack_.push_back(func);
    }
    static void exit_flow() { 
        if (flow_depth_ > 0) flow_depth_--; 
        if (!call_stack_.empty()) call_stack_.pop_back();
    }

    static std::string get_stack_string() {
        std::string s;
        for (size_t i = 0; i < call_stack_.size(); ++i) {
            s += (i == 0 ? "" : " -> ") + std::string(call_stack_[i]);
        }
        return s;
    }

private:
    /**
     * @brief Auto-generated description for AsyncLogger.
     */
    AsyncLogger() = default;
    /**
     * @brief Auto-generated description for ~AsyncLogger.
     */
    ~AsyncLogger() { stop(); }

    static inline thread_local uint32_t flow_depth_ = 0;
    static inline thread_local std::vector<const char*> call_stack_;
    static constexpr size_t MAX_STACK_DEPTH = 32;
    bool json_mode_ = false;

    const char* level_to_str(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    void log_internal(LogLevel level, uint32_t depth, const char* /*func*/, uint64_t duration_ns, const char* fmt, va_list args) {
        LogEntry entry;
        entry.tsc = RDTSCClock::now();
        
        char payload[MAX_MSG_LEN];
        vsnprintf(payload, MAX_MSG_LEN, fmt, args);

        if (json_mode_) {
            snprintf(entry.msg, MAX_MSG_LEN, 
                "{\"ts\":%llu,\"lvl\":\"%s\",\"depth\":%u,\"stack\":\"%s\",\"dur_ns\":%llu,\"msg\":\"%s\"}",
                (unsigned long long)entry.tsc, level_to_str(level), depth, get_stack_string().c_str(), 
                (unsigned long long)duration_ns, payload);
        } else {
            size_t offset = 0;
            for (uint32_t i = 0; i < depth && offset < MAX_MSG_LEN - 3; ++i) {
                entry.msg[offset++] = ' ';
                entry.msg[offset++] = ' ';
            }
            const char* dur_part = duration_ns > 0 ? " [dur=" : "";
            char dur_val[32] = "";
            if (duration_ns > 0) snprintf(dur_val, 32, "%llu ns]", (unsigned long long)duration_ns);

            snprintf(entry.msg + offset, MAX_MSG_LEN - offset, "[%s] %s%s%s", 
                level_to_str(level), payload, dur_part, dur_val);
        }

        queue_.push(entry);
    }

    void flush_loop() {
        LogEntry entry;
        while (running_ || !queue_.empty()) {
            if (queue_.pop(entry)) {
                file_ << entry.msg << "\n";
            } else {
                // No logs, relax the CPU
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        file_.flush();
    }

    std::atomic<bool> running_{false}; ///< int variable representing running_.
    std::ofstream file_; ///< int variable representing file_.
    std::thread flush_thread_; ///< int variable representing flush_thread_.
    SPSCQueue<LogEntry, QUEUE_SIZE> queue_; ///< int variable representing queue_.
};

/**
 * RAII Trace helper for C++ with nanosecond measurement, description, and parameters.
 */
class FlowTracer {
public:
    FlowTracer(const char* func, const std::string& desc = "", const std::string& params = "") 
        : func_(func), start_tsc_(RDTSCClock::now()) {
        
        AsyncLogger::instance().log_flow(AsyncLogger::LogLevel::INFO, func_, 0, "ENTER -> [%s] | Purpose: %s | Params: (%s)", func_, desc.c_str(), params.c_str());
        AsyncLogger::enter_flow(func_);
    }
    ~FlowTracer() {
        uint64_t duration = RDTSCClock::now() - start_tsc_;
        AsyncLogger::exit_flow();
        AsyncLogger::instance().log_flow(AsyncLogger::LogLevel::INFO, func_, duration, "EXIT  <- [%s]", func_);
    }

    template<typename T>
    T trace_return(T val) {
        AsyncLogger::instance().log_flow(AsyncLogger::LogLevel::INFO, func_, 0, "RETURN -> %s", std::to_string(val).c_str());
        return val;
    }

    // Specialization for strings/const char*
    const char* trace_return(const char* val) {
        AsyncLogger::instance().log_flow(AsyncLogger::LogLevel::INFO, func_, 0, "RETURN -> %s", val);
        return val;
    }

private:
    const char* func_;
    uint64_t start_tsc_;
};

// Macros for easier usage
#define ULTRA_LOG(level, fmt, ...) ultra::AsyncLogger::instance().log(ultra::AsyncLogger::LogLevel::level, fmt, ##__VA_ARGS__)
#define ULTRA_LOG_FLOW(level, func, dur, fmt, ...) ultra::AsyncLogger::instance().log_flow(ultra::AsyncLogger::LogLevel::level, func, dur, fmt, ##__VA_ARGS__)
#define ULTRA_TRACE(func, desc, params) ultra::FlowTracer _tracer(func, desc, params)
#define ULTRA_TRACE_SIMPLE(func) ultra::FlowTracer _tracer(func, "Execution Flow", "n/a")
#define ULTRA_TRACE_RET(val) _tracer.trace_return(val)

} // namespace ultra
