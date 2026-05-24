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

#include "gles_backend_napi.h"
#include "../renderer/backend/GLESBackend.h"
#include "../common/common.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储 GLESBackend 实例的映射表
    static std::vector<std::unique_ptr<GLESBackend>> g_glesBackends;
    
    /**
     * 从句柄获取 GLESBackend 指针
     */
    GLESBackend* GetBackendFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_glesBackends.size()) {
            return nullptr;
        }
        return g_glesBackends[handle].get();
    }
}

napi_value CreateGLESBackend(napi_env env, napi_callback_info info) {
    // 创建 GLESBackend 实例
    auto backend = std::make_unique<GLESBackend>();
    
    // 存储到全局映射表
    int64_t handle = static_cast<int64_t>(g_glesBackends.size());
    g_glesBackends.push_back(std::move(backend));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "GLESBackendNAPI", 
        "GLESBackend created, handle=%ld", handle);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    
    return result;
}

napi_value DestroyGLESBackend(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* backend = GetBackendFromHandle(handle);
    if (!backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid backend handle");
        return nullptr;
    }
    
    // 销毁后端
    backend->Destroy();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "GLESBackendNAPI", 
        "GLESBackend destroyed, handle=%ld", handle);
    
    return nullptr;
}

/**
 * 初始化 GLES 后端（离屏模式，用于测试）
 * 
 * ArkTS调用: glesBackendInitialize(handle: number, width: number, height: number, format: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value GLESBackendInitialize(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* backend = GetBackendFromHandle(handle);
    if (!backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid backend handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[1], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[2], &height);
    
    int32_t formatInt = 0;
    napi_get_value_int32(env, args[3], &formatInt);
    PixelFormat format = static_cast<PixelFormat>(formatInt);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "GLESBackendNAPI", 
        "Initializing GLESBackend (offscreen): %dx%d, format=%d", 
        width, height, formatInt);
    
    // ⭐ 使用离屏模式初始化（Pbuffer）
    bool success = backend->InitializeOffscreen(width, height, format);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value GLESBackendRenderFrame(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* backend = GetBackendFromHandle(handle);
    if (!backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid backend handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取像素数据
    void* pixelData = nullptr;
    size_t dataSize = 0;
    napi_get_arraybuffer_info(env, args[1], &pixelData, &dataSize);
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[2], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[3], &height);
    
    // 渲染帧
    bool success = backend->RenderFrame(pixelData, dataSize, width, height);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value IsGLESBackendInitialized(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* backend = GetBackendFromHandle(handle);
    if (!backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "GLESBackendNAPI", "Invalid backend handle");
        return nullptr;
    }
    
    bool initialized = backend->IsInitialized();
    
    napi_value result;
    napi_get_boolean(env, initialized, &result);
    
    return result;
}

} // namespace NativeXComponentSample
