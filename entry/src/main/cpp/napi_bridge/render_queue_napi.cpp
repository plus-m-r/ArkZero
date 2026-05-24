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

#include "render_queue_napi.h"
#include "../renderer/core/RenderQueue.h"
#include <hilog/log.h>
#include <vector>

namespace NativeXComponentSample {

namespace {
    // 存储 RenderQueue 实例的映射表（用于测试）
    static std::vector<std::unique_ptr<RenderQueue>> g_renderQueues;
    
    /**
     * 从句柄获取 RenderQueue 指针
     */
    RenderQueue* GetQueueFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_renderQueues.size()) {
            return nullptr;
        }
        return g_renderQueues[handle].get();
    }
}

napi_value CreateRenderQueue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    // 默认队列大小为 3
    size_t maxSize = 3;
    
    if (argc >= 1) {
        int32_t sizeValue = 0;
        napi_get_value_int32(env, args[0], &sizeValue);
        if (sizeValue > 0) {
            maxSize = static_cast<size_t>(sizeValue);
        }
    }
    
    // 创建 RenderQueue 实例
    auto queue = std::make_unique<RenderQueue>(maxSize);
    
    // 存储到全局映射表
    int64_t handle = static_cast<int64_t>(g_renderQueues.size());
    g_renderQueues.push_back(std::move(queue));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "RenderQueueNAPI", 
        "RenderQueue created, handle=%ld, maxSize=%ld", handle, maxSize);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    
    return result;
}

napi_value DestroyRenderQueue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* queue = GetQueueFromHandle(handle);
    if (!queue) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid queue handle");
        return nullptr;
    }
    
    // 停止队列
    queue->Stop();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "RenderQueueNAPI", 
        "RenderQueue destroyed, handle=%ld", handle);
    
    return nullptr;
}

napi_value QueueSubmit(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 5) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* queue = GetQueueFromHandle(handle);
    if (!queue) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid queue handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取 ArrayBuffer
    void* bufferData = nullptr;
    size_t bufferSize = 0;
    napi_get_arraybuffer_info(env, args[1], &bufferData, &bufferSize);
    
    // 获取参数
    int32_t dataSize = 0;
    napi_get_value_int32(env, args[2], &dataSize);
    
    int32_t width = 0;
    napi_get_value_int32(env, args[3], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[4], &height);
    
    // 创建渲染命令并提交
    RenderCommand cmd(bufferData, static_cast<size_t>(dataSize), width, height);
    bool success = queue->Submit(cmd);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value QueueDequeue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* queue = GetQueueFromHandle(handle);
    if (!queue) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid queue handle");
        return nullptr;
    }
    
    // 消费命令（非阻塞版本，用于测试）
    RenderCommand cmd;
    bool success = queue->TryDequeue(cmd);
    
    if (!success) {
        // 返回 null
        return nullptr;
    }
    
    // 创建 JavaScript 对象
    napi_value result;
    napi_create_object(env, &result);
    
    // 设置属性
    napi_value dataSize;
    napi_create_int32(env, static_cast<int32_t>(cmd.dataSize), &dataSize);
    napi_set_named_property(env, result, "dataSize", dataSize);
    
    napi_value width;
    napi_create_int32(env, cmd.width, &width);
    napi_set_named_property(env, result, "width", width);
    
    napi_value height;
    napi_create_int32(env, cmd.height, &height);
    napi_set_named_property(env, result, "height", height);
    
    return result;
}

napi_value QueueStop(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* queue = GetQueueFromHandle(handle);
    if (!queue) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid queue handle");
        return nullptr;
    }
    
    queue->Stop();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "RenderQueueNAPI", 
        "RenderQueue stopped, handle=%ld", handle);
    
    return nullptr;
}

napi_value GetQueueInfo(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* queue = GetQueueFromHandle(handle);
    if (!queue) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid queue handle");
        return nullptr;
    }
    
    // 创建 JavaScript 对象
    napi_value result;
    napi_create_object(env, &result);
    
    // 设置 size
    napi_value size;
    napi_create_int32(env, static_cast<int32_t>(queue->Size()), &size);
    napi_set_named_property(env, result, "size", size);
    
    // 设置 isRunning
    napi_value isRunning;
    napi_get_boolean(env, queue->IsRunning(), &isRunning);
    napi_set_named_property(env, result, "isRunning", isRunning);
    
    return result;
}

napi_value QueuePeek(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* queue = GetQueueFromHandle(handle);
    if (!queue) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "RenderQueueNAPI", "Invalid queue handle");
        return nullptr;
    }
    
    // 窥视命令（不消费）
    RenderCommand cmd;
    bool success = queue->Peek(cmd);
    
    if (!success) {
        // 返回 null
        return nullptr;
    }
    
    // 创建 JavaScript 对象
    napi_value result;
    napi_create_object(env, &result);
    
    // 设置属性
    napi_value dataSize;
    napi_create_int32(env, static_cast<int32_t>(cmd.dataSize), &dataSize);
    napi_set_named_property(env, result, "dataSize", dataSize);
    
    napi_value width;
    napi_create_int32(env, cmd.width, &width);
    napi_set_named_property(env, result, "width", width);
    
    napi_value height;
    napi_create_int32(env, cmd.height, &height);
    napi_set_named_property(env, result, "height", height);
    
    return result;
}

} // namespace NativeXComponentSample
