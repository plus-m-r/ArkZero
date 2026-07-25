# ArkZero 架构说明

## 目录结构

```
entry/src/main/cpp/
├── napi_init.cpp                    # NAPI 模块注册入口
├── common/
│   └── common.h                     # DirtyRect 等公共类型
├── napi_bridge/                     # NAPI 底层桥接（各组件独立绑定）
│   ├── egl_context_manager_napi.h/.cpp
│   ├── gles_backend_napi.h/.cpp
│   ├── performance_monitor_napi.h/.cpp
│   ├── pixel_format_converter_napi.h/.cpp
│   ├── renderer_manager_napi.h/.cpp
│   ├── surface_manager_napi.h/.cpp
│   ├── texture_manager_napi.h/.cpp
│   ├── texture_pool_napi.h/.cpp
│   └── yuv_shader_manager_napi.h/.cpp
├── renderer/
│   ├── api/
│   │   └── RendererApi.h/.cpp       # NAPI 高层 API（异步 work + deferred promise）
│   ├── backend/
│   │   ├── IRenderBackend.h         # 渲染后端接口
│   │   ├── GLESBackend.h/.cpp       # Facade：协调所有后端组件
│   │   ├── EGLContextManager.h/.cpp # EGL 上下文生命周期 + 线程所有权
│   │   ├── TextureManager.h/.cpp    # 纹理创建/全帧更新/脏区更新
│   │   ├── TextureShader.h/.cpp     # 纹理绘制 Shader
│   │   ├── TexturePool.h/.cpp       # 纹理池（Acquire/Release/Preallocate）
│   │   ├── PixelFormatConverter.h/.cpp # 格式映射工具
│   │   ├── YUVShaderManager.h/.cpp  # YUV→RGB GPU Shader
│   │   ├── factory/
│   │   │   └── TextureFactory.h/.cpp
│   │   └── strategy/
│   │       ├── ITextureStrategy.h
│   │       ├── DirectTextureStrategy.h/.cpp
│   │       └── PoolTextureStrategy.h/.cpp
│   ├── core/
│   │   ├── Renderer.h/.cpp          # 外观类，通过 RenderThread 异步执行
│   │   ├── RenderThread.h/.cpp      # 专用渲染线程 + 命令队列
│   │   ├── PerformanceMonitor.h/.cpp # 性能统计
│   │   └── command/
│   │       ├── IRenderCommand.h
│   │       ├── RenderFrameCommand.h/.cpp
│   │       └── PreallocateTextureCommand.h/.cpp
│   └── manager/
│       ├── RendererManager.h/.cpp   # 渲染器实例管理（handle 映射，线程安全）
│       └── SurfaceManager.h/.cpp    # NativeWindow 管理（surfaceId → window）
└── types/
    └── libnativerender/
        └── Index.d.ts               # TypeScript 类型声明
```

```
entry/src/main/ets/
├── entryability/
│   └── EntryAbility.ets
├── pages/
│   ├── Index.ets                    # 首页（导航按钮）
│   └── SurfaceDemoPage.ets          # XComponent 演示页
├── components/rendering/
│   ├── ArkZeroRenderer.ets          # ArkTS 渲染器封装层
│   └── ArkZeroSurfaceView.ets       # 视口组件（比例布局 + 归一化触控）
└── integration/
    ├── observer/                    # XComponent 生命周期观察者
    ├── components/                  # 测试结果面板
    ├── pages/                       # IntegrationTestPage
    └── tests/                       # 集成测试 + 增量基准测试
```

## 五层架构

```
┌──────────────────────────────────────────────────────────────┐
│  ArkTS UI 层 (SurfaceDemoPage.ets)                           │
│  - 触控反馈绘制到 pixelBuffer                                  │
│  - 使用 touch.pixelX/Y 直接写入，零计算                        │
└──────────────┬───────────────────────────────────────────────┘
               │ NormalizedTouchEvent
┌──────────────▼───────────────────────────────────────────────┐
│  ArkTS 视口层 (ArkZeroSurfaceView.ets)                       │
│  - 比例视口布局：originRatio + sizeRatio                       │
│  - 归一化触控：ratioX/Y + pixelX/Y 便利字段                    │
│  - 预计算 scaleX/Y = renderWidth/Height / areaWidth/Height    │
│  - display.getDefaultDisplaySync() 获取屏幕尺寸                │
└──────────────┬───────────────────────────────────────────────┘
               │ import nativerender
┌──────────────▼───────────────────────────────────────────────┐
│  NAPI 异步层 (RendererApi.cpp)                               │
│  - napi_create_async_work：JS 线程复制数据后立即返回            │
│  - libuv worker 线程：enqueue 到 RenderThread, 等待 future    │
│  - 完成回调：resolve/reject deferred promise                   │
│  - 同步 API：Create/Resize/Destroy（不频繁，阻塞可接受）       │
└──────────────┬───────────────────────────────────────────────┘
               │ C++ 调用
┌──────────────▼───────────────────────────────────────────────┐
│  Core 层 (Renderer + RenderThread + Manager)                 │
│  - Renderer: 外观类，所有方法返回 future<bool>                 │
│  - RenderThread: 专用 std::thread + 命令队列                   │
│    - mutex/condition_variable 同步                             │
│    - promise/future 完成信号                                   │
│    - 拥有 GLESBackend，所有 GL 操作仅在此线程执行               │
│  - RendererManager: handle ↔ 实例映射（m_mutex 线程安全）      │
└──────────────┬───────────────────────────────────────────────┘
               │ 委托
┌──────────────▼───────────────────────────────────────────────┐
│  Backend 层 (GLESBackend + 组件)                              │
│  - GLESBackend: Facade 协调（仅 RenderThread 调用）            │
│  - EGLContextManager: EGL 生命周期 + m_ownerThread 所有权      │
│  - TextureManager: 纹理管理                                    │
│  - TextureShader: 绘制 Shader                                 │
│  - TexturePool + Strategy: 纹理复用                            │
│  - YUVShaderManager: YUV GPU 转换                             │
└──────────────────────────────────────────────────────────────┘
```

