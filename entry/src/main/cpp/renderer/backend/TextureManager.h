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

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <GLES3/gl3.h>
#include <cstdint>
#include <cstddef>

namespace NativeXComponentSample {

/**
 * OpenGL 纹理管理器
 * 
 * 职责：
 * - 创建/销毁 OpenGL 纹理
 * - 更新纹理数据（零拷贝上传到 GPU）
 * - 管理纹理参数和状态
 * 
 * 设计原则：单一职责 - 仅负责纹理生命周期管理
 */
class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    /**
     * 创建纹理
     * @param width 宽度
     * @param height 高度
     * @param internalFormat GL 内部格式（GL_RGBA, GL_RGB 等）
     * @param format GL 像素格式（GL_RGBA, GL_RGB, GL_BGRA_EXT 等）
     * @return true 成功，false 失败
     */
    bool Create(int32_t width, int32_t height, GLint internalFormat, GLenum format);

    /**
     * 销毁纹理
     */
    void Destroy();

    /**
     * 更新纹理数据（零拷贝：直接使用指针上传到 GPU）
     * @param pixelData 像素数据指针
     * @param width 宽度
     * @param height 高度
     * @param format GL 像素格式
     * @return true 成功，false 失败
     */
    bool Update(const void* pixelData, int32_t width, int32_t height, GLenum format);

    /**
     * 获取纹理 ID
     */
    GLuint GetTextureId() const { return m_textureId; }

    /**
     * 检查是否已创建
     */
    bool IsCreated() const { return m_textureId != 0; }

private:
    GLuint m_textureId;
};

} // namespace NativeXComponentSample

#endif // TEXTURE_MANAGER_H
