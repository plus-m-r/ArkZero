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

#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <cstdint>
#include <chrono>
#include <deque>
#include <mutex>

namespace NativeXComponentSample {

/**
 * 性能监控器
 * 
 * 🎯 职责：
 * - 统计 FPS、FrameTime、DropRate
 * - 计算滑动平均值
 * - 提供性能数据查询接口
 * 
 * 设计原则：单一职责 - 仅负责性能数据统计
 */
class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();

    /**
     * 记录一帧的开始
     */
    void BeginFrame();

    /**
     * 记录一帧的结束
     * @param dropped 是否丢帧
     */
    void EndFrame(bool dropped = false);

    /**
     * 获取当前 FPS
     * @return 帧率（帧/秒）
     */
    float GetFPS() const;

    /**
     * 获取平均帧耗时
     * @return 毫秒
     */
    float GetAverageFrameTime() const;

    /**
     * 获取丢帧率
     * @return 0.0 - 1.0
     */
    float GetDropRate() const;

    /**
     * 获取总帧数
     */
    uint64_t GetTotalFrames() const { return m_totalFrames; }

    /**
     * 获取丢帧数
     */
    uint64_t GetDroppedFrames() const { return m_droppedFrames; }

    /**
     * 重置统计数据
     */
    void Reset();

    /**
     * 性能统计快照
     */
    struct Stats {
        float fps;              // 帧率
        float frameTimeMs;      // 平均帧耗时（毫秒）
        float dropRate;         // 丢帧率（0.0 - 1.0）
        uint64_t totalFrames;   // 总帧数
        uint64_t droppedFrames; // 丢帧数
        
        std::string ToString() const;
    };

    /**
     * 获取完整统计快照
     */
    Stats GetStats() const;

private:
    /**
     * 清理过期数据
     */
    void CleanupOldData();

    mutable std::mutex m_mutex;
    
    std::deque<std::chrono::steady_clock::time_point> m_frameTimes;
    
    uint64_t m_totalFrames;
    uint64_t m_droppedFrames;
    
    static constexpr int MAX_SAMPLE_COUNT = 120;  // 最多保留 120 帧数据（约 2 秒@60fps）
    static constexpr int FPS_UPDATE_INTERVAL = 60; // FPS 更新间隔（帧数）
};

} // namespace NativeXComponentSample

#endif // PERFORMANCE_MONITOR_H
