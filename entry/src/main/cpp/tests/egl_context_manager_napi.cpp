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

#include "egl_context_manager_napi.h"
#include "../renderer/backend/EGLContextManager.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储 EGLContextManager 实例的映射表（用于测试）
    static std::vector<std::unique_ptr<EGLContextManager>> g_eglContexts;
    
    /**
     * 从句柄获取 EGLContextManager 指针
     */
    EGLContextManager* GetContextFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_eglContexts.size()) {
            return nullptr;
        }
        return g_eglContexts[handle].get();
    }
}

napi_value CreateEGLContext(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 2) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[0], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[1], &height);
    
    // 创建 EGLContextManager 实例
    auto context = std::make_unique<EGLContextManager>();
    
    // 使用离屏模式初始化（Pbuffer）
    bool success = context->InitializeOffscreen(width, height);
    
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", 
            "Failed to initialize offscreen EGL context");
        return nullptr;
    }
    
    // 存储到全局映射表
    int64_t handle = static_cast<int64_t>(g_eglContexts.size());
    g_eglContexts.push_back(std::move(context));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "EGLCtxNAPI", 
        "EGLContext created (offscreen), handle=%ld, size=%dx%d", handle, width, height);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    
    return result;
}

napi_value DestroyEGLContext(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* context = GetContextFromHandle(handle);
    if (!context) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid context handle");
        return nullptr;
    }
    
    // 销毁 EGL 上下文
    context->Destroy();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "EGLCtxNAPI", 
        "EGLContext destroyed, handle=%ld", handle);
    
    return nullptr;
}

napi_value EGLMakeCurrent(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* context = GetContextFromHandle(handle);
    if (!context) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid context handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 使上下文当前化
    bool success = context->MakeCurrent();
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value IsEGLInitialized(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* context = GetContextFromHandle(handle);
    if (!context) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "EGLCtxNAPI", "Invalid context handle");
        return nullptr;
    }
    
    bool initialized = context->IsInitialized();
    
    napi_value result;
    napi_get_boolean(env, initialized, &result);
    
    return result;
}

} // namespace NativeXComponentSample