## 核心设计决策

### 1. 异步渲染线程

渲染在专用 `std::thread` 上执行，UI 线程零阻塞：

```
UI 线程 (TID 18966)                RenderThread (TID 19568)
─────────────────────              ─────────────────────────
renderFrame(data)                  
  ↓ memcpy(data) → cmd.pixelData   
  ↓ enqueue(cmd) ──────────────→   cmd = dequeue()
  ↓ return promise                  backend.RenderFrame(cmd.data)
                                    cmd.completion.set_value(true)
  ↓                                SwapBuffers() → VSync
napi async complete:                
  resolve(deferred)                 
```

**权衡**：像素数据在入队前被复制（640×480 RGBA ≈ 0.3ms），防止 JS 线程返回后 use-after-free。

### 2. 比例视口系统

ArkZeroSurfaceView 的位置和尺寸全部用屏幕比例表达：

```typescript
interface SurfaceViewConfig {
  renderWidth: number;      // 渲染内容宽度（像素，定义坐标空间）
  renderHeight: number;     // 渲染内容高度（像素）
  originRatioX?: number;    // 组件左上角 X 位置 = 屏幕宽度 × 此值（默认 0.0）
  originRatioY?: number;    // 组件左上角 Y 位置 = 屏幕高度 × 此值（默认 0.0）
  sizeRatioX?: number;      // 组件宽度 = 屏幕宽度 × 此值（默认 1.0）
  sizeRatioY?: number;      // 组件高度（省略则按 renderWidth/renderHeight 宽高比自动计算）
}
```

**fit-to-screen**：当 sizeRatioY 省略时，组件高度按渲染内容宽高比计算；若超出屏幕高度，则以高度为基准重算宽度，确保组件完整可见。

**跨设备一致性**：同一份配置在不同屏幕尺寸上自动适配，无需硬编码像素值。

### 3. 归一化触控输出

触摸事件输出为归一化数据，零计算即可使用：

```typescript
interface TouchPoint {
  id: number;
  ratioX: number;   // 0.0~1.0 相对于组件面积的水平位置
  ratioY: number;   // 0.0~1.0 相对于组件面积的垂直位置
  pixelX: number;   // ratioX × renderWidth（pixelBuffer 直接可用）
  pixelY: number;   // ratioY × renderHeight（pixelBuffer 直接可用）
}
```

**预计算优化**：`scaleX = renderWidth / areaWidth` 在 `onAreaChange` 时计算一次，触摸回调中直接 `pixelX = floor(touch.x × scaleX)`，省去先除再乘。

### 4. Handle 而非指针

ArkTS 层通过 `number` 类型 handle 引用 Native 对象，而非直接暴露 C++ 指针。

```typescript
const handle: number = await nativerender.create(surfaceId, 1920, 1080, format);
```

优势：
- 防止 ArkTS 直接操作 C++ 对象
- 可验证 handle 有效性
- RendererManager 通过 `unordered_map<handle, unique_ptr<Renderer>>` 管理生命周期

### 5. 分离式 API

```
updateDirtyRegions(data, w, h, regions)  → 仅上传纹理
presentFrame()                            → 绘制 + eglSwapBuffers
```

支持累积模式：多次 `updateDirtyRegions` + 一次 `presentFrame`，避免每帧 VSync 等待。

### 6. 纹理策略模式

GLESBackend 通过 `ITextureStrategy` 接口支持两种纹理获取方式：
- **DirectStrategy**：每次创建新纹理
- **PoolStrategy**：从 TexturePool 复用（默认）

```cpp
// TexturePool 不变量：每次 Acquire 必须在下次 Acquire 前 Release
// 缓存纹理会破坏此不变量，导致池耗尽和画面错乱
```

### 7. XComponent 集成

ArkZeroSurfaceView 内部使用 XComponent（不指定 libraryname），通过 surfaceId 方式创建 NativeWindow（多实例安全）。

### 8. VSync

生产模式启用 VSync：`eglSwapInterval(display, 1)`，每次 SwapBuffers 等待一次 VSync 信号（~14ms@60Hz）。

累积模式绕过此限制：N 次 update（无 VSync 等待）+ 1 次 present（1 次 VSync 等待）。

## 关键约束

| 约束 | 说明 |
|------|------|
| 像素数据复制 | 异步渲染的权衡：memcpy ≈ 0.3ms/帧（640×480 RGBA） |
| RenderThread 独占 GL | 所有 GL 操作仅在 RenderThread 执行，EGLContextManager 跟踪 m_ownerThread |
| 纹理池不变量 | Acquire 后必须 Release，缓存会破坏池 |
| napi_create_async_work | libuv worker 线程池可能被耗尽——队列堆积时需注意 |
| 模拟器软件 GPU | 性能数据为软件渲染，真实设备预期 2-5ms/frame |
| 10% 脏区盈亏平衡 | 超过此比例增量开销抵消收益 |
| Y 翻转 Shader 已移除 | `1.0 - vTexCoord.y` 在软件 GPU 上额外开销 ~14ms |
| glScissor 已移除 | EGL 后缓冲区在 SwapBuffers 后内容未定义 |
| 触控叠加层只捕获 | 单一职责：不负责视觉渲染，反馈通过渲染管线输出 |
| hilog `%{public}f` 损坏 | 对 float 无效，需转 int 或用字符串拼接 |
