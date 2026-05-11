# XComponent 零拷贝渲染组件

基于 HarmonyOS `XComponent` 与 `NativeWindow` 实现的高性能图像渲染方案。  

**核心特性**：绕过 `PixelMap`，直接在 Native 层操作显存，实现真正的零拷贝渲染。

## 📖 项目背景

HarmonyOS 原生 `Image` 组件在配合 `PixelMap` 使用时存在以下问题：
- 修改 `PixelMap` 内容后，UI 不会自动刷新（需调用 `ImageController.reload()` 或替换引用，仍存在拷贝开销）。
- 对于实时视频流、动态绘图、高性能滤镜等场景，`PixelMap` 路径无法满足性能要求。

本项目基于官方 Codelab 源码进行重构，提供一套 **零拷贝、低延迟、可复用** 的渲染组件模板。

## ✨ 功能特性

- ✅ **零拷贝渲染**：C++ 层直接填充 `SurfaceBuffer`，数据从 Native 侧直达屏幕。
- ✅ **高性能**：支持 OpenGL ES / 直接像素填充，适用于实时图像处理、视频渲染等场景。
- ✅ **封装复用**：提供 ArkTS 自定义组件 `MyRenderView`，可在应用中像普通组件一样多处调用。
- ✅ **增量调试**：支持 DevEco Studio `Apply Changes`，C++ 代码修改后无需重装应用。
- ✅ **ScreenRenderer API**：新增极简的零拷贝GPU渲染接口，支持多实例、自动回调更新UI。

## 🛠️ 环境要求

| 项目 | 要求 |
| :--- | :--- |
| HarmonyOS SDK | API 12 及以上 |
| DevEco Studio | 6.0.2 或更高版本 |
| 运行设备 | HarmonyOS 5.0.5+ 真机（推荐）或模拟器 |
| 开发语言 | ArkTS + C++ (NAPI) |

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/your-repo/xcomponent-zero-copy.git
```

使用 DevEco Studio 打开项目。

### 2. 编译与运行

连接真机（开启 USB 调试）或启动模拟器。

点击 Run 按钮（或使用 Shift+F10）构建并部署应用。

若修改了 C++ 代码，可使用 Apply Changes（Ctrl+F10）快速增量更新。

## 📚 ScreenRenderer API 使用指南

### 核心设计理念

- **极简接口**：只暴露一个核心渲染方法 `renderFrame`
- **基本类型**：NAPI接口仅使用 `number`、`ArrayBuffer` 等基本类型
- **写入即通知**：渲染完成后自动调用回调，无需手动更新UI
- **零GC优化**：通过BufferPool复用缓冲区，无每帧内存分配
- **多实例支持**：轻松支持多显示场景（主屏幕、预览窗口等）

### 快速示例

```typescript
import { ScreenRenderer } from '@fangcunkong/screen-renderer';

// 1. 创建渲染器
const renderer = new ScreenRenderer({ width: 1920, height: 1080 });
await renderer.initialize();

// 2. 设置渲染完成回调（自动更新UI）
renderer.setOnFrameRendered((textureId) => {
  this.screenTextureId = textureId;
});

// 3. 渲染帧（ArrayBuffer来自任意来源）
const pixelData = getPixelData(); // RGBA格式
await renderer.renderFrame(pixelData, 1920, 1080);
// ⭐ 无需手动更新UI，回调已自动处理

// 4. UI层绑定
Image(this.screenTextureId)
  .width('100%')
  .height('100%');
```

### NAPI接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `create` | width, height | Promise&lt;number&gt; | 创建渲染器，返回句柄 |
| `renderFrame` | handle, pixelData, width, height | Promise&lt;void&gt; | 渲染帧，零拷贝传输 |
| `getTextureId` | handle | number | 获取纹理ID |
| `resize` | handle, width, height | Promise&lt;void&gt; | 调整尺寸 |
| `destroy` | handle | Promise&lt;void&gt; | 销毁渲染器 |

详细API文档请参考：[zero-copy-display-api-simplified.md](zero-copy-display-api-simplified.md)
