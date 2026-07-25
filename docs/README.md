# ArkZeroRenderer - 超低延迟零拷贝增量渲染器

基于 HarmonyOS XComponent (SURFACE) 与 NativeWindow 实现的高性能渲染方案，支持全帧渲染和脏区增量渲染。

## 核心特性

- 零拷贝渲染：NAPI 直接获取 ArrayBuffer 指针，无中间拷贝
- 增量脏区上传：`glPixelStorei(GL_UNPACK_ROW_LENGTH)` + `glTexSubImage2D` 仅上传变化区域
- 分离式 API：`updateDirtyRegions`（上传）+ `presentFrame`（绘制+交换），支持累积模式
- 多格式支持：RGBA/RGB/BGRA/NV21/NV12，YUV 格式通过 GPU Shader 转换
- 纹理池复用：预分配常用分辨率纹理，Resize 耗时 <1ms
- VSync 同步：`eglSwapInterval(display, 1)` 锁定显示刷新率

## 数据流

```
ArkTS ArrayBuffer → NAPI Pointer → glTexSubImage2D → GPU → eglSwapBuffers → Screen
```

全帧模式：`renderFrame(data, w, h)` — 上传 + 绘制 + 交换，一步完成。

增量模式（分离式）：
```
updateDirtyRegions(data, w, h, regions)  — 仅上传脏区到纹理
updateDirtyRegions(data, w, h, regions)  — 可多次累积
presentFrame()                            — 绘制 + eglSwapBuffers
```

累积模式是最大性能增益：10 次 update + 1 次 present ≈ 1.4ms/frame（-95%）。

## 环境要求

| 项目 | 要求 |
|:---|:---|
| HarmonyOS SDK | API 12+ (HarmonyOS 5.0+) |
| DevEco Studio | 6.0+ |
| 开发语言 | ArkTS + C++ (NAPI) |
| 图形后端 | OpenGL ES 3.0+ |

## 快速开始

```typescript
import { ArkZeroRenderer, PixelFormat, DirtyRect } from '../components/rendering/ArkZeroRenderer';

// 1. 创建并初始化
const renderer = new ArkZeroRenderer({
  width: 1920,
  height: 1080,
  format: PixelFormat.RGBA
});
await renderer.initialize(surfaceId);

// 2. 全帧渲染
await renderer.renderFrame(pixelData, 1920, 1080);

// 3. 增量渲染
const dirtyRegions: DirtyRect[] = [{ x: 100, y: 50, w: 200, h: 150 }];
await renderer.updateDirtyRegions(pixelData, 1920, 1080, dirtyRegions);
await renderer.presentFrame();

// 4. 清理
renderer.dispose();
```

## 性能基准

在 MatePad Pro 13 模拟器（软件 GPU）上测试结果：

| 测试场景 | ms/frame | vs 全帧基线 |
|----------|----------|-------------|
| 全帧 50 帧（基线） | 30.6 | — |
| 分离式 1% 脏区 | 26.1 | -15% |
| 分离式 10% 脏区 | 32.5 | +6% |
| 累积 3 帧 1 次 present | ~8.6 | -72% |
| 累积 10 帧 1 次 present | ~1.4 | -95% |
| 分离式 100 帧耐力 | 21.1 | -31% |

关键结论：
- 10% 脏区是盈亏平衡点，超过此比例增量开销抵消收益
- 累积模式是最大性能增益来源
- 真实设备（硬件 GPU）预期 2-5ms/frame

## 项目结构

```
entry/src/main/
├── cpp/
│   ├── renderer/          # 核心渲染引擎
│   │   ├── api/           # NAPI 高层桥接（RendererApi — create/renderFrame/resize/destroy 等）
│   │   ├── backend/       # 渲染后端
│   │   │   ├── GLESBackend.h/.cpp        # Facade：协调 EGL/Texture/Shader/Pool
│   │   │   ├── EGLContextManager.h/.cpp  # EGL 生命周期管理
│   │   │   ├── TextureManager.h/.cpp     # OpenGL 纹理创建/更新/脏区更新
│   │   │   ├── TextureShader.h/.cpp      # 纹理绘制 Shader
│   │   │   ├── TexturePool.h/.cpp        # 纹理池复用
│   │   │   ├── PixelFormatConverter.h/.cpp # 格式映射
│   │   │   ├── YUVShaderManager.h/.cpp   # YUV→RGB GPU 转换
│   │   │   ├── factory/                  # TextureFactory
│   │   │   └── strategy/                 # DirectStrategy / PoolStrategy
│   │   ├── core/           # Renderer 外观类 + Command + PerformanceMonitor
│   │   └── manager/        # RendererManager + SurfaceManager
│   ├── napi_bridge/        # NAPI 底层桥接（各组件独立 NAPI 绑定文件）
│   ├── common/             # 公共类型（DirtyRect 等）
│   └── types/              # Index.d.ts 类型声明
└── ets/
    ├── components/rendering/  # ArkZeroRenderer 封装组件
    ├── pages/                 # Index, SurfaceDemoPage
    └── integration/           # 集成测试 + 增量基准测试
```
