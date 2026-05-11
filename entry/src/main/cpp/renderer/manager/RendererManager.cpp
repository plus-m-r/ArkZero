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

#include "ScreenRendererManager.h"
#include <hilog/log.h>

#include "../../common/common.h"

namespace NativeXComponentSample {

RendererManager::RendererManager() : m_nextHandle(1) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "Manager created");
}

RendererManager::~RendererManager() {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "Manager destroying, cleaning up all renderers");
    
    // 清理所有renderer
    for (auto& pair : m_renderers) {
        if (pair.second) {
            pair.second->Destroy();
        }
    }
    m_renderers.clear();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "Manager destroyed");
}

RendererManager& RendererManager::GetInstance() {
    static RendererManager instance;
    return instance;
}

int32_t RendererManager::CreateRenderer(int32_t width, int32_t height, PixelFormat format) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "Creating renderer: %{public}dx%{public}d, format=%{public}d", 
        width, height, static_cast<int>(format));
    
    auto renderer = std::make_unique<Renderer>(width, height, format);
    if (!renderer->Initialize()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "ScreenRendererManager", "Failed to initialize renderer");
        return -1;
    }
    
    int32_t handle = m_nextHandle++;
    m_renderers[handle] = std::move(renderer);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "✅ Created renderer: handle=%{public}d", handle);
    
    return handle;
}

Renderer* RendererManager::GetRenderer(int32_t handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_renderers.find(handle);
    if (it == m_renderers.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "ScreenRendererManager", "Renderer not found: handle=%{public}d", handle);
        return nullptr;
    }
    
    return it->second.get();
}

bool RendererManager::DestroyRenderer(int32_t handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_renderers.find(handle);
    if (it == m_renderers.end()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "ScreenRendererManager", "Renderer not found for destroy: handle=%{public}d", handle);
        return false;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "Destroying renderer: handle=%{public}d", handle);
    
    it->second->Destroy();
    m_renderers.erase(it);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "ScreenRendererManager", "♻️ Destroyed renderer: handle=%{public}d", handle);
    
    return true;
}

} // namespace NativeXComponentSample
