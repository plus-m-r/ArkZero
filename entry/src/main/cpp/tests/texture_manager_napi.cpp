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

#include "texture_manager_napi.h"
#include "../renderer/backend/TextureManager.h"
#include <hilog/log.h>
#include <vector>
#include <memory>

namespace NativeXComponentSample {

namespace {
    // 存储 TextureManager 实例的映射表（用于测试）
    static std::vector<std::unique_ptr<TextureManager>> g_textureManagers;
    
    /**
     * 从句柄获取 TextureManager 指针
     */
    TextureManager* GetManagerFromHandle(int64_t handle) {
        if (handle < 0 || static_cast<size_t>(handle) >= g_textureManagers.size()) {
            return nullptr;
        }
        return g_textureManagers[handle].get();
    }
}

napi_value CreateTextureManager(napi_env env, napi_callback_info info) {
    // 创建 TextureManager 实例
    auto manager = std::make_unique<TextureManager>();
    
    // 存储到全局映射表
    int64_t handle = static_cast<int64_t>(g_textureManagers.size());
    g_textureManagers.push_back(std::move(manager));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TextureMgrNAPI", 
        "TextureManager created, handle=%ld", handle);
    
    napi_value result;
    napi_create_int64(env, handle, &result);
    
    return result;
}

napi_value DestroyTextureManager(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* manager = GetManagerFromHandle(handle);
    if (!manager) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid manager handle");
        return nullptr;
    }
    
    // 销毁纹理
    manager->Destroy();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TextureMgrNAPI", 
        "TextureManager destroyed, handle=%ld", handle);
    
    return nullptr;
}

napi_value TextureCreate(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 5) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* manager = GetManagerFromHandle(handle);
    if (!manager) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid manager handle");
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
    
    // 创建纹理
    bool success = manager->Create(width, height, internalFormat, static_cast<GLenum>(format));
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value TextureUpdate(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 5) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid arguments");
        return nullptr;
    }
    
    // 获取句柄
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* manager = GetManagerFromHandle(handle);
    if (!manager) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid manager handle");
        napi_value falseVal;
        napi_get_boolean(env, false, &falseVal);
        return falseVal;
    }
    
    // 获取 ArrayBuffer
    void* bufferData = nullptr;
    size_t bufferSize = 0;
    napi_get_arraybuffer_info(env, args[1], &bufferData, &bufferSize);
    
    // 获取参数
    int32_t width = 0;
    napi_get_value_int32(env, args[2], &width);
    
    int32_t height = 0;
    napi_get_value_int32(env, args[3], &height);
    
    int32_t format = 0;
    napi_get_value_int32(env, args[4], &format);
    
    // 更新纹理
    bool success = manager->Update(bufferData, width, height, static_cast<GLenum>(format));
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    
    return result;
}

napi_value TextureDestroy(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* manager = GetManagerFromHandle(handle);
    if (!manager) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid manager handle");
        return nullptr;
    }
    
    manager->Destroy();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "TextureMgrNAPI", 
        "Texture destroyed, handle=%ld", handle);
    
    return nullptr;
}

napi_value GetTextureId(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* manager = GetManagerFromHandle(handle);
    if (!manager) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid manager handle");
        return nullptr;
    }
    
    GLuint textureId = manager->GetTextureId();
    
    napi_value result;
    napi_create_uint32(env, textureId, &result);
    
    return result;
}

napi_value IsTextureCreated(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* manager = GetManagerFromHandle(handle);
    if (!manager) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "TextureMgrNAPI", "Invalid manager handle");
        return nullptr;
    }
    
    bool created = manager->IsCreated();
    
    napi_value result;
    napi_get_boolean(env, created, &result);
    
    return result;
}

} // namespace NativeXComponentSample
