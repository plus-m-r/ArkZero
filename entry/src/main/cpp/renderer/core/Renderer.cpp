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
#include <cstring>

#include "../../common/common.h"

namespace NativeXComponentSample {

Renderer::Renderer(int32_t width, int32_t height, PixelFormat format)
    : m_width(width)
    , m_height(height)
    , m_format(format)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Constructor: %{public}dx%{public}d, format=%{public}d", 
        width, height, static_cast<int>(format));
}

Renderer::~Renderer()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Destructor");
    Destroy();
}

bool Renderer::Initialize(void* nativeWindow)
{
    if (m_backend && m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "Renderer", "Already initialized");
        return true;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Initializing with NativeWindow...");

    // 1. 初始化渲染后端
    auto glesBackend = std::make_unique<GLESBackend>();
    bool success = glesBackend->Initialize(nativeWindow, m_width, m_height, m_format);
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Failed to initialize backend");
        return false;
    }
    m_backend = std::move(glesBackend);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "✅ Initialized");
    
    return true;
}

bool Renderer::RenderFrame(const void* pixelData, size_t dataSize, int32_t width, int32_t height)
{
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Not initialized");
        return false;
    }

    if (!pixelData || dataSize == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Invalid pixel data");
        return false;
    }

    std::lock_guard<std::mutex> lock(m_renderMutex);

    // ⭐ 单槽 ownership transfer：调用期间独占这块 buffer，直到返回才允许 ArkTS 复用
    bool renderSuccess = m_backend->RenderFrame(pixelData, dataSize, width, height);
    if (!renderSuccess) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Failed to render frame");
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
        "Renderer", "🎨 Frame rendered (single-slot ownership transfer)");

    return true;
}

bool Renderer::Resize(int32_t width, int32_t height)
{
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Not initialized");
        return false;
    }

    bool success = m_backend->Resize(width, height);
    if (success) {
        m_width = width;
        m_height = height;
    }
    return success;
}

void Renderer::Destroy()
{
    if (!m_backend) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Destroying...");

    // 直接销毁后端，无队列、无后台线程
    if (m_backend) {
        m_backend->Destroy();
        m_backend.reset();
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "♻️ Destroyed");
}

bool Renderer::IsInitialized() const
{
    return m_backend && m_backend->IsInitialized();
}

// ========================================
// 后台渲染线程主函数
// ========================================

} // namespace NativeXComponentSample
