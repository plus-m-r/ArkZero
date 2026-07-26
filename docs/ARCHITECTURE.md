# ArkZero 架构说明

## 目录结构

```
library/src/main/cpp/
├── napi_init.cpp                    # NAPI 模块注册入口
├── common/
│   └── common.h                     # DirtyRect, TileRegion 等公共类型
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
│   │   ├── RendererApi.h/.cpp       # NAPI 高层 API（fire-and-forget，立即 resolve）
│   │   └── RefCleaner.h/.cpp        # napi_ref 跨线程安全清理（napi_threadsafe_function）
│   ├── backend/
│   │   ├── IRenderBackend.h         # 渲染后端接口
│   │   ├── GLESBackend.h/.cpp       # Facade：双缓冲 + Upload/SwapAndPresent 分离
│   │   ├── EGLContextManager.h/.cpp # EGL 上下文生命周期 + VSync 控制
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
│   │   ├── Renderer.h/.cpp          # 外观类，fire-and-forget 入队
│   │   └── RenderThread.h/.cpp      # 常驻渲染线程 + 命令队列
│   └── manager/
│       ├── RendererManager.h/.cpp   # 渲染器实例管理（handle 映射，线程安全）
│       └── SurfaceManager.h/.cpp    # NativeWindow 管理（surfaceId → window）
└── types/
    └── libnativerender/
        └── Index.d.ts               # TypeScript 类型声明
```

```
library/src/main/ets/
├── components/rendering/
│   ├── ArkZeroRenderer.ets          # ArkTS 渲染器封装层
│   ├── ArkZeroRenderSession.ets     # 会话层（生命周期管理）
│   └── ArkZeroSurfaceView.ets       # 视口组件（比例布局 + 归一化触控）
```

```
entry/src/main/ets/
├── entryability/
│   └── EntryAbility.ets
├── pages/
│   ├── Index.ets                    # 首页（导航按钮）
│   └── SurfaceDemoPage.ets          # XComponent 演示页
├── integration/
│   ├── observer/                    # XComponent 生命周期观察者
│   ├── components/                  # 测试结果面板
│   ├── pages/                       # IntegrationTestPage
│   └── tests/                       # 集成测试（Tile/Session/基础）
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
│  - 归一化交互输出：ratioX/Y + pixelX/Y 便利字段                │
│  - 预计算 scaleX/Y = renderWidth/Height / areaWidth/Height    │
│  - display.getDefaultDisplaySync() 获取屏幕尺寸                │
└──────────────┬───────────────────────────────────────────────┘
               │ import nativerender
┌──────────────▼───────────────────────────────────────────────┐
│  NAPI Fire-and-Forget 层 (RendererApi.cpp)                   │
│  - 创建 napi_ref 防止 GC，仅传指针+长度入队（零 memcpy）       │
│  - JS 线程零等待，不经过 napi_async_work / libuv 线程池        │
│  - 同步 API 仅：CreateRenderer（阻塞等初始化完成）             │
└──────────────┬───────────────────────────────────────────────┘
               │ C++ 调用
┌──────────────▼───────────────────────────────────────────────┐
│  Core 层 (Renderer + RenderThread + Manager)                 │
│  - Renderer: 外观类，void 方法，仅入队不等待                    │
│  - RenderThread: 常驻 std::thread + 命令队列                   │
│    - mutex/condition_variable 同步                             │
│    - 无 promise/future，命令执行后自动 SwapAndPresent           │
│    - 拥有 GLESBackend，所有 GL 操作仅在此线程执行               │
│  - RendererManager: handle ↔ 实例映射（m_mutex 线程安全）      │
└──────────────┬───────────────────────────────────────────────┘
               │ 委托
┌──────────────▼───────────────────────────────────────────────┐
│  Backend 层 (GLESBackend + 组件)                              │
│  - GLESBackend: Facade，双缓冲架构                             │
│    - Front/Back 纹理对：Back 接上传，Front 接显示               │
│    - Upload*() → 写入 Back 纹理 + 标记 dirty                   │
│    - SwapAndPresent() → swap(Front,Back) + Draw + SwapBuffers │
│  - EGLContextManager: EGL 生命周期 + VSync 控制                │
│  - TextureManager: 纹理管理                                    │
│  - TextureShader: 绘制 Shader                                 │
│  - TexturePool + Strategy: 纹理复用                            │
│  - YUVShaderManager: YUV GPU 转换                             │
└──────────────────────────────────────────────────────────────┘
```

## 核心设计决策

### 1. Fire-and-Forget 架构

渲染调用在 JS 线程只做指针入队（零 memcpy），立即返回，不等 VSync：

