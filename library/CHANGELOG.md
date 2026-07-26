# Changelog

## 1.2.0 (2026-07-27)

### Architecture: Deferred Memcpy (零JS线程拷贝)
- **RendererApi**: JS 线程不再执行 memcpy，改为创建 napi_ref 防止 GC，仅传递原始指针+长度入队
- **RenderThread**: 渲染线程 dequeue 后执行 memcpy，完成后通过 RefCleaner 调度 napi_ref 释放
- **RefCleaner**: 基于 napi_threadsafe_function 的安全 napi_ref 清理工具，跨线程安全
- **Renderer::Destroy**: 销毁前先渲染纯黑帧清屏，避免随机画面残留
- **Performance**: 全帧渲染 JS 感知延迟 33ms→1ms (33×)，连续多帧 24.6ms/帧→2ms/帧 (12×)

### Tests
- RendererIntegration: 增量渲染测试替换为 Tile 渲染测试

## 1.1.0 (2026-07-27)

### Architecture: Fire-and-Forget + Double Buffer
- **GLESBackend**: Front/Back double buffer, Upload/SwapAndPresent separation
- **RenderThread**: Persistent thread loop, no future/promise blocking
- **Renderer**: void async methods, immediate return to caller
- **RendererApi**: Resolve promise immediately after enqueue (no napi_async_work)
- **Performance**: Single tile 64ms→1ms, multi-tile 16ms→4ms

### Features
- renderTileRegions API (ratio-based tile positioning, native C++ implementation)
- setVSync API (full chain: NAPI→Renderer→RenderThread→GLESBackend→EGLContextManager)
- ArkZeroRenderSession session layer
- TileRenderIntegration + RenderSessionIntegration tests

## 1.0.0 (2026-07-26)

### Features
- Async render thread with command queue
- Ratio-based viewport system (originRatio + sizeRatio, fit-to-screen)
- Normalized touch output (ratioX/Y + pixelX/Y convenience)
- Incremental dirty region rendering
- Separated update/present API
- Multi-format support (RGBA/RGB/BGRA/NV21/NV12)
- Texture pool with strategy pattern
