#include "RenderThread.h"
#include "../backend/GLESBackend.h"
#include <hilog/log.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

RenderThread::RenderThread() {
}

RenderThread::~RenderThread() {
    Stop();
}

void RenderThread::Start() {
    if (m_running.load()) {
        return;
    }
    m_running.store(true);
    m_thread = std::thread(&RenderThread::ThreadLoop, this);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Started");
}

void RenderThread::Stop() {
    if (!m_running.load()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        RenderCommand shutdownCmd;
        shutdownCmd.type = RenderCommandType::SHUTDOWN;
        m_queue.push(std::move(shutdownCmd));
    }
    m_cv.notify_one();
    m_vsyncCv.notify_one();

    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_running.store(false);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Stopped");
}

void RenderThread::EnqueueCommand(RenderCommand cmd) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(cmd));
    }
    m_cv.notify_one();
}

bool RenderThread::IsRunning() const {
    return m_running.load();
}

void RenderThread::NotifyVSync() {
    m_vsyncSignaled.store(true);
    m_vsyncCv.notify_one();
}

void RenderThread::ThreadLoop() {
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Loop entered");

    while (m_running.load()) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] {
            return !m_queue.empty() || !m_running.load();
        });

        if (!m_running.load() && m_queue.empty()) {
            break;
        }

        if (m_queue.empty()) {
            continue;
        }

        RenderCommand cmd = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();

        if (cmd.type == RenderCommandType::SHUTDOWN) {
            if (m_backend) {
                m_backend->Destroy();
                m_backend.reset();
            }
            break;
        }

        if (cmd.type == RenderCommandType::INIT) {
            ProcessInit(cmd);
            continue;
        }

        if (cmd.type == RenderCommandType::DESTROY) {
            ProcessDestroy(cmd);
            continue;
        }

        if (cmd.type == RenderCommandType::SET_VSYNC) {
            ProcessSetVSync(cmd);
            continue;
        }

        if (cmd.type == RenderCommandType::RESIZE) {
            ProcessResize(cmd);
            continue;
        }

        bool needSwap = (cmd.type == RenderCommandType::RENDER_FRAME ||
                         cmd.type == RenderCommandType::RENDER_FRAME_REGIONS ||
                         cmd.type == RenderCommandType::RENDER_TILE_REGIONS ||
                         cmd.type == RenderCommandType::PRESENT_FRAME ||
                         cmd.type == RenderCommandType::UPLOAD_FRAME ||
                         cmd.type == RenderCommandType::UPLOAD_TILE_REGIONS);

        ProcessCommand(cmd);

        if (needSwap && m_backend && m_backend->IsInitialized()) {
            m_backend->SwapAndPresent();
        }
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Loop exited");
}

void RenderThread::DrainQueue() {
    while (!m_queue.empty()) {
        RenderCommand cmd = std::move(m_queue.front());
        m_queue.pop();
        if (cmd.type != RenderCommandType::SHUTDOWN) {
            ProcessCommand(cmd);
        }
    }
}

void RenderThread::ProcessCommand(RenderCommand& cmd) {
    try {
        switch (cmd.type) {
            case RenderCommandType::RENDER_FRAME:
                ProcessRenderFrame(cmd);
                break;
            case RenderCommandType::RENDER_FRAME_REGIONS:
                ProcessRenderFrameRegions(cmd);
                break;
            case RenderCommandType::RENDER_TILE_REGIONS:
                ProcessRenderTileRegions(cmd);
                break;
            case RenderCommandType::UPDATE_DIRTY:
                ProcessUpdateDirty(cmd);
                break;
            case RenderCommandType::UPLOAD_FRAME:
                ProcessUploadFrame(cmd);
                break;
            case RenderCommandType::UPLOAD_FRAME_REGIONS:
                ProcessUploadFrameRegions(cmd);
                break;
            case RenderCommandType::UPLOAD_TILE_REGIONS:
                ProcessUploadTileRegions(cmd);
                break;
            case RenderCommandType::PRESENT_FRAME:
                ProcessPresentFrame(cmd);
                break;
            default:
                break;
        }
    } catch (const std::exception& e) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] Exception: %{public}s", e.what());
    } catch (...) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] Unknown exception");
    }
}

void RenderThread::ProcessInit(RenderCommand& cmd) {
    if (m_backend && m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] Already initialized");
        return;
    }

    m_width = cmd.width;
    m_height = cmd.height;
    m_format = cmd.format;

    auto backend = std::make_unique<GLESBackend>();
    bool success = backend->Initialize(cmd.nativeWindow, m_width, m_height, m_format);
    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] GLESBackend::Initialize FAILED");
        return;
    }
    m_backend = std::move(backend);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] GLESBackend initialized on render thread");
}

void RenderThread::ProcessRenderFrame(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] RenderFrame: backend not initialized");
        return;
    }

    m_backend->UploadFrame(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.width,
        cmd.height);
}

void RenderThread::ProcessRenderFrameRegions(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->UploadFrameRegions(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.frameWidth,
        cmd.frameHeight,
        cmd.regions.data(),
        static_cast<int32_t>(cmd.regions.size()));
}

void RenderThread::ProcessRenderTileRegions(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->UploadTileRegions(
        cmd.tiles.data(),
        static_cast<int32_t>(cmd.tiles.size()),
        cmd.frameWidth,
        cmd.frameHeight);
}

void RenderThread::ProcessUpdateDirty(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->UploadFrameRegions(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.frameWidth,
        cmd.frameHeight,
        cmd.regions.data(),
        static_cast<int32_t>(cmd.regions.size()));
}

void RenderThread::ProcessUploadFrame(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->UploadFrame(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.width,
        cmd.height);
}

void RenderThread::ProcessUploadFrameRegions(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->UploadFrameRegions(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.frameWidth,
        cmd.frameHeight,
        cmd.regions.data(),
        static_cast<int32_t>(cmd.regions.size()));
}

void RenderThread::ProcessUploadTileRegions(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->UploadTileRegions(
        cmd.tiles.data(),
        static_cast<int32_t>(cmd.tiles.size()),
        cmd.frameWidth,
        cmd.frameHeight);
}

void RenderThread::ProcessPresentFrame(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }
}

void RenderThread::ProcessResize(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    bool success = m_backend->Resize(cmd.width, cmd.height);
    if (success) {
        m_width = cmd.width;
        m_height = cmd.height;
    }
}

void RenderThread::ProcessSetVSync(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        return;
    }

    m_backend->SetVSync(cmd.vsyncEnabled);
}

void RenderThread::ProcessDestroy(RenderCommand& cmd) {
    if (m_backend) {
        m_backend->Destroy();
        m_backend.reset();
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Backend destroyed");
}

} // namespace NativeXComponentSample
