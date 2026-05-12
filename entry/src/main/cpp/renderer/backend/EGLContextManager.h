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

#ifndef EGL_CONTEXT_MANAGER_H
#define EGL_CONTEXT_MANAGER_H

#include <EGL/egl.h>
#include <cstdint>

namespace NativeXComponentSample {

/**
 * EGL 上下文管理器
 * 
 * 职责：
 * - 初始化/释放 EGL Display、Context、Surface
 * - 管理 EGL 配置和属性
 * - 提供 VSync 控制
 * 
 * 设计原则：单一职责 - 仅负责 EGL 生命周期管理
 */
class EGLContextManager {
public:
    EGLContextManager();
    ~EGLContextManager();

    /**
     * 使用 NativeWindow 初始化 EGL 上下文（XComponent Surface 模式）
     * @param nativeWindow NativeWindow 指针
     * @param width 宽度
     * @param height 高度
     * @param enableVSync 是否启用 VSync（推荐 true，消除画面撕裂）
     * @return true 成功，false 失败
     */
    bool Initialize(void* nativeWindow, int32_t width, int32_t height, bool enableVSync = true);

    /**
     * 释放 EGL 资源
     */
    void Destroy();

    /**
     * 使 EGL 上下文当前化（渲染前必须调用）
     * @return true 成功，false 失败
     */
    bool MakeCurrent();

    /**
     * 交换缓冲区（渲染后调用，触发 VSync）
     * @return true 成功，false 失败
     */
    bool SwapBuffers();

    /**
     * 获取 EGL Display
     */
    EGLDisplay GetDisplay() const { return m_eglDisplay; }

    /**
     * 获取 EGL Context
     */
    EGLContext GetContext() const { return m_eglContext; }

    /**
     * 获取 EGL Surface
     */
    EGLSurface GetSurface() const { return m_eglSurface; }

    /**
     * 检查是否已初始化
     */
    bool IsInitialized() const { return m_eglDisplay != EGL_NO_DISPLAY; }

private:
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    EGLSurface m_eglSurface;
};

} // namespace NativeXComponentSample

#endif // EGL_CONTEXT_MANAGER_H
