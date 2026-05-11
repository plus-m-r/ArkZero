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

Renderer::Renderer(int32_t width, int32_t height, PixelFormat format)
    : m_width(width)
    , m_height(height)
    , m_format(format)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Constructor: width=%{public}d, height=%{public}d, format=%{public}d", 
        width, height, static_cast<int>(format));
}

Renderer::~Renderer() {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Destructor");
    Destroy();
}

bool Renderer::Initialize() {
    if (m_backend && m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "Renderer", "Already initialized");
        return true;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "Initializing...");

    // ⭐ 直接创建 OpenGL ES 后端（HarmonyOS 平台默认支持）
    m_backend = std::make_unique<GLESBackend>();
    if (!m_backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Failed to create any render backend");
        return false;
    }

    // ⭐ 委托给后端初始化
    bool success = m_backend->Initialize(m_width, m_height, m_format);
    if (success) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "Renderer", "✅ Initialized with backend: %s", m_backend->GetBackendName());
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Failed to initialize backend");
        m_backend.reset();
    }

    return success;
}

bool Renderer::RenderFrame(const void* pixelData, size_t dataSize, 
                                 int32_t width, int32_t height) {
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "Renderer", "Not initialized");
        return false;
    }

    // ⭐ 委托给后端渲染
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

    // ⭐ 委托给后端销毁
    m_backend->Destroy();
    m_backend.reset();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "Renderer", "♻️ Renderer destroyed");
}

bool Renderer::IsInitialized() const {
    return m_backend && m_backend->IsInitialized();
}

const char* Renderer::GetBackendName() const {
    return m_backend ? m_backend->GetBackendName() : "None";
}

} // namespace NativeXComponentSample
