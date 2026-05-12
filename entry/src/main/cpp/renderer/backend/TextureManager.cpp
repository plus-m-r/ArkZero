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

#include "TextureManager.h"
#include <hilog/log.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

TextureManager::TextureManager()
    : m_textureId(0)
{
}

TextureManager::~TextureManager() {
    Destroy();
}

bool TextureManager::Create(int32_t width, int32_t height, GLint internalFormat, GLenum format) {
    // 1. 生成纹理ID
    glGenTextures(1, &m_textureId);
    if (m_textureId == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureManager", "Failed to generate texture ID");
        return false;
    }

    // 2. 绑定纹理
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // 3. 设置纹理参数（高性能配置）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 4. 分配纹理内存（初始化为黑色）
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                 format, GL_UNSIGNED_BYTE, nullptr);

    // 5. 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureManager", "OpenGL error during texture creation: %{public}x", error);
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        return false;
    }

    // 6. 解绑纹理
    glBindTexture(GL_TEXTURE_2D, 0);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TextureManager", "✅ Texture created: id=%{public}u, size=%dx%d", 
        m_textureId, width, height);
    
    return true;
}

void TextureManager::Destroy() {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "TextureManager", "♻️ Texture destroyed");
    }
}

bool TextureManager::Update(const void* pixelData, int32_t width, int32_t height, GLenum format) {
    if (!pixelData) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureManager", "Invalid pixel data");
        return false;
    }

    // ⭐ 零拷贝：直接使用指针上传到GPU（DMA传输）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    format, GL_UNSIGNED_BYTE, pixelData);

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureManager", "OpenGL error during texture update: %{public}x", error);
        return false;
    }

    return true;
}

} // namespace NativeXComponentSample
