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

#include "renderer_manager_napi.h"
#include "../renderer/manager/RendererManager.h"
#include "../renderer/manager/SurfaceManager.h"
#include "../common/common.h"
#include <hilog/log.h>

namespace NativeXComponentSample {

/**
 * 创建支持真实 Surface 的渲染器
 */
napi_value ManagerCreateSurfaceRenderer(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererManagerNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取 surfaceId 字符串
    char surfaceIdBuf[256];
    size_t result = 0;
    napi_get_value_string_utf8(env, args[0], surfaceIdBuf, sizeof(surfaceIdBuf), &result);
    std::string surfaceId(surfaceIdBuf, result);
    
    int32_t width = 0;
    int32_t height = 0;
    int32_t format = 0;
    
    napi_get_value_int32(env, args[1], &width);
    napi_get_value_int32(env, args[2], &height);
    napi_get_value_int32(env, args[3], &format);
    
    // 使用 SurfaceManager 获取真实的 NativeWindow（这是保留的核心接口）
    void* nativeWindow = SurfaceManager::GetInstance().CreateNativeWindow(surfaceId);
    if (!nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererManagerNAPI", "Failed to get NativeWindow from surfaceId");
        napi_value negOneVal;
        napi_create_int32(env, -1, &negOneVal);
        return negOneVal;
    }
    
    int32_t handle = RendererManager::GetInstance().CreateRenderer(
        nativeWindow, width, height, static_cast<PixelFormat>(format)
    );
    
    napi_value resultVal;
    napi_create_int32(env, handle, &resultVal);
    return resultVal;
}

/**
 * 销毁渲染器（通过 Manager）
 */
napi_value ManagerDestroyRenderer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererManagerNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int32_t handle = 0;
    napi_get_value_int32(env, args[0], &handle);
    
    bool success = RendererManager::GetInstance().DestroyRenderer(handle);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

} // namespace NativeXComponentSample
