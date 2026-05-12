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

#include "texture_pool_napi.h"
#include "../renderer/backend/TexturePool.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储 TexturePool 实例的映射表
    static std::vector<std::unique_ptr<TexturePool>> g_texturePools;
    
    /**
     * 从句柄获取 TexturePool 指针
     */
    TexturePool* GetPoolFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_texturePools.size()) {
            return nullptr;
        }
        return g_texturePools[handle].get();
    }
}

napi_value CreateTexturePool(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    // 默认最大池大小为 10
    size_t maxSize = 10;
    
    if (argc >= 1) {
        int32_t sizeParam = 0;
        napi_get_value_int32(env, args[0], &sizeParam);
        if (sizeParam > 0) {
            maxSize = static_cast<size_t>(sizeParam);
        }
    }
    
    // 创建 TexturePool 实例
    auto pool = std::make_unique<TexturePool>(maxSize);
    
    // 存储到全局映射表
    int64_t handle = static_cast<int64_t>(g_texturePools.size());
    g_texturePools.push_back(std::move(pool));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TexturePoolNAPI", 
        "TexturePool created, handle=%ld, maxSize=%zu", handle, maxSize);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    
    return result;
}

napi_value DestroyTexturePool(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        return nullptr;
    }
    
    // 清空池
    pool->Clear();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TexturePoolNAPI", 
        "TexturePool destroyed, handle=%ld", handle);
    
    return nullptr;
}

napi_value TexturePoolAcquire(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 5) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[1], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[2], &height);
    
    int32_t internalFormat = 0;
    napi_get_value_int32(env, args[3], &internalFormat);
    
    int32_t format = 0;
    napi_get_value_int32(env, args[4], &format);
    
    // 获取或创建纹理
    auto* texture = pool->Acquire(width, height, internalFormat, format);
    
    bool success = (texture != nullptr);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value TexturePoolRelease(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        return nullptr;
    }
    
    // ⚠️ 注意：当前实现中 Release 需要 TextureManager* 参数
    // 但在测试中，我们简化为只调用 Clear 来模拟归还
    // 实际使用时需要通过 Acquire 返回的纹理指针来归还
    
    OH_LOG_Print(LOG_APP, LOG_WARN, 0, "TexturePoolNAPI", 
        "Release not fully implemented in test mode");
    
    return nullptr;
}

napi_value TexturePoolPreallocate(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        return nullptr;
    }
    
    // 获取分辨率列表（简化：这里不解析数组，直接跳过）
    // 在实际实现中，需要解析 ArkTS 传入的数组
    
    int32_t internalFormat = 0;
    napi_get_value_int32(env, args[2], &internalFormat);
    
    int32_t format = 0;
    napi_get_value_int32(env, args[3], &format);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TexturePoolNAPI", 
        "Preallocate called (simplified in test mode)");
    
    return nullptr;
}

napi_value TexturePoolClear(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        return nullptr;
    }
    
    // 清空池
    pool->Clear();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TexturePoolNAPI", 
        "TexturePool cleared, handle=%ld", handle);
    
    return nullptr;
}

napi_value TexturePoolSize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        return nullptr;
    }
    
    size_t size = pool->Size();
    
    napi_value result;
    napi_create_int64(env, static_cast<int64_t>(size), &result);
    
    return result;
}

napi_value TexturePoolGetStats(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* pool = GetPoolFromHandle(handle);
    if (!pool) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TexturePoolNAPI", "Invalid pool handle");
        return nullptr;
    }
    
    auto stats = pool->GetStats();
    
    // 创建 JavaScript 对象
    napi_value result;
    napi_create_object(env, &result);
    
    // 设置 hitCount
    napi_value hitCount;
    napi_create_int64(env, static_cast<int64_t>(stats.hitCount), &hitCount);
    napi_set_named_property(env, result, "hitCount", hitCount);
    
    // 设置 missCount
    napi_value missCount;
    napi_create_int64(env, static_cast<int64_t>(stats.missCount), &missCount);
    napi_set_named_property(env, result, "missCount", missCount);
    
    // 设置 hitRate
    napi_value hitRate;
    napi_create_double(env, static_cast<double>(stats.GetHitRate()), &hitRate);
    napi_set_named_property(env, result, "hitRate", hitRate);
    
    return result;
}

} // namespace NativeXComponentSample
