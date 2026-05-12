/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PerformanceMonitor.h"
#include <hilog/log.h>
#include "../../common/common.h"
#include <sstream>
#include <algorithm>

namespace NativeXComponentSample {

PerformanceMonitor::PerformanceMonitor()
    : m_totalFrames(0)
    , m_droppedFrames(0)
{
}

PerformanceMonitor::~PerformanceMonitor() {
}

void PerformanceMonitor::BeginFrame() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto now = std::chrono::steady_clock::now();
    m_frameTimes.push_back(now);
    
    // 清理过期数据
    CleanupOldData();
}

void PerformanceMonitor::EndFrame(bool dropped) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_totalFrames++;
    
    if (dropped) {
        m_droppedFrames++;
    }
}

float PerformanceMonitor::GetFPS() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_frameTimes.size() < 2) {
        return 0.0f;
    }
    
    // 计算时间跨度
    auto first = m_frameTimes.front();
    auto last = m_frameTimes.back();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(last - first).count();
    
    if (duration == 0) {
        return 0.0f;
    }
    
    // FPS = 帧数 / 时间（秒）
    float fps = static_cast<float>(m_frameTimes.size() - 1) / (duration / 1000.0f);
    
    return fps;
}

float PerformanceMonitor::GetAverageFrameTime() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_frameTimes.size() < 2) {
        return 0.0f;
    }
    
    // 计算平均帧耗时
    float totalTime = 0.0f;
    auto prev = m_frameTimes.front();
    
    for (auto it = m_frameTimes.begin() + 1; it != m_frameTimes.end(); ++it) {
        auto current = *it;
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(current - prev).count();
        totalTime += static_cast<float>(delta);
        prev = current;
    }
    
    float avgTimeUs = totalTime / (m_frameTimes.size() - 1);
    return avgTimeUs / 1000.0f;  // 转换为毫秒
}

float PerformanceMonitor::GetDropRate() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_totalFrames == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(m_droppedFrames) / m_totalFrames;
}

void PerformanceMonitor::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_frameTimes.clear();
    m_totalFrames = 0;
    m_droppedFrames = 0;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "PerformanceMonitor", "Stats reset");
}

PerformanceMonitor::Stats PerformanceMonitor::GetStats() const {
    Stats stats;
    stats.fps = GetFPS();
    stats.frameTimeMs = GetAverageFrameTime();
    stats.dropRate = GetDropRate();
    stats.totalFrames = m_totalFrames;
    stats.droppedFrames = m_droppedFrames;
    
    return stats;
}

std::string PerformanceMonitor::Stats::ToString() const {
    std::ostringstream oss;
    oss << "FPS=" << fps 
        << ", FrameTime=" << frameTimeMs << "ms"
        << ", DropRate=" << (dropRate * 100) << "%"
        << ", Frames=" << totalFrames
        << ", Dropped=" << droppedFrames;
    return oss.str();
}

void PerformanceMonitor::CleanupOldData() {
    // 保留最近 MAX_SAMPLE_COUNT 帧数据
    while (m_frameTimes.size() > MAX_SAMPLE_COUNT) {
        m_frameTimes.pop_front();
    }
}

} // namespace NativeXComponentSample
