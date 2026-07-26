#include "GLESBackend.h"
#include <hilog/log.h>
#include <cstring>

#include "../../common/common.h"
#include "factory/TextureFactory.h"

namespace NativeXComponentSample {

GLESBackend::GLESBackend()
    : m_width(0)
    , m_height(0)
    , m_format(PixelFormat::RGBA)
    , m_isInitialized(false)
    , m_frontTexture(nullptr)
    , m_backTexture(nullptr)
    , m_textureWidth(0)
    , m_textureHeight(0)
    , m_backDirty(false)
{
    m_textureStrategy = std::make_unique<PoolTextureStrategy>(10);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Using texture strategy: %s", m_textureStrategy->GetName());
}

GLESBackend::~GLESBackend() {
    Destroy();
}

bool GLESBackend::Initialize(void* nativeWindow, int32_t width, int32_t height, PixelFormat format) {
    if (!nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Invalid nativeWindow");
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = format;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Initializing: %dx%d, format=%d", width, height, static_cast<int>(format));
    
    if (!m_eglManager.Initialize(nativeWindow, width, height, true)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize EGL context");
        return false;
    }

    if (IsYUVFormat(format)) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Using YUV Shader for format=%d", static_cast<int>(format));
        
        if (!m_yuvShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize YUV shader");
            m_eglManager.Destroy();
            return false;
        }
    } else {
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
        GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
        
        if (std::string(m_textureStrategy->GetName()) == "PoolStrategy") {
            m_textureStrategy->Preallocate(800, 600, internalFormat, glFormat);
            m_textureStrategy->Preallocate(1920, 1080, internalFormat, glFormat);
            m_textureStrategy->Preallocate(3840, 2160, internalFormat, glFormat);
            m_textureStrategy->Preallocate(1280, 720, internalFormat, glFormat);
            m_textureStrategy->Preallocate(640, 480, internalFormat, glFormat);
            
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Preallocated textures for common resolutions");
        }
        
        if (!m_textureShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize TextureShader");
            m_eglManager.Destroy();
            return false;
        }
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Using texture strategy: %s", m_textureStrategy->GetName());
    }

    m_isInitialized = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Initialized");
    
    return true;
}

bool GLESBackend::InitializeOffscreen(int32_t width, int32_t height, PixelFormat format) {
    m_width = width;
    m_height = height;
    m_format = format;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Initializing offscreen: %dx%d, format=%d", width, height, static_cast<int>(format));
    
    if (!m_eglManager.InitializeOffscreen(width, height)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to initialize offscreen EGL context");
        return false;
    }

    if (IsYUVFormat(format)) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Using YUV Shader for format=%d", static_cast<int>(format));
        
        if (!m_yuvShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize YUV shader");
            m_eglManager.Destroy();
            return false;
        }
    } else {
        GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(format);
        GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
        
        if (std::string(m_textureStrategy->GetName()) == "PoolStrategy") {
            m_textureStrategy->Preallocate(800, 600, internalFormat, glFormat);
            m_textureStrategy->Preallocate(1920, 1080, internalFormat, glFormat);
            m_textureStrategy->Preallocate(3840, 2160, internalFormat, glFormat);
            m_textureStrategy->Preallocate(1280, 720, internalFormat, glFormat);
            m_textureStrategy->Preallocate(640, 480, internalFormat, glFormat);
            
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Preallocated textures for common resolutions");
        }
        
        if (!m_textureShader.Initialize()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                "GLESBackend", "Failed to initialize TextureShader");
            m_eglManager.Destroy();
            return false;
        }
        
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Using texture strategy: %s", m_textureStrategy->GetName());
    }

    m_isInitialized = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "Offscreen initialized");
    
    return true;
}

