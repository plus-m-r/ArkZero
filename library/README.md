# @plusml/arkzero

Low-latency asynchronous graphics rendering library for OpenHarmony, provides proportional viewport management and standardized touch event processing.

**Full source code available at:** https://github.com/plus-m-r/ArkZero

The native `.so` files bundled in this package are built from the C++ source code in `src/main/cpp/` of this repository. Build instructions are documented in the project root README.

## Quick Start

```typescript
import { ArkZeroSurfaceView, SurfaceViewConfig, NormalizedTouchEvent } from '@plusml/arkzero';
import { ArkZeroRenderer, PixelFormat } from '@plusml/arkzero';

const config: SurfaceViewConfig = {
  renderWidth: 640,
  renderHeight: 480,
  format: PixelFormat.RGBA,
  originRatioX: 0.056,
  sizeRatioX: 1.0,
  enableTouch: true
};

const onTouch = (event: NormalizedTouchEvent) => {
  for (const touch of event.touches) {
    // touch.ratioX/Y: 0.0~1.0 normalized ratio (device-independent)
    // touch.pixelX/Y: pixel coordinates mapped to renderWidth/renderHeight
    drawCircle(pixelBuffer, touch.pixelX, touch.pixelY);
  }
};

ArkZeroSurfaceView({
  config: config,
  touchHandler: onTouch,
  onSurfaceLoaded: (surfaceId: string) => {
    renderer.initialize(surfaceId);
  }
})
```

## API Overview

### SurfaceViewConfig

| Property | Type | Default | Description |
|---|---|---|---|
| renderWidth | number | 640 | Render buffer width in pixels |
| renderHeight | number | 480 | Render buffer height in pixels |
| format | PixelFormat | RGBA | Pixel format (RGBA/RGB/BGRA/NV21/NV12) |
| originRatioX | number | 0.0 | Viewport X position as screen ratio |
| originRatioY | number | 0.0 | Viewport Y position as screen ratio |
| sizeRatioX | number | 1.0 | Viewport width as screen ratio |
| sizeRatioY | number | auto | Viewport height as screen ratio (auto = aspect-fit) |
| enableTouch | boolean | true | Enable normalized touch capture |
| xComponentId | string | 'arkzero_surface' | XComponent instance ID |

### NormalizedTouchEvent

| Property | Type | Description |
|---|---|---|
| type | TouchType | Down/Move/Up |
| touches | TouchPoint[] | Active touch points |

### TouchPoint

| Property | Type | Description |
|---|---|---|
| id | number | Touch point ID |
| ratioX | number | X position as 0.0~1.0 ratio of component width |
| ratioY | number | Y position as 0.0~1.0 ratio of component height |
| pixelX | number | X in renderWidth pixel coordinates |
| pixelY | number | Y in renderHeight pixel coordinates |

### ArkZeroRenderer

| Method | Description |
|---|---|
| initialize(surfaceId) | Initialize renderer with surface ID |
| renderFrame(data, w, h) | Render full frame |
| renderFrameRegions(data, fw, fh, regions, swap) | Render dirty regions |
| updateDirtyRegions(data, fw, fh, regions) | Update dirty regions without presenting |
| presentFrame() | Present accumulated frame |
| resize(w, h) | Resize render buffer |
| dispose() / disposeAsync() | Release resources |

## License

Apache-2.0
