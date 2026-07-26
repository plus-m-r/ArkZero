# ArkZero - High-Performance GPU Renderer

ArkZero 是一个基于 HarmonyOS 的高性能 GPU 渲染库，支持零拷贝异步渲染、瓦片增量渲染和双缓冲呈现。

## 文档

- [docs/README.md](./docs/README.md) - 项目概述和入门指南
- [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md) - 系统架构说明
- [docs/API_REFERENCE.md](./docs/API_REFERENCE.md) - API 详细说明
- [docs/UNIT_TEST_DESIGN.md](./docs/UNIT_TEST_DESIGN.md) - 测试设计

## 主要功能

- 零拷贝异步渲染 - 专用 std::thread + 命令队列，JS 线程零拷贝、零阻塞
- 瓦片增量渲染 - 仅上传变化区域，C++ 原生实现跳过拼装
- 双缓冲呈现 - Front/Back 纹理分离，Upload 与 SwapAndPresent 并行
- 比例视口系统 - 位置和尺寸全部用屏幕比例表达，跨设备零适配
- 归一化交互输出 - ratio (0.0~1.0) 和 pixel 坐标，调用方零计算
- 多格式支持 - RGBA/RGB/BGRA/NV21/NV12，GPU 端零转换渲染
- 多实例支持 - 每个 ArkZeroRenderer 独立管理 Native 资源

## 性能指标（MatePad Pro 13 模拟器，软件 GPU）

| 模式 | JS 感知延迟 | 说明 |
|------|------------|------|
| 全帧渲染 | ~1ms | deferred memcpy，JS 零拷贝 |
| 单瓦片渲染 | ~1ms | C++ 原生 tile 上传 |
| 多瓦片 (4个) | ~4ms | 批量 tile 上传 |
| 连续多帧 | ~2ms/帧 | fire-and-forget |

真实设备（硬件 GPU）预期更低。

## 快速开始

### 1. 构建应用

```powershell
hvigorw --mode module -p module=entry -p product=default assembleHap
```

### 2. 安装到设备

```powershell
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

### 3. 代码示例

```typescript
import { ArkZeroSurfaceView, SurfaceViewConfig } from '../components/rendering/ArkZeroSurfaceView';
import { ArkZeroRenderer, PixelFormat } from '../components/rendering/ArkZeroRenderer';

const config: SurfaceViewConfig = {
  renderWidth: 640,
  renderHeight: 480,
  format: PixelFormat.RGBA,
  originRatioX: 0.056,
  sizeRatioX: 1.0
};

ArkZeroSurfaceView({
  config: config,
  onSurfaceLoaded: (surfaceId: string) => {
    renderer.initialize(surfaceId);
  }
})
```

## 项目结构

```
ArkZero/
├── docs/                    # 技术文档
├── entry/src/main/
│   ├── cpp/
│   │   ├── renderer/        # 核心渲染引擎
│   │   │   ├── api/         # NAPI 高层桥接（RendererApi，异步 work）
│   │   │   ├── backend/     # GLESBackend + EGL/Texture/YUV/Pool/Strategy
│   │   │   ├── core/        # Renderer + RenderThread + PerformanceMonitor
│   │   │   └── manager/     # RendererManager + SurfaceManager
│   │   ├── napi_bridge/     # NAPI 底层桥接（各组件独立 NAPI 绑定）
│   │   ├── common/          # 公共常量与类型（DirtyRect 等）
│   │   └── types/           # TypeScript 类型定义（Index.d.ts）
│   └── ets/
│       ├── components/      # ArkZeroRenderer + ArkZeroSurfaceView
│       ├── pages/           # 示例页面（Index, SurfaceDemoPage）
│       └── integration/     # 集成测试 + 基准测试
```

## 许可证

Apache License 2.0
