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

#ifndef TEXTURE_SHADER_H
#define TEXTURE_SHADER_H

#include <GLES3/gl3.h>
#include <cstdint>

namespace NativeXComponentSample {

/**
 * RGBA/RGB 纹理 Shader 渲染器
 *
 * 🎯 职责：
 * - 编译和管理单纹理全屏渲染的 Shader 程序
 * - 管理 VAO/VBO（全屏四边形，STATIC_DRAW 一次性设置）
 * - 提供将 RGBA/RGB 纹理绘制到 framebuffer 的能力
 *
 * 设计原则：单一职责 - 仅负责单纹理全屏四边形绘制
 *
 * ⭐ 与 YUVShaderManager 对称设计：
 *   YUVShaderManager → 双纹理（Y+UV）+ YUV→RGB 转换 Shader
 *   TextureShader    → 单纹理（RGBA）+ 直接采样 Shader
 */
class TextureShader {
public:
    TextureShader();
    ~TextureShader();

    /**
     * 初始化 Shader 程序和 VAO/VBO
     * @return true 成功，false 失败
     */
    bool Initialize();

    /**
     * 销毁所有 GL 资源
     */
    void Destroy();

    /**
     * 将指定纹理渲染为全屏四边形
     *
     * @param textureId 要渲染的 OpenGL 纹理 ID
     * @param width 视口宽度
     * @param height 视口高度
     * @return true 成功，false 失败
     */
    bool Draw(GLuint textureId, int32_t width, int32_t height);

    /**
     * 检查是否已初始化
     */
    bool IsInitialized() const { return m_isInitialized; }

private:
    /**
     * 编译并链接着色器程序
     */
    bool CompileShaders();

    /**
     * 设置全屏四边形 VAO/VBO（STATIC_DRAW，只需设置一次）
     */
    void SetupVertexBuffer();

    /** Shader 程序句柄 */
    GLuint m_program;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;

    /** 全屏四边形 VAO/VBO */
    GLuint m_vao;
    GLuint m_vbo;

    /** Attrib/Uniform 位置缓存（避免每帧 glGet* 调用）
     */
    GLint m_positionLoc;
    GLint m_texCoordLoc;
    GLint m_textureLoc;

    bool m_isInitialized;
};

} // namespace NativeXComponentSample

#endif // TEXTURE_SHADER_H
