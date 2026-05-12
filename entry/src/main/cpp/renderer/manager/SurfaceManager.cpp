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

#include "SurfaceManager.h"
#include <hilog/log.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

SurfaceManager& SurfaceManager::GetInstance() {
    static SurfaceManager instance;
    return instance;
}

void SurfaceManager::RegisterSurface(const std::string& surfaceId, void* nativeWindow) {
    if (surfaceId.empty() || !nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "RegisterSurface: invalid parameters");
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_surfaceMap[surfaceId] = nativeWindow;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "SurfaceManager", "✅ Registered surface: %{public}s", surfaceId.c_str());
}

void SurfaceManager::UnregisterSurface(const std::string& surfaceId) {
    if (surfaceId.empty()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "UnregisterSurface: empty surfaceId");
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_surfaceMap.find(surfaceId);
    if (it != m_surfaceMap.end()) {
        m_surfaceMap.erase(it);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "♻️ Unregistered surface: %{public}s", surfaceId.c_str());
    } else {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "⚠️ Surface not found: %{public}s", surfaceId.c_str());
    }
}

void* SurfaceManager::GetNativeWindow(const std::string& surfaceId) {
    if (surfaceId.empty()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "GetNativeWindow: empty surfaceId");
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_surfaceMap.find(surfaceId);
    if (it != m_surfaceMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "Found NativeWindow for surface: %{public}s", surfaceId.c_str());
        return it->second;
    }
    
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
        "SurfaceManager", "⚠️ NativeWindow not found for surface: %{public}s", surfaceId.c_str());
    return nullptr;
}

} // namespace NativeXComponentSample
