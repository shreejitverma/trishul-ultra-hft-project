#pragma once
#include <thread>
#include <atomic>

namespace ultra::telemetry {

class MetricsPublisher {
public:
    void start();
    void stop();

private:
    void publish_snapshot();
    
    std::atomic<bool> running_{false};
    std::thread publisher_thread_;
};

} // namespace ultra::telemetry
