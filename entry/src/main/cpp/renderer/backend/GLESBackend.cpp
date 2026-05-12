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

#include "GLESBackend.h"
#include <hilog/log.h>
#include <cstring>

#include "../../common/common.h"

namespace NativeXComponentSample {

GLESBackend::GLESBackend()
    : m_width(0)
    , m_height(0)
    , m_format(PixelFormat::RGBA)
    , m_textureId(0)
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglContext(EGL_NO_CONTEXT)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_isInitialized(false)
{
}

GLESBackend::~GLESBackend() {
    Destroy();
}

bool GLESBackend::Initialize(void* nativeWindow, int32_t width, int32_t height, PixelFormat format) {
    if (!nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Invalid nativeWindow");
        return false;
    }
    
    m_width = width;
    m_height = height;
    m_format = format;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Initializing with XComponent Surface: %dx%d", width, height);
    
    // 初始化EGL上下文（使用 Window Surface）
    if (!InitEGLContextWithSurface(nativeWindow)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize EGL context with surface");
        return false;
    }

    // 创建OpenGL纹理
    if (!CreateTexture()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to create texture");
        ReleaseEGLContext();
        return false;
    }

    m_isInitialized = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "✅ OpenGL ES backend initialized, textureId=%{public}u", m_textureId);
    
    return true;
}

bool GLESBackend::RenderFrame(const void* pixelData, size_t dataSize, 
                               int32_t width, int32_t height) {
    if (!m_isInitialized) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Not initialized");
        return false;
    }

    if (!pixelData) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Invalid pixel data");
        return false;
    }

    // 验证数据大小
    int bytesPerPixel = GetBytesPerPixel();
    size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * bytesPerPixel;
    if (dataSize < expectedSize) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", 
            "Data size mismatch: expected=%{public}zu, actual=%{public}zu", 
            expectedSize, dataSize);
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
        "GLESBackend", 
        "🎨 Rendering frame: %{public}dx%{public}d, format=%{public}d", 
        width, height, static_cast<int>(m_format));

    // 使EGL上下文当前化
    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to make EGL context current: %{public}x", eglGetError());
        return false;
    }

    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // ⭐ 零拷贝：直接使用指针上传到GPU（DMA传输）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GetGLFormat(), GL_UNSIGNED_BYTE, pixelData);

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "OpenGL error during render: %{public}x", error);
        return false;
    }

    // 解绑纹理
    glBindTexture(GL_TEXTURE_2D, 0);

    // 刷新EGL表面
    eglSwapBuffers(m_eglDisplay, m_eglSurface);

    return true;
}

bool GLESBackend::Resize(int32_t width, int32_t height) {
    if (!m_isInitialized) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Not initialized");
        return false;
    }

    if (width <= 0 || height <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Invalid size: %{public}dx%{public}d", width, height);
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", 
        "📐 Resizing: %{public}dx%{public}d -> %{public}dx%{public}d", 
        m_width, m_height, width, height);

    // 删除旧纹理
    DestroyTexture();

    // 创建新纹理
    if (!CreateTexture()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to create new texture");
        return false;
    }

    // 更新尺寸
    m_width = width;
    m_height = height;

    return true;
}

void GLESBackend::Destroy() {
    if (!m_isInitialized) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Destroying OpenGL ES backend...");

    // 销毁OpenGL纹理
    DestroyTexture();

    // 释放EGL上下文
    ReleaseEGLContext();

    m_isInitialized = false;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "♻️ OpenGL ES backend destroyed");
}

