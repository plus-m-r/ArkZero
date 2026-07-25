# ArkZero API 参考文档

## 概述

ArkZero 提供三层 API：

1. **ArkZeroSurfaceView**（`ArkZeroSurfaceView.ets`）— 视口组件，比例布局 + 归一化触控
2. **ArkZeroRenderer**（`ArkZeroRenderer.ets`）— 渲染器封装，类型安全，生命周期管理
3. **NAPI 模块**（`libnativerender.so`）— 底层，handle 直操作，用于测试或特殊场景

---

## ArkZeroSurfaceView

**文件**：`entry/src/main/ets/components/rendering/ArkZeroSurfaceView.ets`

### 类型定义

```typescript
interface SurfaceViewConfig {
  renderWidth: number;      // 渲染内容宽度（像素），定义 pixelX 坐标空间
  renderHeight: number;     // 渲染内容高度（像素），定义 pixelY 坐标空间
  format: PixelFormat;      // 像素格式
  originRatioX?: number;    // 组件左上角 X = 屏幕宽度 × 此值（默认 0.0，可超过 1.0）
  originRatioY?: number;    // 组件左上角 Y = 屏幕高度 × 此值（默认 0.0）
  sizeRatioX?: number;      // 组件宽度 = 屏幕宽度 × 此值（默认 1.0）
  sizeRatioY?: number;      // 组件高度（省略则按 renderWidth/renderHeight 宽高比自动计算，
                            // 若超出屏幕高度则 fit-to-screen）
  enableTouch?: boolean;    // 启用触控捕获（默认 true）
  xComponentId?: string;    // XComponent ID（默认 'arkzero_surface'）
}

interface TouchPoint {
  id: number;              // 触摸点 ID
  ratioX: number;          // 0.0~1.0 相对于组件面积的水平位置
  ratioY: number;          // 0.0~1.0 相对于组件面积的垂直位置
  pixelX: number;          // ratioX × renderWidth（pixelBuffer 直接可用）
  pixelY: number;          // ratioY × renderHeight（pixelBuffer 直接可用）
}

interface NormalizedTouchEvent {
  type: TouchType;         // Down / Move / Up / Cancel
  touches: TouchPoint[];   // 所有活跃触摸点（归一化坐标）
}
```

### ArkZeroSurfaceView 组件

```typescript
@Component
export struct ArkZeroSurfaceView {
  @Prop config: SurfaceViewConfig;
  touchHandler?: (event: NormalizedTouchEvent) => void;
  onSurfaceLoaded?: (surfaceId: string) => void;
}
```

#### 使用示例

```typescript
// 全屏视口，640×480 渲染内容
ArkZeroSurfaceView({
  config: {
    renderWidth: 640,
    renderHeight: 480,
    format: PixelFormat.RGBA,
    sizeRatioX: 1.0,  // 占满屏幕宽度
    enableTouch: true
  },
  touchHandler: (event: NormalizedTouchEvent) => {
    // event.touches[0].pixelX/Y 直接对应 640×480 pixelBuffer 坐标
    // event.touches[0].ratioX/Y 跨设备通用的归一化比例
  },
  onSurfaceLoaded: (surfaceId: string) => {
    renderer.initialize(surfaceId);
  }
})

// 小窗口视口，右下角 30% 屏幕区域
ArkZeroSurfaceView({
  config: {
    renderWidth: 320,
    renderHeight: 240,
    format: PixelFormat.RGBA,
    originRatioX: 0.7,
    originRatioY: 0.7,
    sizeRatioX: 0.3,
    enableTouch: true
  },
  touchHandler: (event) => { ... },
  onSurfaceLoaded: (surfaceId) => { ... }
})
```

#### 视口布局算法

1. 获取屏幕 vp 尺寸：`display.getDefaultDisplaySync().width / densityPixels`
2. 计算组件 vp 尺寸：
   - `componentVpW = screenVpW × sizeRatioX`
   - 若指定 `sizeRatioY`：`componentVpH = screenVpH × sizeRatioY`
   - 若省略：`componentVpH = componentVpW × (renderHeight / renderWidth)`，若超出屏幕高度则 fit-to-screen
3. 计算位置：`posX = screenVpW × originRatioX`

