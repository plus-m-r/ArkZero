#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <cstddef>
#include <memory>
#include <mutex>

#include "../core/RenderThread.h"
#include "../../common/common.h"

namespace NativeXComponentSample {

class Renderer {
public:
    Renderer(int32_t width, int32_t height, PixelFormat format);
    
    ~Renderer();

    bool Initialize(void* nativeWindow);

    void RenderFrameAsync(const void* pixelData, size_t dataSize,
                          int32_t width, int32_t height);

    void RenderFrameRegionsAsync(const void* pixelData, size_t dataSize,
                                 int32_t frameWidth, int32_t frameHeight,
                                 const DirtyRect* regions, int32_t regionCount,
                                 bool swapBuffers);

    void RenderTileRegionsAsync(
        std::vector<TileRegion>&& tiles,
        std::vector<std::vector<uint8_t>>&& tilePixelBuffers,
        int32_t frameWidth, int32_t frameHeight,
        bool swapBuffers);

    void UpdateDirtyRegionsAsync(const void* pixelData, size_t dataSize,
                                 int32_t frameWidth, int32_t frameHeight,
                                 const DirtyRect* regions, int32_t regionCount);

    void PresentFrameAsync();

    void ResizeAsync(int32_t width, int32_t height);

    void SetVSyncAsync(bool enabled);

    void Destroy();

    bool IsInitialized() const;

private:
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    RenderThread m_renderThread;
    std::mutex m_mutex;

    int32_t m_width;
    int32_t m_height;
    PixelFormat m_format;
    bool m_initialized = false;
};

} // namespace NativeXComponentSample

#endif // RENDERER_H
