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

#include "yuv_shader_manager_napi.h"
#include "../renderer/backend/YUVShaderManager.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储 YUVShaderManager 实例的映射表（用于测试）
    static std::vector<std::unique_ptr<YUVShaderManager>> g_yuvShaders;
    
    /**
     * 从句柄获取 YUVShaderManager 指针
     */
    YUVShaderManager* GetShaderFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_yuvShaders.size()) {
            return nullptr;
        }
        return g_yuvShaders[handle].get();
    }
}

napi_value CreateYUVShaderManager(napi_env env, napi_callback_info info) {
    // 创建 YUVShaderManager 实例
    auto shader = std::make_unique<YUVShaderManager>();
    
    // 存储到全局映射表
    int64_t handle = static_cast<int64_t>(g_yuvShaders.size());
    g_yuvShaders.push_back(std::move(shader));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "YUVShaderNAPI", 
        "YUVShaderManager created, handle=%ld", handle);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    
    return result;
}

napi_value DestroyYUVShaderManager(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* shader = GetShaderFromHandle(handle);
    if (!shader) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid shader handle");
        return nullptr;
    }
    
    // 销毁 Shader
    shader->Destroy();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "YUVShaderNAPI", 
        "YUVShaderManager destroyed, handle=%ld", handle);
    
    return nullptr;
}

napi_value YUVShaderInitialize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* shader = GetShaderFromHandle(handle);
    if (!shader) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid shader handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 初始化 Shader
    bool success = shader->Initialize();
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value YUVShaderRenderNV21(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 5) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* shader = GetShaderFromHandle(handle);
    if (!shader) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid shader handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取 Y 平面数据
    void* yData = nullptr;
    size_t ySize = 0;
    napi_get_arraybuffer_info(env, args[1], &yData, &ySize);
    
    // 获取 UV 平面数据
    void* uvData = nullptr;
    size_t uvSize = 0;
    napi_get_arraybuffer_info(env, args[2], &uvData, &uvSize);
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[3], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[4], &height);
    
    // 渲染 NV21
    bool success = shader->RenderNV21(
        static_cast<const uint8_t*>(yData),
        static_cast<const uint8_t*>(uvData),
        width, height
    );
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value YUVShaderRenderNV12(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 5) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* shader = GetShaderFromHandle(handle);
    if (!shader) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid shader handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取 Y 平面数据
    void* yData = nullptr;
    size_t ySize = 0;
    napi_get_arraybuffer_info(env, args[1], &yData, &ySize);
    
    // 获取 UV 平面数据
    void* uvData = nullptr;
    size_t uvSize = 0;
    napi_get_arraybuffer_info(env, args[2], &uvData, &uvSize);
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[3], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[4], &height);
    
    // 渲染 NV12
    bool success = shader->RenderNV12(
        static_cast<const uint8_t*>(yData),
        static_cast<const uint8_t*>(uvData),
        width, height
    );
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value IsYUVShaderInitialized(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* shader = GetShaderFromHandle(handle);
    if (!shader) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "YUVShaderNAPI", "Invalid shader handle");
        return nullptr;
    }
    
    bool initialized = shader->IsInitialized();
    
    napi_value result;
    napi_get_boolean(env, initialized, &result);
    
    return result;
}

} // namespace NativeXComponentSample
