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
    , m_isInitialized(false)
    , m_enableTexturePool(true)  // ⭐ 默认启用纹理池
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
        "GLESBackend", "Initializing: %dx%d, format=%d", width, height, static_cast<int>(format));
    
    // ⭐ 1. 初始化 EGL 上下文（使用 XComponent Surface）
    if (!m_eglManager.Initialize(nativeWindow, width, height, true)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize EGL context");
        return false;
    }

    // ⭐ 2. 根据格式选择渲染路径
    if (IsYUVFormat(format)) {
        // YUV 格式：使用 GPU Shader 渲染
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Using YUV Shader for format=%d", static_cast<int>(format));
        
        if (!m_yuvShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize YUV shader");
            m_eglManager.Destroy();
            return false;
        }
    } else {
        // RGBA/RGB 格式：使用传统纹理渲染
        
        // ⭐ 如果启用纹理池，创建并预分配
        if (m_enableTexturePool) {
            m_texturePool = std::make_unique<TexturePool>(10);  // 最多 10 个纹理
            
            // 预分配常用分辨率
            std::vector<std::pair<int32_t, int32_t>> resolutions = {
                {1920, 1080},   // 1080p
                {3840, 2160},   // 4K
                {1280, 720},    // 720p
            };
            
            GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
            GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
            
            m_texturePool->Preallocate(resolutions, internalFormat, glFormat);
            
            // 从池中获取当前尺寸的纹理
            TextureManager* texture = m_texturePool->Acquire(width, height, internalFormat, glFormat);
            if (!texture) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "GLESBackend", "Failed to acquire texture from pool");
                m_eglManager.Destroy();
                return false;
            }
            
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "✅ Using texture pool (hitRate=%.1f%%)", 
                m_texturePool->GetStats().GetHitRate() * 100);
        } else {
            // 不使用纹理池，直接创建
            GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
            GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
            
            if (!m_textureManager.Create(width, height, internalFormat, glFormat)) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "GLESBackend", "Failed to create texture");
                m_eglManager.Destroy();
                return false;
            }
        }
    }

    m_isInitialized = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "✅ Initialized");
    
    return true;
}

