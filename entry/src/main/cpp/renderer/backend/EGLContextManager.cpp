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

#include "EGLContextManager.h"
#include <hilog/log.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

EGLContextManager::EGLContextManager()
    : m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglContext(EGL_NO_CONTEXT)
    , m_eglSurface(EGL_NO_SURFACE)
{
}

EGLContextManager::~EGLContextManager() {
    Destroy();
}

bool EGLContextManager::Initialize(void* nativeWindow, int32_t width, int32_t height, bool enableVSync) {
    if (!nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Invalid nativeWindow");
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "Initializing EGL context: %dx%d, VSync=%s", 
        width, height, enableVSync ? "enabled" : "disabled");

    // 1. 获取默认显示
    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to get EGL display: %{public}x", eglGetError());
        return false;
    }

    // 2. 初始化EGL
    EGLint major, minor;
    if (!eglInitialize(m_eglDisplay, &major, &minor)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to initialize EGL: %{public}x", eglGetError());
        m_eglDisplay = EGL_NO_DISPLAY;
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "EGL initialized: version=%{public}d.%{public}d", major, minor);

    // 3. 配置EGL（使用 WINDOW_BIT）
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, EGL_BLUE_SIZE_DEFAULT,
        EGL_GREEN_SIZE, EGL_GREEN_SIZE_DEFAULT,
        EGL_RED_SIZE, EGL_RED_SIZE_DEFAULT,
        EGL_ALPHA_SIZE, EGL_ALPHA_SIZE_DEFAULT,
        EGL_DEPTH_SIZE, EGL_DEPTH_SIZE_DEFAULT,
        EGL_STENCIL_SIZE, EGL_STENCIL_SIZE_DEFAULT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(m_eglDisplay, configAttribs, &config, 1, &numConfigs)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to choose EGL config: %{public}x", eglGetError());
        return false;
    }

    // 4. 创建 Window Surface（使用 NativeWindow）
    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, config, 
                                          (EGLNativeWindowType)nativeWindow, nullptr);
    if (m_eglSurface == EGL_NO_SURFACE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to create EGL window surface: %{public}x", eglGetError());
        return false;
    }

    // 5. 创建EGL上下文
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, OPENGL_ES_VERSION,
        EGL_NONE
    };

    m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (m_eglContext == EGL_NO_CONTEXT) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to create EGL context: %{public}x", eglGetError());
        return false;
    }

    // 6. 使上下文当前化
    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to make context current: %{public}x", eglGetError());
        return false;
    }

    // 7. 启用 VSync（关键！消除画面撕裂）
    if (enableVSync) {
        eglSwapInterval(m_eglDisplay, 1);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "✅ VSync enabled");
    } else {
        eglSwapInterval(m_eglDisplay, 0);
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "⚠️ VSync disabled (may cause tearing)");
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "✅ EGL context initialized successfully");
    
    return true;
}

bool EGLContextManager::InitializeOffscreen(int32_t width, int32_t height) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "Initializing offscreen EGL context: %dx%d", width, height);

    // 1. 获取默认显示
    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to get EGL display: %{public}x", eglGetError());
        return false;
    }

    // 2. 初始化EGL
    EGLint major, minor;
    if (!eglInitialize(m_eglDisplay, &major, &minor)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to initialize EGL: %{public}x", eglGetError());
        m_eglDisplay = EGL_NO_DISPLAY;
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "EGL initialized: version=%{public}d.%{public}d", major, minor);

    // 3. 配置EGL（使用 PBUFFER_BIT）
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,  // ⭐ 离屏表面
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(m_eglDisplay, configAttribs, &config, 1, &numConfigs)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to choose EGL config: %{public}x", eglGetError());
        return false;
    }

    // 4. 创建 Pbuffer Surface（离屏）
    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };

    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, config, pbufferAttribs);
    if (m_eglSurface == EGL_NO_SURFACE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to create EGL pbuffer surface: %{public}x", eglGetError());
        return false;
    }

    // 5. 创建EGL上下文
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, OPENGL_ES_VERSION,
        EGL_NONE
    };

    m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (m_eglContext == EGL_NO_CONTEXT) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to create EGL context: %{public}x", eglGetError());
        return false;
    }

    // 6. 使上下文当前化
    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to make context current: %{public}x", eglGetError());
        return false;
    }

    // 7. 禁用 VSync（离屏不需要）
    eglSwapInterval(m_eglDisplay, 0);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "✅ Offscreen EGL context initialized successfully");
    
    return true;
}

void EGLContextManager::Destroy() {
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "Destroying EGL context...");

    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (m_eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(m_eglDisplay, m_eglContext);
        m_eglContext = EGL_NO_CONTEXT;
    }

    if (m_eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(m_eglDisplay, m_eglSurface);
        m_eglSurface = EGL_NO_SURFACE;
    }

    eglTerminate(m_eglDisplay);
    m_eglDisplay = EGL_NO_DISPLAY;

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "EGLContextManager", "♻️ EGL context destroyed");
}

bool EGLContextManager::MakeCurrent() {
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "EGL not initialized");
        return false;
    }

    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to make context current: %{public}x", eglGetError());
        return false;
    }

    return true;
}

bool EGLContextManager::SwapBuffers() {
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "EGL not initialized");
        return false;
    }

    if (!eglSwapBuffers(m_eglDisplay, m_eglSurface)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "EGLContextManager", "Failed to swap buffers: %{public}x", eglGetError());
        return false;
    }

    return true;
}

} // namespace NativeXComponentSample
