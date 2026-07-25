# ArkZero API 参考文档

## 概述

ArkZeroRenderer 提供两层 API：

1. **ArkTS 封装层**（`ArkZeroRenderer.ets`）— 推荐，类型安全，生命周期管理
2. **NAPI 模块**（`libnativerender.so`）— 底层，handle 直操作，用于测试或特殊场景

---

## ArkTS 封装层

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

- `pixelData`：像素数据，ArrayBuffer 引用传递（零拷贝）
- RGBA: width × height × 4 字节
- NV21/NV12: width × height × 1.5 字节

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

调整渲染目标尺寸。内部重建 EGL Surface 和纹理。

```typescript
await renderer.resize(1280, 720);
```

#### dispose(): void

同步安全销毁。若有飞行中的渲染（`pendingRenders > 0`），标记 `destroyScheduled`，待渲染完成后自动销毁。设置 2 秒安全超时。

```typescript
renderer.dispose();
```

#### async disposeAsync(options?: DisposeAsyncOptions): Promise\<void\>

异步安全销毁。等待飞行渲染完成或超时后销毁。

- `options.wait`：是否等待（默认 `true`）
- `options.timeoutMs`：超时毫秒（默认 `2000`）

```typescript
await renderer.disposeAsync({ wait: true, timeoutMs: 3000 });
```

#### async awaitIdle(timeoutMs?: number): Promise\<void\>

等待所有飞行中的渲染完成。超时时 reject。

```typescript
await renderer.awaitIdle(5000);
```

### 生命周期管理

ArkZeroRenderer 内部维护 `pendingRenders` 计数器：

- `renderFrame`、`renderFrameRegions(swap=true)`、`presentFrame` 进入时 +1，完成时 -1
- `updateDirtyRegions` 不计入（无 SwapBuffers）
- `pendingRenders === 0` 时触发 `idleWaiters` 和 `destroyScheduled` 检查

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

### Renderer（外观类）

**文件**：`renderer/core/Renderer.h`

```cpp
class Renderer {
public:
  Renderer(int width, int height, PixelFormat format);
  bool Initialize(void* nativeWindow);
  void RenderFrame(const void* data, size_t size, int w, int h);
  void RenderFrameRegions(const void* data, size_t size, int fw, int fh, const std::vector<DirtyRect>& regions, bool swap);
  void UpdateDirtyRegions(const void* data, size_t size, int fw, int fh, const std::vector<DirtyRect>& regions);
  void PresentFrame();
  void Resize(int w, int h);
  void Destroy();
  bool IsInitialized() const;
};
```

### GLESBackend（Facade）

**文件**：`renderer/backend/GLESBackend.h`

协调 EGL、Texture、Shader、Pool 等组件，实现 `IRenderBackend` 接口。

关键方法：
- `RenderFrame()` — 全帧上传 + 绘制 + SwapBuffers
- `RenderFrameRegions()` — 脏区上传 + 绘制 + 可选 SwapBuffers
- `UpdateDirtyRegions()` — 仅脏区上传（从 Pool Acquire → TextureManager::UpdateRegion → Pool Release）
- `PresentFrame()` — 绘制 + SwapBuffers

### TextureManager

**文件**：`renderer/backend/TextureManager.h`

```cpp
class TextureManager {
public:
  bool Create(int w, int h, int internalFormat, int format);
  void Destroy();
  void Update(const void* data, int w, int h, int format);           // 全帧更新
  void UpdateRegion(const void* data, int fw, int fh, int format,    // 脏区更新
                    int x, int y, int regionW, int regionH);
  GLuint GetTextureId() const;
  bool IsCreated() const;
};
```

`UpdateRegion` 内部使用：
- `glPixelStorei(GL_UNPACK_ROW_LENGTH, frameWidth)` — 设置行跨度
- `glPixelStorei(GL_UNPACK_SKIP_ROWS, glY)` / `GL_UNPACK_SKIP_PIXELS, glX` — 设置偏移
- `glTexSubImage2D(...)` — 上传子区域

### EGLContextManager

**文件**：`renderer/backend/EGLContextManager.h`

关键特性：
- `m_isCurrent` 标志 — 跳过已当前的 `eglMakeCurrent`
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

ArkTS 组件可能在 async 完成前被回收。使用 `.then()` 链替代 fire-and-forget async IIFE：

```typescript
aboutToDisappear() {
  this.running = false;
  this.renderer.dispose();  // 同步标记销毁
}
```

### XComponent libraryname 限制

`napi_init.cpp` 中的 `g_nativeXComponent` / `g_xcomponentId` 是全局静态变量，仅支持一个 XComponent。多页面场景应使用 surfaceId 方式（不指定 `libraryname`）。