#### 触控坐标映射

1. `onAreaChange` 测量组件实际 vp 尺寸
2. 预计算缩放因子：`scaleX = renderWidth / areaWidth`，`scaleY = renderHeight / areaHeight`
3. 触摸回调中：`pixelX = floor(touch.x × scaleX)`，`ratioX = touch.x / areaWidth`

---

## ArkZeroRenderer

**文件**：`entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`

### 类型定义

```typescript
enum PixelFormat {
  RGBA = 0,   // RGBA8888 (4字节/像素)
  RGB = 1,    // RGB888 (3字节/像素)
  BGRA = 2,   // BGRA8888 (4字节/像素)
  NV21 = 3,   // YUV420半平面 (1.5字节/像素)
  NV12 = 4    // YUV420半平面 (1.5字节/像素)
}

interface ArkZeroRendererConfig {
  width: number;
  height: number;
  format: PixelFormat;
  surfaceId?: string;
}

interface DirtyRect {
  x: number;
  y: number;
  w: number;
  h: number;
}

interface DisposeAsyncOptions {
  wait?: boolean;
  timeoutMs?: number;
}
```

### ArkZeroRenderer 类

#### constructor(config: ArkZeroRendererConfig)

创建渲染器实例。需调用 `initialize()` 后才能使用。

```typescript
const renderer = new ArkZeroRenderer({
  width: 1920,
  height: 1080,
  format: PixelFormat.RGBA
});
```

#### async initialize(surfaceId: string): Promise\<void\>

初始化渲染器，创建 EGL 上下文和 GPU 资源。

- `surfaceId`：从 XComponent `onLoad` 回调获取
- 内部调用 `nativerender.create(surfaceId, width, height, format)` 获取 handle

```typescript
await renderer.initialize(surfaceId);
```

#### setOnFrameRendered(callback: () => void): void

设置帧渲染完成回调。在 `renderFrame`、`renderFrameRegions(swap=true)`、`presentFrame` 完成后触发。

```typescript
renderer.setOnFrameRendered(() => {
  console.log('Frame rendered');
});
```

#### async renderFrame(pixelData: ArrayBuffer, width: number, height: number): Promise\<void\>

全帧渲染。上传完整像素数据到 GPU 纹理，绘制并交换缓冲区。

- `pixelData`：像素数据，ArrayBuffer 引用传递
- RGBA: width × height × 4 字节
- NV21/NV12: width × height × 1.5 字节
- 异步执行：NAPI async work → RenderThread 命令队列

```typescript
await renderer.renderFrame(pixelData, 1920, 1080);
```

#### async renderFrameRegions(pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: DirtyRect[], swap?: boolean): Promise\<void\>

脏区渲染（一步式）。上传脏区像素，可选是否立即交换缓冲区。

- `regions`：脏区矩形数组，坐标为像素坐标（左上角为原点）
- `swap`：默认 `true`，为 `false` 时仅上传不交换（手动调用 `presentFrame`）

```typescript
await renderer.renderFrameRegions(pixelData, 1920, 1080, [
  { x: 100, y: 50, w: 200, h: 150 }
], true);
```

#### async updateDirtyRegions(pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: DirtyRect[]): Promise\<void\>

分离式脏区上传。仅上传脏区像素到 GPU 纹理，不绘制也不交换缓冲区。

用于累积模式：多次 `updateDirtyRegions` 后一次 `presentFrame`。

```typescript
for (let i = 0; i < 10; i++) {
  await renderer.updateDirtyRegions(data, 1920, 1080, dirtyRects);
}
await renderer.presentFrame();
```

#### async presentFrame(): Promise\<void\>

绘制当前纹理到屏幕并交换缓冲区（`eglSwapBuffers`）。触发 VSync 等待。

```typescript
await renderer.presentFrame();
```

#### async resize(width: number, height: number): Promise\<void\>

调整渲染目标尺寸。同步执行（不频繁，阻塞可接受）。

```typescript
await renderer.resize(1280, 720);
```

#### dispose(): void

同步销毁渲染器。

```typescript
renderer.dispose();
```

#### async disposeAsync(options?: DisposeAsyncOptions): Promise\<void\>

异步销毁。等待飞行渲染完成或超时后销毁。

