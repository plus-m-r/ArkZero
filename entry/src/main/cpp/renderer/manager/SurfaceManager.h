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

#ifndef SURFACE_MANAGER_H
#define SURFACE_MANAGER_H

#include <string>
#include <mutex>
#include <unordered_map>

namespace NativeXComponentSample {

/**
 * SurfaceManager - XComponent Surface 管理器
 * 
 * 🎯 职责：
 * - 通过 surfaceId 创建和管理 OHNativeWindow
 * - 提供线程安全的 Surface 访问
 * - 封装 HarmonyOS NDK 的 OH_NativeWindow API
 * 
 * 📊 设计原则：
 * - NativeWindow 的创建逻辑完全隐藏在 C++ 层
 * - ArkTS 层只传递 surfaceId（字符串）
 * - 符合单一职责原则（SRP）
 * 
 * 🔧 核心 API：
 * ```cpp
 * // 通过 surfaceId 直接创建 NativeWindow（HarmonyOS NDK API）
 * OH_NativeWindow_CreateNativeWindowFromSurfaceId(surfaceId, &nativeWindow)
 * ```
 */
class SurfaceManager {
public:
    /**
     * 获取单例实例
     */
    static SurfaceManager& GetInstance();
    
    /**
     * 通过 surfaceId 创建并获取 NativeWindow 指针
     * 
     * ⭐ 内部调用 OH_NativeWindow_CreateNativeWindowFromSurfaceId()
     * 
     * @param surfaceId XComponent 的 surface ID（字符串）
     * @return NativeWindow 指针，如果创建失败返回 nullptr
     */
    void* CreateNativeWindow(const std::string& surfaceId);
    
    /**
     * 销毁 NativeWindow
     * 
     * ⚠️ 必须与 CreateNativeWindow 配对使用，避免内存泄漏
     * 
     * @param nativeWindow NativeWindow 指针
     */
    void DestroyNativeWindow(void* nativeWindow);

private:
    SurfaceManager() = default;
    ~SurfaceManager() = default;
    
    // 禁止拷贝
    SurfaceManager(const SurfaceManager&) = delete;
    SurfaceManager& operator=(const SurfaceManager&) = delete;
};

} // namespace NativeXComponentSample

#endif // SURFACE_MANAGER_H