TextureManager* GLESBackend::EnsureBackTexture(int32_t width, int32_t height) {
    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
    GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);

    if (m_backTexture && m_textureWidth == width && m_textureHeight == height) {
        return m_backTexture;
    }

    if (m_backTexture) {
        m_textureStrategy->Release(m_backTexture);
        m_backTexture = nullptr;
    }

    m_backTexture = m_textureStrategy->Acquire(width, height, internalFormat, glFormat);
    if (!m_backTexture) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] EnsureBackTexture: Acquire FAILED");
        return nullptr;
    }

    m_textureWidth = width;
    m_textureHeight = height;

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[DBBUF] Back texture allocated: %dx%d", width, height);

    return m_backTexture;
}

void GLESBackend::UploadFrame(const void* pixelData, size_t dataSize,
                               int32_t width, int32_t height) {
    if (!m_isInitialized || !pixelData) {
        return;
    }

    if (IsYUVFormat(m_format)) {
        return;
    }

    if (!m_eglManager.MakeCurrent()) {
        return;
    }

    TextureManager* back = EnsureBackTexture(width, height);
    if (!back) {
        return;
    }

    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
    back->Update(pixelData, width, height, glFormat);
    m_backDirty = true;
}

void GLESBackend::UploadFrameRegions(const void* pixelData, size_t dataSize,
                                      int32_t frameWidth, int32_t frameHeight,
                                      const DirtyRect* regions, int32_t regionCount) {
    if (!m_isInitialized || !pixelData || !regions || regionCount <= 0) {
        return;
    }

    if (IsYUVFormat(m_format)) {
        return;
    }

    if (!m_eglManager.MakeCurrent()) {
        return;
    }

    TextureManager* back = EnsureBackTexture(frameWidth, frameHeight);
    if (!back) {
        return;
    }

    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
    int32_t bytesPerPixel = PixelFormatConverter::GetBytesPerPixel(m_format);

    for (int32_t i = 0; i < regionCount; i++) {
        int32_t rx = regions[i].x;
        int32_t ry = regions[i].y;
        int32_t rw = regions[i].w;
        int32_t rh = regions[i].h;

        size_t rowOffset = static_cast<size_t>(ry) * static_cast<size_t>(frameWidth) * bytesPerPixel;
        size_t colOffset = static_cast<size_t>(rx) * bytesPerPixel;
        const uint8_t* regionPtr = static_cast<const uint8_t*>(pixelData) + rowOffset + colOffset;

        int32_t glY = frameHeight - ry - rh;

        back->UpdateRegion(regionPtr, rx, glY, rw, rh, frameWidth, glFormat);
    }

    m_backDirty = true;
}

void GLESBackend::UploadTileRegions(const TileRegion* tiles, int32_t tileCount,
                                     int32_t frameWidth, int32_t frameHeight) {
    if (!m_isInitialized || !tiles || tileCount <= 0) {
        return;
    }

    if (IsYUVFormat(m_format)) {
        return;
    }

    if (!m_eglManager.MakeCurrent()) {
        return;
    }

    TextureManager* back = EnsureBackTexture(frameWidth, frameHeight);
    if (!back) {
        return;
    }

    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
    int32_t bytesPerPixel = PixelFormatConverter::GetBytesPerPixel(m_format);

    for (int32_t i = 0; i < tileCount; i++) {
        if (!tiles[i].pixelData) {
            continue;
        }

        int32_t tw = tiles[i].tilePixelWidth;
        int32_t th = tiles[i].tilePixelHeight;
        size_t expectedSize = static_cast<size_t>(tw) * static_cast<size_t>(th) * bytesPerPixel;
        if (tiles[i].dataSize < expectedSize) {
            continue;
        }

        int32_t pixelX = static_cast<int32_t>(tiles[i].ratioX * frameWidth);
        int32_t pixelY = static_cast<int32_t>(tiles[i].ratioY * frameHeight);

        if (pixelX < 0 || pixelY < 0 || pixelX + tw > frameWidth || pixelY + th > frameHeight) {
            continue;
        }

        int32_t glY = frameHeight - pixelY - th;

        back->UpdateRegion(tiles[i].pixelData, pixelX, glY, tw, th, tw, glFormat);
    }

    m_backDirty = true;
}