```typescript
await renderer.disposeAsync({ wait: true, timeoutMs: 3000 });
```

---

## NAPI 模块层

**文件**：`entry/src/main/cpp/types/libnativerender/Index.d.ts`

### 渲染器核心 API

```typescript
declare module 'libnativerender.so' {
  export function create(surfaceId: string, width: number, height: number, format: number): Promise<number>;
  export function renderFrame(handle: number, pixelData: ArrayBuffer, width: number, height: number): Promise<void>;
  export function renderFrameRegions(handle: number, pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: Array<DirtyRect>, swap: boolean): Promise<void>;
  export function updateDirtyRegions(handle: number, pixelData: ArrayBuffer, frameWidth: number, frameHeight: number, regions: Array<DirtyRect>): Promise<void>;
  export function presentFrame(handle: number): Promise<void>;
  export function resize(handle: number, width: number, height: number): Promise<void>;
  export function destroy(handle: number): Promise<void>;
}
```

### RendererManager API

```typescript
export function managerCreateSurfaceRenderer(surfaceId: string, width: number, height: number, format: number): number;
export function managerDestroyRenderer(handle: number): boolean;
```

### SurfaceManager API

```typescript
export function surfaceManagerCreateNativeWindow(surfaceId: string): number;
export function surfaceManagerDestroyNativeWindow(windowPtr: number): boolean;
```

### EGLContextManager API

```typescript
export function createEGLContext(width: number, height: number): number;
export function destroyEGLContext(handle: number): void;
export function eglMakeCurrent(handle: number): boolean;
export function isEGLInitialized(handle: number): boolean;
```

### GLESBackend API

```typescript
export function createGLESBackend(): number;
export function destroyGLESBackend(handle: number): void;
export function glesBackendInitialize(handle: number, width: number, height: number, format: number): boolean;
export function glesBackendRenderFrame(handle: number, pixelData: ArrayBuffer, width: number, height: number): boolean;
export function isGLESBackendInitialized(handle: number): boolean;
```

### TextureManager API

```typescript
export function createTextureManager(): number;
export function destroyTextureManager(handle: number): void;
export function textureCreate(handle: number, width: number, height: number, internalFormat: number, format: number): boolean;
export function textureUpdate(handle: number, pixelData: ArrayBuffer, width: number, height: number, format: number): boolean;
export function textureDestroy(handle: number): void;
export function getTextureId(handle: number): number;
export function isTextureCreated(handle: number): boolean;
```

### TexturePool API

```typescript
export function createTexturePool(maxSize?: number): number;
export function destroyTexturePool(handle: number): void;
export function texturePoolAcquire(handle: number, width: number, height: number, internalFormat: number, format: number): boolean;
export function texturePoolRelease(handle: number): void;
export function texturePoolPreallocate(handle: number, resolutions: Array<Resolution>, internalFormat: number, format: number): void;
export function texturePoolClear(handle: number): void;
export function texturePoolSize(handle: number): number;
export function texturePoolGetStats(handle: number): TexturePoolStats;
```

### PixelFormatConverter API

```typescript
export function getGLInternalFormat(format: number): number;
export function getGLFormat(format: number): number;
export function getBytesPerPixel(format: number): number;
```

### YUVShaderManager API

```typescript
export function createYUVShaderManager(): number;
export function destroyYUVShaderManager(handle: number): void;
export function yuvShaderInitialize(handle: number): boolean;
export function yuvShaderRenderNV21(handle: number, yBuffer: ArrayBuffer, uvBuffer: ArrayBuffer, width: number, height: number): boolean;
export function yuvShaderRenderNV12(handle: number, yBuffer: ArrayBuffer, uvBuffer: ArrayBuffer, width: number, height: number): boolean;
export function isYUVShaderInitialized(handle: number): boolean;
```

### PerformanceMonitor API

```typescript
export function createPerformanceMonitor(): number;
export function destroyPerformanceMonitor(handle: number): void;
export function monitorBeginFrame(handle: number): void;
export function monitorEndFrame(handle: number, dropped: boolean): void;
export function monitorReset(handle: number): void;
export function getMonitorStats(handle: number): MonitorStats;
```

### NAPI 类型

