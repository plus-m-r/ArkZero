# Changelog

## 2.0.1 (2026-08-02)

### Bug Fix: BGRA PixelFormat Mapping

**Critical Fix**: `PixelFormatConverter` mapped `PixelFormat.BGRA` to `GL_RGBA` instead of `GL_BGRA_EXT`, causing red-blue channel swap in BGRA mode.

- **GetGLInternalFormat(BGRA)**: `GL_RGBA` → `GL_BGRA_EXT` (0x80E1) — internalFormat must match client byte order
- **GetGLFormat(BGRA)**: Removed `#ifdef GL_BGRA_EXT` conditional fallback to `GL_RGBA`; now always returns `GL_BGRA_EXT`
- **PixelFormatConverter.h**: Added `#ifndef GL_BGRA_EXT / #define GL_BGRA_EXT 0x80E1` — OpenHarmony `<GLES3/gl3.h>` does not define this extension constant

### Bug Fix: RenderingCorrectnessPage

- **Inner tile D color**: `0xFFFFFF00` (cyan) → `0xFF00FFFF` (yellow) — Uint32 color value was incorrect

### Added

- **BGRA format test phase**: Phase 5 in RenderingCorrectnessPage — creates BGRA session, renders 6 color bands using BGRA byte order, validates `GL_BGRA_EXT` path produces correct colors (red=red, blue=blue, no channel swap)

## 2.0.0 (2026-08-01)

### Architecture: Single-Texture Direct-Retain + Zero-Copy Upload

**Breaking Change**: Double-buffer pattern replaced with single-texture direct-render-retain model.

- **GLESBackend**: Removed front/back texture swap; single `m_texture` updated in-place via `glTexSubImage2D`, dirty regions naturally persist across frames
- **SwapAndPresent**: Simplified to Draw + SwapBuffers — no texture swap, no `glBlitFramebuffer` full-frame copy (eliminated ~8MB GPU blit per frame at 1080p)
- **RenderThread**: `cmd.swapBuffers` flag now respected — only calls `SwapAndPresent` when explicitly requested; `RENDER_FRAME_REGIONS`, `RENDER_TILE_REGIONS`, `UPLOAD_FRAME`, `UPLOAD_TILE_REGIONS` no longer auto-present
- **PRESENT_FRAME**: Now actually calls `SwapAndPresent()` (was previously a no-op)
- **Zero-copy upload**: Removed deferred `memcpy` from RenderThread — `srcData` (NAPI ArrayBuffer pointer) passed directly to GPU upload path, eliminating ~8MB CPU copy per frame at 1080p
- **Renderer**: Removed eager `memcpy` in `RenderFrameAsync`, `RenderFrameRegionsAsync`, `UpdateDirtyRegionsAsync` — all paths use deferred pointer passthrough
- **TextureManager::Update**: PBO path now maps only the required upload size instead of full PBO capacity

### Performance Improvements
- Eliminated `glBlitFramebuffer` full-frame GPU blit every present (single-texture retain)
- Eliminated per-frame deferred memcpy (~8MB at 1080p) — direct pointer upload to GPU
- Conditional SwapAndPresent — batch multiple dirty-rect/tile uploads before single present
- TextureManager PBO maps only needed bytes, not full buffer

### Removed
- `m_frontTexture` / `m_backTexture` double-buffer members from GLESBackend
- `m_backDirty` flag (replaced by `m_dirty`)
- `EnsureBackTexture()` (replaced by `EnsureTexture()`)
- `Renderer::m_mutex` (was unused outside Initialize)
- `GLESBackend::m_textureManager` (dead code member)
- `ExecuteDeferredCopy()` (replaced by `CleanupDeferredCopy()`)

### Added
- `INCREMENTAL_TILE_PARALLEL` test phase in RenderingCorrectnessPage — validates multi-tile single-submit rendering
- `drawTileLabel()` helper for tile-local text rendering
- `CleanupDeferredCopy()` — lightweight ref cleanup without memcpy

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
- Normalized interaction output (ratioX/Y + pixelX/Y convenience)
- Incremental dirty region rendering
- Separated update/present API
- Multi-format support (RGBA/RGB/BGRA/NV21/NV12)
- Texture pool with strategy pattern
