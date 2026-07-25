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

#include "surface_manager_napi.h"
#include "../renderer/manager/SurfaceManager.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储模拟的 NativeWindow 指针（用于测试）
    static std::vector<void*> g_mockWindows;
    
    /**
     * 生成模拟的 NativeWindow 指针
     */
    void* CreateMockNativeWindow(const std::string& surfaceId) {
        // 在测试环境中，返回一个非空的模拟指针
        // 实际项目中会调用 OH_NativeWindow_CreateNativeWindowFromSurfaceId
        void* mockPtr = reinterpret_cast<void*>(g_mockWindows.size() + 1);
        g_mockWindows.push_back(mockPtr);
        
        OH_LOG_Print(LOG_APP, LOG_INFO, 0, "SurfaceManagerNAPI", 
            "Created mock NativeWindow for surfaceId: %s, ptr=%p", 
            surfaceId.c_str(), mockPtr);
        
        return mockPtr;
    }
}

/**
 * 通过 surfaceId 创建 NativeWindow（模拟）
 */
napi_value SurfaceManagerCreateNativeWindow(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "SurfaceManagerNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取 surfaceId 字符串
    char surfaceIdBuf[256];
    size_t result = 0;
    napi_get_value_string_utf8(env, args[0], surfaceIdBuf, sizeof(surfaceIdBuf), &result);
    std::string surfaceId(surfaceIdBuf, result);
    
    if (surfaceId.empty()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, 0, "SurfaceManagerNAPI", "Empty surfaceId, returning 0");
        napi_value zeroVal;
        napi_create_int64(env, 0, &zeroVal);
        return zeroVal;
    }
    
    // ⭐ 在测试环境中使用模拟的 NativeWindow
    void* windowPtr = CreateMockNativeWindow(surfaceId);
    
    napi_value resultVal;
    napi_create_int64(env, reinterpret_cast<int64_t>(windowPtr), &resultVal);
    return resultVal;
}

/**
 * 销毁 NativeWindow（模拟）
 */
napi_value SurfaceManagerDestroyNativeWindow(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "SurfaceManagerNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t windowPtr = 0;
    napi_get_value_int64(env, args[0], &windowPtr);
    
    if (windowPtr == 0) {
        OH_LOG_Print(LOG_APP, LOG_WARN, 0, "SurfaceManagerNAPI", "Null window pointer");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "SurfaceManagerNAPI", 
        "Destroyed mock NativeWindow: ptr=%lld", windowPtr);
    
    napi_value trueVal;
    napi_get_boolean(env, true, &trueVal);
    return trueVal;
}

} // namespace NativeXComponentSample
