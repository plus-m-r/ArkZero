#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <cstdint>
#include <chrono>

namespace NativeXComponentSample {

struct MonitorStats {
    int64_t totalFrames = 0;
    int64_t droppedFrames = 0;
    double fps = 0.0;
    double dropRate = 0.0;
    double frameTimeMs = 0.0;
};

class PerformanceMonitor {
public:
    PerformanceMonitor() = default;
    ~PerformanceMonitor() = default;

    void BeginFrame();
    void EndFrame(bool dropped);
    void Reset();
    MonitorStats GetStats() const;

private:
    MonitorStats stats_{};
    std::chrono::steady_clock::time_point frameStart_{};
    bool inFrame_ = false;
};

} // namespace NativeXComponentSample

#endif // PERFORMANCE_MONITOR_H
