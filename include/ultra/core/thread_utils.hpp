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
        } else {
            // std::cout << "Thread pinned to core " << core_id << std::endl;
        }
#else
        (void)core_id;
        // macOS does not support standard pthread_setaffinity_np
        // Thread affinity is managed by the OS kernel (XNU)
        // We can use thread_policy_set, but it's often ignored.
#endif
    }
};

} // namespace ultra
