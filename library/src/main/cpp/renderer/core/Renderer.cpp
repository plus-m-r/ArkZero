#include "Renderer.h"
#include "../backend/GLESBackend.h"
#include <hilog/log.h>
#include <cstring>

#include "../../common/common.h"

namespace NativeXComponentSample {

Renderer::Renderer(int32_t width, int32_t height, PixelFormat format)
    : m_width(width)
    , m_height(height)
    , m_format(format)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Constructor: %{public}dx%{public}d, format=%{public}d",
        width, height, static_cast<int>(format));
}

Renderer::~Renderer()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Destructor");
    Destroy();
}

bool Renderer::Initialize(void* nativeWindow)
{
    if (m_initialized) {
        OH_LOG_Print(LOG_APP, LOG_WARN, 0x0001,
            "ArkZeroRenderer", "[Renderer] Already initialized");
        return true;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Initializing with NativeWindow...");

    m_renderThread.Start();

    RenderCommand cmd;
    cmd.type = RenderCommandType::INIT;
    cmd.nativeWindow = nativeWindow;
    cmd.width = m_width;
    cmd.height = m_height;
    cmd.format = m_format;

    std::future<bool> fut = m_renderThread.EnqueueCommand(std::move(cmd));
    bool success = fut.get();

    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[Renderer] Init command FAILED on render thread");
        m_renderThread.Stop();
        return false;
    }

    m_initialized = true;

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Initialized on render thread");
    return true;
}

std::future<bool> Renderer::RenderFrameAsync(const void* pixelData, size_t dataSize,
                                              int32_t width, int32_t height)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::RENDER_FRAME;
    cmd.width = width;
    cmd.height = height;

    if (pixelData && dataSize > 0) {
        cmd.pixelData.resize(dataSize);
        memcpy(cmd.pixelData.data(), pixelData, dataSize);
    }

    return m_renderThread.EnqueueCommand(std::move(cmd));
}

std::future<bool> Renderer::RenderFrameRegionsAsync(const void* pixelData, size_t dataSize,
                                                     int32_t frameWidth, int32_t frameHeight,
                                                     const DirtyRect* regions, int32_t regionCount,
                                                     bool swapBuffers)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::RENDER_FRAME_REGIONS;
    cmd.frameWidth = frameWidth;
    cmd.frameHeight = frameHeight;
    cmd.swapBuffers = swapBuffers;

    if (pixelData && dataSize > 0) {
        cmd.pixelData.resize(dataSize);
        memcpy(cmd.pixelData.data(), pixelData, dataSize);
    }

    if (regions && regionCount > 0) {
        cmd.regions.assign(regions, regions + regionCount);
    }

    return m_renderThread.EnqueueCommand(std::move(cmd));
}

std::future<bool> Renderer::RenderTileRegionsAsync(
    std::vector<TileRegion>&& tiles,
    std::vector<std::vector<uint8_t>>&& tilePixelBuffers,
    int32_t frameWidth, int32_t frameHeight,
    bool swapBuffers)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::RENDER_TILE_REGIONS;
    cmd.frameWidth = frameWidth;
    cmd.frameHeight = frameHeight;
    cmd.swapBuffers = swapBuffers;
    cmd.tiles = std::move(tiles);
    cmd.tilePixelBuffers = std::move(tilePixelBuffers);

    return m_renderThread.EnqueueCommand(std::move(cmd));
}

std::future<bool> Renderer::UpdateDirtyRegionsAsync(const void* pixelData, size_t dataSize,
                                                     int32_t frameWidth, int32_t frameHeight,
                                                     const DirtyRect* regions, int32_t regionCount)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::UPDATE_DIRTY;
    cmd.frameWidth = frameWidth;
    cmd.frameHeight = frameHeight;

    if (pixelData && dataSize > 0) {
        cmd.pixelData.resize(dataSize);
        memcpy(cmd.pixelData.data(), pixelData, dataSize);
    }

    if (regions && regionCount > 0) {
        cmd.regions.assign(regions, regions + regionCount);
    }

    return m_renderThread.EnqueueCommand(std::move(cmd));
}

std::future<bool> Renderer::PresentFrameAsync()
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::PRESENT_FRAME;
    return m_renderThread.EnqueueCommand(std::move(cmd));
}

std::future<bool> Renderer::ResizeAsync(int32_t width, int32_t height)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::RESIZE;
    cmd.width = width;
    cmd.height = height;
    return m_renderThread.EnqueueCommand(std::move(cmd));
}

std::future<bool> Renderer::SetVSyncAsync(bool enabled)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SET_VSYNC;
    cmd.vsyncEnabled = enabled;
    return m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::Destroy()
{
    if (!m_initialized) {
        return;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Destroying...");

    m_initialized = false;

    RenderCommand cmd;
    cmd.type = RenderCommandType::DESTROY;
    std::future<bool> fut = m_renderThread.EnqueueCommand(std::move(cmd));
    fut.get();

    m_renderThread.Stop();

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Destroyed");
}

bool Renderer::IsInitialized() const
{
    return m_initialized;
}

} // namespace NativeXComponentSample
