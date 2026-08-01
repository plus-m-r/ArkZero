# @plusml/arkzero

High-performance GPU rendering library for OpenHarmony. Single-texture direct-render-retain architecture with zero-copy async upload, tile-based incremental rendering, and conditional swap presentation.

**Source:** https://github.com/plus-m-r/ArkZero

## Install

```bash
ohpm install @plusml/arkzero
```

## Architecture

```
┌─────────────┐    napi_ref     ┌──────────────┐    raw ptr     ┌────────────┐
│  ArkTS Thread │ ────────────▶ │  RenderThread │ ────────────▶ │  GLESBackend│
│  (zero-copy) │  no memcpy     │  (GPU upload) │  direct upload │  (GL/GLES) │
└─────────────┘                 └──────────────┘                └────────────┘
                                       │                              │
                                  cmd.swapBuffers?              Single Texture
                                  ──────────────────▶          (direct retain)
                                   SwapAndPresent()            glTexSubImage2D
                                                              only dirty regions
```

**Key design decisions:**
- **Single texture** — one GL texture updated in-place; unmodified regions naturally persist, no double-buffer swap or full-frame blit
- **Zero-copy upload** — NAPI ArrayBuffer pointer passed directly to GPU upload, no intermediate CPU memcpy
- **Conditional present** — `cmd.swapBuffers` flag controls when SwapAndPresent fires; batch multiple uploads then present once
- **Direct-render-retain** — `glTexSubImage2D` updates only dirty regions; texture retains previous content automatically

## Performance

| Scenario | Latency |
|----------|---------|
| Full frame render + present | ~1ms |
| Single tile render (no swap) | ~1ms |
| Multi-tile batch render (4 tiles) | ~4ms |
| Dirty region update (no swap) | <1ms |
| Present only | ~1ms |

Compared to v1.x double-buffer: eliminated ~8MB GPU blit/frame (1080p) and ~8MB CPU memcpy/frame.

## Quick Start

### 1. Add Surface to Page

```typescript
import { ArkZeroSurfaceView, SurfaceViewConfig, ArkZeroRenderSession, RenderSessionConfig, PixelFormat } from '@plusml/arkzero';

@Entry
@Component
struct DemoPage {
  private session?: ArkZeroRenderSession;

  private surfaceConfig: SurfaceViewConfig = {
    renderWidth: 640,
    renderHeight: 480,
    format: PixelFormat.RGBA,
    sizeRatioX: 0.65,
    enableTouch: false,
    xComponentId: 'demo_surface'
  };

  build() {
    Column() {
      ArkZeroSurfaceView({
        config: this.surfaceConfig,
        onSurfaceLoaded: (surfaceId: string) => {
          this.initSession(surfaceId);
        }
      })
    }
  }

  private async initSession(surfaceId: string) {
    const config: RenderSessionConfig = {
      width: 640,
      height: 480,
      format: PixelFormat.RGBA
    };
    this.session = new ArkZeroRenderSession(config);
    await this.session.attach(surfaceId);
  }
}
```

### 2. Full Frame Render

```typescript
const buffer = new ArrayBuffer(640 * 480 * 4);
const pixels = new Uint32Array(buffer);
pixels.fill(0xFF00FFFF); // Yellow (RGBA as ABGR on little-endian)

await this.session.renderFullFrame(buffer, 640, 480);
```

### 3. Incremental Dirty Region Render

```typescript
// Update only changed regions, then present
const regions: DirtyRect[] = [
  { x: 0, y: 0, w: 320, h: 240 },
  { x: 320, y: 0, w: 320, h: 240 }
];

await this.session.renderDirtyRegions(buffer, 640, 480, regions);
await this.session.presentFrame();
```

### 4. Tile Render (Parallel Multi-Tile)

