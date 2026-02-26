#pragma once
#include <thread>
#include <atomic>

namespace ultra::telemetry {

class MetricsPublisher {
public:
         /**
          * @brief Auto-generated description for start.
          */
    void start();
         /**
          * @brief Auto-generated description for stop.
          */
    void stop();

private:
         /**
          * @brief Auto-generated description for publish_snapshot.
          */
    void publish_snapshot();
    
    std::atomic<bool> running_{false}; ///< int variable representing running_.
    std::thread publisher_thread_; ///< int variable representing publisher_thread_.
};

} // namespace ultra::telemetry
