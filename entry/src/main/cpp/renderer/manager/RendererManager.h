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

#ifndef NATIVE_XCOMPONENT_RENDERER_MANAGER_H
#define NATIVE_XCOMPONENT_RENDERER_MANAGER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "../core/Renderer.h"

namespace NativeXComponentSample {

/**
 * Renderer管理器
 * 
 * 职责：
 * 1. 管理所有Renderer实例的生命周期
 * 2. 通过handle映射到具体的C++对象
 * 3. 线程安全访问
 */
class RendererManager {
public:
    /**
     * 获取单例实例
     * @return RendererManager引用
     */
    static RendererManager& GetInstance();

    /**
     * 创建新的Renderer实例
     * @param nativeWindow NativeWindow 指针
     * @param width 宽度
     * @param height 高度
     * @param format 像素格式
     * @return handle（用于后续操作），失败返回-1
     */
    int32_t CreateRenderer(void* nativeWindow, int32_t width, int32_t height, PixelFormat format);

    /**
     * 获取Renderer实例
     * @param handle 句柄
     * @return 指针，如果不存在返回nullptr
     */
    Renderer* GetRenderer(int32_t handle);

    /**
     * 销毁Renderer实例
     * @param handle 句柄
     * @return true表示成功，false表示失败
     */
    bool DestroyRenderer(int32_t handle);

private:
    RendererManager();
    ~RendererManager();

    // 禁止拷贝
    RendererManager(const RendererManager&) = delete;
    RendererManager& operator=(const RendererManager&) = delete;

private:
    std::unordered_map<int32_t, std::unique_ptr<Renderer>> m_renderers;
    std::mutex m_mutex;
    int32_t m_nextHandle;
};

} // namespace NativeXComponentSample

#endif // NATIVE_XCOMPONENT_RENDERER_MANAGER_H