void GLESBackend::SwapAndPresent() {
    if (!m_isInitialized) {
        return;
    }

    if (!m_backDirty || !m_backTexture) {
        return;
    }

    if (!m_eglManager.MakeCurrent()) {
        return;
    }

    std::swap(m_frontTexture, m_backTexture);
    m_backDirty = false;

    const int32_t viewportWidth = m_eglManager.GetSurfaceWidth() > 0 ? m_eglManager.GetSurfaceWidth() : m_width;
    const int32_t viewportHeight = m_eglManager.GetSurfaceHeight() > 0 ? m_eglManager.GetSurfaceHeight() : m_height;
    glViewport(0, 0, viewportWidth, viewportHeight);

    bool success = m_textureShader.Draw(m_frontTexture->GetTextureId(), viewportWidth, viewportHeight);
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] SwapAndPresent: Draw FAILED");
        return;
    }

    if (!m_eglManager.SwapBuffers()) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] SwapAndPresent: SwapBuffers FAILED");
        return;
    }

    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);
    GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
    m_backTexture = m_textureStrategy->Acquire(m_textureWidth, m_textureHeight, internalFormat, glFormat);
}

bool GLESBackend::RenderFrame(const void* pixelData, size_t dataSize, 
                               int32_t width, int32_t height) {
    if (!m_isInitialized) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] GLESBackend::RenderFrame: NOT initialized");
        return false;
    }

    if (!pixelData) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] GLESBackend::RenderFrame: pixelData is null");
        return false;
    }

    int bytesPerPixel = PixelFormatConverter::GetBytesPerPixel(m_format);
    size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * bytesPerPixel;
    if (dataSize < expectedSize) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] GLESBackend::RenderFrame: data size mismatch expected=%{public}zu actual=%{public}zu",
            expectedSize, dataSize);
        return false;
    }

    if (IsYUVFormat(m_format)) {
        if (!m_eglManager.MakeCurrent()) {
            return false;
        }

        const int32_t viewportWidth = m_eglManager.GetSurfaceWidth() > 0 ? m_eglManager.GetSurfaceWidth() : width;
        const int32_t viewportHeight = m_eglManager.GetSurfaceHeight() > 0 ? m_eglManager.GetSurfaceHeight() : height;
        glViewport(0, 0, viewportWidth, viewportHeight);

        const uint8_t* data = static_cast<const uint8_t*>(pixelData);
        const uint8_t* yPlane = data;
        const uint8_t* uvPlane = data + width * height;

        bool success = false;
        if (m_format == PixelFormat::NV21) {
            success = m_yuvShader.RenderNV21(yPlane, uvPlane, width, height);
        } else if (m_format == PixelFormat::NV12) {
            success = m_yuvShader.RenderNV12(yPlane, uvPlane, width, height);
        }

        if (!success) {
            return false;
        }

        if (!m_eglManager.SwapBuffers()) {
            return false;
        }

        return true;
    }

    UploadFrame(pixelData, dataSize, width, height);
    SwapAndPresent();
    return true;
}

bool GLESBackend::RenderFrameRegions(const void* pixelData, size_t dataSize,
                                     int32_t frameWidth, int32_t frameHeight,
                                     const DirtyRect* regions, int32_t regionCount,
                                     bool swapBuffers) {
    if (!m_isInitialized || !pixelData || !regions || regionCount <= 0) {
        return false;
    }

    if (IsYUVFormat(m_format)) {
        return false;
    }

    UploadFrameRegions(pixelData, dataSize, frameWidth, frameHeight, regions, regionCount);

    if (swapBuffers) {
        SwapAndPresent();
    }

    return true;
}

bool GLESBackend::UpdateDirtyRegions(const void* pixelData, size_t dataSize,
                                     int32_t frameWidth, int32_t frameHeight,
                                     const DirtyRect* regions, int32_t regionCount) {
    if (!m_isInitialized || !pixelData || !regions || regionCount <= 0) {
        return false;
    }

    if (IsYUVFormat(m_format)) {
        return false;
    }

    UploadFrameRegions(pixelData, dataSize, frameWidth, frameHeight, regions, regionCount);
    return true;
}

