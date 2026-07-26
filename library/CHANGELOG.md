# Changelog

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
