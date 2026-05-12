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
 * - 管理 surfaceId 到 NativeWindow 的映射
 * - 提供线程安全的 Surface 访问
 * 
 * 📊 设计原则：
 * - NativeWindow 的获取逻辑完全隐藏在 C++ 层
 * - NAPI 层只传递 surfaceId（字符串）和 NativeWindow 指针（number）
 * - 符合单一职责原则（SRP）
 * 
 * 🔧 使用示例：
 * ```cpp
 * // ArkTS 层在 XComponent onLoad 时调用 registerSurface
 * // C++ 层存储映射关系
 * 
 * // 渲染器初始化时获取 NativeWindow
 * void* window = SurfaceManager::GetInstance().GetNativeWindow("surface_123");
 * if (window) {
 *     glesBackend.InitializeWithSurface(window, width, height, format);
 * }
 * ```
 */
class SurfaceManager {
public:
    /**
     * 获取单例实例
     */
    static SurfaceManager& GetInstance();
    
    /**
     * 注册 Surface
     * @param surfaceId XComponent 的 surface ID
     * @param nativeWindow NativeWindow 指针
     */
    void RegisterSurface(const std::string& surfaceId, void* nativeWindow);
    
    /**
     * 注销 Surface
     * @param surfaceId XComponent 的 surface ID
     */
    void UnregisterSurface(const std::string& surfaceId);
    
    /**
     * 获取 NativeWindow 指针
     * @param surfaceId XComponent 的 surface ID
     * @return NativeWindow 指针，如果未找到返回 nullptr
     */
    void* GetNativeWindow(const std::string& surfaceId);

private:
    SurfaceManager() = default;
    ~SurfaceManager() = default;
    
    // 禁止拷贝
    SurfaceManager(const SurfaceManager&) = delete;
    SurfaceManager& operator=(const SurfaceManager&) = delete;
    
private:
    std::mutex m_mutex;
    std::unordered_map<std::string, void*> m_surfaceMap; // surfaceId -> NativeWindow
};

} // namespace NativeXComponentSample

#endif // SURFACE_MANAGER_H
