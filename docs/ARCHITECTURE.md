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
│   │   └── RendererApi.h/.cpp       # NAPI 高层 API（create/renderFrame/resize/destroy）
│   ├── backend/
│   │   ├── IRenderBackend.h         # 渲染后端接口
│   │   ├── GLESBackend.h/.cpp       # Facade：协调所有后端组件
│   │   ├── EGLContextManager.h/.cpp # EGL 上下文生命周期
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
│   │   ├── Renderer.h/.cpp          # 外观类，转发到 Backend
│   │   ├── PerformanceMonitor.h/.cpp # 性能统计
│   │   └── command/
│   │       ├── IRenderCommand.h
│   │       ├── RenderFrameCommand.h/.cpp
│   │       └── PreallocateTextureCommand.h/.cpp
│   └── manager/
│       ├── RendererManager.h/.cpp   # 渲染器实例管理（handle 映射）
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
│   └── ArkZeroRenderer.ets          # ArkTS 封装层
└── integration/
    ├── observer/                    # XComponent 生命周期观察者
    ├── components/                  # 测试结果面板
    ├── pages/                       # IntegrationTestPage
    └── tests/                       # 集成测试 + 增量基准测试
```

## 四层架构

```
┌──────────────────────────────────────────┐
│  ArkTS 层 (ArkZeroRenderer.ets)          │  UI 层调用
│  - initialize / renderFrame              │
│  - updateDirtyRegions / presentFrame     │
│  - dispose / disposeAsync / awaitIdle    │
└──────────────┬───────────────────────────┘
               │ import nativerender
┌──────────────▼───────────────────────────┐
│  NAPI 高层 (RendererApi.cpp)             │  高层业务 API
│  - create / renderFrame / resize         │
│  - renderFrameRegions                    │
│  - updateDirtyRegions / presentFrame     │
│  - destroy                               │
└──────────────┬───────────────────────────┘
               │ C++ 调用
┌──────────────▼───────────────────────────┐
│  Core 层 (Renderer + Manager)            │  业务逻辑
│  - Renderer: 外观类，转发到 Backend      │
│  - RendererManager: handle ↔ 实例映射    │
│  - SurfaceManager: surfaceId → window    │
└──────────────┬───────────────────────────┘
               │ 委托
┌──────────────▼───────────────────────────┐
│  Backend 层 (GLESBackend + 组件)         │  渲染实现
│  - GLESBackend: Facade 协调              │
│  - EGLContextManager: EGL 生命周期       │
│  - TextureManager: 纹理管理              │
│  - TextureShader: 绘制 Shader            │
│  - TexturePool + Strategy: 纹理复用      │
│  - YUVShaderManager: YUV GPU 转换       │
└──────────────────────────────────────────┘
```

另外，`napi_bridge/` 目录提供了每个后端组件的独立 NAPI 绑定，用于底层测试和直接访问。这些是 RendererApi 之外的独立入口点。

## 核心设计决策

### 1. Handle 而非指针

ArkTS 层通过 `number` 类型 handle 引用 Native 对象，而非直接暴露 C++ 指针。

```typescript
const handle: number = await nativerender.create(surfaceId, 1920, 1080, format);
```

优势：
- 防止 ArkTS 直接操作 C++ 对象
- 可验证 handle 有效性
- RendererManager 通过 `unordered_map<handle, unique_ptr<Renderer>>` 管理生命周期

### 2. 零拷贝数据传递

```
ArkTS ArrayBuffer
    ↓ napi_get_arraybuffer_info() — 获取指针，无拷贝
C++ void* 指针
    ↓ glTexSubImage2D() — DMA 传输到 GPU
GPU 显存
```

### 3. 增量脏区渲染

`TextureManager::UpdateRegion` 使用 `glPixelStorei(GL_UNPACK_ROW_LENGTH, stride)` + `glTexSubImage2D` 仅上传脏区像素，避免全帧上传。

Y 坐标翻转：`glY = frameHeight - screenY - regionH`

### 4. 分离式 API

```
updateDirtyRegions(data, w, h, regions)  → 仅上传纹理
presentFrame()                            → 绘制 + eglSwapBuffers
```

支持累积模式：多次 `updateDirtyRegions` + 一次 `presentFrame`，避免每帧 VSync 等待。

### 5. 纹理策略模式

GLESBackend 通过 `ITextureStrategy` 接口支持两种纹理获取方式：
- **DirectStrategy**：每次创建新纹理
- **PoolStrategy**：从 TexturePool 复用（默认）

```cpp
// TexturePool 不变量：每次 Acquire 必须在下次 Acquire 前 Release
// 缓存纹理会破坏此不变量，导致池耗尽和画面错乱
```

### 6. XComponent 集成

两种路径：
- **libraryname 方式**：XComponent 指定 `libraryname: 'nativerender'`，通过 `g_nativeXComponent` 全局变量获取 NativeWindow（单实例限制）
- **surfaceId 方式**：XComponent 不指定 libraryname，通过 `OH_NativeWindow_CreateNativeWindowFromSurfaceId(surfaceId)` 创建（多实例安全）

当前 SurfaceDemoPage 使用 surfaceId 方式。

### 7. 线程模型

- EGL 上下文是线程局部的，`SwapBuffers` 必须在创建上下文的线程调用
- `RendererApi` 中的 `renderFrame` / `presentFrame` 在 UI 线程同步执行
- `eglMakeCurrent` 跳过已当前的上下文（`m_isCurrent` 标志优化）

### 8. VSync

生产模式启用 VSync：`eglSwapInterval(display, 1)`，每次 SwapBuffers 等待一次 VSync 信号（~14ms@60Hz）。这是全帧渲染的主要耗时来源。

累积模式绕过此限制：N 次 update（无 VSync 等待）+ 1 次 present（1 次 VSync 等待）。

## 关键约束

| 约束 | 说明 |
|------|------|
| napi_init.cpp 全局变量 | `g_nativeXComponent` / `g_xcomponentId` 仅支持一个 XComponent |
| 纹理池不变量 | Acquire 后必须 Release，缓存会破坏池 |
| EGL 线程亲和 | SwapBuffers 必须在 UI 线程 |
| 模拟器软件 GPU | 性能数据为软件渲染，真实设备预期 2-5ms/frame |
| 10% 脏区盈亏平衡 | 超过此比例增量开销抵消收益 |
| Y 翻转 Shader 已移除 | `1.0 - vTexCoord.y` 在软件 GPU 上额外开销 ~14ms |
| glScissor 已移除 | EGL 后缓冲区在 SwapBuffers 后内容未定义 |
| PBO 已从 UpdateRegion 移除 | 直接 glTexSubImage2D 更简单且正确 |
