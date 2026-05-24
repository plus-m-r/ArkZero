# ArkZero - Ultra Low Latency Renderer

ArkZero 是一个基于 HarmonyOS 的超低延迟渲染器项目，支持零拷贝渲染和自动后端检测。

## 📚 文档

所有技术文档已移动到 **[docs](./docs/)** 目录：

- **[📖 文档中心](./docs/INDEX.md)** - 从这里开始浏览所有文档
- **[🚀 快速开始](./docs/README.md)** - 项目概述和入门指南
- **[🏗️ 架构设计](./docs/ARCHITECTURE.md)** - 系统架构说明
- **[🧪 测试文档](./docs/UNIT_TEST_DESIGN.md)** - 完整的测试设计（⭐核心）
- **[📖 API 参考](./docs/API_REFERENCE.md)** - API 详细说明

## 🎯 主要功能

- ✅ **零拷贝渲染** - ArrayBuffer 直接传递指针到 Native 层
- ✅ **自动后端检测** - Vulkan > OpenGL ES > CPU 软渲染
- ✅ **异步渲染** - renderFrame 返回 Promise，完成后自动回调
- ✅ **多实例支持** - 每个 ArkZeroRenderer 独立管理 Native 资源
- ✅ **XComponent 集成** - Direct Surface Rendering，延迟 <10ms

## 🚀 快速开始

### 1. 构建应用

```powershell
hvigorw --mode module -p module=entry -p product=default assembleHap
```

### 2. 安装到设备

```powershell
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

### 3. 运行测试

在应用首页点击 **"🧪 运行集成测试"** 按钮即可运行可视化测试。

详细使用方法请参考：**[docs/UNIT_TEST_DESIGN.md](./docs/UNIT_TEST_DESIGN.md)** 第 4.2.3 章节

## 📂 项目结构

```
ArkZero/
├── docs/                    # 📚 所有技术文档
│   ├── INDEX.md            # 文档导航
│   ├── README.md           # 项目概述
│   ├── ARCHITECTURE.md     # 架构设计
│   ├── UNIT_TEST_DESIGN.md # 测试设计 ⭐
│   └── API_REFERENCE.md    # API 参考
├── entry/                   # 主模块
│   └── src/
│       ├── main/           # 主代码
│       │   ├── ets/        # ArkTS 代码
│       │   └── cpp/        # C++ 代码
│       └── ohosTest/       # 单元测试
└── ...
```

## 🔗 相关链接

- [HarmonyOS 官方文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides)
- [Hypium 测试框架](https://gitee.com/openharmony/testfwk_arkxtest)

## 📄 许可证

本项目采用 Apache-2.0 许可证。详见 [LICENSE](./LICENSE) 文件。

---

**维护者**: ArkZeroRenderer Team  
**最后更新**: 2026-05-13
