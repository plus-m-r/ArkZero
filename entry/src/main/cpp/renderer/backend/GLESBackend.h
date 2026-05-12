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

#ifndef GLES_BACKEND_H
#define GLES_BACKEND_H

#include "IRenderBackend.h"
#include "EGLContextManager.h"
#include "TextureManager.h"
#include "PixelFormatConverter.h"
#include "YUVShaderManager.h"
#include "TexturePool.h"
#include <cstdint>
#include <cstddef>

namespace NativeXComponentSample {

/**
 * OpenGL ES 渲染后端（Facade）
 * 
 * 🎯 职责：
 * - 协调 EGLContextManager、TextureManager、PixelFormatConverter、YUVShaderManager
 * - 实现 IRenderBackend 接口
 * - 提供高层渲染逻辑
 * - 自动检测 YUV 格式并使用 GPU Shader 渲染
 * 
 * 设计原则：单一职责 - 作为 Facade 协调各组件，不直接管理底层资源
 */
class GLESBackend : public IRenderBackend {
public:
    GLESBackend();
    ~GLESBackend() override;
    
    bool Initialize(void* nativeWindow, int32_t width, int32_t height, PixelFormat format) override;
    
    /**
     * 初始化离屏渲染后端（用于测试）
     * @param width 宽度
     * @param height 高度
     * @param format 像素格式
     * @return true 成功，false 失败
     */
    bool InitializeOffscreen(int32_t width, int32_t height, PixelFormat format);
    
    bool RenderFrame(const void* pixelData, size_t dataSize, 
                    int32_t width, int32_t height) override;
    bool Resize(int32_t width, int32_t height) override;
    void Destroy() override;
    const char* GetBackendName() const override { return "OpenGL ES"; }
    bool IsInitialized() const override { return m_isInitialized; }

private:
    /**
     * 检查是否为 YUV 格式
     */
    bool IsYUVFormat(PixelFormat format) const;

    // ⭐ 组合五个单一职责的组件
    EGLContextManager m_eglManager;      // EGL 上下文管理
    TextureManager m_textureManager;     // 纹理管理（RGBA/RGB）
    YUVShaderManager m_yuvShader;        // YUV Shader 渲染（NV21/NV12）
    std::unique_ptr<TexturePool> m_texturePool;  // ⭐ 纹理池（可选）
    
    int32_t m_width;
    int32_t m_height;
    PixelFormat m_format;
    bool m_isInitialized;
    bool m_enableTexturePool;  // ⭐ 是否启用纹理池
};

} // namespace NativeXComponentSample

#endif // GLES_BACKEND_H