```typescript
const tileW = 160, tileH = 120;
const tile1Buf = new ArrayBuffer(tileW * tileH * 4);
const tile2Buf = new ArrayBuffer(tileW * tileH * 4);

// Fill tile buffers...
new Uint32Array(tile1Buf).fill(0xFF0000FF); // Red
new Uint32Array(tile2Buf).fill(0xFF00FF00); // Green

// Submit all tiles in one call, auto-present
await this.session.renderTileRegions([
  { ratioX: 0.0, ratioY: 0.0, tilePixelWidth: tileW, tilePixelHeight: tileH, pixelData: tile1Buf },
  { ratioX: 0.5, ratioY: 0.0, tilePixelWidth: tileW, tilePixelHeight: tileH, pixelData: tile2Buf }
], 640, 480, true);
```

## API Reference

### Enums

#### PixelFormat

| Value | Description |
|-------|-------------|
| `RGBA` (0) | RGBA 8-bit per channel |
| `RGB` (1) | RGB 8-bit per channel |
| `BGRA` (2) | BGRA 8-bit per channel |
| `NV21` (3) | Android YUV NV21 format |
| `NV12` (4) | YUV NV12 format |

### Interfaces

#### SurfaceViewConfig

| Property | Type | Default | Description |
|---|---|---|---|
| renderWidth | number | 640 | Render buffer width in pixels |
| renderHeight | number | 480 | Render buffer height in pixels |
| format | PixelFormat | RGBA | Pixel format for rendering |
| originRatioX | number | 0.0 | Viewport X position as screen width ratio |
| originRatioY | number | 0.0 | Viewport Y position as screen height ratio |
| sizeRatioX | number | 1.0 | Viewport width as screen width ratio |
| sizeRatioY | number | auto | Viewport height as screen height ratio (auto = aspect-fit) |
| enableTouch | boolean | true | Enable touch event capture overlay |
| xComponentId | string | 'arkzero_surface' | XComponent instance ID |

#### NormalizedTouchEvent

| Property | Type | Description |
|---|---|---|
| type | TouchType | Down / Move / Up |
| touches | TouchPoint[] | Active touch points |

#### TouchPoint

| Property | Type | Description |
|---|---|---|
| id | number | Touch point ID |
| ratioX | number | X position as 0.0~1.0 ratio of component width |
| ratioY | number | Y position as 0.0~1.0 ratio of component height |
| pixelX | number | X in renderWidth pixel coordinates |
| pixelY | number | Y in renderHeight pixel coordinates |

#### DirtyRect

| Property | Type | Description |
|---|---|---|
| x | number | Region X offset in pixels |
| y | number | Region Y offset in pixels |
| w | number | Region width in pixels |
| h | number | Region height in pixels |

#### TileRegion

| Property | Type | Description |
|---|---|---|
| ratioX | number | Frame X position as 0.0~1.0 ratio of frameWidth |
| ratioY | number | Frame Y position as 0.0~1.0 ratio of frameHeight |
| tilePixelWidth | number | Tile width in pixels |
| tilePixelHeight | number | Tile height in pixels |
| pixelData | ArrayBuffer | Tile pixel data (format matches session) |

#### ArkZeroRendererConfig

| Property | Type | Description |
|---|---|---|
| width | number | Render buffer width |
| height | number | Render buffer height |
| format | PixelFormat | Pixel format |
| surfaceId | string | (optional) Surface ID for deferred init |

#### RenderSessionConfig

| Property | Type | Description |
|---|---|---|
| width | number | Render buffer width |
| height | number | Render buffer height |
| format | PixelFormat | Pixel format |
| onError | (msg: string) => void | (optional) Error callback |

#### DisposeAsyncOptions

| Property | Type | Description |
|---|---|---|
| wait | boolean | (optional) Wait for render thread drain |
| timeoutMs | number | (optional) Drain timeout in ms |

### Classes

#### ArkZeroRenderer

Low-level renderer with direct NAPI binding.

