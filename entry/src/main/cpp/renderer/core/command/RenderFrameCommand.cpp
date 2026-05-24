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

#include "RenderFrameCommand.h"
#include <hilog/log.h>
#include <cstring>
#include "../../../common/common.h"

namespace NativeXComponentSample {

RenderFrameCommand::RenderFrameCommand(
    GLESBackend* backend,
    const void* pixelData,
    size_t dataSize,
    int32_t width,
    int32_t height)
    : m_backend(backend)
    , m_dataSize(dataSize)
    , m_width(width)
    , m_height(height) {
    
    // ⭐ 深拷贝像素数据，确保命令拥有独立的数据副本
    m_pixelData = std::malloc(dataSize);
    if (m_pixelData) {
        std::memcpy(const_cast<void*>(m_pixelData), pixelData, dataSize);
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "RenderFrameCommand", "Failed to allocate memory for pixel data");
    }
}

RenderFrameCommand::~RenderFrameCommand() {
    // ⭐ 释放像素数据内存
    if (m_pixelData) {
        std::free(const_cast<void*>(m_pixelData));
        m_pixelData = nullptr;
    }
}

bool RenderFrameCommand::Execute() {
    if (!m_backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "RenderFrameCommand", "Backend is null");
        return false;
    }
    
    if (!m_pixelData) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "RenderFrameCommand", "Pixel data is null");
        return false;
    }
    
    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN,
        "RenderFrameCommand", "🎨 Executing render frame: %dx%d", m_width, m_height);
    
    // ⭐ 执行渲染
    bool success = m_backend->RenderFrame(m_pixelData, m_dataSize, m_width, m_height);
    
    if (success) {
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN,
            "RenderFrameCommand", "✅ Render frame completed");
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "RenderFrameCommand", "❌ Render frame failed");
    }
    
    return success;
}

} // namespace NativeXComponentSample
