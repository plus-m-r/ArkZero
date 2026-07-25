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

    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_running.store(false);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Stopped");
}

std::future<bool> RenderThread::EnqueueCommand(RenderCommand cmd) {
    std::future<bool> fut = cmd.completion.get_future();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(cmd));
    }
    m_cv.notify_one();
    return fut;
}

bool RenderThread::IsRunning() const {
    return m_running.load();
}

void RenderThread::ThreadLoop() {
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Loop entered");

    while (m_running.load()) {
        RenderCommand cmd;
        {
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

            cmd = std::move(m_queue.front());
            m_queue.pop();
        }

        if (cmd.type == RenderCommandType::SHUTDOWN) {
            if (m_backend) {
                m_backend->Destroy();
                m_backend.reset();
            }
            cmd.completion.set_value(true);
            break;
        }

        ProcessCommand(cmd);
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Loop exited");
}

void RenderThread::ProcessCommand(RenderCommand& cmd) {
    try {
        switch (cmd.type) {
            case RenderCommandType::INIT:
                ProcessInit(cmd);
                break;
            case RenderCommandType::RENDER_FRAME:
                ProcessRenderFrame(cmd);
                break;
            case RenderCommandType::RENDER_FRAME_REGIONS:
                ProcessRenderFrameRegions(cmd);
                break;
            case RenderCommandType::UPDATE_DIRTY:
                ProcessUpdateDirty(cmd);
                break;
            case RenderCommandType::PRESENT_FRAME:
                ProcessPresentFrame(cmd);
                break;
            case RenderCommandType::RESIZE:
                ProcessResize(cmd);
                break;
            case RenderCommandType::DESTROY:
                ProcessDestroy(cmd);
                break;
            default:
                cmd.completion.set_value(false);
                break;
        }
    } catch (const std::exception& e) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] Exception: %{public}s", e.what());
        cmd.completion.set_value(false);
    } catch (...) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] Unknown exception");
        cmd.completion.set_value(false);
    }
}

void RenderThread::ProcessInit(RenderCommand& cmd) {
    if (m_backend && m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] Already initialized");
        cmd.completion.set_value(true);
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
        cmd.completion.set_value(false);
        return;
    }
    m_backend = std::move(backend);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] GLESBackend initialized on render thread");
    cmd.completion.set_value(true);
}

void RenderThread::ProcessRenderFrame(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] RenderFrame: backend not initialized");
        cmd.completion.set_value(false);
        return;
    }

    bool success = m_backend->RenderFrame(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.width,
        cmd.height);

    if (!success) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[RENDER-THREAD] RenderFrame FAILED");
    }
    cmd.completion.set_value(success);
}

void RenderThread::ProcessRenderFrameRegions(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        cmd.completion.set_value(false);
        return;
    }

    bool success = m_backend->RenderFrameRegions(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.frameWidth,
        cmd.frameHeight,
        cmd.regions.data(),
        static_cast<int32_t>(cmd.regions.size()),
        cmd.swapBuffers);

    cmd.completion.set_value(success);
}

void RenderThread::ProcessUpdateDirty(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        cmd.completion.set_value(false);
        return;
    }

    bool success = m_backend->UpdateDirtyRegions(
        cmd.pixelData.data(),
        cmd.pixelData.size(),
        cmd.frameWidth,
        cmd.frameHeight,
        cmd.regions.data(),
        static_cast<int32_t>(cmd.regions.size()));

    cmd.completion.set_value(success);
}

void RenderThread::ProcessPresentFrame(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        cmd.completion.set_value(false);
        return;
    }

    bool success = m_backend->PresentFrame();
    cmd.completion.set_value(success);
}

void RenderThread::ProcessResize(RenderCommand& cmd) {
    if (!m_backend || !m_backend->IsInitialized()) {
        cmd.completion.set_value(false);
        return;
    }

    bool success = m_backend->Resize(cmd.width, cmd.height);
    if (success) {
        m_width = cmd.width;
        m_height = cmd.height;
    }
    cmd.completion.set_value(success);
}

void RenderThread::ProcessDestroy(RenderCommand& cmd) {
    if (m_backend) {
        m_backend->Destroy();
        m_backend.reset();
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[RENDER-THREAD] Backend destroyed");
    cmd.completion.set_value(true);
}

} // namespace NativeXComponentSample
