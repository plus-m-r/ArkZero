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

#include "Renderer.h"
#include "../backend/GLESBackend.h"
#include <hilog/log.h>

#include "../../common/common.h"

namespace NativeXComponentSample {

Renderer::Renderer(int32_t width, int32_t height, PixelFormat format, bool enableAsync)
    : m_width(width)
    , m_height(height)
    , m_format(format)
    , m_enableAsync(enableAsync)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Constructor: %dx%d, format=%d, async=%s", 
        width, height, static_cast<int>(format), enableAsync ? "enabled" : "disabled");
}

Renderer::~Renderer() {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Destructor");
    Destroy();
}

bool Renderer::Initialize(void* nativeWindow) {
    if (m_backend && m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "Renderer", "Already initialized");
        return true;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Initializing with XComponent Surface...");

    // ⭐ 1. 创建 OpenGL ES 后端
    auto glesBackend = std::make_unique<GLESBackend>();
    
    // ⭐ 2. 使用 NativeWindow 初始化
    bool success = glesBackend->Initialize(nativeWindow, m_width, m_height, m_format);
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Failed to initialize backend with surface");
        return false;
    }
    
    m_backend = std::move(glesBackend);

    // ⭐ 3. 如果启用异步渲染，启动后台线程
    if (m_enableAsync) {
        m_renderQueue = std::make_unique<RenderQueue>(3);  // 3 帧缓冲
        m_renderThread = std::thread(&Renderer::renderLoop, this);
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "Renderer", "✅ Async rendering thread started");
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "✅ Initialized");
    
    return true;
}

bool Renderer::InitializeOffscreen(int32_t width, int32_t height) {
    if (m_backend && m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "Renderer", "Already initialized");
        return true;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Initializing offscreen renderer: %dx%d", width, height);

    // ⭐ 1. 创建 OpenGL ES 后端
    auto glesBackend = std::make_unique<GLESBackend>();
    
    // ⭐ 2. 使用离屏模式初始化
    bool success = glesBackend->InitializeOffscreen(width, height, m_format);
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Failed to initialize backend offscreen");
        return false;
    }
    
    m_backend = std::move(glesBackend);

    // ⭐ 3. 如果启用异步渲染，启动后台线程
    if (m_enableAsync) {
        m_renderQueue = std::make_unique<RenderQueue>(3);  // 3 帧缓冲
        m_renderThread = std::thread(&Renderer::renderLoop, this);
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "Renderer", "✅ Async rendering thread started (offscreen)");
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "✅ Offscreen renderer initialized");
    
    return true;
}

bool Renderer::RenderFrame(const void* pixelData, size_t dataSize, 
                                 int32_t width, int32_t height) {
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Not initialized");
        return false;
    }

    // ⭐ 异步模式：提交到队列，立即返回
    if (m_enableAsync && m_renderQueue) {
        RenderCommand cmd(pixelData, dataSize, width, height);
        bool success = m_renderQueue->Submit(cmd);
        
        if (!success) {
            OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
                "Renderer", "Failed to submit render command");
        }
        
        return success;
    }
    
    // ⭐ 同步模式：直接渲染（保留用于调试）
    return m_backend->RenderFrame(pixelData, dataSize, width, height);
}

bool Renderer::Resize(int32_t width, int32_t height) {
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Not initialized");
        return false;
    }

    // ⭐ 委托给后端
    bool success = m_backend->Resize(width, height);
    if (success) {
        m_width = width;
        m_height = height;
    }
    return success;
}

void Renderer::Destroy() {
    if (!m_backend) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Destroying renderer...");

    // ⭐ 1. 停止异步渲染队列和线程
    if (m_renderQueue) {
        m_renderQueue->Stop();
    }
    
    if (m_renderThread.joinable()) {
        m_renderThread.join();
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "Renderer", "Render thread joined");
    }

    // ⭐ 2. 销毁后端
    m_backend->Destroy();
    m_backend.reset();
    
    // ⭐ 3. 清理队列
    m_renderQueue.reset();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "♻️ Renderer destroyed");
}

bool Renderer::IsInitialized() const {
    return m_backend && m_backend->IsInitialized();
}

const char* Renderer::GetBackendName() const {
    return m_backend ? m_backend->GetBackendName() : "None";
}

std::string Renderer::GetPerformanceStats() const {
    auto stats = m_perfMonitor.GetStats();
    return stats.ToString();
}

void Renderer::renderLoop() {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "🚀 Render thread started");
    
    while (m_renderQueue && m_renderQueue->IsRunning()) {
        RenderCommand cmd;
        
        // ⭐ 从队列获取渲染命令（阻塞等待）
        if (!m_renderQueue->Dequeue(cmd)) {
            break;  // 队列已停止
        }
        
        // ⭐ 记录帧开始
        m_perfMonitor.BeginFrame();
        
        // ⭐ 执行实际渲染
        bool success = false;
        if (m_backend && m_backend->IsInitialized()) {
            success = m_backend->RenderFrame(
                cmd.pixelData, 
                cmd.dataSize, 
                cmd.width, 
                cmd.height
            );
        }
        
        // ⭐ 记录帧结束
        m_perfMonitor.EndFrame(!success);
        
        if (!success) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "Renderer", "Render frame failed");
        }
        
        // 每 60 帧输出一次性能统计
        if (m_perfMonitor.GetTotalFrames() % 60 == 0) {
            auto stats = m_perfMonitor.GetStats();
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "Renderer", "📊 %s", stats.ToString().c_str());
        }
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "🛑 Render thread exited");
}

} // namespace NativeXComponentSample
