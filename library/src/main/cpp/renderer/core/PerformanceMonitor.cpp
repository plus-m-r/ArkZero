#include "PerformanceMonitor.h"

namespace NativeXComponentSample {

void PerformanceMonitor::BeginFrame()
{
    frameStart_ = std::chrono::steady_clock::now();
    inFrame_ = true;
}

void PerformanceMonitor::EndFrame(bool dropped)
{
    if (!inFrame_) {
        return;
    }
    inFrame_ = false;

    auto frameEnd = std::chrono::steady_clock::now();
    double frameMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart_).count();

    stats_.totalFrames++;
    if (dropped) {
        stats_.droppedFrames++;
    }

    if (stats_.totalFrames > 0) {
        stats_.dropRate = static_cast<double>(stats_.droppedFrames) / static_cast<double>(stats_.totalFrames);
    }

    if (frameMs > 0.0) {
        stats_.fps = 1000.0 / frameMs;
        stats_.frameTimeMs = frameMs;
    }
}

void PerformanceMonitor::Reset()
{
    stats_ = MonitorStats{};
    inFrame_ = false;
}

MonitorStats PerformanceMonitor::GetStats() const
{
    return stats_;
}

} // namespace NativeXComponentSample
