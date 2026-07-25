# ArkZero - Ultra Low Latency Renderer

ArkZero 是一个基于 HarmonyOS 的超低延迟增量渲染器，支持异步渲染线程、比例视口系统和触控捕获。

## 文档

- [docs/README.md](./docs/README.md) - 项目概述和入门指南
- [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md) - 系统架构说明
- [docs/API_REFERENCE.md](./docs/API_REFERENCE.md) - API 详细说明
- [docs/UNIT_TEST_DESIGN.md](./docs/UNIT_TEST_DESIGN.md) - 测试设计

## 主要功能

- 异步渲染线程 - 专用 std::thread + 命令队列，UI 线程零阻塞
- 比例视口系统 - 位置和尺寸全部用屏幕比例表达，跨设备零适配
- 归一化触控 - 触摸输出同时提供 ratio (0.0~1.0) 和 pixel 坐标，调用方零计算
- 增量脏区渲染 - 仅上传变化区域，节省 GPU 带宽
- 分离式 API - `updateDirtyRegions` + `presentFrame`，支持累积模式
- 多格式支持 - RGBA/RGB/BGRA/NV21/NV12，GPU 端零转换渲染
- 多实例支持 - 每个 ArkZeroRenderer 独立管理 Native 资源
- XComponent 直出 - 绕过 UI 合成器，延迟 <10ms

## 性能指标（MatePad Pro 13 模拟器，软件 GPU）

| 模式 | ms/frame | 说明 |
|------|----------|------|
| 全帧渲染 | 30.6 | 基线 |
| 分离式 1% 脏区 | 26.1 | -15% |
| 累积 3 帧 1 次 present | ~8.6 | -72% |
| 累积 10 帧 1 次 present | ~1.4 | -95% |

真实设备（硬件 GPU）预期 2-5ms/frame。

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
import { ArkZeroSurfaceView, SurfaceViewConfig, NormalizedTouchEvent } from '../components/rendering/ArkZeroSurfaceView';
import { ArkZeroRenderer, PixelFormat } from '../components/rendering/ArkZeroRenderer';

// 配置视口：全屏宽度，高度按渲染内容宽高比自适应，居中
const config: SurfaceViewConfig = {
  renderWidth: 640,
  renderHeight: 480,
  format: PixelFormat.RGBA,
  originRatioX: 0.056,  // 居中偏移（屏幕比组件宽时）
  sizeRatioX: 1.0,       // 占屏幕 100% 宽度
  enableTouch: true
};

// 触控回调：直接使用 pixelX/pixelY 写入 pixelBuffer，零计算
const onTouch = (event: NormalizedTouchEvent) => {
  for (const touch of event.touches) {
    // touch.ratioX/Y: 0.0~1.0 归一化比例（跨设备通用）
    // touch.pixelX/Y: 直接对应 renderWidth/renderHeight 像素坐标
    drawCircle(pixelBuffer, touch.pixelX, touch.pixelY);
  }
};

// 在 build() 中使用
ArkZeroSurfaceView({
  config: config,
  touchHandler: onTouch,
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
