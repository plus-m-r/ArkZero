#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <atomic>
#include <memory>
#include "../../common/common.h"

namespace NativeXComponentSample {

class GLESBackend;

enum class RenderCommandType {
    INIT,
    RENDER_FRAME,
    RENDER_FRAME_REGIONS,
    UPDATE_DIRTY,
    PRESENT_FRAME,
    RESIZE,
    DESTROY,
    SHUTDOWN
};

struct RenderCommand {
    RenderCommandType type;

    std::vector<uint8_t> pixelData;
    int32_t width = 0;
    int32_t height = 0;
    int32_t frameWidth = 0;
    int32_t frameHeight = 0;
    PixelFormat format = PixelFormat::RGBA;

    std::vector<DirtyRect> regions;
    bool swapBuffers = true;

    void* nativeWindow = nullptr;

    std::promise<bool> completion;
};

class RenderThread {
public:
    RenderThread();
    ~RenderThread();

    void Start();

    void Stop();

    std::future<bool> EnqueueCommand(RenderCommand cmd);

    bool IsRunning() const;

private:
    void ThreadLoop();
    void ProcessCommand(RenderCommand& cmd);
    void ProcessInit(RenderCommand& cmd);
    void ProcessRenderFrame(RenderCommand& cmd);
    void ProcessRenderFrameRegions(RenderCommand& cmd);
    void ProcessUpdateDirty(RenderCommand& cmd);
    void ProcessPresentFrame(RenderCommand& cmd);
    void ProcessResize(RenderCommand& cmd);
    void ProcessDestroy(RenderCommand& cmd);

    std::thread m_thread;
    std::queue<RenderCommand> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running{false};

    std::unique_ptr<GLESBackend> m_backend;
    int32_t m_width = 0;
    int32_t m_height = 0;
    PixelFormat m_format = PixelFormat::RGBA;
};

} // namespace NativeXComponentSample

#endif // RENDER_THREAD_H
