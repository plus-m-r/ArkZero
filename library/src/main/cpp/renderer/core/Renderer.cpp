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

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_renderThread.EnqueueCommand(std::move(cmd));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    m_initialized = true;

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Initialized (fire-and-forget)");
    return true;
}

void Renderer::RenderFrameAsync(const void* pixelData, size_t dataSize,
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

    m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::RenderFrameRegionsAsync(const void* pixelData, size_t dataSize,
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

    m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::RenderTileRegionsAsync(
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

    m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::UpdateDirtyRegionsAsync(const void* pixelData, size_t dataSize,
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

    m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::PresentFrameAsync()
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::PRESENT_FRAME;
    m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::ResizeAsync(int32_t width, int32_t height)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::RESIZE;
    cmd.width = width;
    cmd.height = height;
    m_renderThread.EnqueueCommand(std::move(cmd));
}

void Renderer::SetVSyncAsync(bool enabled)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SET_VSYNC;
    cmd.vsyncEnabled = enabled;
    m_renderThread.EnqueueCommand(std::move(cmd));
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
    m_renderThread.EnqueueCommand(std::move(cmd));

    m_renderThread.Stop();

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[Renderer] Destroyed");
}

bool Renderer::IsInitialized() const
{
    return m_initialized;
}

} // namespace NativeXComponentSample