bool GLESBackend::InitEGLContext() {
    // 获取默认显示
    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to get EGL display: %{public}x", eglGetError());
        return false;
    }

    // 初始化EGL
    EGLint major, minor;
    if (!eglInitialize(m_eglDisplay, &major, &minor)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize EGL: %{public}x", eglGetError());
        m_eglDisplay = EGL_NO_DISPLAY;
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "EGL initialized: version=%{public}d.%{public}d", major, minor);

    // 配置EGL
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
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
            "GLESBackend", "Failed to choose EGL config: %{public}x", eglGetError());
        return false;
    }

    // 创建Pbuffer表面（离屏渲染）
    const EGLint surfaceAttribs[] = {
        EGL_WIDTH, m_width,
        EGL_HEIGHT, m_height,
        EGL_NONE
    };

    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, config, surfaceAttribs);
    if (m_eglSurface == EGL_NO_SURFACE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to create EGL surface: %{public}x", eglGetError());
        return false;
    }

    // 创建EGL上下文
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, OPENGL_ES_VERSION,
        EGL_NONE
    };

    m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (m_eglContext == EGL_NO_CONTEXT) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to create EGL context: %{public}x", eglGetError());
        return false;
    }

    // 使上下文当前化
    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to make context current: %{public}x", eglGetError());
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "✅ EGL context created");
    
    return true;
}

bool GLESBackend::InitEGLContextWithSurface(void* nativeWindow) {
    // 获取默认显示
    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to get EGL display: %{public}x", eglGetError());
        return false;
    }

    // 初始化EGL
    EGLint major, minor;
    if (!eglInitialize(m_eglDisplay, &major, &minor)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize EGL: %{public}x", eglGetError());
        m_eglDisplay = EGL_NO_DISPLAY;
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "EGL initialized: version=%{public}d.%{public}d", major, minor);

    // 配置EGL（使用 WINDOW_BIT）
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,  // ⭐ Window Surface
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
            "GLESBackend", "Failed to choose EGL config: %{public}x", eglGetError());
        return false;
    }

    // ⭐ 创建 Window Surface（使用 NativeWindow）
    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, config, 
                                          (EGLNativeWindowType)nativeWindow, nullptr);
    if (m_eglSurface == EGL_NO_SURFACE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to create EGL window surface: %{public}x", eglGetError());
        return false;
    }

    // 创建EGL上下文
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, OPENGL_ES_VERSION,
        EGL_NONE
    };

    m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (m_eglContext == EGL_NO_CONTEXT) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to create EGL context: %{public}x", eglGetError());
        return false;
    }

    // 使上下文当前化
    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to make context current: %{public}x", eglGetError());
        return false;
    }
    
    // ⭐ 启用 VSync（关键！）
    eglSwapInterval(m_eglDisplay, 1);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "✅ EGL context created with XComponent Surface (VSync enabled)");
    
    return true;
}

void GLESBackend::ReleaseEGLContext() {
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        return;
    }

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
}

bool GLESBackend::CreateTexture() {
    // 生成纹理ID
    glGenTextures(1, &m_textureId);
    if (m_textureId == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to generate texture ID");
        return false;
    }

    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 分配纹理内存（初始化为黑色）
    glTexImage2D(GL_TEXTURE_2D, 0, GetGLInternalFormat(), m_width, m_height, 0,
                 GetGLFormat(), GL_UNSIGNED_BYTE, nullptr);

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "OpenGL error during texture creation: %{public}x", error);
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        return false;
    }

    // 解绑纹理
    glBindTexture(GL_TEXTURE_2D, 0);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Texture created: id=%{public}u, size=%{public}dx%{public}d", 
        m_textureId, m_width, m_height);
    
    return true;
}

void GLESBackend::DestroyTexture() {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
}

GLint GLESBackend::GetGLInternalFormat() const {
    switch (m_format) {
        case PixelFormat::RGBA:
        case PixelFormat::BGRA:
            return GL_RGBA;
        case PixelFormat::RGB:
            return GL_RGB;
        default:
            return GL_RGBA;
    }
}

GLenum GLESBackend::GetGLFormat() const {
    switch (m_format) {
        case PixelFormat::RGBA:
            return GL_RGBA;
        case PixelFormat::RGB:
            return GL_RGB;
        case PixelFormat::BGRA:
#ifdef GL_BGRA
            return GL_BGRA;
#else
            return GL_RGBA;
#endif
        default:
            return GL_RGBA;
    }
}

int GLESBackend::GetBytesPerPixel() const {
    switch (m_format) {
        case PixelFormat::RGBA:
        case PixelFormat::BGRA:
            return 4;
        case PixelFormat::RGB:
            return 3;
        default:
            return 4;
    }
}

} // namespace NativeXComponentSample
