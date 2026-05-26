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
// ⭐ 设计模式：工厂模式
#include "factory/TextureFactory.h"

namespace NativeXComponentSample {

GLESBackend::GLESBackend()
    : m_width(0)
    , m_height(0)
    , m_format(PixelFormat::RGBA)
    , m_isInitialized(false)
{
    // ⭐ 默认使用池化策略（高性能）
    m_textureStrategy = std::make_unique<PoolTextureStrategy>(10);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Using texture strategy: %s", m_textureStrategy->GetName());
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
    
    // ⭐ 保存 NativeWindow 引用，用于后续 Surface 恢复
    m_nativeWindow = nativeWindow;
    
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
        // RGBA/RGB 格式：使用策略模式管理纹理
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
        GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
        
        // ⭐ 预分配常用分辨率（仅池化策略支持）
        if (std::string(m_textureStrategy->GetName()) == "PoolStrategy") {
            m_textureStrategy->Preallocate(1920, 1080, internalFormat, glFormat);
            m_textureStrategy->Preallocate(3840, 2160, internalFormat, glFormat);
            m_textureStrategy->Preallocate(1280, 720, internalFormat, glFormat);
            
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "✅ Preallocated textures for common resolutions");
        }
        
        // ⭐ 从策略中获取当前尺寸的纹理
        TextureManager* texture = m_textureStrategy->Acquire(width, height, internalFormat, glFormat);
        if (!texture) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to acquire texture from strategy");
            m_eglManager.Destroy();
            return false;
        }
        
        // ⭐ 初始化 TextureShader（全屏四边形绘制）
        if (!m_textureShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize TextureShader");
            m_textureStrategy->Release(texture);
            m_eglManager.Destroy();
            return false;
        }
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "✅ Using texture strategy: %s", m_textureStrategy->GetName());
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
        // RGBA/RGB 格式：使用策略模式管理纹理
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
        GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
        
        // ⭐ 预分配常用分辨率（仅池化策略支持）
        if (std::string(m_textureStrategy->GetName()) == "PoolStrategy") {
            m_textureStrategy->Preallocate(1920, 1080, internalFormat, glFormat);
            m_textureStrategy->Preallocate(3840, 2160, internalFormat, glFormat);
            m_textureStrategy->Preallocate(1280, 720, internalFormat, glFormat);
            
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "✅ Preallocated textures for common resolutions");
        }
        
        // ⭐ 从策略中获取当前尺寸的纹理
        TextureManager* texture = m_textureStrategy->Acquire(width, height, internalFormat, glFormat);
        if (!texture) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to acquire texture from strategy");
            m_eglManager.Destroy();
            return false;
        }
        
        // ⭐ 初始化 TextureShader（全屏四边形绘制）
        if (!m_textureShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize TextureShader");
            m_textureStrategy->Release(texture);
            m_eglManager.Destroy();
            return false;
        }
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "✅ Using texture strategy: %s", m_textureStrategy->GetName());
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

    // ⭐ 1.5 检查并恢复失效的 Surface
    if (m_eglManager.IsSurfaceInvalidated() && m_nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "GLESBackend", "⚠️ Detecting invalidated surface, attempting recovery...");
        
        if (!m_eglManager.RecoverSurface(m_nativeWindow)) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "❌ Failed to recover surface");
            return false;
        }
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "✅ Surface recovered, continuing render");
    }

    const int32_t viewportWidth = m_eglManager.GetSurfaceWidth() > 0 ? m_eglManager.GetSurfaceWidth() : width;
    const int32_t viewportHeight = m_eglManager.GetSurfaceHeight() > 0 ? m_eglManager.GetSurfaceHeight() : height;
    glViewport(0, 0, viewportWidth, viewportHeight);

    // ⭐ 2. 直接进入渲染路径，避免 clear 暴露出未覆盖区域
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
        // RGBA/RGB 格式：使用策略模式
        GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
        
        // ⭐ 从策略中获取纹理
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
        TextureManager* texture = m_textureStrategy->Acquire(width, height, internalFormat, glFormat);
        
        if (texture) {
            // ⭐ 绑定纹理并更新像素数据
            glBindTexture(GL_TEXTURE_2D, texture->GetTextureId());

            success = texture->Update(pixelData, width, height, glFormat);
            
            if (success) {
                // ⭐ 关键修复：将纹理绘制为全屏四边形输出到 framebuffer
                OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
                    "GLESBackend", "🎨 Drawing fullscreen quad with texture #%u", 
                    texture->GetTextureId());
                success = m_textureShader.Draw(texture->GetTextureId(), viewportWidth, viewportHeight);
            }
            
            // ⭐ 渲染完成后归还纹理到池中
            m_textureStrategy->Release(texture);
        } else {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to acquire texture from strategy");
            success = false;
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

    // ⭐ YUV 格式不需要 Resize，Shader 会自动适配
    if (IsYUVFormat(m_format)) {
        m_width = width;
        m_height = height;
        return true;
    }
    
    // ⭐ 使用策略模式处理 Resize
    GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
    
    TextureManager* texture = m_textureStrategy->Acquire(width, height, internalFormat, glFormat);
    
    if (texture) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "✅ Resized using strategy: %s", m_textureStrategy->GetName());
        
        // ⭐ Resize 完成后归还纹理
        m_textureStrategy->Release(texture);
        
        m_width = width;
        m_height = height;
        return true;
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to acquire texture from strategy");
        return false;
    }
}

void GLESBackend::Destroy() {
    if (!m_isInitialized) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Destroying...");

    // ⭐ 销毁 TextureShader 或 YUV Shader
    // 注意：此时EGL上下文应该还有效（在Destroy()前调用）
    if (IsYUVFormat(m_format)) {
        m_yuvShader.Destroy();
    } else {
        if (m_textureShader.IsInitialized()) {
            m_textureShader.Destroy();
        }
        
        // ⭐ 清空纹理策略
        if (m_textureStrategy) {
            m_textureStrategy->Clear();
            m_textureStrategy.reset();
        }
    }

    m_isInitialized = false;
    m_nativeWindow = nullptr;

    // ⭐ 最后释放 EGL 上下文（此时GL对象已销毁）
    m_eglManager.Destroy();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "♻️ Destroyed");
}

bool GLESBackend::IsYUVFormat(PixelFormat format) const {
    return format == PixelFormat::NV21 || format == PixelFormat::NV12;
}

void GLESBackend::SetTextureStrategy(const char* strategyType) {
    if (!strategyType) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "GLESBackend", "Invalid strategy type");
        return;
    }
    
    std::string type(strategyType);
    
    if (type == "pool") {
        // 切换到池化策略（高性能）
        m_textureStrategy = std::make_unique<PoolTextureStrategy>(10);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "GLESBackend", "✅ Switched to PoolStrategy");
    } else if (type == "direct") {
        // 切换到直接创建策略（低内存）
        m_textureStrategy = std::make_unique<DirectTextureStrategy>();
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "GLESBackend", "✅ Switched to DirectStrategy");
    } else {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN,
            "GLESBackend", "Unknown strategy type: %s, keeping current strategy", strategyType);
    }
}

const char* GLESBackend::GetCurrentStrategyName() const {
    if (m_textureStrategy) {
        return m_textureStrategy->GetName();
    }
    return "None";
}

} // namespace NativeXComponentSample
