# ArkZero - Ultra Low Latency Renderer

ArkZero 是一个基于 HarmonyOS 的超低延迟增量渲染器，支持零拷贝渲染、脏区增量上传和累积呈现。

## 文档

- [docs/README.md](./docs/README.md) - 项目概述和入门指南
- [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md) - 系统架构说明
- [docs/API_REFERENCE.md](./docs/API_REFERENCE.md) - API 详细说明
- [docs/UNIT_TEST_DESIGN.md](./docs/UNIT_TEST_DESIGN.md) - 测试设计

## 主要功能

- 零拷贝渲染 - ArrayBuffer 直接传递指针到 Native 层
- 增量脏区渲染 - 仅上传变化区域，节省 GPU 带宽
- 分离式 API - `updateDirtyRegions` + `presentFrame`，支持累积模式
- 多格式支持 - RGBA/RGB/BGRA/NV21/NV12，GPU 端零转换渲染
- 异步渲染 - renderFrame 返回 Promise，完成后自动回调
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
import { ArkZeroRenderer, PixelFormat, DirtyRect } from '../components/rendering/ArkZeroRenderer';

const renderer = new ArkZeroRenderer({
  width: 1920,
  height: 1080,
  format: PixelFormat.RGBA
});

await renderer.initialize(surfaceId);

// 全帧渲染
await renderer.renderFrame(pixelData, 1920, 1080);

// 增量渲染（分离式）
await renderer.updateDirtyRegions(pixelData, 1920, 1080, [{ x: 0, y: 0, w: 100, h: 100 }]);
await renderer.presentFrame();

// 累积模式：多次 update + 一次 present
for (let i = 0; i < 10; i++) {
  await renderer.updateDirtyRegions(data, w, h, dirtyRects);
}
await renderer.presentFrame();

// 清理
renderer.dispose();
```

## 项目结构

```
ArkZero/
├── docs/                    # 技术文档
├── entry/src/main/
│   ├── cpp/
│   │   ├── renderer/        # 核心渲染引擎
│   │   │   ├── api/         # NAPI 高层桥接（RendererApi）
│   │   │   ├── backend/     # GLESBackend + EGL/Texture/YUV/Pool/Strategy
│   │   │   ├── core/        # Renderer 外观类 + Command + PerformanceMonitor
│   │   │   └── manager/     # RendererManager + SurfaceManager
│   │   ├── napi_bridge/     # NAPI 底层桥接（各组件独立 NAPI 绑定）
│   │   ├── common/          # 公共常量与类型（DirtyRect 等）
│   │   └── types/           # TypeScript 类型定义（Index.d.ts）
│   └── ets/
│       ├── components/      # ArkZeroRenderer 封装组件
│       ├── pages/           # 示例页面（Index, SurfaceDemoPage）
│       └── integration/     # 集成测试 + 基准测试
```

## 许可证

Apache License 2.0