| Method | Returns | Description |
|---|---|---|
| `initialize(surfaceId)` | Promise\<void\> | Initialize renderer with XComponent surface ID |
| `renderFrame(pixelData, width, height)` | Promise\<void\> | Upload full frame and present (swap=true) |
| `renderFrameRegions(pixelData, frameWidth, frameHeight, regions, swap)` | Promise\<void\> | Upload dirty regions; if swap=true, present immediately |
| `renderTileRegions(tiles, frameWidth, frameHeight, swap)` | Promise\<void\> | Upload tile regions; if swap=true, present immediately |
| `updateDirtyRegions(pixelData, frameWidth, frameHeight, regions)` | Promise\<void\> | Upload dirty regions without presenting (swap=false) |
| `presentFrame()` | Promise\<void\> | Present the current texture (SwapAndPresent) |
| `resize(width, height)` | Promise\<void\> | Resize render buffer |
| `setVSync(enabled)` | Promise\<void\> | Enable/disable VSync |
| `setOnFrameRendered(callback)` | void | Set callback invoked after each presented frame |
| `dispose()` | void | Release all resources synchronously |
| `disposeAsync(options?)` | Promise\<void\> | Release resources (async variant) |

#### ArkZeroRenderSession

High-level session wrapper with error handling and first-frame buffering.

| Method / Property | Returns | Description |
|---|---|---|
| `attach(surfaceId)` | Promise\<void\> | Initialize and attach to surface; flushes any pending first frame |
| `renderFullFrame(pixelData, width, height)` | Promise\<void\> | Render full frame (auto-presents) |
| `renderDirtyRegions(pixelData, frameWidth, frameHeight, regions)` | Promise\<void\> | Update dirty regions without presenting |
| `renderTileRegions(tiles, frameWidth, frameHeight, swap)` | Promise\<void\> | Render tile regions; swap controls presentation |
| `presentFrame()` | Promise\<void\> | Present current frame |
| `resize(width, height)` | Promise\<void\> | Resize render buffer |
| `setVSync(enabled)` | Promise\<void\> | Enable/disable VSync |
| `detach()` | void | Release renderer and clear state |
| `ready` | boolean | Whether session is initialized and attached |
| `width` | number | Current render buffer width |
| `height` | number | Current render buffer height |

#### ArkZeroSurfaceView

ArkUI component providing an XComponent surface with ratio-based viewport layout and optional touch overlay.

| Prop / Callback | Type | Description |
|---|---|---|
| `config` | SurfaceViewConfig | Surface configuration |
| `onSurfaceLoaded` | (surfaceId: string) => void | Called when XComponent surface is ready |
| `touchHandler` | (event: NormalizedTouchEvent) => void | (optional) Touch event handler |

## Render Pipeline

```
ArkTS call (renderFrame/renderTileRegions/etc.)
    │
    ├─ napi_ref ← prevents ArrayBuffer GC
    ├─ raw pointer + size enqueued
    │  (zero memcpy on JS thread)
    │
RenderThread dequeues command
    │
    ├─ srcData → GLESBackend::Upload* directly
    │  (zero intermediate copy, GPU PBO upload)
    │
    ├─ if cmd.swapBuffers:
    │   └─ GLESBackend::SwapAndPresent()
    │       ├─ glViewport + Shader::Draw(m_texture)
    │       └─ eglSwapBuffers()
    │
    └─ CleanupDeferredCopy() → schedule napi_ref deletion
```

## Color Format (Little-Endian Uint32)

When using `Uint32Array` view of pixel buffers on little-endian devices:

```
Uint32 = A<<24 | B<<16 | G<<8 | R

Red    = 0xFF0000FF
Green  = 0xFF00FF00
Blue   = 0xFFFF0000
White  = 0xFFFFFFFF
Black  = 0xFF000000
Yellow = 0xFF00FFFF
Cyan   = 0xFFFFFF00
```

## License

Apache-2.0
