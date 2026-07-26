# ArkZero API 参考文档

## 概述

ArkZero 提供三层 API：

1. **ArkZeroSurfaceView**（`ArkZeroSurfaceView.ets`）— 视口组件，比例布局 + 归一化触控
2. **ArkZeroRenderer**（`ArkZeroRenderer.ets`）— 渲染器封装，类型安全，生命周期管理
3. **ArkZeroRenderSession**（`ArkZeroRenderSession.ets`）— 会话层，自动生命周期管理

所有渲染方法采用 **fire-and-forget** 模式：JS 调用立即返回，VSync 等待在常驻渲染线程自动完成。

---

## ArkZeroSurfaceView

### 类型定义

```typescript
interface SurfaceViewConfig {
  renderWidth: number;      // 渲染内容宽度（像素），定义 pixelX 坐标空间
  renderHeight: number;     // 渲染内容高度（像素），定义 pixelY 坐标空间
  format: PixelFormat;      // 像素格式
  originRatioX?: number;    // 组件左上角 X = 屏幕宽度 × 此值（默认 0.0）
  originRatioY?: number;    // 组件左上角 Y = 屏幕高度 × 此值（默认 0.0）
  sizeRatioX?: number;      // 组件宽度 = 屏幕宽度 × 此值（默认 1.0）
  sizeRatioY?: number;      // 组件高度（省略则按宽高比自动计算，fit-to-screen）
  enableTouch?: boolean;    // 启用触控捕获（默认 true）
  xComponentId?: string;    // XComponent ID（默认 'arkzero_surface'）
}

interface TouchPoint {
  id: number;
  ratioX: number;          // 0.0~1.0 相对于组件面积的水平位置
  ratioY: number;          // 0.0~1.0 相对于组件面积的垂直位置
  pixelX: number;          // ratioX × renderWidth（pixelBuffer 直接可用）
  pixelY: number;          // ratioY × renderHeight（pixelBuffer 直接可用）
}

interface NormalizedTouchEvent {
  type: TouchType;
  touches: TouchPoint[];
}
```

---

## ArkZeroRenderer

### 类型定义

```typescript
enum PixelFormat {
  RGBA = 0,   // RGBA8888 (4字节/像素)
  RGB = 1,    // RGB888 (3字节/像素)
  BGRA = 2,   // BGRA8888 (4字节/像素)
  NV21 = 3,   // YUV420半平面 (1.5字节/像素)
  NV12 = 4    // YUV420半平面 (1.5字节/像素)
}

interface DirtyRect {
  x: number;
  y: number;
  w: number;
  h: number;
}

interface TileRegion {
  ratioX: number;          // 帧内 X 位置（0.0~1.0 比例）
  ratioY: number;          // 帧内 Y 位置（0.0~1.0 比例）
  tilePixelWidth: number;  // 瓦片宽度（像素）
  tilePixelHeight: number; // 瓦片高度（像素）
  pixelData: ArrayBuffer;  // 瓦片像素数据
}
```

### 方法

#### async initialize(surfaceId: string): Promise\<void\>

初始化渲染器，创建 EGL 上下文和 GPU 资源。

#### async renderFrame(pixelData: ArrayBuffer, width: number, height: number): Promise\<void\>

全帧渲染。fire-and-forget：立即返回，渲染线程自动 Upload + SwapAndPresent。

#### async renderFrameRegions(pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: DirtyRect[], swap?: boolean): Promise\<void\>

脏区渲染。`swap=false` 时仅上传不交换。

#### async renderTileRegions(tiles: TileRegion[], frameWidth: number, frameHeight: number, swap?: boolean): Promise\<void\>

瓦片渲染。每个瓦片独立上传到帧内比例位置，跳过全帧拼装。C++ 原生实现。

```typescript
const tile: TileRegion = {
  ratioX: 0.25,
  ratioY: 0.0,
  tilePixelWidth: 320,
  tilePixelHeight: 240,
  pixelData: tileBuffer
};
await renderer.renderTileRegions([tile], 640, 480, true);
```

#### async updateDirtyRegions(pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: DirtyRect[]): Promise\<void\>

分离式脏区上传。仅上传到 Back 纹理，不绘制不交换。

#### async presentFrame(): Promise\<void\>

绘制当前 Front 纹理到屏幕 + SwapBuffers（触发 VSync 等待）。

#### async setVSync(enabled: boolean): Promise\<void\>

启用/禁用 VSync。默认启用，关闭可能导致移动 GPU 管线阻塞。

#### async resize(width: number, height: number): Promise\<void\>

调整渲染目标尺寸。会释放并重建双缓冲纹理。

#### dispose(): void / async disposeAsync(options?): Promise\<void\>

销毁渲染器，释放所有 GPU 资源。

---

## ArkZeroRenderSession

会话层，自动管理 renderer 生命周期和错误处理。

```typescript
const session = new ArkZeroRenderSession({ width: 640, height: 480, format: PixelFormat.RGBA });
await session.attach(surfaceId);
await session.renderTileRegions(tiles, 640, 480);
await session.detach();
```

---

## NAPI 模块层

```typescript
declare module 'libnativerender.so' {
  export function create(surfaceId: string, width: number, height: number, format: number): Promise<number>;
  export function renderFrame(handle: number, pixelData: ArrayBuffer, width: number, height: number): Promise<void>;
  export function renderFrameRegions(handle: number, pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: Array<DirtyRect>, swap: boolean): Promise<void>;
  export function renderTileRegions(handle: number, tiles: Array<TileRegion>, frameWidth: number, frameHeight: number, swap: boolean): Promise<void>;
  export function updateDirtyRegions(handle: number, pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: Array<DirtyRect>): Promise<void>;
  export function presentFrame(handle: number): Promise<void>;
  export function resize(handle: number, width: number, height: number): Promise<void>;
  export function setVSync(handle: number, enabled: boolean): Promise<void>;
  export function destroy(handle: number): Promise<void>;
}
```

---

## C++ 层架构

### Fire-and-Forget 数据流

```
JS 线程                              RenderThread (常驻)
─────────────                        ─────────────────
renderFrame(data)
  ↓ memcpy → cmd.pixelData
  ↓ enqueue(cmd) ──────────────→    dequeue(cmd)
  ↓ resolve(promise)                 UploadFrame(cmd.data) → Back纹理
  ↓ return                           SwapAndPresent()
                                       ↳ swap(Front, Back)
                                       ↳ Draw(Front)
                                       ↳ SwapBuffers() → VSync
                                       ↳ Acquire new Back
```

### GLESBackend 双缓冲

| 纹理 | 职责 | 操作 |
|------|------|------|
| Back | 上传目标 | Upload*() 写入，标记 m_backDirty |
| Front | 显示目标 | SwapAndPresent() 时 Draw + SwapBuffers |

SwapAndPresent 流程：
1. `swap(m_frontTexture, m_backTexture)` — 指针交换，零拷贝
2. `m_textureShader.Draw(frontTexture)` — 绘制 Front 到屏幕
3. `m_eglManager.SwapBuffers()` — VSync 等待
4. `m_textureStrategy->Acquire()` — 分配新 Back 纹理

---

## 性能参考

MatePad Pro 13 模拟器（软件 GPU）：

| 场景 | JS 感知延迟 |
|------|------------|
| renderFrame | ~1ms |
| renderTileRegions (单瓦片) | ~1ms |
| renderTileRegions (4瓦片) | ~4ms |
| updateDirtyRegions + presentFrame | ~1ms + ~1ms |

真实设备预期更低。