bool GLESBackend::InitializeOffscreen(int32_t width, int32_t height, PixelFormat format) {
    m_width = width;
    m_height = height;
    m_format = format;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Initializing offscreen: %dx%d, format=%d", width, height, static_cast<int>(format));
    
    // ⭐ 1. 初始化离屏 EGL 上下文
    if (!m_eglManager.InitializeOffscreen(width, height)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize offscreen EGL context");
        return false;
    }

    // ⭐ 2. 根据格式选择渲染路径
    if (IsYUVFormat(format)) {
        // YUV 格式：使用 GPU Shader 渲染
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Using YUV Shader for format=%d", static_cast<int>(format));
        
        if (!m_yuvShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize YUV shader");
            m_eglManager.Destroy();
            return false;
        }
    } else {
        // RGBA/RGB 格式：使用传统纹理渲染
        
        // ⭐ 如果启用纹理池，创建并预分配
        if (m_enableTexturePool) {
            m_texturePool = std::make_unique<TexturePool>(10);  // 最多 10 个纹理
            
            // 预分配常用分辨率
            std::vector<std::pair<int32_t, int32_t>> resolutions = {
                {1920, 1080},   // 1080p
                {3840, 2160},   // 4K
                {1280, 720},    // 720p
            };
            
            GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
            GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
            
            m_texturePool->Preallocate(resolutions, internalFormat, glFormat);
            
            // 从池中获取当前尺寸的纹理
            TextureManager* texture = m_texturePool->Acquire(width, height, internalFormat, glFormat);
            if (!texture) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "GLESBackend", "Failed to acquire texture from pool");
                m_eglManager.Destroy();
                return false;
            }
            
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "✅ Using texture pool (hitRate=%.1f%%)", 
                m_texturePool->GetStats().GetHitRate() * 100);
        } else {
            // 不使用纹理池，直接创建
            GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
            GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
            
            if (!m_textureManager.Create(width, height, internalFormat, glFormat)) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "GLESBackend", "Failed to create texture");
                m_eglManager.Destroy();
                return false;
            }
        }
    }

    m_isInitialized = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "✅ Offscreen initialized");
    
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
    int bytesPerPixel = PixelFormatConverter::GetBytesPerPixel(m_format);
    size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * bytesPerPixel;
    if (dataSize < expectedSize) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", 
            "Data size mismatch: expected=%zu, actual=%zu", 
            expectedSize, dataSize);
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
        "GLESBackend", 
        "🎨 Rendering frame: %dx%d, format=%d", 
        width, height, static_cast<int>(m_format));

    // ⭐ 1. 使 EGL 上下文当前化
    if (!m_eglManager.MakeCurrent()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to make EGL context current");
        return false;
    }

    // ⭐ 2. 根据格式选择渲染路径
    bool success = false;
    if (IsYUVFormat(m_format)) {
        // YUV 格式：使用 GPU Shader 渲染（零 CPU 开销）
        const uint8_t* data = static_cast<const uint8_t*>(pixelData);
        const uint8_t* yPlane = data;
        const uint8_t* uvPlane = data + width * height;
        
        if (m_format == PixelFormat::NV21) {
            success = m_yuvShader.RenderNV21(yPlane, uvPlane, width, height);
        } else if (m_format == PixelFormat::NV12) {
            success = m_yuvShader.RenderNV12(yPlane, uvPlane, width, height);
        }
    } else {
        // RGBA/RGB 格式：使用传统纹理渲染
        GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
        
        // ⭐ 如果启用纹理池，从池中获取纹理
        if (m_enableTexturePool && m_texturePool) {
            GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
            TextureManager* texture = m_texturePool->Acquire(width, height, internalFormat, glFormat);
            
            if (texture) {
                success = texture->Update(pixelData, width, height, glFormat);
            } else {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "GLESBackend", "Failed to acquire texture from pool");
                success = false;
            }
        } else {
            // 不使用纹理池
            success = m_textureManager.Update(pixelData, width, height, glFormat);
        }
    }
    
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to render frame");
        return false;
    }

    // ⭐ 3. 交换缓冲区（触发 VSync）
    if (!m_eglManager.SwapBuffers()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to swap buffers");
        return false;
    }

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
            "GLESBackend", "Invalid size: %dx%d", width, height);
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", 
        "📐 Resizing: %dx%d -> %dx%d", 
        m_width, m_height, width, height);

    // ⭐ 如果启用纹理池，从池中获取新尺寸的纹理
    if (m_enableTexturePool && m_texturePool && !IsYUVFormat(m_format)) {
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
        GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
        
        TextureManager* texture = m_texturePool->Acquire(width, height, internalFormat, glFormat);
        
        if (texture) {
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "✅ Resized using texture pool (hitRate=%.1f%%)", 
                m_texturePool->GetStats().GetHitRate() * 100);
            
            m_width = width;
            m_height = height;
            return true;
        } else {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to acquire texture from pool");
            return false;
        }
    }
    
    // ⭐ 不使用纹理池或 YUV 格式，直接创建新纹理
    if (IsYUVFormat(m_format)) {
        // YUV Shader 不需要 Resize，Shader 会自动适配
        m_width = width;
        m_height = height;
        return true;
    } else {
        // 传统纹理：销毁旧纹理，创建新纹理
        m_textureManager.Destroy();
        
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
        GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
        
        if (!m_textureManager.Create(width, height, internalFormat, glFormat)) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to create new texture");
            return false;
        }
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
        "GLESBackend", "Destroying...");

    // ⭐ 1. 销毁纹理池或 YUV Shader 或传统纹理
    if (IsYUVFormat(m_format)) {
        m_yuvShader.Destroy();
    } else if (m_enableTexturePool && m_texturePool) {
        m_texturePool->Clear();
        m_texturePool.reset();
    } else {
        m_textureManager.Destroy();
    }

    // ⭐ 2. 释放 EGL 上下文
    m_eglManager.Destroy();

    m_isInitialized = false;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "♻️ Destroyed");
}

bool GLESBackend::IsYUVFormat(PixelFormat format) const {
    return format == PixelFormat::NV21 || format == PixelFormat::NV12;
}

} // namespace NativeXComponentSample
