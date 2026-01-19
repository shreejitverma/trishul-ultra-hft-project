#pragma once
#include <thread>
#include <vector>
#include <iostream>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#endif

namespace ultra {

class ThreadUtils {
public:
    static void pin_thread(int core_id) {
#if defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);

        pthread_t current_thread = pthread_self();
        int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
        
        if (result != 0) {
            std::cerr << "Failed to pin thread to core " << core_id << std::endl;
        }
#else
        (void)core_id;
        // macOS does not support standard pthread_setaffinity_np
#endif
    }

    /**
     * Sets the thread to Real-Time priority (SCHED_FIFO).
     * Requires CAP_SYS_NICE or running as root on Linux.
     */
    static void set_realtime_priority(int priority = 99) {
#if defined(__linux__)
        struct sched_param param;
        param.sched_priority = priority;
        
        pthread_t current_thread = pthread_self();
        int result = pthread_setschedparam(current_thread, SCHED_FIFO, &param);
        
        if (result != 0) {
             std::cerr << "Failed to set real-time priority (SCHED_FIFO). Need root/CAP_SYS_NICE?\n";
        }
#else
        (void)priority;
        // macOS: standard pthread_setschedparam often fails or requires special entitlements
#endif
    }

    static void isolate_thread(int core_id, int priority = 99) {
        pin_thread(core_id);
        set_realtime_priority(priority);
    }
};

} // namespace ultra
