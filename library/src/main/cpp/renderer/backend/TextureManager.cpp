#include "TextureManager.h"
#include <hilog/log.h>
#include "../../common/common.h"
#include <cstring>

namespace NativeXComponentSample {

TextureManager::TextureManager()
    : m_textureId(0)
    , m_internalFormat(GL_RGBA)
    , m_format(GL_RGBA)
    , m_width(0)
    , m_height(0)
    , m_currentPbo(0)
    , m_pboReady(false)
    , m_pboSize(0)
{
    m_pbos[0] = 0;
    m_pbos[1] = 0;
}

TextureManager::~TextureManager() {
    Destroy();
}

bool TextureManager::Create(int32_t width, int32_t height, GLint internalFormat, GLenum format) {
    glGenTextures(1, &m_textureId);
    if (m_textureId == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureManager", "Failed to generate texture ID");
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                 format, GL_UNSIGNED_BYTE, nullptr);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TextureManager", "OpenGL error during texture creation: %{public}x", error);
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    m_internalFormat = internalFormat;
    m_format = format;
    m_width = width;
    m_height = height;

    size_t pboSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    CreatePBOs(pboSize);

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TextureManager", "Texture created: id=%{public}u, size=%{public}dx%{public}d, PBO=%{public}s", 
        m_textureId, width, height, m_pboReady ? "on" : "off");
    
    return true;
}

void TextureManager::Destroy() {
    DestroyPBOs();

    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
}

void TextureManager::CreatePBOs(size_t size) {
    glGenBuffers(PBO_COUNT, m_pbos);
    if (m_pbos[0] == 0 || m_pbos[1] == 0) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN,
            "TextureManager", "PBO creation failed, falling back to direct upload");
        m_pboReady = false;
        return;
    }

    for (int i = 0; i < PBO_COUNT; i++) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    m_pboSize = size;
    m_pboReady = true;
    m_currentPbo = 0;
}

void TextureManager::DestroyPBOs() {
    if (m_pbos[0] != 0) {
        glDeleteBuffers(PBO_COUNT, m_pbos);
        m_pbos[0] = 0;
        m_pbos[1] = 0;
        m_pboReady = false;
    }
}

bool TextureManager::Update(const void* pixelData, int32_t width, int32_t height, GLenum format) {
    if (!pixelData) {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_pboReady) {
        int uploadPbo = m_currentPbo;
        int nextPbo = 1 - m_currentPbo;

        size_t uploadSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[uploadPbo]);
        void* pboPtr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, uploadSize,
                                         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if (pboPtr) {
            memcpy(pboPtr, pixelData, uploadSize);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

            if (width != m_width || height != m_height) {
                glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0,
                             format, GL_UNSIGNED_BYTE, nullptr);
                m_width = width;
                m_height = height;
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                            format, GL_UNSIGNED_BYTE, nullptr);
        } else {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            if (width != m_width || height != m_height) {
                glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0,
                             format, GL_UNSIGNED_BYTE, pixelData);
                m_width = width;
                m_height = height;
            } else {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                                format, GL_UNSIGNED_BYTE, pixelData);
            }
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_currentPbo = nextPbo;
    } else {
        if (width != m_width || height != m_height) {
            glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0,
                         format, GL_UNSIGNED_BYTE, pixelData);
            m_width = width;
            m_height = height;
        } else {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                            format, GL_UNSIGNED_BYTE, pixelData);
        }
    }

    return true;
}

bool TextureManager::UpdateRegion(const void* pixelData, int32_t x, int32_t y,
                                   int32_t width, int32_t height, int32_t stride, GLenum format) {
    if (!pixelData) {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
                    format, GL_UNSIGNED_BYTE, pixelData);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    return true;
}

} // namespace NativeXComponentSample