```
JS 线程                              RenderThread
─────────────                        ─────────────────
renderFrame(data)
  ↓ napi_ref(data) — 防 GC
  ↓ cmd.srcData = data_ptr
  ↓ enqueue(cmd) ──────────────→    cmd = dequeue()
  ↓ resolve(promise)                 memcpy(cmd.pixelData, srcData)
  ↓ return (~0.1ms)                  RefCleaner.ScheduleDelete(ref)
                                     UploadFrame(cmd.data)
                                     SwapAndPresent()
                                      ↳ swap(Front,Back)
                                      ↳ Draw(Front)
                                      ↳ SwapBuffers() → VSync
```

**关键**：JS 调用方感知延迟 ~0.1ms（仅 enqueue），memcpy 和 VSync 等待全部在渲染线程完成。

### 2. 双缓冲纹理

GLESBackend 维护 Front/Back 纹理对，实现上传与显示并行：

```
时间线：
────────────────────────────────────────────────→
JS:   Upload→  Upload→  Upload→
RT:         Swap   Swap   Swap
             ↓      ↓      ↓
       [B→F] [B→F] [B→F]    ← 指针交换，无数据拷贝
       DrawF  DrawF  DrawF
       VSync  VSync  VSync
```

- **Back 纹理**：Upload 写入目标，JS 线程可连续写入
- **Front 纹理**：显示目标，VSync 时绘制到屏幕
- **SwapAndPresent**：swap 指针 → Draw Front → SwapBuffers → 重新 Acquire Back

### 3. 常驻渲染线程

不再使用 `napi_async_work` / libuv 线程池。渲染线程在 `Start()` 时创建，持续运行直到 `Stop()`：

- **消除调度延迟**：libuv 线程池调度 ~5-15ms → 直接 dequeue ~0.01ms
- **消除 future.get() 阻塞**：Renderer 方法不再返回 future，void 即发即忘
- **自动 Swap**：RENDER_FRAME/RENDER_TILE_REGIONS 等命令执行后自动 SwapAndPresent

### 4. 比例视口系统

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

### 5. Tile 定位（比例坐标）

TileRegion 使用比例坐标定位，像素尺寸定义数据量：

```typescript
interface TileRegion {
  ratioX: number;          // 帧内 X 位置 = frameWidth × ratioX
  ratioY: number;          // 帧内 Y 位置 = frameHeight × ratioY
  tilePixelWidth: number;  // 瓦片宽度（像素）
  tilePixelHeight: number; // 瓦片高度（像素）
  pixelData: ArrayBuffer;  // 瓦片像素数据
}
```

**C++ 原生实现**：每个 tile 独立上传到 Back 纹理的对应区域，跳过全帧拼装。

### 6. Handle 而非指针

ArkTS 层通过 `number` 类型 handle 引用 Native 对象，而非直接暴露 C++ 指针。

### 7. VSync

生产模式启用 VSync：`eglSwapInterval(display, 1)`。可通过 `setVSync(false)` 关闭（不推荐，移动 GPU 可能导致管线阻塞）。

## 性能数据（MatePad Pro 13 模拟器，软件 GPU）

| 场景 | 旧架构 (napi_async_work) | Fire-and-forget (v1.1) | Deferred memcpy (v1.2) | 总提升 |
|------|--------------------------|------------------------|------------------------|--------|
| 单瓦片 | 64ms | 1ms | **1ms** | 64× |
| 多瓦片 | 16ms | 4ms | **4ms** | 4× |
| 瓦片 + Present | 21ms | 1ms | **1ms** | 21× |
| 全帧 | ~30ms | ~33ms | **1ms** | 30× |
| 连续多帧 | — | ~24.6ms/帧 | **~2ms/帧** | 12× |

## 关键约束

| 约束 | 说明 |
|------|------|
| Deferred memcpy | JS 线程零拷贝，memcpy 在渲染线程执行；napi_ref 防止 ArrayBuffer 被 GC |
| RefCleaner | 通过 napi_threadsafe_function 安全删除 napi_ref，跨线程安全 |
| RenderThread 独占 GL | 所有 GL 操作仅在 RenderThread 执行 |
| 双缓冲 | SwapAndPresent 后 Back 纹理重新 Acquire，确保上传/显示不冲突 |
| 销毁清屏 | Destroy 前渲染纯黑帧，避免随机画面残留 |
| VSync 保持 ON | 移动 GPU 上 eglSwapInterval(0) 导致管线阻塞 |
| 模拟器软件 GPU | 性能数据为软件渲染，真实设备预期更低 |
| 交互叠加层只捕获 | 单一职责：不负责视觉渲染 |
