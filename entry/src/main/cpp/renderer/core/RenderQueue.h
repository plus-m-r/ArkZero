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

#ifndef RENDER_QUEUE_H
#define RENDER_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <cstddef>

namespace NativeXComponentSample {

/**
 * 渲染命令
 * 
 * 封装单次渲染所需的所有数据
 */
struct RenderCommand {
    const void* pixelData;   // 像素数据指针（零拷贝）
    size_t dataSize;         // 数据大小
    int32_t width;           // 宽度
    int32_t height;          // 高度
    
    RenderCommand() 
        : pixelData(nullptr)
        , dataSize(0)
        , width(0)
        , height(0) {}
    
    RenderCommand(const void* data, size_t size, int32_t w, int32_t h)
        : pixelData(data)
        , dataSize(size)
        , width(w)
        , height(h) {}
};

/**
 * 线程安全的渲染队列（生产者-消费者模型）
 * 
 * 🎯 职责：
 * - ArkTS 主线程提交渲染命令（生产者）
 * - 后台渲染线程消费命令并执行渲染（消费者）
 * - 自动丢弃过时帧以维持低延迟
 * 
 * 设计原则：单一职责 - 仅负责任务调度和线程同步
 */
class RenderQueue {
public:
    /**
     * 构造函数
     * @param maxQueueSize 最大队列长度（默认 3 帧缓冲）
     */
    explicit RenderQueue(size_t maxQueueSize = 3);
    
    ~RenderQueue();

    /**
     * 提交渲染命令（ArkTS 主线程调用）
     * 
     * ⭐ 非阻塞：如果队列已满，丢弃最旧的帧
     * ⭐ 耗时 <0.5ms
     * 
     * @param cmd 渲染命令
     * @return true 成功提交，false 队列已停止
     */
    bool Submit(const RenderCommand& cmd);

    /**
     * 获取下一个渲染命令（后台渲染线程调用）
     * 
     * ⭐ 阻塞等待：队列为空时等待新命令
     * ⭐ 自动丢弃过时帧：只返回最新的一帧
     * 
     * @param cmd 输出参数，接收渲染命令
     * @return true 成功获取，false 队列已停止
     */
    bool Dequeue(RenderCommand& cmd);

    /**
     * 尝试获取下一个渲染命令（非阻塞版本，用于测试）
     * 
     * ⭐ 非阻塞：如果队列为空，立即返回 false
     * ⭐ 自动丢弃过时帧：只返回最新的一帧
     * 
     * @param cmd 输出参数，接收渲染命令
     * @return true 成功获取，false 队列为空或已停止
     */
    bool TryDequeue(RenderCommand& cmd);

    /**
     * 窥视队列头部命令（不消费，用于测试）
     * 
     * ⭐ 非阻塞：如果队列为空，立即返回 false
     * ⭐ 不移除命令：保持队列不变
     * 
     * @param cmd 输出参数，接收渲染命令
     * @return true 成功获取，false 队列为空
     */
    bool Peek(RenderCommand& cmd);

    /**
     * 停止队列
     * 
     * 唤醒所有等待的线程，退出循环
     */
    void Stop();

    /**
     * 检查队列是否正在运行
     */
    bool IsRunning() const { return m_running; }

    /**
     * 获取队列当前长度
     */
    size_t Size() const;

private:
    std::queue<RenderCommand> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    size_t m_maxQueueSize;
    bool m_running;
};

} // namespace NativeXComponentSample

#endif // RENDER_QUEUE_H
