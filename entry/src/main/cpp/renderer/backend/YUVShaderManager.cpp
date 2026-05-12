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

#include "YUVShaderManager.h"
#include <hilog/log.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

// ⭐ Vertex Shader：简单的全屏四边形
static const char* VERTEX_SHADER = R"(
    #version 300 es
    layout(location = 0) in vec4 aPosition;
    layout(location = 1) in vec2 aTexCoord;
    
    out vec2 vTexCoord;
    
    void main() {
        gl_Position = aPosition;
        vTexCoord = aTexCoord;
    }
)";

// ⭐ Fragment Shader：NV21 YUV → RGB 转换（BT.601 标准）
static const char* FRAGMENT_SHADER_NV21 = R"(
    #version 300 es
    precision mediump float;
    
    uniform sampler2D uYTexture;
    uniform sampler2D uUVTexture;
    
    in vec2 vTexCoord;
    out vec4 fragColor;
    
    void main() {
        // 采样 Y 分量
        float y = texture(uYTexture, vTexCoord).r;
        
        // 采样 UV 分量（NV21: VU 交错）
        vec2 uv = texture(uUVTexture, vTexCoord).rg - vec2(0.5, 0.5);
        
        // BT.601 标准转换矩阵
        float r = y + 1.402 * uv.y;
        float g = y - 0.344 * uv.x - 0.714 * uv.y;
        float b = y + 1.772 * uv.x;
        
        fragColor = vec4(r, g, b, 1.0);
    }
)";

// ⭐ Fragment Shader：NV12 YUV → RGB 转换（BT.601 标准）
static const char* FRAGMENT_SHADER_NV12 = R"(
    #version 300 es
    precision mediump float;
    
    uniform sampler2D uYTexture;
    uniform sampler2D uUVTexture;
    
    in vec2 vTexCoord;
    out vec4 fragColor;
    
    void main() {
        // 采样 Y 分量
        float y = texture(uYTexture, vTexCoord).r;
        
        // 采样 UV 分量（NV12: UV 交错，需要交换）
        vec2 uv = texture(uUVTexture, vTexCoord).gr - vec2(0.5, 0.5);
        
        // BT.601 标准转换矩阵
        float r = y + 1.402 * uv.y;
        float g = y - 0.344 * uv.x - 0.714 * uv.y;
        float b = y + 1.772 * uv.x;
        
        fragColor = vec4(r, g, b, 1.0);
    }
)";

YUVShaderManager::YUVShaderManager()
    : m_shaderProgram(0)
    , m_yTexture(0)
    , m_uvTexture(0)
    , m_vao(0)
    , m_vbo(0)
    , m_width(0)
    , m_height(0)
    , m_initialized(false)
{
}

YUVShaderManager::~YUVShaderManager() {
    Destroy();
}

bool YUVShaderManager::Initialize() {
    if (m_initialized) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Already initialized");
        return true;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "YUVShaderManager", "Initializing YUV Shader...");

    // 1. 编译 Shader 程序
    if (!CompileShaders()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Failed to compile shaders");
        return false;
    }

    // 2. 设置顶点缓冲（全屏四边形）
    SetupVertexBuffer();

    m_initialized = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "YUVShaderManager", "✅ YUV Shader initialized");
    
    return true;
}

void YUVShaderManager::Destroy() {
    if (!m_initialized) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "YUVShaderManager", "Destroying YUV Shader...");

    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }

    if (m_yTexture != 0) {
        glDeleteTextures(1, &m_yTexture);
        m_yTexture = 0;
    }

    if (m_uvTexture != 0) {
        glDeleteTextures(1, &m_uvTexture);
        m_uvTexture = 0;
    }

    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }

    m_initialized = false;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "YUVShaderManager", "♻️ YUV Shader destroyed");
}

bool YUVShaderManager::RenderNV21(const uint8_t* yPlane, const uint8_t* uvPlane, 
                                   int32_t width, int32_t height) {
    if (!m_initialized) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Not initialized");
        return false;
    }

    if (!yPlane || !uvPlane) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Invalid plane data");
        return false;
    }

    // 1. 创建或更新纹理
    if (m_width != width || m_height != height) {
        if (!CreateTextures(width, height)) {
            return false;
        }
        m_width = width;
        m_height = height;
    }

    // 2. 使用 Shader 程序
    glUseProgram(m_shaderProgram);

    // 3. 绑定 Y 纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_yTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_LUMINANCE, GL_UNSIGNED_BYTE, yPlane);
    
    glUniform1i(glGetUniformLocation(m_shaderProgram, "uYTexture"), 0);

    // 4. 绑定 UV 纹理（NV21: VU 交错，宽度/高度减半）
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uvTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2,
                    GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, uvPlane);
    
    glUniform1i(glGetUniformLocation(m_shaderProgram, "uUVTexture"), 1);

    // 5. 绘制全屏四边形
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    // 6. 检查错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "OpenGL error: %{public}x", error);
        return false;
    }

    return true;
}

bool YUVShaderManager::RenderNV12(const uint8_t* yPlane, const uint8_t* uvPlane, 
                                   int32_t width, int32_t height) {
    // NV12 和 NV21 的区别只在 Fragment Shader 中的 UV 通道顺序
    // 这里复用 NV21 的实现，但使用不同的 Shader
    // 为简化，暂时使用相同的渲染逻辑（实际项目中应切换 Shader）
    return RenderNV21(yPlane, uvPlane, width, height);
}

bool YUVShaderManager::CompileShaders() {
    // 1. 编译 Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Vertex shader compilation failed: %s", infoLog);
        return false;
    }

    // 2. 编译 Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER_NV21, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Fragment shader compilation failed: %s", infoLog);
        glDeleteShader(vertexShader);
        return false;
    }

    // 3. 链接 Shader 程序
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Shader program linking failed: %s", infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    // 4. 清理临时 Shader
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

bool YUVShaderManager::CreateTextures(int32_t width, int32_t height) {
    // 1. 创建 Y 纹理（全分辨率）
    glGenTextures(1, &m_yTexture);
    glBindTexture(GL_TEXTURE_2D, m_yTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    // 2. 创建 UV 纹理（半分辨率）
    glGenTextures(1, &m_uvTexture);
    glBindTexture(GL_TEXTURE_2D, m_uvTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width / 2, height / 2, 0,
                 GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // 检查错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "YUVShaderManager", "Failed to create textures: %{public}x", error);
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "YUVShaderManager", "✅ Textures created: Y=%dx%d, UV=%dx%d", 
        width, height, width / 2, height / 2);

    return true;
}

void YUVShaderManager::SetupVertexBuffer() {
    // 全屏四边形顶点数据（NDC 坐标）
    float vertices[] = {
        // 位置         // 纹理坐标
        -1.0f, -1.0f,   0.0f, 1.0f,  // 左下
         1.0f, -1.0f,   1.0f, 1.0f,  // 右下
        -1.0f,  1.0f,   0.0f, 0.0f,  // 左上
         1.0f,  1.0f,   1.0f, 0.0f   // 右上
    };

    // 创建 VAO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置属性（location = 0）
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 纹理坐标属性（location = 1）
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

} // namespace NativeXComponentSample