bool GLESBackend::RenderTileRegions(const TileRegion* tiles, int32_t tileCount,
                                     int32_t frameWidth, int32_t frameHeight,
                                     bool swapBuffers) {
    if (!m_isInitialized || !tiles || tileCount <= 0) {
        return false;
    }

    if (IsYUVFormat(m_format)) {
        return false;
    }

    UploadTileRegions(tiles, tileCount, frameWidth, frameHeight);

    if (swapBuffers) {
        SwapAndPresent();
    }

    return true;
}

bool GLESBackend::PresentFrame() {
    if (!m_isInitialized) {
        return false;
    }

    SwapAndPresent();
    return true;
}

bool GLESBackend::Resize(int32_t width, int32_t height) {
    if (!m_isInitialized) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "GLESBackend", "Not initialized");
        return false;
    }

    if (width <= 0 || height <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "GLESBackend", "Invalid size: %dx%d", width, height);
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "GLESBackend", "Resizing: %dx%d -> %dx%d",
        m_width, m_height, width, height);

    if (IsYUVFormat(m_format)) {
        m_width = width;
        m_height = height;
        return true;
    }

    if (m_frontTexture) {
        m_textureStrategy->Release(m_frontTexture);
        m_frontTexture = nullptr;
    }
    if (m_backTexture) {
        m_textureStrategy->Release(m_backTexture);
        m_backTexture = nullptr;
    }
    m_textureWidth = 0;
    m_textureHeight = 0;
    m_backDirty = false;

    GLint internalFormat = PixelFormatConverter::GetGLInternalFormat(m_format);
    GLenum glFormat = PixelFormatConverter::GetGLFormat(m_format);

    TextureManager* texture = m_textureStrategy->Acquire(width, height, internalFormat, glFormat);
    if (texture) {
        m_textureStrategy->Release(texture);
        m_width = width;
        m_height = height;
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "GLESBackend", "Resized using strategy: %s", m_textureStrategy->GetName());
        return true;
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "GLESBackend", "Failed to acquire texture from strategy");
        return false;
    }
}

void GLESBackend::Destroy() {
    if (!m_isInitialized) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "GLESBackend", "Destroying...");

    if (m_frontTexture) {
        m_textureStrategy->Release(m_frontTexture);
        m_frontTexture = nullptr;
    }
    if (m_backTexture) {
        m_textureStrategy->Release(m_backTexture);
        m_backTexture = nullptr;
    }
    m_textureWidth = 0;
    m_textureHeight = 0;
    m_backDirty = false;

    if (IsYUVFormat(m_format)) {
        m_yuvShader.Destroy();
    } else {
        if (m_textureShader.IsInitialized()) {
            m_textureShader.Destroy();
        }

        if (m_textureStrategy) {
            m_textureStrategy->Clear();
            m_textureStrategy.reset();
        }
    }

    m_isInitialized = false;

    m_eglManager.Destroy();

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "GLESBackend", "Destroyed");
}

bool GLESBackend::IsYUVFormat(PixelFormat format) const {
    return format == PixelFormat::NV21 || format == PixelFormat::NV12;
}

void GLESBackend::SetTextureStrategy(const char* strategyType) {
    if (!strategyType) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "GLESBackend", "Invalid strategy type");
        return;
    }

    std::string type(strategyType);

    if (type == "pool") {
        m_textureStrategy = std::make_unique<PoolTextureStrategy>(10);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "GLESBackend", "Switched to PoolStrategy");
    } else if (type == "direct") {
        m_textureStrategy = std::make_unique<DirectTextureStrategy>();
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "GLESBackend", "Switched to DirectStrategy");
    } else {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN,
            "GLESBackend", "Unknown strategy type: %s, keeping current strategy", strategyType);
    }
}

const char* GLESBackend::GetCurrentStrategyName() const {
    if (m_textureStrategy) {
        return m_textureStrategy->GetName();
    }
    return "None";
}

void GLESBackend::SetVSync(bool enabled) {
    m_eglManager.SetVSync(enabled);
}

bool GLESBackend::IsVSyncEnabled() const {
    return m_eglManager.IsVSyncEnabled();
}

} // namespace NativeXComponentSample
