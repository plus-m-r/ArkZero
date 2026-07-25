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

#include "TextureShader.h"
#include <hilog/log.h>
#include <cstring>
#include "../../common/common.h"

namespace NativeXComponentSample {

namespace {
    // 顶点着色器：全屏四边形（OpenGL ES 3.0）
    const char* VERTEX_SHADER = R"(
        #version 300 es
        layout(location = 0) in vec4 aPosition;
        layout(location = 1) in vec2 aTexCoord;
        
        out vec2 vTexCoord;
        
        void main() {
            gl_Position = aPosition;
            vTexCoord = aTexCoord;
        }
    )";

    // 片段着色器：纹理采样（OpenGL ES 3.0）
    const char* FRAGMENT_SHADER = R"(
        #version 300 es
        precision mediump float;
        
        in vec2 vTexCoord;
        uniform sampler2D uTexture;
        
        out vec4 fragColor;
        
        void main() {
            fragColor = texture(uTexture, vTexCoord);
        }
    )";
}

TextureShader::TextureShader()
    : m_isInitialized(false),
      m_program(0),
      m_vertexShader(0),
      m_fragmentShader(0),
      m_vao(0),
      m_vbo(0),
      m_positionLoc(-1),
      m_texCoordLoc(-1),
      m_textureLoc(-1)
{
}

TextureShader::~TextureShader() {
    Destroy();
}

bool TextureShader::Initialize() {
    if (m_isInitialized) {
        return true;
    }

    // 1. 编译顶点着色器
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &VERTEX_SHADER, nullptr);
    glCompileShader(m_vertexShader);

    GLint compiled = 0;
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(m_vertexShader, 512, nullptr, infoLog);
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureShader", "Vertex shader compilation failed: %s", infoLog);
        return false;
    }

    // 2. 编译片段着色器
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &FRAGMENT_SHADER, nullptr);
    glCompileShader(m_fragmentShader);

    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(m_fragmentShader, 512, nullptr, infoLog);
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureShader", "Fragment shader compilation failed: %s", infoLog);
        glDeleteShader(m_vertexShader);
        return false;
    }

    // 3. 链接 Shader 程序
    m_program = glCreateProgram();
    glAttachShader(m_program, m_vertexShader);
    glAttachShader(m_program, m_fragmentShader);
    glLinkProgram(m_program);

    GLint linked = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureShader", "Shader program linking failed: %s", infoLog);
        glDeleteShader(m_vertexShader);
        glDeleteShader(m_fragmentShader);
        return false;
    }

    // 4. 清理临时 Shader
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);

    // 获取Uniform位置（着色器中使用location
    m_textureLoc = glGetUniformLocation(m_program, "uTexture");

    // 5. 设置 VAO/VBO
    SetupVertexBuffer();

    m_isInitialized = true;

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TextureShader", "✅ Initialized with OpenGL ES 3.0");

    return true;
}

void TextureShader::Destroy() {
    if (!m_isInitialized) {
        return;
    }

    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }

    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }

    m_isInitialized = false;

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TextureShader", "♻️ Destroyed");
}

bool TextureShader::Draw(GLuint textureId, int32_t width, int32_t height) {
    if (!m_isInitialized || !m_program) {
        return false;
    }

    if (textureId == 0) {
        return false;
    }

    glUseProgram(m_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(m_textureLoc, 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return true;
}

void TextureShader::SetupVertexBuffer() {
    // 全屏四边形顶点数据（NDC 坐标）
    float vertices[] = {
        // 位置         // 纹理坐标
        -1.0f, -1.0f,   0.0f, 1.0f,  // 左下
         1.0f, -1.0f,   1.0f, 1.0f,  // 右下
        -1.0f,  1.0f,   0.0f, 0.0f,  // 左上
         1.0f,  1.0f,   1.0f, 0.0f   // 右上
    };

    // 创建 VAO/VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 与着色器中location=0对应
    GLsizei stride = 4 * sizeof(float);
    
    // 位置属性（location = 0）
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性（location = 1）
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TextureShader", "✅ VAO/VBO setup complete");
}

} // namespace NativeXComponentSample
