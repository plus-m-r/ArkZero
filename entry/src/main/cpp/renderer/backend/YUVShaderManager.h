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

#ifndef YUV_SHADER_MANAGER_H
#define YUV_SHADER_MANAGER_H

#include <GLES3/gl3.h>
#include <cstdint>

namespace NativeXComponentSample {

/**
 * YUV Shader 管理器
 * 
 * 🎯 职责：
 * - 编译和管理 YUV→RGB 转换的 Shader 程序
 * - 管理 Y 和 UV 双纹理
 * - 提供零 CPU 开销的 NV21/NV12 渲染能力
 * 
 * 设计原则：单一职责 - 仅负责 YUV Shader 渲染
 */
class YUVShaderManager {
public:
    YUVShaderManager();
    ~YUVShaderManager();

    /**
     * 初始化 Shader 程序
     * @return true 成功，false 失败
     */
    bool Initialize();

    /**
     * 销毁资源
     */
    void Destroy();

    /**
     * 渲染 NV21 数据（GPU Shader 转换）
     * 
     * ⭐ 零 CPU 开销：YUV→RGB 完全在 GPU 完成
     * ⭐ 双纹理：Y 平面 + UV 平面分别上传
     * 
     * @param yPlane Y 分量数据指针
     * @param uvPlane UV 分量数据指针
     * @param width 宽度
     * @param height 高度
     * @return true 成功，false 失败
     */
    bool RenderNV21(const uint8_t* yPlane, const uint8_t* uvPlane, 
                    int32_t width, int32_t height);

    /**
     * 渲染 NV12 数据（GPU Shader 转换）
     * 
     * @param yPlane Y 分量数据指针
     * @param uvPlane UV 分量数据指针（注意：NV12 是 UV 顺序，NV21 是 VU 顺序）
     * @param width 宽度
     * @param height 高度
     * @return true 成功，false 失败
     */
    bool RenderNV12(const uint8_t* yPlane, const uint8_t* uvPlane, 
                    int32_t width, int32_t height);

    /**
     * 检查是否已初始化
     */
    bool IsInitialized() const { return m_initialized; }

private:
    /**
     * 编译 Shader 程序
     */
    bool CompileShaders();

    /**
     * 创建和配置纹理
     */
    bool CreateTextures(int32_t width, int32_t height);

    /**
     * 设置顶点缓冲
     */
    void SetupVertexBuffer();

    GLuint m_shaderProgram;
    GLuint m_yTexture;
    GLuint m_uvTexture;
    GLuint m_vao;
    GLuint m_vbo;
    
    int32_t m_width;
    int32_t m_height;
    bool m_initialized;
};

} // namespace NativeXComponentSample

#endif // YUV_SHADER_MANAGER_H
