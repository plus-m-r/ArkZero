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

#include "renderer_napi.h"
#include "../renderer/core/Renderer.h"
#include "../common/common.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储 Renderer 实例的映射表
    static std::vector<std::unique_ptr<Renderer>> g_renderers;
    
    /**
     * 从句柄获取 Renderer 指针
     */
    Renderer* GetRendererFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_renderers.size()) {
            return nullptr;
        }
        return g_renderers[handle].get();
    }
}

/**
 * 创建渲染器（测试专用）
 */
napi_value CreateTestRenderer(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int32_t width = 0;
    int32_t height = 0;
    int32_t format = 0;
    bool enableAsync = true;
    
    napi_get_value_int32(env, args[0], &width);
    napi_get_value_int32(env, args[1], &height);
    napi_get_value_int32(env, args[2], &format);
    
    if (argc >= 4) {
        napi_get_value_bool(env, args[3], &enableAsync);
    }
    
    auto renderer = std::make_unique<Renderer>(width, height, static_cast<PixelFormat>(format), enableAsync);
    int64_t handle = static_cast<int64_t>(g_renderers.size());
    g_renderers.push_back(std::move(renderer));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "RendererNAPI", "Created renderer: handle=%lld", handle);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    return result;
}

/**
 * 销毁渲染器（测试专用）
 */
napi_value DestroyTestRenderer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (renderer) {
        renderer->Destroy();
        OH_LOG_Print(LOG_APP, LOG_INFO, 0, "RendererNAPI", "Destroyed renderer: handle=%lld", handle);
    }
    
    return nullptr;
}

/**
 * 初始化渲染器（离屏模式）
 */
napi_value RendererInitialize(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (!renderer) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid renderer handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    int32_t width = 0;
    int32_t height = 0;
    napi_get_value_int32(env, args[1], &width);
    napi_get_value_int32(env, args[2], &height);
    
    bool success = renderer->InitializeOffscreen(width, height);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

/**
 * 渲染帧
 */
napi_value RendererRenderFrame(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (!renderer) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid renderer handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取 ArrayBuffer
    void* data = nullptr;
    size_t dataSize = 0;
    napi_get_arraybuffer_info(env, args[1], &data, &dataSize);
    
    int32_t width = 0;
    int32_t height = 0;
    napi_get_value_int32(env, args[2], &width);
    napi_get_value_int32(env, args[3], &height);
    
    bool success = renderer->RenderFrame(data, dataSize, width, height);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

/**
 * 调整尺寸
 */
napi_value RendererResize(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (!renderer) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid renderer handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    int32_t width = 0;
    int32_t height = 0;
    napi_get_value_int32(env, args[1], &width);
    napi_get_value_int32(env, args[2], &height);
    
    bool success = renderer->Resize(width, height);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

/**
 * 检查是否已初始化
 */
napi_value IsRendererInitialized(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (!renderer) {
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    bool initialized = renderer->IsInitialized();
    
    napi_value result;
    napi_get_boolean(env, initialized, &result);
    return result;
}

/**
 * 获取后端名称
 */
napi_value GetRendererBackendName(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (!renderer) {
        napi_value nullVal;
        napi_get_null(env, &nullVal);
        return nullVal;
    }
    
    const char* name = renderer->GetBackendName();
    
    napi_value result;
    napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &result);
    return result;
}

/**
 * 获取性能统计
 */
napi_value GetRendererPerformanceStats(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RendererNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* renderer = GetRendererFromHandle(handle);
    if (!renderer) {
        napi_value nullVal;
        napi_get_null(env, &nullVal);
        return nullVal;
    }
    
    std::string stats = renderer->GetPerformanceStats();
    
    napi_value result;
    napi_create_string_utf8(env, stats.c_str(), stats.length(), &result);
    return result;
}

} // namespace NativeXComponentSample
