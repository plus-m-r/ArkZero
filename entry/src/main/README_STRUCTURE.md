# ArkZero Renderer 代码结构说明

## 📁 目录结构总览

```
src/
├── main/
│   ├── cpp/                          # Native C++ 代码
│   │   ├── common/                   # 公共常量和工具
│   │   ├── renderer/                 # 渲染器核心代码
│   │   │   ├── api/                  # NAPI API 层
│   │   │   ├── backend/              # 渲染后端实现
│   │   │   │   ├── strategy/         # ⭐ 策略模式
│   │   │   │   ├── factory/          # ⭐ 工厂模式
│   │   │   │   └── ...               # 其他后端组件
│   │   │   ├── core/                 # 核心渲染逻辑
│   │   │   │   ├── command/          # ⭐ 命令模式（待实现）
│   │   │   │   └── ...               # 其他核心组件
│   │   │   └── manager/              # 资源管理器
│   │   ├── napi_bridge/              # NAPI 桥接层（原 tests/）
│   │   ├── types/                    # TypeScript 类型定义
│   │   ├── CMakeLists.txt            # CMake 构建配置
│   │   └── napi_init.cpp             # NAPI 初始化
│   ├── ets/                          # ArkTS 代码
│   │   ├── components/               # UI 组件
│   │   │   └── rendering/            # 渲染相关组件
│   │   │       ├── facade/           # ⭐ 外观模式（待实现）
│   │   │       └── ArkZeroRenderer.ets
│   │   ├── integration/              # 集成测试
│   │   │   ├── observer/             # ⭐ 观察者模式（待实现）
│   │   │   ├── components/           # 测试组件
│   │   │   ├── pages/                # 测试页面
│   │   │   └── tests/                # 测试用例
│   │   ├── pages/                    # 应用页面
│   │   └── entryability/             # 应用入口
│   └── resources/                    # 资源文件
└── ohosTest/                         # 单元测试
    └── ets/
        ├── test/                     # 测试代码
        │   ├── arkts/                # ArkTS 层测试
        │   ├── native/               # Native 层测试
        │   └── List.test.ets         # 测试入口
        └── ...
```

---

## 🎨 设计模式映射

### Native 层 (C++)

| 设计模式 | 目录位置 | 状态 |
|---------|---------|------|
| **Strategy Pattern** | `renderer/backend/strategy/` | ✅ 已实现 |
| **Factory Pattern** | `renderer/backend/factory/` | ✅ 已实现 |
| **Command Pattern** | `renderer/core/command/` | ⏸️ 待实现 |

### ArkTS 层

| 设计模式 | 目录位置 | 状态 |
|---------|---------|------|
| **Observer Pattern** | `integration/observer/` | ⏸️ 待实现 |
| **Facade Pattern** | `components/rendering/facade/` | ⏸️ 待实现 |

---

## 📂 详细说明

### 1. Native 层 (cpp/)

#### common/
- **用途**: 公共常量、枚举、工具函数
- **文件**: `common.h`

#### renderer/

##### api/
- **用途**: NAPI API 层，暴露给 ArkTS 的接口
- **文件**: 
  - `RendererApi.h/.cpp` - 渲染器 API

##### backend/
- **用途**: 渲染后端实现（OpenGL ES / Vulkan）
- **子目录**:
  - `strategy/` - 策略模式实现
    - `ITextureStrategy.h` - 策略接口
    - `PoolTextureStrategy.h/.cpp` - 池化策略
    - `DirectTextureStrategy.h/.cpp` - 直接创建策略
  - `factory/` - 工厂模式实现
    - `TextureFactory.h/.cpp` - 纹理工厂
  - 其他后端组件:
    - `EGLContextManager.h/.cpp` - EGL 上下文管理
    - `GLESBackend.h/.cpp` - OpenGL ES 后端
    - `TextureManager.h/.cpp` - 纹理管理
    - `TexturePool.h/.cpp` - 纹理池
    - `YUVShaderManager.h/.cpp` - YUV Shader 管理
    - `PixelFormatConverter.h/.cpp` - 像素格式转换

##### core/
- **用途**: 核心渲染逻辑
- **子目录**:
  - `command/` - 命令模式（待实现）
    - `IRenderCommand.h` - 命令接口
    - `RenderFrameCommand.h/.cpp` - 渲染帧命令
    - `PreallocateTextureCommand.h/.cpp` - 预分配命令
- **文件**:
  - `Renderer.h/.cpp` - 渲染器核心
  - `PerformanceMonitor.h/.cpp` - 性能监控