```typescript
interface DirtyRect {
  x: number;
  y: number;
  w: number;
  h: number;
}

interface Resolution {
  width: number;
  height: number;
}

interface TexturePoolStats {
  maxSize: number;
  currentSize: number;
  totalAcquires: number;
  totalReleases: number;
  cacheHits: number;
  cacheMisses: number;
}

interface MonitorStats {
  totalFrames: number;
  droppedFrames: number;
  averageFrameTimeMs: number;
  maxFrameTimeMs: number;
  fps: number;
}
```

---

## C++ 层关键类

### RenderThread

**文件**：`renderer/core/RenderThread.h/.cpp`

专用渲染线程，拥有 GLESBackend，执行所有 GL 操作。

命令类型：INIT, RENDER_FRAME, RENDER_FRAME_REGIONS, UPDATE_DIRTY, PRESENT_FRAME, RESIZE, DESTROY, SHUTDOWN

线程模型：
- `EnqueueCommand()` → 加锁入队 + `notify_one()` → 返回 `future<bool>`
- `ThreadLoop()` → `wait()` 取命令 → 执行 → `completion.set_value()`
- NAPI async work 的 execute 回调在 libuv worker 线程上 `future.get()` 等待完成

### Renderer（外观类）

**文件**：`renderer/core/Renderer.h/.cpp`

通过 `m_renderThread` 异步执行所有渲染操作。方法返回 `std::future<bool>`。

### GLESBackend（Facade）

**文件**：`renderer/backend/GLESBackend.h`

协调 EGL、Texture、Shader、Pool 等组件，实现 `IRenderBackend` 接口。

关键方法：
- `RenderFrame()` — 全帧上传 + 绘制 + SwapBuffers
- `RenderFrameRegions()` — 脏区上传 + 绘制 + 可选 SwapBuffers
- `UpdateDirtyRegions()` — 仅脏区上传（从 Pool Acquire → TextureManager::UpdateRegion → Pool Release）
- `PresentFrame()` — 绘制 + SwapBuffers

### EGLContextManager

**文件**：`renderer/backend/EGLContextManager.h`

关键特性：
- `m_ownerThread` — 线程所有权跟踪，`ReleaseCurrent()` 释放当前线程绑定
- `eglSwapInterval(display, 1)` — VSync 启用
- `IsSurfaceInvalidated()` — 检测 Surface 是否有效

### TexturePool

**文件**：`renderer/backend/TexturePool.h`

- `Acquire(w, h, intFmt, fmt)` — 获取纹理（从池或新建）
- `Release(texture)` — 归还纹理到池
- `Preallocate(resolutions, ...)` — 预分配常用分辨率
- **不变量**：每次 Acquire 必须在下次 Acquire 前 Release

---

## 使用注意事项

### 触控与渲染的单一职责

ArkZeroSurfaceView 的触控叠加层只负责捕获触摸事件并输出归一化坐标，不负责视觉渲染。触控反馈应通过回调传出 → 写入 pixelBuffer → 渲染器渲染。

### DirtyRect 坐标系

- 原点在左上角
- Y 翻转由 Native 层自动处理：`glY = frameHeight - screenY - regionH`
- stride 使用 `glPixelStorei(GL_UNPACK_ROW_LENGTH, frameWidth)` 处理

### 累积模式

累积模式是最大性能增益来源，适用于：
- 远程桌面/屏幕共享（多小区域变化 → 一次刷新）
- 视频播放（多帧解码 → 一次上屏）

```typescript
for (let i = 0; i < N; i++) {
  await renderer.updateDirtyRegions(data, w, h, dirtyRects);
}
await renderer.presentFrame();
```

### 10% 脏区盈亏平衡

增量渲染在脏区 < 10% 时有正收益，超过 10% 开销抵消收益。大范围变化应使用 `renderFrame`。

### aboutToDisappear 中不要用 async IIFE

ArkTS 组件可能在 async 完成前被回收。使用 `.then()` 链替代 fire-and-forget async IIFE。

### XComponent libraryname 限制

`napi_init.cpp` 中的 `g_nativeXComponent` / `g_xcomponentId` 是全局静态变量，仅支持一个 XComponent。ArkZeroSurfaceView 不使用 libraryname，通过 surfaceId 方式创建。
