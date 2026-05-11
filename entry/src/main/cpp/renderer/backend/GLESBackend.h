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
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace NativeXComponentSample {

/**
 * OpenGL ES 渲染后端
 * 
 * 🎯 职责：
 * - 只负责 OpenGL ES 相关的初始化和渲染
 * - 独立的 EGL 上下文管理
 * - 可独立测试和复用
 */
class GLESBackend : public IRenderBackend {
public:
    GLESBackend();
    ~GLESBackend() override;

    bool Initialize(int32_t width, int32_t height, PixelFormat format) override;
    bool RenderFrame(const void* pixelData, size_t dataSize, 
                    int32_t width, int32_t height) override;
    uint64_t GetTextureId() const override;
    bool Resize(int32_t width, int32_t height) override;
    void Destroy() override;
    const char* GetBackendName() const override { return "OpenGL ES"; }
    bool IsInitialized() const override { return m_isInitialized; }

private:
    bool InitEGLContext();
    void ReleaseEGLContext();
    bool CreateTexture();
    void DestroyTexture();
    GLint GetGLInternalFormat() const;
    GLenum GetGLFormat() const;
    int GetBytesPerPixel() const;

    int32_t m_width;
    int32_t m_height;
    PixelFormat m_format;
    uint32_t m_textureId;
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    EGLSurface m_eglSurface;
    bool m_isInitialized;
};

} // namespace NativeXComponentSample

#endif // GLES_BACKEND_H
