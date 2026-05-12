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
#include <native_window/external_window.h>  // ⭐ OH_NativeWindow API
#include "../../common/common.h"
#include <cstdint>
#include <sstream>

namespace NativeXComponentSample {

SurfaceManager& SurfaceManager::GetInstance() {
    static SurfaceManager instance;
    return instance;
}

void* SurfaceManager::CreateNativeWindow(const std::string& surfaceId) {
    if (surfaceId.empty()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "CreateNativeWindow: empty surfaceId");
        return nullptr;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "SurfaceManager", "Creating NativeWindow from surfaceId: %{public}s", surfaceId.c_str());
    
    // ⭐ 将 surfaceId 字符串转换为 uint64_t
    // surfaceId 格式通常为 "surface_xxx"，需要提取数字部分
    uint64_t surfaceIdNum = 0;
    try {
        // 尝试直接转换（如果 surfaceId 是纯数字）
        surfaceIdNum = std::stoull(surfaceId);
    } catch (...) {
        // 如果包含前缀，提取数字部分
        // 例如："surface_123456" -> 123456
        size_t pos = surfaceId.find_last_of('_');
        if (pos != std::string::npos && pos + 1 < surfaceId.length()) {
            try {
                surfaceIdNum = std::stoull(surfaceId.substr(pos + 1));
            } catch (...) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "SurfaceManager", "Failed to parse surfaceId: %{public}s", surfaceId.c_str());
                return nullptr;
            }
        } else {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "SurfaceManager", "Invalid surfaceId format: %{public}s", surfaceId.c_str());
            return nullptr;
        }
    }
    
    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
        "SurfaceManager", "Parsed surfaceId number: %{public}llu", surfaceIdNum);
    
    // ⭐ 调用 HarmonyOS NDK API 创建 NativeWindow
    OHNativeWindow* nativeWindow = nullptr;
    int32_t result = OH_NativeWindow_CreateNativeWindowFromSurfaceId(surfaceIdNum, &nativeWindow);
    
    if (result != 0 || !nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "❌ Failed to create NativeWindow: result=%{public}d", result);
        return nullptr;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "SurfaceManager", "✅ Created NativeWindow: %{public}p", nativeWindow);
    
    return reinterpret_cast<void*>(nativeWindow);
}

void SurfaceManager::DestroyNativeWindow(void* nativeWindow) {
    if (!nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "SurfaceManager", "DestroyNativeWindow: null pointer");
        return;
    }
    
    OHNativeWindow* window = reinterpret_cast<OHNativeWindow*>(nativeWindow);
    
    // ⭐ 调用 HarmonyOS NDK API 销毁 NativeWindow
    OH_NativeWindow_DestroyNativeWindow(window);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "SurfaceManager", "♻️ Destroyed NativeWindow: %{public}p", nativeWindow);
}

} // namespace NativeXComponentSample