##### manager/
- **用途**: 资源管理器
- **文件**:
  - `RendererManager.h/.cpp` - 渲染器管理器
  - `SurfaceManager.h/.cpp` - Surface 管理器

#### napi_bridge/ (原 tests/)
- **用途**: NAPI 桥接代码，连接 C++ 和 ArkTS
- **注意**: 虽然名为 "tests"，但实际是 NAPI 桥接层
- **文件**: 各模块的 `*_napi.h/.cpp` 文件

#### types/
- **用途**: TypeScript 类型定义
- **文件**:
  - `libnativerender/Index.d.ts` - 类型声明
  - `libnativerender/oh-package.json5` - 包配置

---

### 2. ArkTS 层 (ets/)

#### components/rendering/
- **用途**: 渲染相关的 UI 组件
- **子目录**:
  - `facade/` - 外观模式（待实现）
    - `ArkZeroRendererFacade.ets` - 渲染器外观类
- **文件**:
  - `ArkZeroRenderer.ets` - 渲染器组件封装

#### integration/
- **用途**: 集成测试代码
- **子目录**:
  - `observer/` - 观察者模式（待实现）
    - `XComponentLifecycleObserver.ts` - 生命周期观察者接口
    - `XComponentLifecycleManager.ts` - 生命周期管理器
    - `TexturePreallocator.ts` - 纹理预分配器
  - `components/` - 测试组件
    - `TestResultPanel.ets` - 测试结果面板
  - `pages/` - 测试页面
    - `IntegrationTestPage.ets` - 集成测试主页面
  - `tests/` - 测试用例
    - `RendererIntegration.test.ets` - 渲染器集成测试
  - `README.md` - 集成测试说明

#### pages/
- **用途**: 应用页面
- **文件**:
  - `Index.ets` - 主页
  - `SurfaceDemoPage.ets` - Surface 演示页面

#### entryability/
- **用途**: 应用入口
- **文件**:
  - `EntryAbility.ets` - 入口 Ability

---

### 3. 单元测试 (ohosTest/)

#### ets/test/
- **arkts/**: ArkTS 层单元测试
  - `ArkZeroRenderer.test.ets`
  - `ArkZeroRendererConfig.test.ets`
  - `PixelFormat.test.ets`
- **native/**: Native 层单元测试
  - `EGLContextManager.test.ets`
  - `GLESBackend.test.ets`
  - `TextureManager.test.ets`
  - `TexturePool.test.ets`
  - 等等...
- **List.test.ets**: 测试入口文件

---

## 🔄 重构历史

### 2026-05-14
- ✅ 创建 `strategy/` 目录，实现策略模式
- ✅ 创建 `factory/` 目录，实现工厂模式
- ✅ 重命名 `tests/` 为 `napi_bridge/`（更准确反映用途）
- ✅ 创建 `command/` 目录（命令模式，待实现）
- ✅ 创建 `observer/` 目录（观察者模式，待实现）
- ✅ 创建 `facade/` 目录（外观模式，待实现）

---

## 💡 使用建议

### 添加新功能时

1. **Native 层新功能**:
   - 策略相关 → `renderer/backend/strategy/`
   - 工厂相关 → `renderer/backend/factory/`
   - 命令相关 → `renderer/core/command/`
   - 后端实现 → `renderer/backend/`

2. **ArkTS 层新功能**:
   - 观察者相关 → `integration/observer/`
   - 外观类 → `components/rendering/facade/`
   - 组件 → `components/rendering/`
   - 页面 → `pages/`

3. **测试代码**:
   - 单元测试 → `ohosTest/ets/test/`
   - 集成测试 → `main/ets/integration/`

### 查找代码时

- **NAPI 桥接**: `cpp/napi_bridge/`
- **渲染后端**: `cpp/renderer/backend/`
- **核心逻辑**: `cpp/renderer/core/`
- **类型定义**: `cpp/types/`
- **UI 组件**: `ets/components/`
- **测试代码**: `ohosTest/ets/test/`

---

## 📝 注意事项

1. **napi_bridge/ 目录**: 虽然原名是 `tests/`，但实际是 NAPI 桥接层，不是测试代码
2. **设计模式目录**: 每个设计模式都有独立的子目录，便于维护和扩展
3. **测试分离**: 单元测试在 `ohosTest/`，集成测试在 `main/ets/integration/`
4. **向后兼容**: 重构保持了原有功能，只是调整了目录结构

---

**最后更新**: 2026-05-14  
**维护者**: ArkZeroRenderer Team
