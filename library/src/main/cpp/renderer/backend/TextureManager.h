#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <GLES3/gl3.h>
#include <cstdint>
#include <cstddef>

namespace NativeXComponentSample {

class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    bool Create(int32_t width, int32_t height, GLint internalFormat, GLenum format);
    void Destroy();

    bool Update(const void* pixelData, int32_t width, int32_t height, GLenum format);

    bool UpdateRegion(const void* pixelData, int32_t x, int32_t y,
                      int32_t width, int32_t height, int32_t stride, GLenum format);

    GLuint GetTextureId() const { return m_textureId; }
    bool IsCreated() const { return m_textureId != 0; }

private:
    void CreatePBOs(size_t size);
    void DestroyPBOs();

    GLuint m_textureId;
    GLint m_internalFormat;
    GLenum m_format;
    int32_t m_width;
    int32_t m_height;

    static constexpr int PBO_COUNT = 2;
    GLuint m_pbos[PBO_COUNT];
    int m_currentPbo;
    bool m_pboReady;
    size_t m_pboSize;
};

} // namespace NativeXComponentSample

#endif // TEXTURE_MANAGER_H
