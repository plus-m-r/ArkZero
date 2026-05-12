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

#include "RenderQueue.h"
#include <hilog/log.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

RenderQueue::RenderQueue(size_t maxQueueSize)
    : m_maxQueueSize(maxQueueSize)
    , m_running(true)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RenderQueue", "Created with maxQueueSize=%zu", maxQueueSize);
}

RenderQueue::~RenderQueue() {
    Stop();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RenderQueue", "Destroyed");
}

bool RenderQueue::Submit(const RenderCommand& cmd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_running) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "RenderQueue", "Queue is stopped, cannot submit");
        return false;
    }
    
    // ⭐ 如果队列已满，丢弃最旧的帧（维持低延迟）
    if (m_queue.size() >= m_maxQueueSize) {
        m_queue.pop();
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
            "RenderQueue", "Queue full, dropped oldest frame");
    }
    
    m_queue.push(cmd);
    
    // 通知消费者线程
    m_condition.notify_one();
    
    return true;
}

bool RenderQueue::Dequeue(RenderCommand& cmd) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    // ⭐ 阻塞等待：队列为空时等待新命令或停止信号
    m_condition.wait(lock, [this]() {
        return !m_queue.empty() || !m_running;
    });
    
    if (!m_running && m_queue.empty()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "RenderQueue", "Queue stopped, exiting dequeue");
        return false;
    }
    
    // ⭐ 自动丢弃过时帧：只返回最新的一帧
    while (m_queue.size() > 1) {
        m_queue.pop();
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
            "RenderQueue", "Dropped %zu outdated frames", m_queue.size());
    }
    
    cmd = m_queue.front();
    m_queue.pop();
    
    return true;
}

bool RenderQueue::TryDequeue(RenderCommand& cmd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // ⭐ 非阻塞：如果队列为空或已停止，立即返回 false
    if (!m_running || m_queue.empty()) {
        return false;
    }
    
    // ⭐ FIFO：按顺序消费，不丢弃帧
    cmd = m_queue.front();
    m_queue.pop();
    
    return true;
}

bool RenderQueue::Peek(RenderCommand& cmd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // ⭐ 非阻塞：如果队列为空，立即返回 false
    if (m_queue.empty()) {
        return false;
    }
    
    // ⭐ 不移除命令：只查看
    cmd = m_queue.front();
    
    return true;
}

void RenderQueue::Stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) {
            return;
        }
        m_running = false;
    }
    
    // 唤醒所有等待的线程
    m_condition.notify_all();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RenderQueue", "Stopped");
}

size_t RenderQueue::Size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

} // namespace NativeXComponentSample
