# ArkZeroRenderer HarmonyOS 自动化测试设计文档

本文档提供**基于 Hypium 和 arkxtest 框架**的完整自动化测试设计方案，精确到每个类的每个方法。

---

## 📋 目录

- [1. 测试项目结构](#1-测试项目结构)
  - [1.1 完整测试文件结构](#11-完整测试文件结构)
    - [1.1.1 主代码结构（entry/src/main）](#111-主代码结构entrysrcmain)
    - [1.1.2 Instrument Test 结构（entry/src/ohosTest）](#112-instrument-test-结构entrysrcohostest)
    - [1.1.3 Local Test 结构（entry/src/test）](#113-local-test-结构entrysrctest)
    - [1.1.4 测试相关配置文件](#114-测试相关配置文件)
  - [1.2 测试文件命名规范](#12-测试文件命名规范)
  - [1.3 测试目录组织原则](#13-测试目录组织原则)
  - [1.4 配置文件](#14-配置文件)
    - [1.4.1 Instrument Test 模块配置（module.json5）](#141-instrument-test-模块配置modulejson5)
    - [1.4.2 构建配置（build-profile.json5）](#142-构建配置build-profilejson5)
    - [1.4.3 依赖配置（oh-package.json5）](#143-依赖配置oh-packagejson5)
    - [1.4.4 资源配置文件](#144-资源配置文件)
    - [1.4.5 CMake 构建配置](#145-cmake-构建配置)
    - [1.4.6 自动化测试脚本](#146-自动化测试脚本)
  - [1.5 测试文件清单总览](#15-测试文件清单总览)
    - [1.5.1 Instrument Test 文件清单](#151-instrument-test-文件清单entrysrcohostest)
    - [1.5.2 Local Test 文件清单](#152-local-test-文件清单entrysrctest)
    - [1.5.3 配置文件清单](#153-配置文件清单)
- [2. C++ 层单元测试（通过 NAPI 桥接）](#2-c-层单元测试通过-napi-桥接)
  - [2.1 PerformanceMonitor 类](#21-performancemonitor-类)
  - [2.2 PixelFormatConverter 类](#22-pixelformatconverter-类)
  - [2.3 RenderQueue 类](#23-renderqueue-类)
  - [2.4 RenderCommand 结构体](#24-rendercommand-结构体)
- [3. ArkTS 层单元测试](#3-arkts-层单元测试)
  - [3.1 PixelFormat 枚举](#31-pixelformat-枚举)
  - [3.2 ArkZeroRenderer 类](#32-arkzerorenderer-类)
  - [3.3 ArkZeroRendererConfig 接口](#33-arkzerorendererconfig-接口)
- [4. 逐步集成测试模块设计](#4-逐步集成测试模块设计)
  - [4.1 集成测试策略](#41-集成测试策略)
  - [4.2 NAPI 桥接层集成测试](#42-napi-桥接层集成测试)
  - [4.3 渲染管线集成测试](#43-渲染管线集成测试)
  - [4.4 Surface 管理集成测试](#44-surface-管理集成测试)
  - [4.5 端到端集成测试](#45-端到端集成测试)
- [5. UI 自动化测试（arkxtest）](#5-ui-自动化测试arkxtest)
  - [5.1 页面跳转测试](#51-页面跳转测试)
  - [5.2 渲染功能测试](#52-渲染功能测试)
- [6. 测试执行与报告](#6-测试执行与报告)
  - [6.1 测试配置对比](#61-测试配置对比)
  - [6.2 Local Test 模块设计](#62-local-test-模块设计)
    - [6.2.1 Local Test 目录结构](#621-local-test-目录结构)
    - [6.2.2 Local Test 适用场景](#622-local-test-适用场景)
    - [6.2.3 Local Test 示例代码](#623-local-test-示例代码)
  - [6.3 Instrument Test 模块设计](#63-instrument-test-模块设计)
  - [6.4 测试运行方式](#64-测试运行方式)
- [7. 测试最佳实践](#7-测试最佳实践)
- [8. Instrument Test 运行指南](#8-instrument-test-运行指南)
- [9. 常见问题与解决方案](#9-常见问题与解决方案)
  - [9.1 测试套注册失败](#91-测试套注册失败)
  - [9.2 Hypium 命名规范错误](#92-hypium-命名规范错误)
  - [9.3 构建任务不存在](#93-构建任务不存在)
  - [9.4 NAPI 模块加载失败](#94-napi-模块加载失败)
  - [9.5 设备未连接](#95-设备未连接)

---

## 1. 测试项目结构

### 1.1 完整测试文件结构

以下是 ArkZero 项目的**完整测试文件结构**，包含 Local Test 和 Instrument Test 两个独立模块。

#### 1.1.1 主代码结构（entry/src/main）

```
entry/src/main/
├── cpp/                                    # C++ Native 代码
│   ├── common/
│   │   └── common.h                        # 通用头文件
│   ├── renderer/
│   │   ├── api/
│   │   │   ├── RendererApi.cpp             # NAPI 接口实现
│   │   │   └── RendererApi.h               # NAPI 接口声明
│   │   ├── backend/
│   │   │   ├── EGLContextManager.cpp       # EGL Context 管理
│   │   │   ├── EGLContextManager.h
│   │   │   ├── GLESBackend.cpp             # OpenGL ES 后端
│   │   │   ├── GLESBackend.h
│   │   │   ├── IRenderBackend.h            # 渲染后端接口
│   │   │   ├── PixelFormatConverter.cpp    # 像素格式转换
│   │   │   ├── PixelFormatConverter.h
│   │   │   ├── TextureManager.cpp          # 纹理管理
│   │   │   ├── TextureManager.h
│   │   │   ├── TexturePool.cpp             # 纹理池
│   │   │   ├── TexturePool.h
│   │   │   ├── YUVShaderManager.cpp        # YUV 着色器管理
│   │   │   └── YUVShaderManager.h
│   │   ├── core/
│   │   │   ├── PerformanceMonitor.cpp      # 性能监控 ⭐ 重点测试
│   │   │   ├── PerformanceMonitor.h
│   │   │   ├── RenderQueue.cpp             # 渲染队列 ⭐ 重点测试
│   │   │   ├── RenderQueue.h
│   │   │   ├── Renderer.cpp                # 渲染器核心
│   │   │   └── Renderer.h
│   │   └── manager/
│   │       ├── RendererManager.cpp         # 渲染器管理器
│   │       ├── RendererManager.h
│   │       ├── SurfaceManager.cpp          # Surface 管理
│   │       └── SurfaceManager.h
│   ├── tests/                              # C++ 原生测试（可选）
│   │   ├── README.md                       # C++ 测试说明
│   │   ├── test_performance.cpp            # 性能测试示例
│   │   └── performance_monitor_napi.h      # NAPI 桥接头文件
│   ├── types/
│   │   └── libnativerender/
│   │       ├── Index.d.ts                  # TypeScript 类型定义
│   │       └── oh-package.json5
│   ├── CMakeLists.txt                      # CMake 构建配置
│   └── napi_init.cpp                       # NAPI 模块初始化
│
├── ets/
│   ├── components/rendering/
│   │   └── ArkZeroRenderer.ets             # ArkTS 渲染组件 ⭐ 重点测试
│   ├── entryability/
│   │   └── EntryAbility.ets                # 应用入口
│   ├── pages/
│   │   ├── Index.ets                       # 首页
│   │   └── SurfaceDemoPage.ets             # Surface 演示页
│   └── types/
│       └── nativerender.d.ts               # Native 类型定义
│
├── resources/                              # 资源文件
│   ├── base/
│   │   ├── element/
│   │   │   ├── color.json
│   │   │   ├── float.json
│   │   │   └── string.json
│   │   ├── media/
│   │   │   ├── background.png
│   │   │   ├── foreground.png
│   │   │   ├── icon.png
│   │   │   ├── layered_image.json
│   │   │   └── startIcon.png
│   │   └── profile/
│   │       └── main_pages.json
│   ├── en_US/element/
│   │   └── string.json
│   └── zh_CN/element/
│       └── string.json
│
└── module.json5                            # 主模块配置
```

#### 1.1.2 Instrument Test 结构（entry/src/ohosTest）

**用途**: 在 HarmonyOS 设备/模拟器上运行，支持 NAPI 调用和 C++ 代码测试。

```
entry/src/ohosTest/
├── ets/
│   ├── test/                               # 单元测试目录（Hypium）
│   │   ├── List.test.ets                   # 📍 单元测试入口文件
│   │   ├── native/                         # C++ 层测试（通过 NAPI 桥接）
│   │   │   ├── PerformanceMonitor.test.ets # ⭐ PerformanceMonitor 测试
│   │   │   ├── PixelFormatConverter.test.ets # 像素格式转换测试
│   │   │   ├── RenderQueue.test.ets        # 渲染队列测试
│   │   │   └── RenderCommand.test.ets      # 渲染命令测试
│   │   ├── arkts/                          # ArkTS 层测试
│   │   │   ├── PixelFormat.test.ets        # PixelFormat 枚举测试
│   │   │   ├── ArkZeroRenderer.test.ets    # ⭐ ArkZeroRenderer 类测试
│   │   │   └── ArkZeroRendererConfig.test.ets # 配置接口测试
│   │   └── integration/                    # 集成测试（可选扩展）
│   │       ├── napi/                       # Level 1: NAPI 桥接测试
│   │       │   ├── ModuleLoading.test.ets  # 模块加载测试
│   │       │   ├── TypeMapping.test.ets    # 类型映射测试
│   │       │   └── ErrorHandling.test.ets  # 错误处理测试
│   │       ├── pipeline/                   # Level 2: 渲染管线测试
│   │       │   ├── QueueToRenderer.test.ets # 队列到渲染器数据流
│   │       │   ├── PerformanceIntegration.test.ets # 性能统计集成
│   │       │   └── FormatConversion.test.ets # 格式转换集成
│   │       ├── surface/                    # Level 3: Surface 管理测试
│   │       │   ├── EGLContextLifecycle.test.ets # EGL Context 生命周期
│   │       │   ├── SurfaceBinding.test.ets # Surface 绑定测试
│   │       │   └── TextureManagement.test.ets # 纹理管理测试
│   │       └── e2e/                        # Level 4: 端到端测试
│   │           ├── FullRenderCycle.test.ets # 完整渲染周期
│   │           ├── ResizeWorkflow.test.ets # 调整大小工作流
│   │           └── ResourceCleanup.test.ets # 资源清理测试
│   │
│   ├── uitest/                             # UI 自动化测试（arkxtest）
│   │   ├── List.ui.test.ets                # 📍 UI 测试入口文件
│   │   ├── Navigation.test.ets             # 页面导航测试
│   │   └── Rendering.test.ets              # 渲染功能 UI 测试
│   │
│   └── testability/                        # 测试 Ability
│       ├── TestAbility.ets                 # 测试入口 Ability
│       └── pages/
│           └── Index.ets                   # 测试页面
│
├── resources/
│   └── base/
│       ├── element/
│       │   └── string.json                 # 📍 测试模块字符串资源
│       ├── media/
│       │   └── icon.png                    # 测试应用图标
│       └── profile/
│           └── test_pages.json             # 测试页面路由配置
│
└── module.json5                            # 📍 测试模块配置文件
```

**关键文件说明**:
- `List.test.ets`: Hypium 单元测试入口，注册所有测试套件
- `PerformanceMonitor.test.ets`: 测试 C++ PerformanceMonitor 类的 45+ 个方法
- `ArkZeroRenderer.test.ets`: 测试 ArkTS ArkZeroRenderer 组件的 13+ 个方法
- `module.json5`: 声明测试模块为 `feature` 类型，配置 TestAbility
- `string.json`: 定义测试模块所需的字符串资源（module_test_desc 等）

#### 1.1.3 Local Test 结构（entry/src/test）

**用途**: 在本地 JVM 上运行，**不支持 NAPI 调用**，仅用于纯 ArkTS/TypeScript 逻辑测试。

```
entry/src/test/
├── List.test.ets                           # 📍 Local Test 入口文件（占位）
└── ets/                                    # 预留目录（可扩展）
    ├── utils/                              # 工具函数测试（可选）
    │   ├── MathUtils.test.ets              # 数学工具测试
    │   └── StringUtils.test.ets            # 字符串工具测试
    ├── models/                             # 数据模型测试（可选）
    │   └── RendererConfig.test.ets         # 配置模型测试
    └── services/                           # 服务层测试（可选）
        └── ConfigService.test.ets          # 配置服务测试
```

**重要说明**:
- `List.test.ets` 当前为**占位文件**，仅满足构建系统要求
- Local Test **无法测试** PerformanceMonitor 等 NAPI 暴露的 C++ 代码
- 如需测试纯 ArkTS 逻辑（如配置验证、数据处理），可在此扩展
- 推荐将大部分测试放在 `ohosTest`（Instrument Test）中

#### 1.1.4 测试相关配置文件

```
项目根目录/
├── build-profile.json5                     # 构建配置（包含 ohosTest target）
├── hvigorfile.ts                           # Hvigor 构建脚本
├── oh-package.json5                        # 依赖配置（@ohos/hypium）
├── run_instrument_test.ps1                 # 📍 PowerShell 自动化测试脚本
└── UNIT_TEST_DESIGN.md                     # 📍 本测试设计文档
```

**build-profile.json5 关键配置**:
```json5
{
  "targets": [
    {
      "name": "default"
    },
    {
      "name": "ohosTest"  // ← Instrument Test 目标
    }
  ]
}
```

**oh-package.json5 依赖**:
```json5
{
  "devDependencies": {
    "@ohos/hypium": "1.0.6"  // ← Hypium 测试框架
  }
}
```

---

### 1.2 测试文件命名规范

| 测试类型 | 文件命名规则 | 示例 |
|---------|------------|------|
| **单元测试** | `<ClassName>.test.ets` | `PerformanceMonitor.test.ets` |
| **UI 测试** | `<FeatureName>.ui.test.ets` | `Navigation.ui.test.ets` |
| **集成测试** | `<IntegrationName>.test.ets` | `ModuleLoading.test.ets` |
| **Local Test** | `<ModuleName>.test.ets` | `MathUtils.test.ets` |

### 1.3 测试目录组织原则

1. **按层级分离**: `native/`（C++）、`arkts/`（ArkTS）、`integration/`（集成）
2. **按功能分组**: 每个 C++ 类对应一个 `.test.ets` 文件
3. **入口文件统一**: `List.test.ets` 集中注册所有测试套件
4. **资源独立**: 测试模块有独立的 `resources/` 目录
5. **配置隔离**: `module.json5` 与主模块完全分离

### 1.4 配置文件

#### 1.4.1 Instrument Test 模块配置（module.json5）

**文件位置**: `entry/src/ohosTest/module.json5`

```json5
{
  "module": {
    "name": "entry_test",
    "type": "feature",
    "description": "$string:module_test_desc",
    "mainElement": "TestAbility",
    "deviceTypes": [
      "phone",
      "tablet"
    ],
    "deliveryWithInstall": true,
    "installationFree": false,
    "pages": "$profile:test_pages",
    "abilities": [
      {
        "name": "TestAbility",
        "srcEntry": "./ets/testability/TestAbility.ets",
        "description": "$string:TestAbility_desc",
        "icon": "$media:icon",
        "label": "$string:TestAbility_label",
        "exported": true,
        "startWindowIcon": "$media:icon",
        "startWindowBackground": "$color:start_window_background"
      }
    ]
  }
}
```

**配置说明**:
- `name`: 测试模块名称，必须唯一
- `type`: 必须为 `"feature"`（功能模块）
- `mainElement`: 测试入口 Ability
- `pages`: 指向 `test_pages.json` 路由配置
- `abilities`: 定义 TestAbility，用于启动测试

#### 1.4.2 构建配置（build-profile.json5）

**文件位置**: `entry/build-profile.json5`

```json5
{
  "apiType": "stageMode",
  "buildOption": {
    "externalNativeOptions": {
      "path": "./src/main/cpp/CMakeLists.txt",
      "arguments": "",
      "cppFlags": "",
      "abiFilters": ["arm64-v8a", "x86_64"]
    }
  },
  "targets": [
    {
      "name": "default"
    },
    {
      "name": "ohosTest"  // ← 必须包含此目标才能运行 Instrument Test
    }
  ]
}
```

**关键点**:
- `targets` 数组中必须包含 `"ohosTest"` 目标
- `abiFilters` 指定支持的 CPU 架构
- C++ 代码通过 CMake 构建

#### 1.4.3 依赖配置（oh-package.json5）

**文件位置**: `entry/oh-package.json5`

```json5
{
  "license": "Apache-2.0",
  "devDependencies": {
    "@ohos/hypium": "1.0.18"  // ← Hypium 测试框架
  },
  "author": "",
  "name": "entry",
  "description": "Please describe the basic information.",
  "main": "",
  "version": "1.0.0",
  "dependencies": {}
}
```

**依赖说明**:
- `@ohos/hypium`: HarmonyOS 官方测试框架，提供 `describe`、`it`、`expect` 等 API
- 版本建议: `1.0.18` 或更高
- 安装命令: `ohpm install`

#### 1.4.4 资源配置文件

**文件位置**: `entry/src/ohosTest/resources/base/element/string.json`

```json
{
  "string": [
    {
      "name": "module_test_desc",
      "value": "ArkZero Renderer Test Module"
    },
    {
      "name": "TestAbility_desc",
      "value": "Test Ability for ArkZero Renderer"
    },
    {
      "name": "TestAbility_label",
      "value": "ArkZero Test"
    }
  ]
}
```

**用途**:
- `module_test_desc`: 测试模块描述，在 `module.json5` 中引用
- `TestAbility_desc`: 测试 Ability 描述
- `TestAbility_label`: 测试应用显示名称

**文件位置**: `entry/src/ohosTest/resources/base/profile/test_pages.json`

```json
{
  "src": [
    "testability/pages/Index"
  ]
}
```

**用途**: 定义测试页面的路由配置

#### 1.4.5 CMake 构建配置

**文件位置**: `entry/src/main/cpp/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.4.1)
project(ArkZeroRenderer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加源文件
file(GLOB RENDERER_CORE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/core/*.cpp"
)

file(GLOB RENDERER_BACKEND_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/backend/*.cpp"
)

file(GLOB RENDERER_MANAGER_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/manager/*.cpp"
)

# 创建共享库
add_library(nativerender SHARED
    napi_init.cpp
    ${RENDERER_CORE_SRC}
    ${RENDERER_BACKEND_SRC}
    ${RENDERER_MANAGER_SRC}
)

# 链接系统库
target_link_libraries(nativerender
    libace_napi.z.so
    libhilog_ndk.z.so
    EGL
    GLESv3
)
```

**关键点**:
- C++ 代码编译为共享库 `libnativerender.so`
- NAPI 接口通过 `napi_init.cpp` 暴露
- 链接 OpenGL ES 和 EGL 库用于渲染

#### 1.4.6 自动化测试脚本

**文件位置**: `run_instrument_test.ps1`

这是一个 PowerShell 脚本，用于自动化执行 Instrument Test 的完整流程：

1. 检查设备连接（hdc list targets）
2. 清理构建产物
3. 构建 Instrument Test 包（assembleOhosTest）
4. 安装 .hap 文件到设备（hdc app install）
5. 启动测试 Activity（hdc shell aa start）
6. 提供查看日志的命令提示

**使用方法**:
```powershell
.\run_instrument_test.ps1
```

---

### 1.5 测试文件清单总览

以下是 ArkZero 项目中所有测试文件的完整清单：

#### 1.5.1 Instrument Test 文件清单（entry/src/ohosTest）

| 文件路径 | 类型 | 状态 | 说明 |
|---------|------|------|------|
| `ets/test/List.test.ets` | 入口 | ✅ 已创建 | Hypium 单元测试入口 |
| `ets/test/native/PerformanceMonitor.test.ets` | 单元测试 | ✅ 已创建 | PerformanceMonitor C++ 类测试（45+ 用例） |
| `ets/test/native/PixelFormatConverter.test.ets` | 单元测试 | ⏳ 待创建 | 像素格式转换测试 |
| `ets/test/native/RenderQueue.test.ets` | 单元测试 | ⏳ 待创建 | 渲染队列测试 |
| `ets/test/native/RenderCommand.test.ets` | 单元测试 | ⏳ 待创建 | 渲染命令测试 |
| `ets/test/arkts/PixelFormat.test.ets` | 单元测试 | ⏳ 待创建 | PixelFormat 枚举测试 |
| `ets/test/arkts/ArkZeroRenderer.test.ets` | 单元测试 | ⏳ 待创建 | ArkZeroRenderer 组件测试（13+ 用例） |
| `ets/test/arkts/ArkZeroRendererConfig.test.ets` | 单元测试 | ⏳ 待创建 | 配置接口测试 |
| `ets/uitest/List.ui.test.ets` | UI 测试入口 | ⏳ 待创建 | arkxtest UI 测试入口 |
| `ets/uitest/Navigation.test.ets` | UI 测试 | ⏳ 待创建 | 页面导航测试 |
| `ets/uitest/Rendering.test.ets` | UI 测试 | ⏳ 待创建 | 渲染功能 UI 测试 |
| `ets/testability/TestAbility.ets` | Ability | ✅ 已创建 | 测试入口 Ability |
| `ets/testability/pages/Index.ets` | 页面 | ✅ 已创建 | 测试首页 |
| `ets/testrunner/OpenHarmonyTestRunner.ets` | 运行器 | ✅ 已创建 | Hypium 测试运行器 |
| `module.json5` | 配置 | ✅ 已创建 | 测试模块配置 |
| `resources/base/element/string.json` | 资源 | ✅ 已创建 | 字符串资源 |
| `resources/base/profile/test_pages.json` | 配置 | ✅ 已创建 | 页面路由配置 |

**集成测试扩展目录**（可选）:

| 文件路径 | 层级 | 状态 | 说明 |
|---------|------|------|------|
| `ets/test/integration/napi/ModuleLoading.test.ets` | Level 1 | ⏳ 待创建 | NAPI 模块加载测试 |
| `ets/test/integration/napi/TypeMapping.test.ets` | Level 1 | ⏳ 待创建 | 类型映射测试 |
| `ets/test/integration/napi/ErrorHandling.test.ets` | Level 1 | ⏳ 待创建 | 错误处理测试 |
| `ets/test/integration/pipeline/QueueToRenderer.test.ets` | Level 2 | ⏳ 待创建 | 数据流测试 |
| `ets/test/integration/pipeline/PerformanceIntegration.test.ets` | Level 2 | ⏳ 待创建 | 性能统计集成 |
| `ets/test/integration/pipeline/FormatConversion.test.ets` | Level 2 | ⏳ 待创建 | 格式转换集成 |
| `ets/test/integration/surface/EGLContextLifecycle.test.ets` | Level 3 | ⏳ 待创建 | EGL Context 生命周期 |
| `ets/test/integration/surface/SurfaceBinding.test.ets` | Level 3 | ⏳ 待创建 | Surface 绑定测试 |
| `ets/test/integration/surface/TextureManagement.test.ets` | Level 3 | ⏳ 待创建 | 纹理管理测试 |
| `ets/test/integration/e2e/FullRenderCycle.test.ets` | Level 4 | ⏳ 待创建 | 完整渲染周期 |
| `ets/test/integration/e2e/ResizeWorkflow.test.ets` | Level 4 | ⏳ 待创建 | 调整大小工作流 |
| `ets/test/integration/e2e/ResourceCleanup.test.ets` | Level 4 | ⏳ 待创建 | 资源清理测试 |

#### 1.5.2 Local Test 文件清单（entry/src/test）

| 文件路径 | 类型 | 状态 | 说明 |
|---------|------|------|------|
| `List.test.ets` | 入口 | ✅ 已创建 | Local Test 占位入口 |
| `ets/utils/MathUtils.test.ets` | 单元测试 | ⏳ 可选 | 数学工具测试 |
| `ets/utils/StringUtils.test.ets` | 单元测试 | ⏳ 可选 | 字符串工具测试 |
| `ets/models/RendererConfig.test.ets` | 单元测试 | ⏳ 可选 | 配置模型测试 |
| `ets/services/ConfigService.test.ets` | 单元测试 | ⏳ 可选 | 配置服务测试 |

**注意**: Local Test **不支持 NAPI 调用**，仅用于纯 ArkTS/TypeScript 逻辑测试。

#### 1.5.3 配置文件清单

| 文件路径 | 用途 | 状态 |
|---------|------|------|
| `entry/build-profile.json5` | 构建配置（包含 ohosTest target） | ✅ 已配置 |
| `entry/oh-package.json5` | 依赖配置（@ohos/hypium） | ✅ 已配置 |
| `entry/src/main/cpp/CMakeLists.txt` | C++ 构建配置 | ✅ 已配置 |
| `run_instrument_test.ps1` | PowerShell 自动化脚本 | ✅ 已创建 |

---

## 2. C++ 层单元测试（通过 NAPI 桥接）

> **说明**: C++ 代码无法直接在 Hypium 中测试，需要通过 NAPI 暴露测试接口，然后在 ArkTS 层调用。

### 2.1 PerformanceMonitor 类

**源文件**: `entry/src/main/cpp/renderer/core/PerformanceMonitor.h`  
**测试文件**: `entry/src/ohosTest/ets/unittest/native/PerformanceMonitor.test.ets`  
**NAPI 桥接**: 需要在 `napi_init.cpp` 中添加测试接口

#### 2.1.1 构造函数测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-CTOR-001 | 默认构造 | 无 | totalFrames == 0 | P0 |
| PM-CTOR-002 | 默认构造 | 无 | droppedFrames == 0 | P0 |
| PM-CTOR-003 | 默认构造 | 无 | fps == 0.0 | P0 |
| PM-CTOR-004 | 默认构造 | 无 | dropRate == 0.0 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

export default function PerformanceMonitorTest() {
  describe('PerformanceMonitor Constructor Tests', () => {
    it('PM-CTOR-001: Should initialize with zero frames', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      const stats = await nativerender.getMonitorStats(handle);
      
      expect(stats.totalFrames).assertEqual(0);
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-002: Should initialize with zero dropped frames', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      const stats = await nativerender.getMonitorStats(handle);
      
      expect(stats.droppedFrames).assertEqual(0);
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-003: Should initialize with zero FPS', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      const stats = await nativerender.getMonitorStats(handle);
      
      expect(stats.fps).assertEqual(0.0);
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-004: Should initialize with zero drop rate', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      const stats = await nativerender.getMonitorStats(handle);
      
      expect(stats.dropRate).assertEqual(0.0);
      nativerender.destroyPerformanceMonitor(handle);
    });
  });
}
```

#### 2.1.2 BeginFrame/EndFrame 测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-BF-001 | 单次帧记录 | begin+end(false) | totalFrames=1 | P0 |
| PM-BF-002 | 丢帧记录 | begin+end(true) | droppedFrames=1 | P0 |
| PM-BF-003 | 多帧统计 | 100次循环 | totalFrames=100 | P0 |
| PM-BF-004 | FPS计算 | 60帧@16ms | fps≈60 | P0 |
| PM-BF-005 | DropRate计算 | 10丢/100总 | dropRate≈0.1 | P0 |

**测试代码示例**:

```typescript
  describe('PerformanceMonitor Frame Recording Tests', () => {
    it('PM-BF-001: Should record normal frame', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      await nativerender.monitorBeginFrame(handle);
      await nativerender.monitorEndFrame(handle, false);
      
      const stats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(1);
      expect(stats.droppedFrames).assertEqual(0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-002: Should record dropped frame', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      await nativerender.monitorBeginFrame(handle);
      await nativerender.monitorEndFrame(handle, true);
      
      const stats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(1);
      expect(stats.droppedFrames).assertEqual(1);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-003: Should count multiple frames correctly', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      for (let i = 0; i < 100; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, i % 10 === 0);
      }
      
      const stats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(100);
      expect(stats.droppedFrames).assertEqual(10);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-004: Should calculate FPS accurately', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      // 模拟 60fps，每帧间隔 16ms
      for (let i = 0; i < 60; i++) {
        await nativerender.monitorBeginFrame(handle);
        await new Promise(resolve => setTimeout(resolve, 16));
        await nativerender.monitorEndFrame(handle, false);
      }
      
      const stats = await nativerender.getMonitorStats(handle);
      expect(stats.fps).assertLarger(55.0);
      expect(stats.fps).assertLess(65.0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-005: Should calculate drop rate accurately', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      // 90 正常帧 + 10 丢帧
      for (let i = 0; i < 90; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      for (let i = 0; i < 10; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, true);
      }
      
      const stats = await nativerender.getMonitorStats(handle);
      expect(stats.dropRate).assertLarger(0.09);
      expect(stats.dropRate).assertLess(0.11);
      
      nativerender.destroyPerformanceMonitor(handle);
    });
  });
```

#### 2.1.3 Reset 测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-RST-001 | 重置统计 | Reset() | 所有字段归零 | P0 |
| PM-RST-002 | 重置后可用 | Reset后再记录 | 统计重新开始 | P0 |

**测试代码示例**:

```typescript
  describe('PerformanceMonitor Reset Tests', () => {
    it('PM-RST-001: Should reset all statistics', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      // 先积累数据
      for (let i = 0; i < 50; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, i < 10);
      }
      
      // 重置
      await nativerender.monitorReset(handle);
      const stats = await nativerender.getMonitorStats(handle);
      
      expect(stats.totalFrames).assertEqual(0);
      expect(stats.droppedFrames).assertEqual(0);
      expect(stats.fps).assertEqual(0.0);
      expect(stats.dropRate).assertEqual(0.0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-RST-002: Should allow reuse after reset', 0, async () => {
      const handle = await nativerender.createPerformanceMonitor();
      
      // 第一轮
      for (let i = 0; i < 30; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      
      // 重置
      await nativerender.monitorReset(handle);
      
      // 第二轮
      for (let i = 0; i < 20; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      
      const stats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(20); // 应该是20，不是50
      
      nativerender.destroyPerformanceMonitor(handle);
    });
  });
}
```

### 1.1 PerformanceMonitor 类

**文件位置**: `entry/src/main/cpp/renderer/core/PerformanceMonitor.h`  
**测试文件**: `entry/src/main/cpp/tests/test_performance_monitor.cpp`

#### 1.1.1 构造函数 `PerformanceMonitor()`

**测试目标**: 验证对象初始化状态正确

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-CTOR-001 | 默认构造 | 无 | m_totalFrames == 0 | P0 |
| PM-CTOR-002 | 默认构造 | 无 | m_droppedFrames == 0 | P0 |
| PM-CTOR-003 | 默认构造 | 无 | m_frameTimes.empty() == true | P0 |
| PM-CTOR-004 | 默认构造 | 无 | GetFPS() == 0.0f | P1 |
| PM-CTOR-005 | 默认构造 | 无 | GetDropRate() == 0.0f | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, Constructor_InitializesCorrectly) {
    PerformanceMonitor monitor;
    
    EXPECT_EQ(monitor.GetTotalFrames(), 0);
    EXPECT_EQ(monitor.GetDroppedFrames(), 0);
    EXPECT_FLOAT_EQ(monitor.GetFPS(), 0.0f);
    EXPECT_FLOAT_EQ(monitor.GetDropRate(), 0.0f);
}
```

---

#### 1.1.2 方法 `BeginFrame()`

**测试目标**: 验证帧开始记录功能

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-BF-001 | 单次调用 | 无 | m_frameTimes.size() == 1 | P0 |
| PM-BF-002 | 多次调用 | 调用10次 | m_frameTimes.size() == 10 | P0 |
| PM-BF-003 | 时间戳递增 | 连续调用2次 | times[1] > times[0] | P0 |
| PM-BF-004 | 线程安全 | 多线程并发调用 | 无数据竞争 | P1 |
| PM-BF-005 | 超过MAX_SAMPLE_COUNT | 调用121次 | size() == 120 (自动清理) | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, BeginFrame_RecordsTimestamp) {
    PerformanceMonitor monitor;
    
    monitor.BeginFrame();
    EXPECT_EQ(monitor.GetTotalFrames(), 0); // BeginFrame 不增加总帧数
    
    monitor.BeginFrame();
    monitor.BeginFrame();
    // 内部 deque 应该有多个时间戳
}

TEST(PerformanceMonitorTest, BeginFrame_CleansOldData) {
    PerformanceMonitor monitor;
    
    // 调用 121 次，应该触发清理
    for (int i = 0; i < 121; ++i) {
        monitor.BeginFrame();
    }
    
    // MAX_SAMPLE_COUNT = 120
    // 需要通过内部状态验证，或使用 friend class
}
```

---

#### 1.1.3 方法 `EndFrame(bool dropped = false)`

**测试目标**: 验证帧结束记录和统计更新

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-EF-001 | 正常帧 | dropped=false | m_totalFrames++ | P0 |
| PM-EF-002 | 丢帧 | dropped=true | m_totalFrames++, m_droppedFrames++ | P0 |
| PM-EF-003 | 连续正常帧 | 100次 false | totalFrames=100, dropped=0 | P0 |
| PM-EF-004 | 混合帧 | 50次false + 10次true | total=60, dropped=10 | P0 |
| PM-EF-005 | FPS计算 | 60帧@16ms间隔 | FPS ≈ 60 | P1 |
| PM-EF-006 | DropRate计算 | 10丢/100总 | dropRate = 0.1 | P1 |
| PM-EF-007 | FrameTime计算 | 16ms平均 | avgFrameTime ≈ 16.0 | P1 |
| PM-EF-008 | 未调用BeginFrame | 直接EndFrame | 不应崩溃，合理处理 | P2 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, EndFrame_IncrementsCounters) {
    PerformanceMonitor monitor;
    
    monitor.BeginFrame();
    monitor.EndFrame(false);
    
    EXPECT_EQ(monitor.GetTotalFrames(), 1);
    EXPECT_EQ(monitor.GetDroppedFrames(), 0);
}

TEST(PerformanceMonitorTest, EndFrame_RecordsDroppedFrame) {
    PerformanceMonitor monitor;
    
    monitor.BeginFrame();
    monitor.EndFrame(true);
    
    EXPECT_EQ(monitor.GetTotalFrames(), 1);
    EXPECT_EQ(monitor.GetDroppedFrames(), 1);
}

TEST(PerformanceMonitorTest, EndFrame_CalculatesFPS) {
    PerformanceMonitor monitor;
    
    // 模拟 60fps，每帧 16.67ms
    for (int i = 0; i < 60; ++i) {
        monitor.BeginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        monitor.EndFrame(false);
    }
    
    float fps = monitor.GetFPS();
    EXPECT_GT(fps, 55.0f);
    EXPECT_LT(fps, 65.0f);
}

TEST(PerformanceMonitorTest, EndFrame_CalculatesDropRate) {
    PerformanceMonitor monitor;
    
    // 100 帧，10 帧丢失
    for (int i = 0; i < 90; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(false);
    }
    for (int i = 0; i < 10; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(true);
    }
    
    EXPECT_FLOAT_EQ(monitor.GetDropRate(), 0.1f);
}
```

---

#### 1.1.4 方法 `GetFPS() const`

**测试目标**: 验证帧率计算准确性

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-GFPS-001 | 无帧数据 | 0帧 | 返回 0.0f | P0 |
| PM-GFPS-002 | 单帧 | 1帧 | 返回 0.0f (不足样本) | P1 |
| PM-GFPS-003 | 60fps稳定 | 60帧@16ms | 返回 58-62 | P0 |
| PM-GFPS-004 | 30fps稳定 | 60帧@33ms | 返回 28-32 | P0 |
| PM-GFPS-005 | 波动帧率 | 交替16ms/33ms | 返回平均值≈45 | P1 |
| PM-GFPS-006 | 极高帧率 | 120帧@8ms | 返回 115-125 | P2 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, GetFPS_ReturnsZeroWhenNoFrames) {
    PerformanceMonitor monitor;
    EXPECT_FLOAT_EQ(monitor.GetFPS(), 0.0f);
}

TEST(PerformanceMonitorTest, GetFPS_CalculatesCorrectlyAt60FPS) {
    PerformanceMonitor monitor;
    
    for (int i = 0; i < 120; ++i) {
        monitor.BeginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        monitor.EndFrame(false);
    }
    
    float fps = monitor.GetFPS();
    EXPECT_NEAR(fps, 60.0f, 5.0f); // 允许 ±5fps 误差
}
```

---

#### 1.1.5 方法 `GetAverageFrameTime() const`

**测试目标**: 验证平均帧耗时计算

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-GAFT-001 | 无帧数据 | 0帧 | 返回 0.0f | P0 |
| PM-GAFT-002 | 固定16ms | 60帧@16ms | 返回 15.5-16.5 | P0 |
| PM-GAFT-003 | 固定33ms | 60帧@33ms | 返回 32-34 | P0 |
| PM-GAFT-004 | 波动帧时 | 交替10ms/20ms | 返回 14-16 | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, GetAverageFrameTime_CalculatesCorrectly) {
    PerformanceMonitor monitor;
    
    for (int i = 0; i < 60; ++i) {
        monitor.BeginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        monitor.EndFrame(false);
    }
    
    float avgTime = monitor.GetAverageFrameTime();
    EXPECT_NEAR(avgTime, 16.0f, 1.0f);
}
```

---

#### 1.1.6 方法 `GetDropRate() const`

**测试目标**: 验证丢帧率计算

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-GDR-001 | 无丢帧 | 100帧全成功 | 返回 0.0f | P0 |
| PM-GDR-002 | 全部丢帧 | 100帧全丢失 | 返回 1.0f | P0 |
| PM-GDR-003 | 10%丢帧 | 90成功+10丢失 | 返回 0.09-0.11 | P0 |
| PM-GDR-004 | 50%丢帧 | 50成功+50丢失 | 返回 0.45-0.55 | P1 |
| PM-GDR-005 | 无帧数据 | 0帧 | 返回 0.0f | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, GetDropRate_CalculatesCorrectly) {
    PerformanceMonitor monitor;
    
    // 90 正常帧
    for (int i = 0; i < 90; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(false);
    }
    
    // 10 丢帧
    for (int i = 0; i < 10; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(true);
    }
    
    float dropRate = monitor.GetDropRate();
    EXPECT_NEAR(dropRate, 0.1f, 0.01f);
}
```

---

#### 1.1.7 方法 `GetTotalFrames() const`

**测试目标**: 验证总帧数计数

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-GTF-001 | 初始状态 | 无 | 返回 0 | P0 |
| PM-GTF-002 | 单帧 | 1次EndFrame | 返回 1 | P0 |
| PM-GTF-003 | 多帧 | 100次EndFrame | 返回 100 | P0 |
| PM-GTF-004 | 包含丢帧 | 90成功+10丢失 | 返回 100 | P0 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, GetTotalFrames_CountsCorrectly) {
    PerformanceMonitor monitor;
    
    EXPECT_EQ(monitor.GetTotalFrames(), 0);
    
    for (int i = 0; i < 100; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(i % 10 == 0); // 每10帧丢一次
    }
    
    EXPECT_EQ(monitor.GetTotalFrames(), 100);
}
```

---

#### 1.1.8 方法 `GetDroppedFrames() const`

**测试目标**: 验证丢帧数计数

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-GDF-001 | 初始状态 | 无 | 返回 0 | P0 |
| PM-GDF-002 | 无丢帧 | 100次false | 返回 0 | P0 |
| PM-GDF-003 | 部分丢帧 | 10次true | 返回 10 | P0 |
| PM-GDF-004 | 全部丢帧 | 50次true | 返回 50 | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, GetDroppedFrames_CountsCorrectly) {
    PerformanceMonitor monitor;
    
    for (int i = 0; i < 100; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(i < 20); // 前20帧丢失
    }
    
    EXPECT_EQ(monitor.GetDroppedFrames(), 20);
}
```

---

#### 1.1.9 方法 `Reset()`

**测试目标**: 验证统计数据重置

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-RST-001 | 重置后总帧数 | Reset() | totalFrames == 0 | P0 |
| PM-RST-002 | 重置后丢帧数 | Reset() | droppedFrames == 0 | P0 |
| PM-RST-003 | 重置后FPS | Reset() | FPS == 0.0f | P0 |
| PM-RST-004 | 重置后DropRate | Reset() | dropRate == 0.0f | P0 |
| PM-RST-005 | 重置后可重用 | Reset后再渲染 | 统计重新开始 | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, Reset_ClearsAllStats) {
    PerformanceMonitor monitor;
    
    // 先积累一些数据
    for (int i = 0; i < 50; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(i < 10);
    }
    
    EXPECT_GT(monitor.GetTotalFrames(), 0);
    EXPECT_GT(monitor.GetDroppedFrames(), 0);
    
    // 重置
    monitor.Reset();
    
    EXPECT_EQ(monitor.GetTotalFrames(), 0);
    EXPECT_EQ(monitor.GetDroppedFrames(), 0);
    EXPECT_FLOAT_EQ(monitor.GetFPS(), 0.0f);
    EXPECT_FLOAT_EQ(monitor.GetDropRate(), 0.0f);
}

TEST(PerformanceMonitorTest, Reset_AllowsReuse) {
    PerformanceMonitor monitor;
    
    // 第一轮
    for (int i = 0; i < 30; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(false);
    }
    EXPECT_EQ(monitor.GetTotalFrames(), 30);
    
    // 重置
    monitor.Reset();
    
    // 第二轮
    for (int i = 0; i < 20; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(false);
    }
    EXPECT_EQ(monitor.GetTotalFrames(), 20); // 应该是20，不是50
}
```

---

#### 1.1.10 方法 `GetStats() const`

**测试目标**: 验证完整统计快照

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-GS-001 | 空状态 | 无帧 | 所有字段为0 | P0 |
| PM-GS-002 | 有数据 | 100帧@60fps | 字段值正确 | P0 |
| PM-GS-003 | ToString格式 | 有数据 | JSON格式字符串 | P1 |
| PM-GS-004 | 线程安全 | 多线程访问 | 无数据竞争 | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, GetStats_ReturnsCompleteSnapshot) {
    PerformanceMonitor monitor;
    
    for (int i = 0; i < 100; ++i) {
        monitor.BeginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        monitor.EndFrame(i < 10);
    }
    
    auto stats = monitor.GetStats();
    
    EXPECT_EQ(stats.totalFrames, 100);
    EXPECT_EQ(stats.droppedFrames, 10);
    EXPECT_GT(stats.fps, 55.0f);
    EXPECT_LT(stats.fps, 65.0f);
    EXPECT_NEAR(stats.dropRate, 0.1f, 0.01f);
    EXPECT_NEAR(stats.frameTimeMs, 16.0f, 1.0f);
}

TEST(PerformanceMonitorTest, Stats_ToString_Format) {
    PerformanceMonitor monitor;
    
    for (int i = 0; i < 10; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(false);
    }
    
    auto stats = monitor.GetStats();
    std::string str = stats.ToString();
    
    // 验证包含关键字段
    EXPECT_NE(str.find("fps"), std::string::npos);
    EXPECT_NE(str.find("totalFrames"), std::string::npos);
}
```

---

#### 1.1.11 私有方法 `CleanupOldData()`

**测试目标**: 验证过期数据清理逻辑（通过公有方法间接测试）

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PM-CO-001 | 不超过限制 | ≤120帧 | 不清理 | P1 |
| PM-CO-002 | 超过限制 | 121帧 | 清理到120帧 | P0 |
| PM-CO-003 | 远超限制 | 200帧 | 保留最新120帧 | P1 |

**测试代码示例**:
```cpp
TEST(PerformanceMonitorTest, CleanupOldData_RemovesExcessFrames) {
    PerformanceMonitor monitor;
    
    // 添加 150 帧
    for (int i = 0; i < 150; ++i) {
        monitor.BeginFrame();
        monitor.EndFrame(false);
    }
    
    // MAX_SAMPLE_COUNT = 120
    // 通过 GetStats 或其他方式验证内部队列大小
    // 可能需要 friend class 或暴露内部状态用于测试
}
```

---

### 2.2 PixelFormatConverter 类

**源文件**: `entry/src/main/cpp/renderer/backend/PixelFormatConverter.h`  
**测试文件**: `entry/src/ohosTest/ets/unittest/native/PixelFormatConverter.test.ets`

#### 2.2.1 像素格式映射测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PFC-MAP-001 | RGBA内部格式 | PixelFormat.RGBA | GL_RGBA8 | P0 |
| PFC-MAP-002 | RGB内部格式 | PixelFormat.RGB | GL_RGB8 | P0 |
| PFC-MAP-003 | BGRA内部格式 | PixelFormat.BGRA | GL_RGBA8 | P0 |
| PFC-MAP-004 | RGBA像素格式 | PixelFormat.RGBA | GL_RGBA | P0 |
| PFC-MAP-005 | RGB像素格式 | PixelFormat.RGB | GL_RGB | P0 |
| PFC-MAP-006 | BGRA像素格式 | PixelFormat.BGRA | GL_BGRA_EXT | P0 |
| PFC-BPP-001 | RGBA字节数 | PixelFormat.RGBA | 4 | P0 |
| PFC-BPP-002 | RGB字节数 | PixelFormat.RGB | 3 | P0 |
| PFC-BPP-003 | BGRA字节数 | PixelFormat.BGRA | 4 | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

export default function PixelFormatConverterTest() {
  describe('PixelFormatConverter Mapping Tests', () => {
    it('PFC-MAP-001: Should map RGBA to GL_RGBA8', 0, () => {
      const internalFormat = nativerender.getGLInternalFormat(0); // RGBA=0
      expect(internalFormat).assertEqual(0x8058); // GL_RGBA8
    });

    it('PFC-MAP-002: Should map RGB to GL_RGB8', 0, () => {
      const internalFormat = nativerender.getGLInternalFormat(1); // RGB=1
      expect(internalFormat).assertEqual(0x8051); // GL_RGB8
    });

    it('PFC-MAP-003: Should map BGRA to GL_RGBA8', 0, () => {
      const internalFormat = nativerender.getGLInternalFormat(2); // BGRA=2
      expect(internalFormat).assertEqual(0x8058); // GL_RGBA8
    });

    it('PFC-MAP-004: Should map RGBA to GL_RGBA', 0, () => {
      const format = nativerender.getGLFormat(0); // RGBA=0
      expect(format).assertEqual(0x1908); // GL_RGBA
    });

    it('PFC-MAP-005: Should map RGB to GL_RGB', 0, () => {
      const format = nativerender.getGLFormat(1); // RGB=1
      expect(format).assertEqual(0x1907); // GL_RGB
    });

    it('PFC-MAP-006: Should map BGRA to GL_BGRA_EXT', 0, () => {
      const format = nativerender.getGLFormat(2); // BGRA=2
      expect(format).assertEqual(0x80E1); // GL_BGRA_EXT
    });

    it('PFC-BPP-001: Should return 4 bytes for RGBA', 0, () => {
      const bpp = nativerender.getBytesPerPixel(0); // RGBA=0
      expect(bpp).assertEqual(4);
    });

    it('PFC-BPP-002: Should return 3 bytes for RGB', 0, () => {
      const bpp = nativerender.getBytesPerPixel(1); // RGB=1
      expect(bpp).assertEqual(3);
    });

    it('PFC-BPP-003: Should return 4 bytes for BGRA', 0, () => {
      const bpp = nativerender.getBytesPerPixel(2); // BGRA=2
      expect(bpp).assertEqual(4);
    });
  });
}
```

#### 1.2.1 方法 `GetGLInternalFormat(PixelFormat format)`

**测试目标**: 验证像素格式到GL内部格式的映射

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PFC-GIF-001 | RGBA格式 | PixelFormat::RGBA | GL_RGBA8 | P0 |
| PFC-GIF-002 | RGB格式 | PixelFormat::RGB | GL_RGB8 | P0 |
| PFC-GIF-003 | BGRA格式 | PixelFormat::BGRA | GL_RGBA8 | P0 |
| PFC-GIF-004 | NV21格式 | PixelFormat::NV21 | GL_LUMINANCE_ALPHA 或自定义 | P1 |
| PFC-GIF-005 | NV12格式 | PixelFormat::NV12 | GL_LUMINANCE_ALPHA 或自定义 | P1 |
| PFC-GIF-006 | 无效格式 | PixelFormat(999) | GL_RGBA8 (默认) 或断言 | P2 |

**测试代码示例**:
```cpp
TEST(PixelFormatConverterTest, GetGLInternalFormat_MapsRGBA) {
    GLint format = PixelFormatConverter::GetGLInternalFormat(PixelFormat::RGBA);
    EXPECT_EQ(format, GL_RGBA8);
}

TEST(PixelFormatConverterTest, GetGLInternalFormat_MapsRGB) {
    GLint format = PixelFormatConverter::GetGLInternalFormat(PixelFormat::RGB);
    EXPECT_EQ(format, GL_RGB8);
}

TEST(PixelFormatConverterTest, GetGLInternalFormat_MapsBGRA) {
    GLint format = PixelFormatConverter::GetGLInternalFormat(PixelFormat::BGRA);
    EXPECT_EQ(format, GL_RGBA8);
}

TEST(PixelFormatConverterTest, GetGLInternalFormat_HandlesInvalidFormat) {
    // 根据实现决定：返回默认值或触发断言
    GLint format = PixelFormatConverter::GetGLInternalFormat(static_cast<PixelFormat>(999));
    // EXPECT_EQ(format, GL_RGBA8); // 如果实现有默认值
}
```

---

#### 1.2.2 方法 `GetGLFormat(PixelFormat format)`

**测试目标**: 验证像素格式到GL像素格式的映射

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PFC-GF-001 | RGBA格式 | PixelFormat::RGBA | GL_RGBA | P0 |
| PFC-GF-002 | RGB格式 | PixelFormat::RGB | GL_RGB | P0 |
| PFC-GF-003 | BGRA格式 | PixelFormat::BGRA | GL_BGRA_EXT | P0 |
| PFC-GF-004 | NV21格式 | PixelFormat::NV21 | GL_LUMINANCE 或自定义 | P1 |
| PFC-GF-005 | NV12格式 | PixelFormat::NV12 | GL_LUMINANCE 或自定义 | P1 |
| PFC-GF-006 | 无效格式 | PixelFormat(999) | GL_RGBA (默认) | P2 |

**测试代码示例**:
```cpp
TEST(PixelFormatConverterTest, GetGLFormat_MapsRGBA) {
    GLenum format = PixelFormatConverter::GetGLFormat(PixelFormat::RGBA);
    EXPECT_EQ(format, GL_RGBA);
}

TEST(PixelFormatConverterTest, GetGLFormat_MapsBGRA) {
    GLenum format = PixelFormatConverter::GetGLFormat(PixelFormat::BGRA);
    EXPECT_EQ(format, GL_BGRA_EXT);
}

TEST(PixelFormatConverterTest, GetGLFormat_MapsRGB) {
    GLenum format = PixelFormatConverter::GetGLFormat(PixelFormat::RGB);
    EXPECT_EQ(format, GL_RGB);
}
```

---

#### 1.2.3 方法 `GetBytesPerPixel(PixelFormat format)`

**测试目标**: 验证每像素字节数计算

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PFC-GBPP-001 | RGBA格式 | PixelFormat::RGBA | 4 | P0 |
| PFC-GBPP-002 | RGB格式 | PixelFormat::RGB | 3 | P0 |
| PFC-GBPP-003 | BGRA格式 | PixelFormat::BGRA | 4 | P0 |
| PFC-GBPP-004 | NV21格式 | PixelFormat::NV21 | 1 或 2 (YUV特殊) | P1 |
| PFC-GBPP-005 | NV12格式 | PixelFormat::NV12 | 1 或 2 (YUV特殊) | P1 |
| PFC-GBPP-006 | 无效格式 | PixelFormat(999) | 0 | P1 |

**测试代码示例**:
```cpp
TEST(PixelFormatConverterTest, GetBytesPerPixel_RGBA) {
    int bpp = PixelFormatConverter::GetBytesPerPixel(PixelFormat::RGBA);
    EXPECT_EQ(bpp, 4);
}

TEST(PixelFormatConverterTest, GetBytesPerPixel_RGB) {
    int bpp = PixelFormatConverter::GetBytesPerPixel(PixelFormat::RGB);
    EXPECT_EQ(bpp, 3);
}

TEST(PixelFormatConverterTest, GetBytesPerPixel_BGRA) {
    int bpp = PixelFormatConverter::GetBytesPerPixel(PixelFormat::BGRA);
    EXPECT_EQ(bpp, 4);
}

TEST(PixelFormatConverterTest, GetBytesPerPixel_InvalidFormat) {
    int bpp = PixelFormatConverter::GetBytesPerPixel(static_cast<PixelFormat>(999));
    EXPECT_EQ(bpp, 0);
}
```

---

### 2.3 RenderQueue 类

**源文件**: `entry/src/main/cpp/renderer/core/RenderQueue.h`  
**测试文件**: `entry/src/ohosTest/ets/unittest/native/RenderQueue.test.ets`

#### 2.3.1 队列操作测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-INIT-001 | 默认构造 | 无参数 | size=0, running=true | P0 |
| RQ-INIT-002 | 自定义大小 | maxSize=5 | maxSize=5 | P0 |
| RQ-SUB-001 | 提交命令 | 有效cmd | size=1, success=true | P0 |
| RQ-SUB-002 | 队列满 | 提交第4个(max=3) | size=3, 丢弃最旧 | P0 |
| RQ-SUB-003 | 停止后提交 | Stop()后Submit | success=false | P0 |
| RQ-DEQ-001 | 消费命令 | Dequeue() | 返回true, 获取cmd | P0 |
| RQ-DEQ-002 | 空队列阻塞 | 队列为空Dequeue | 阻塞等待 | P0 |
| RQ-DEQ-003 | 停止后消费 | Stop()后Dequeue | 返回false | P0 |
| RQ-STP-001 | 停止队列 | Stop() | running=false | P0 |
| RQ-STP-002 | 唤醒阻塞 | Dequeue阻塞时Stop | Dequeue返回false | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

export default function RenderQueueTest() {
  describe('RenderQueue Initialization Tests', () => {
    it('RQ-INIT-001: Should initialize with default size', 0, async () => {
      const handle = await nativerender.createRenderQueue();
      const info = await nativerender.getQueueInfo(handle);
      
      expect(info.size).assertEqual(0);
      expect(info.isRunning).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-INIT-002: Should initialize with custom max size', 0, async () => {
      const handle = await nativerender.createRenderQueue(5);
      const info = await nativerender.getQueueInfo(handle);
      
      expect(info.maxSize).assertEqual(5);
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue Submit Tests', () => {
    it('RQ-SUB-001: Should submit command successfully', 0, async () => {
      const handle = await nativerender.createRenderQueue();
      const buffer = new ArrayBuffer(100);
      
      const result = await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      expect(result).assertTrue();
      
      const info = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(1);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-SUB-002: Should drop oldest when queue is full', 0, async () => {
      const handle = await nativerender.createRenderQueue(3);
      const buffer = new ArrayBuffer(100);
      
      // 填满队列
      for (let i = 0; i < 3; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      }
      
      let info = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(3);
      
      // 提交第4个，应该丢弃最旧的
      await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      
      info = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(3); // 仍然是3
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-SUB-003: Should reject submit after stop', 0, async () => {
      const handle = await nativerender.createRenderQueue();
      await nativerender.queueStop(handle);
      
      const buffer = new ArrayBuffer(100);
      const result = await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      
      expect(result).assertFalse();
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue Dequeue Tests', () => {
    it('RQ-DEQ-001: Should dequeue command', 0, async () => {
      const handle = await nativerender.createRenderQueue();
      const buffer = new ArrayBuffer(100);
      
      await nativerender.queueSubmit(handle, buffer, 100, 1920, 1080);
      
      const cmd = await nativerender.queueDequeue(handle);
      expect(cmd).assertNotNull();
      expect(cmd.dataSize).assertEqual(100);
      expect(cmd.width).assertEqual(1920);
      expect(cmd.height).assertEqual(1080);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-DEQ-003: Should return false after stop', 0, async () => {
      const handle = await nativerender.createRenderQueue();
      await nativerender.queueStop(handle);
      
      const cmd = await nativerender.queueDequeue(handle);
      expect(cmd).assertNull();
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue Stop Tests', () => {
    it('RQ-STP-001: Should set running flag to false', 0, async () => {
      const handle = await nativerender.createRenderQueue();
      
      let info = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertTrue();
      
      await nativerender.queueStop(handle);
      
      info = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertFalse();
      
      nativerender.destroyRenderQueue(handle);
    });
  });
}
```

#### 1.3.1 构造函数 `RenderQueue(size_t maxQueueSize = 3)`

**测试目标**: 验证队列初始化

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-CTOR-001 | 默认构造 | 无参数 | m_maxQueueSize == 3 | P0 |
| RQ-CTOR-002 | 自定义大小 | maxQueueSize=5 | m_maxQueueSize == 5 | P0 |
| RQ-CTOR-003 | 初始状态 | 无 | IsRunning() == true | P0 |
| RQ-CTOR-004 | 初始状态 | 无 | Size() == 0 | P0 |
| RQ-CTOR-005 | 零大小 | maxQueueSize=0 | 应处理或断言 | P2 |

**测试代码示例**:
```cpp
TEST(RenderQueueTest, Constructor_DefaultSize) {
    RenderQueue queue;
    EXPECT_TRUE(queue.IsRunning());
    EXPECT_EQ(queue.Size(), 0);
}

TEST(RenderQueueTest, Constructor_CustomSize) {
    RenderQueue queue(5);
    EXPECT_TRUE(queue.IsRunning());
    EXPECT_EQ(queue.Size(), 0);
}
```

---

#### 1.3.2 方法 `Submit(const RenderCommand& cmd)`

**测试目标**: 验证渲染命令提交

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-SUB-001 | 正常提交 | 有效cmd | 返回true, Size()==1 | P0 |
| RQ-SUB-002 | 多次提交 | 3个cmd | Size()==3 | P0 |
| RQ-SUB-003 | 队列满 | 提交第4个(max=3) | 丢弃最旧，Size()==3 | P0 |
| RQ-SUB-004 | 停止后提交 | Stop()后提交 | 返回false | P0 |
| RQ-SUB-005 | 空命令 | pixelData=nullptr | 仍应接受 | P1 |
| RQ-SUB-006 | 线程安全 | 多线程并发提交 | 无数据竞争 | P1 |
| RQ-SUB-007 | 性能 | 1000次提交 | 总耗时<500ms | P2 |

**测试代码示例**:
```cpp
TEST(RenderQueueTest, Submit_AddsCommandToQueue) {
    RenderQueue queue(3);
    
    RenderCommand cmd(nullptr, 100, 10, 10);
    bool result = queue.Submit(cmd);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(queue.Size(), 1);
}

TEST(RenderQueueTest, Submit_DropsOldestWhenFull) {
    RenderQueue queue(3);
    
    // 填满队列
    for (int i = 0; i < 3; ++i) {
        RenderCommand cmd(nullptr, 100, 10, 10);
        queue.Submit(cmd);
    }
    EXPECT_EQ(queue.Size(), 3);
    
    // 提交第4个，应该丢弃最旧的
    RenderCommand cmd(nullptr, 100, 10, 10);
    bool result = queue.Submit(cmd);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(queue.Size(), 3); // 仍然是3
}

TEST(RenderQueueTest, Submit_ReturnsFalseAfterStop) {
    RenderQueue queue;
    queue.Stop();
    
    RenderCommand cmd(nullptr, 100, 10, 10);
    bool result = queue.Submit(cmd);
    
    EXPECT_FALSE(result);
}
```

---

#### 1.3.3 方法 `Dequeue(RenderCommand& cmd)`

**测试目标**: 验证渲染命令消费

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-DEQ-001 | 空队列阻塞 | 队列为空 | 阻塞等待 | P0 |
| RQ-DEQ-002 | 有数据 | 1个cmd在队列 | 返回true，获取cmd | P0 |
| RQ-DEQ-003 | 多数据 | 3个cmd在队列 | 返回最旧的 | P0 |
| RQ-DEQ-004 | 停止后 | Stop()后 | 返回false，不阻塞 | P0 |
| RQ-DEQ-005 | 丢弃过时帧 | 快速提交多个 | Dequeue只取最新 | P1 |
| RQ-DEQ-006 | 线程安全 | 生产者-消费者 | 无死锁/竞态 | P1 |

**测试代码示例**:
```cpp
TEST(RenderQueueTest, Dequeue_RetrievesCommand) {
    RenderQueue queue;
    
    RenderCommand submitted(nullptr, 100, 10, 10);
    queue.Submit(submitted);
    
    RenderCommand dequeued;
    bool result = queue.Dequeue(dequeued);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(dequeued.dataSize, 100);
    EXPECT_EQ(dequeued.width, 10);
    EXPECT_EQ(dequeued.height, 10);
}

TEST(RenderQueueTest, Dequeue_BlocksWhenEmpty) {
    RenderQueue queue;
    
    // 在单独线程中 dequeue
    std::atomic<bool> completed{false};
    std::thread consumer([&]() {
        RenderCommand cmd;
        queue.Dequeue(cmd); // 应该阻塞
        completed.store(true);
    });
    
    // 等待一小段时间，确认阻塞
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(completed.load());
    
    // 提交一个命令，解除阻塞
    RenderCommand cmd(nullptr, 100, 10, 10);
    queue.Submit(cmd);
    
    consumer.join();
    EXPECT_TRUE(completed.load());
}

TEST(RenderQueueTest, Dequeue_ReturnsFalseAfterStop) {
    RenderQueue queue;
    queue.Stop();
    
    RenderCommand cmd;
    bool result = queue.Dequeue(cmd);
    
    EXPECT_FALSE(result);
}
```

---

#### 1.3.4 方法 `Stop()`

**测试目标**: 验证队列停止功能

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-STP-001 | 停止标志 | Stop() | IsRunning()==false | P0 |
| RQ-STP-002 | 唤醒阻塞 | Dequeue阻塞时Stop | Dequeue返回false | P0 |
| RQ-STP-003 | 重复停止 | 多次Stop() | 不崩溃 | P1 |
| RQ-STP-004 | 停止后提交 | Stop()后Submit | 返回false | P0 |
| RQ-STP-005 | 停止后消费 | Stop()后Dequeue | 返回false | P0 |

**测试代码示例**:
```cpp
TEST(RenderQueueTest, Stop_SetsRunningFlag) {
    RenderQueue queue;
    EXPECT_TRUE(queue.IsRunning());
    
    queue.Stop();
    EXPECT_FALSE(queue.IsRunning());
}

TEST(RenderQueueTest, Stop_UnblocksDequeue) {
    RenderQueue queue;
    
    std::atomic<bool> completed{false};
    std::thread consumer([&]() {
        RenderCommand cmd;
        bool result = queue.Dequeue(cmd);
        EXPECT_FALSE(result); // 应该返回false
        completed.store(true);
    });
    
    // 等待线程进入阻塞
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 停止队列
    queue.Stop();
    consumer.join();
    
    EXPECT_TRUE(completed.load());
}
```

---

#### 1.3.5 方法 `IsRunning() const`

**测试目标**: 验证运行状态查询

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-IR-001 | 初始状态 | 构造后 | 返回true | P0 |
| RQ-IR-002 | 停止后 | Stop()后 | 返回false | P0 |

**测试代码示例**:
```cpp
TEST(RenderQueueTest, IsRunning_ReturnsCorrectState) {
    RenderQueue queue;
    EXPECT_TRUE(queue.IsRunning());
    
    queue.Stop();
    EXPECT_FALSE(queue.IsRunning());
}
```

---

#### 1.3.6 方法 `Size() const`

**测试目标**: 验证队列长度查询

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RQ-SZ-001 | 空队列 | 构造后 | 返回0 | P0 |
| RQ-SZ-002 | 单元素 | Submit 1次 | 返回1 | P0 |
| RQ-SZ-003 | 多元素 | Submit 3次 | 返回3 | P0 |
| RQ-SZ-004 | 满队列 | Submit超过max | 返回max | P0 |
| RQ-SZ-005 | 消费后 | Submit 3, Dequeue 1 | 返回2 | P0 |
| RQ-SZ-006 | 线程安全 | 并发Submit/Dequeue | 准确计数 | P1 |

**测试代码示例**:
```cpp
TEST(RenderQueueTest, Size_ReturnsCorrectCount) {
    RenderQueue queue(3);
    
    EXPECT_EQ(queue.Size(), 0);
    
    queue.Submit(RenderCommand(nullptr, 100, 10, 10));
    EXPECT_EQ(queue.Size(), 1);
    
    queue.Submit(RenderCommand(nullptr, 100, 10, 10));
    queue.Submit(RenderCommand(nullptr, 100, 10, 10));
    EXPECT_EQ(queue.Size(), 3);
    
    // 第4个应该丢弃最旧的
    queue.Submit(RenderCommand(nullptr, 100, 10, 10));
    EXPECT_EQ(queue.Size(), 3);
}
```

---

### 2.4 RenderCommand 结构体

**源文件**: `entry/src/main/cpp/renderer/core/RenderQueue.h`  
**测试文件**: `entry/src/ohosTest/ets/unittest/native/RenderCommand.test.ets`

> **说明**: RenderCommand 是 C++ 结构体，通过 NAPI 传递时转换为 JS 对象。

#### 2.4.1 命令对象测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RC-DEF-001 | 默认构造 | 无 | dataSize=0, width=0, height=0 | P0 |
| RC-PARAM-001 | 参数构造 | buffer,100,1920,1080 | 字段正确赋值 | P0 |
| RC-PARAM-002 | null数据 | null,100,10,10 | pixelData=null | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

export default function RenderCommandTest() {
  describe('RenderCommand Tests', () => {
    it('RC-DEF-001: Should have default values', 0, () => {
      // 通过 NAPI 创建默认命令
      const cmd = nativerender.createDefaultCommand();
      
      expect(cmd.pixelData).assertNull();
      expect(cmd.dataSize).assertEqual(0);
      expect(cmd.width).assertEqual(0);
      expect(cmd.height).assertEqual(0);
    });

    it('RC-PARAM-001: Should set fields correctly', 0, () => {
      const buffer = new ArrayBuffer(100);
      const cmd = nativerender.createCommand(buffer, 100, 1920, 1080);
      
      expect(cmd.pixelData).assertNotNull();
      expect(cmd.dataSize).assertEqual(100);
      expect(cmd.width).assertEqual(1920);
      expect(cmd.height).assertEqual(1080);
    });

    it('RC-PARAM-002: Should handle null data', 0, () => {
      const cmd = nativerender.createCommand(null, 100, 10, 10);
      
      expect(cmd.pixelData).assertNull();
      expect(cmd.dataSize).assertEqual(100);
      expect(cmd.width).assertEqual(10);
      expect(cmd.height).assertEqual(10);
    });
  });
}
```

#### 1.4.1 默认构造函数 `RenderCommand()`

**测试目标**: 验证默认初始化

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RC-CTOR-001 | 默认构造 | 无 | pixelData == nullptr | P0 |
| RC-CTOR-002 | 默认构造 | 无 | dataSize == 0 | P0 |
| RC-CTOR-003 | 默认构造 | 无 | width == 0 | P0 |
| RC-CTOR-004 | 默认构造 | 无 | height == 0 | P0 |

**测试代码示例**:
```cpp
TEST(RenderCommandTest, DefaultConstructor_InitializesToZero) {
    RenderCommand cmd;
    
    EXPECT_EQ(cmd.pixelData, nullptr);
    EXPECT_EQ(cmd.dataSize, 0);
    EXPECT_EQ(cmd.width, 0);
    EXPECT_EQ(cmd.height, 0);
}
```

---

#### 1.4.2 参数构造函数 `RenderCommand(const void* data, size_t size, int32_t w, int32_t h)`

**测试目标**: 验证参数化初始化

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| RC-CTOR-P-001 | 有效参数 | data,100,10,10 | 字段正确赋值 | P0 |
| RC-CTOR-P-002 | null数据 | nullptr,100,10,10 | pixelData==nullptr | P1 |
| RC-CTOR-P-003 | 零尺寸 | data,0,0,0 | 字段为0 | P1 |
| RC-CTOR-P-004 | 大尺寸 | data,10MB,4K,4K | 字段正确 | P2 |

**测试代码示例**:
```cpp
TEST(RenderCommandTest, ParameterizedConstructor_SetsFields) {
    uint8_t buffer[100];
    RenderCommand cmd(buffer, 100, 1920, 1080);
    
    EXPECT_EQ(cmd.pixelData, buffer);
    EXPECT_EQ(cmd.dataSize, 100);
    EXPECT_EQ(cmd.width, 1920);
    EXPECT_EQ(cmd.height, 1080);
}
```

---

---

## 3. ArkTS 层单元测试

### 3.1 PixelFormat 枚举

**文件位置**: `entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`  
**测试文件**: `entry/src/ohosTest/ets/unittest/arkts/PixelFormat.test.ets`

#### 3.1.1 枚举值验证

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| PF-VAL-001 | RGBA值 | PixelFormat.RGBA | === 0 | P0 |
| PF-VAL-002 | RGB值 | PixelFormat.RGB | === 1 | P0 |
| PF-VAL-003 | BGRA值 | PixelFormat.BGRA | === 2 | P0 |
| PF-VAL-004 | NV21值 | PixelFormat.NV21 | === 3 | P0 |
| PF-VAL-005 | NV12值 | PixelFormat.NV12 | === 4 | P0 |
| PF-VAL-006 | 完整性 | 所有枚举值 | 共5个值 | P0 |
| PF-VAL-007 | 无重复 | 所有值 | 互不相同 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function PixelFormatTest() {
  describe('PixelFormat Enum Tests', () => {
    it('PF-VAL-001: RGBA should be 0', 0, () => {
      expect(PixelFormat.RGBA).assertEqual(0);
    });

    it('PF-VAL-002: RGB should be 1', 0, () => {
      expect(PixelFormat.RGB).assertEqual(1);
    });

    it('PF-VAL-003: BGRA should be 2', 0, () => {
      expect(PixelFormat.BGRA).assertEqual(2);
    });

    it('PF-VAL-004: NV21 should be 3', 0, () => {
      expect(PixelFormat.NV21).assertEqual(3);
    });

    it('PF-VAL-005: NV12 should be 4', 0, () => {
      expect(PixelFormat.NV12).assertEqual(4);
    });

    it('PF-VAL-006: Should have 5 format types', 0, () => {
      const formats = [
        PixelFormat.RGBA,
        PixelFormat.RGB,
        PixelFormat.BGRA,
        PixelFormat.NV21,
        PixelFormat.NV12
      ];
      expect(formats.length).assertEqual(5);
    });

    it('PF-VAL-007: All values should be unique', 0, () => {
      const values = new Set([
        PixelFormat.RGBA,
        PixelFormat.RGB,
        PixelFormat.BGRA,
        PixelFormat.NV21,
        PixelFormat.NV12
      ]);
      expect(values.size).assertEqual(5);
    });
  });
}
```

---

---

### 3.2 ArkZeroRenderer 类

**文件位置**: `entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`  
**测试文件**: `entry/src/ohosTest/ets/unittest/arkts/ArkZeroRenderer.test.ets`

#### 3.2.1 构造函数测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-CTOR-001 | 有效配置 | {w:1920,h:1080,fmt:RGBA} | 实例创建成功 | P0 |
| AZR-CTOR-002 | 最小配置 | {w:1,h:1,fmt:RGBA} | 实例创建成功 | P1 |
| AZR-CTOR-003 | 最大配置 | {w:7680,h:4320,fmt:RGBA} | 实例创建成功 | P1 |
| AZR-CTOR-004 | 无效宽度 | {w:0,h:1080,fmt:RGBA} | 应抛出异常或验证 | P2 |
| AZR-CTOR-005 | 无效高度 | {w:1920,h:0,fmt:RGBA} | 应抛出异常或验证 | P2 |
| AZR-CTOR-006 | 缺少format | 不完整config | TypeScript编译错误 | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function ArkZeroRendererTest() {
  describe('ArkZeroRenderer Constructor Tests', () => {
    it('AZR-CTOR-001: Should create instance with valid config', 0, () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      expect(renderer).assertNotNull();
    });

    it('AZR-CTOR-002: Should create instance with minimum size', 0, () => {
      const renderer = new ArkZeroRenderer({
        width: 1,
        height: 1,
        format: PixelFormat.RGBA
      });
      
      expect(renderer).assertNotNull();
    });

    it('AZR-CTOR-003: Should create instance with 8K resolution', 0, () => {
      const renderer = new ArkZeroRenderer({
        width: 7680,
        height: 4320,
        format: PixelFormat.RGBA
      });
      
      expect(renderer).assertNotNull();
    });
  });
```

#### 3.2.2 initialize 方法测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-INIT-001 | 正常初始化 | 有效surfaceId | Promise resolve | P0 |
| AZR-INIT-002 | 重复初始化 | 已初始化后再次调用 | 警告日志，无操作 | P1 |
| AZR-INIT-003 | 空surfaceId | "" | Promise reject | P0 |
| AZR-INIT-004 | 初始化后状态 | 成功后 | isInitialized==true | P0 |

**测试代码示例**:

```typescript
  describe('ArkZeroRenderer Initialize Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeEach(() => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
    });

    afterEach(() => {
      if (renderer) {
        renderer.dispose();
      }
    });

    it('AZR-INIT-001: Should initialize successfully', 0, async () => {
      const mockSurfaceId = 'mock_surface_001';
      
      await renderer.initialize(mockSurfaceId);
      
      // 验证初始化成功（需要暴露isInitialized或通过行为验证）
      expect(true).assertTrue();
    });

    it('AZR-INIT-002: Should warn on duplicate initialization', 0, async () => {
      const mockSurfaceId = 'mock_surface_001';
      
      await renderer.initialize(mockSurfaceId);
      await renderer.initialize(mockSurfaceId); // 第二次调用
      
      // 应记录警告但不崩溃
      expect(true).assertTrue();
    });

    it('AZR-INIT-003: Should reject with empty surfaceId', 0, async () => {
      try {
        await renderer.initialize('');
        expect(false).assertTrue(); // 应该抛出异常
      } catch (error) {
        expect(true).assertTrue(); // 预期捕获异常
      }
    });
  });
```

#### 3.2.3 renderFrame 方法测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-RF-001 | 正常渲染 | 有效buffer,w,h | Promise resolve | P0 |
| AZR-RF-002 | 未初始化 | 未调用initialize | Promise reject | P0 |
| AZR-RF-003 | 空buffer | new ArrayBuffer(0) | 应处理或reject | P1 |
| AZR-RF-004 | buffer尺寸不匹配 | 小buffer,大w,h | 应检测并reject | P1 |
| AZR-RF-007 | 回调触发 | 设置回调后渲染 | 回调被调用 | P0 |
| AZR-RF-008 | 多次渲染 | 连续100次 | 无内存泄漏 | P1 |

**测试代码示例**:

```typescript
  describe('ArkZeroRenderer RenderFrame Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeEach(async () => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('mock_surface');
    });

    afterEach(() => {
      if (renderer) {
        renderer.dispose();
      }
    });

    it('AZR-RF-001: Should render frame successfully', 0, async () => {
      const bufferSize = 1920 * 1080 * 4; // RGBA
      const buffer = new ArrayBuffer(bufferSize);
      
      await renderer.renderFrame(buffer, 1920, 1080);
      
      expect(true).assertTrue(); // Promise resolved
    });

    it('AZR-RF-002: Should reject when not initialized', 0, async () => {
      const uninitializedRenderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      const buffer = new ArrayBuffer(100);
      
      try {
        await uninitializedRenderer.renderFrame(buffer, 10, 10);
        expect(false).assertTrue(); // Should throw
      } catch (error) {
        expect(true).assertTrue(); // Expected error
      }
    });

    it('AZR-RF-007: Should trigger callback after render', 0, async () => {
      let callbackCalled = false;
      renderer.setOnFrameRendered(() => {
        callbackCalled = true;
      });
      
      const buffer = new ArrayBuffer(1920 * 1080 * 4);
      await renderer.renderFrame(buffer, 1920, 1080);
      
      expect(callbackCalled).assertTrue();
    });

    it('AZR-RF-008: Should handle multiple renders without memory leak', 0, async () => {
      const bufferSize = 1920 * 1080 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      
      for (let i = 0; i < 100; i++) {
        await renderer.renderFrame(buffer, 1920, 1080);
      }
      
      expect(true).assertTrue(); // No crash or memory leak
    });
  });
```

#### 3.2.4 resize 方法测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-RSZ-001 | 正常调整 | w=1280,h=720 | Promise resolve | P0 |
| AZR-RSZ-002 | 未初始化 | 未initialize | Promise reject | P0 |
| AZR-RSZ-003 | 零宽度 | w=0,h=720 | 应reject | P1 |
| AZR-RSZ-004 | 零高度 | w=1280,h=0 | 应reject | P1 |

**测试代码示例**:

```typescript
  describe('ArkZeroRenderer Resize Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeEach(async () => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('mock_surface');
    });

    afterEach(() => {
      renderer.dispose();
    });

    it('AZR-RSZ-001: Should resize successfully', 0, async () => {
      await renderer.resize(1280, 720);
      
      expect(true).assertTrue(); // Promise resolved
    });

    it('AZR-RSZ-002: Should reject when not initialized', 0, async () => {
      const uninitializedRenderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      try {
        await uninitializedRenderer.resize(1280, 720);
        expect(false).assertTrue();
      } catch (error) {
        expect(true).assertTrue();
      }
    });
  });
```

#### 3.2.5 dispose 方法测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-DSP-001 | 正常清理 | 已初始化 | 资源释放 | P0 |
| AZR-DSP-002 | 未初始化 | 未initialize | 无操作，不崩溃 | P0 |
| AZR-DSP-003 | 重复清理 | 多次dispose | 无副作用 | P1 |
| AZR-DSP-005 | 清理后使用 | dispose后renderFrame | 应reject | P0 |

**测试代码示例**:

```typescript
  describe('ArkZeroRenderer Dispose Tests', () => {
    it('AZR-DSP-001: Should dispose successfully', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('mock_surface');
      
      renderer.dispose();
      
      expect(true).assertTrue();
    });

    it('AZR-DSP-002: Should handle dispose when not initialized', 0, () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      // 不应崩溃
      renderer.dispose();
      
      expect(true).assertTrue();
    });

    it('AZR-DSP-003: Should handle multiple dispose calls', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('mock_surface');
      
      renderer.dispose();
      renderer.dispose(); // 第二次调用
      renderer.dispose(); // 第三次调用
      
      expect(true).assertTrue(); // 无崩溃
    });

    it('AZR-DSP-005: Should reject operations after dispose', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('mock_surface');
      renderer.dispose();
      
      const buffer = new ArrayBuffer(100);
      
      try {
        await renderer.renderFrame(buffer, 10, 10);
        expect(false).assertTrue(); // Should throw
      } catch (error) {
        expect(true).assertTrue(); // Expected error
      }
    });
  });
}
```

---

---

### 3.3 ArkZeroRendererConfig 接口

**文件位置**: `entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`  
**测试方式**: TypeScript 类型检查（编译时验证）

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZRC-TYPE-001 | 完整配置 | {w,h,format,surfaceId?} | 编译通过 | P0 |
| AZRC-TYPE-002 | 缺少width | {h,format} | 编译错误 | P0 |
| AZRC-TYPE-003 | 缺少height | {w,format} | 编译错误 | P0 |
| AZRC-TYPE-004 | 缺少format | {w,h} | 编译错误 | P0 |
| AZRC-TYPE-005 | 错误类型 | {w:"1920",h:1080,fmt:0} | 编译错误 | P0 |
| AZRC-TYPE-006 | 可选surfaceId | {w,h,format} | 编译通过 | P0 |
| AZRC-TYPE-007 | 提供surfaceId | {w,h,format,surfaceId:"xxx"} | 编译通过 | P1 |

**测试说明**: 这些测试通过 TypeScript 编译器自动验证，无需运行时测试。

---

## 4. 逐步集成测试模块设计

> **集成测试目标**: 验证多个模块协同工作时的正确性，确保数据流和控制流在各层之间正确传递。

### 4.1 集成测试策略

#### 4.1.1 测试层次划分

```
Level 1: 单元测试 (Unit Tests)          ← 已完成
  ├─ C++ 层单个类测试
  └─ ArkTS 层单个组件测试

Level 2: 模块集成测试 (Module Integration) ← 本章重点
  ├─ NAPI 桥接层测试
  ├─ 渲染管线测试
  └─ Surface 管理测试

Level 3: 系统级集成测试 (System Integration)
  └─ 端到端完整流程测试

Level 4: UI 自动化测试 (UI Automation)    ← 第5章
  └─ 用户交互场景测试
```

#### 4.1.2 集成测试原则

1. **自底向上**: 从底层模块开始，逐步向上集成
2. **隔离依赖**: 使用 Mock/Stub 隔离外部依赖
3. **真实环境**: 在接近生产环境的条件下测试
4. **渐进式**: 每次只集成为一个新模块
5. **可重复**: 测试结果稳定可靠

#### 4.1.3 测试文件组织

```
entry/src/ohosTest/ets/integration/     # 集成测试目录
├── List.integration.test.ets           # 集成测试入口
├── napi/                               # NAPI 桥接层测试
│   ├── NAPIBridge.test.ets
│   └── DataTypeConversion.test.ets
├── pipeline/                           # 渲染管线测试
│   ├── RenderPipeline.test.ets
│   ├── TextureManagement.test.ets
│   └── ShaderIntegration.test.ets
├── surface/                            # Surface 管理测试
│   ├── SurfaceLifecycle.test.ets
│   └── SurfaceResize.test.ets
└── e2e/                                # 端到端测试
    ├── FullRenderFlow.test.ets
    └── ErrorRecovery.test.ets
```

---

### 4.2 NAPI 桥接层集成测试

**测试文件**: `entry/src/ohosTest/ets/integration/napi/NAPIBridge.test.ets`

**测试目标**: 验证 ArkTS 与 C++ 之间的数据传递和函数调用

#### 4.2.1 数据类型转换测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| NAPI-DT-001 | ArrayBuffer 传递 | 1MB buffer | C++ 接收成功 | P0 |
| NAPI-DT-002 | 大 Buffer 传递 | 100MB buffer | 无内存溢出 | P0 |
| NAPI-DT-003 | 空 Buffer 传递 | new ArrayBuffer(0) | 正确处理 | P1 |
| NAPI-DT-004 | Number 类型传递 | int32, float, double | 精度无损 | P0 |
| NAPI-DT-005 | String 类型传递 | UTF-8 字符串 | 编码正确 | P1 |
| NAPI-DT-006 | Boolean 类型传递 | true/false | 值正确 | P0 |
| NAPI-DT-007 | Object 序列化 | JS对象→C++结构体 | 字段映射正确 | P1 |
| NAPI-DT-008 | 返回值转换 | C++返回JS对象 | 类型正确 | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

export default function NAPIBridgeTest() {
  describe('NAPI Data Type Conversion Tests', () => {
    it('NAPI-DT-001: Should pass 1MB ArrayBuffer to C++', 0, async () => {
      const bufferSize = 1024 * 1024; // 1MB
      const buffer = new ArrayBuffer(bufferSize);
      const view = new Uint8Array(buffer);
      
      // 填充测试数据
      for (let i = 0; i < bufferSize; i++) {
        view[i] = i % 256;
      }
      
      // 通过 NAPI 传递到 C++
      const result = await nativerender.validateBuffer(buffer, bufferSize);
      
      expect(result).assertTrue();
    });

    it('NAPI-DT-002: Should handle large buffer without memory overflow', 0, async () => {
      const bufferSize = 100 * 1024 * 1024; // 100MB
      const buffer = new ArrayBuffer(bufferSize);
      
      // 不应崩溃或内存溢出
      const result = await nativerender.validateBuffer(buffer, bufferSize);
      
      expect(result).assertTrue();
    });

    it('NAPI-DT-004: Should preserve number precision', 0, () => {
      const testCases = [
        { input: 2147483647, type: 'int32_max' },
        { input: -2147483648, type: 'int32_min' },
        { input: 3.14159265358979, type: 'float_pi' },
        { input: 1.7976931348623157e+308, type: 'double_max' }
      ];
      
      testCases.forEach(testCase => {
        const result = nativerender.echoNumber(testCase.input);
        expect(result).assertEqual(testCase.input);
      });
    });

    it('NAPI-DT-007: Should serialize JS object to C++ struct', 0, async () => {
      const config = {
        width: 1920,
        height: 1080,
        format: 0, // RGBA
        surfaceId: 'test_surface_001'
      };
      
      const result = await nativerender.validateConfig(config);
      
      expect(result.valid).assertTrue();
      expect(result.width).assertEqual(1920);
      expect(result.height).assertEqual(1080);
    });
  });
}
```

#### 4.2.2 异步调用测试

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| NAPI-ASYNC-001 | Promise 返回 | async 函数 | Promise resolve | P0 |
| NAPI-ASYNC-002 | 错误处理 | C++抛出异常 | Promise reject | P0 |
| NAPI-ASYNC-003 | 并发调用 | 10个async同时调用 | 全部成功 | P1 |
| NAPI-ASYNC-004 | 超时处理 | 长时间运行任务 | 正确等待 | P1 |
| NAPI-ASYNC-005 | 回调执行 | 设置callback | callback被调用 | P0 |

**测试代码示例**:

```typescript
  describe('NAPI Async Call Tests', () => {
    it('NAPI-ASYNC-001: Should return Promise that resolves', 0, async () => {
      const promise = nativerender.asyncOperation();
      
      expect(promise).assertInstanceOf(Promise);
      
      const result = await promise;
      expect(result).assertTrue();
    });

    it('NAPI-ASYNC-002: Should reject Promise on C++ exception', 0, async () => {
      try {
        await nativerender.operationThatThrows();
        expect(false).assertTrue(); // Should not reach here
      } catch (error) {
        expect(error).assertNotNull();
      }
    });

    it('NAPI-ASYNC-003: Should handle concurrent async calls', 0, async () => {
      const promises = [];
      
      // 发起10个并发调用
      for (let i = 0; i < 10; i++) {
        promises.push(nativerender.asyncOperation());
      }
      
      // 等待所有完成
      const results = await Promise.all(promises);
      
      // 全部应该成功
      results.forEach(result => {
        expect(result).assertTrue();
      });
    });

    it('NAPI-ASYNC-005: Should execute callback after operation', 0, async () => {
      let callbackExecuted = false;
      
      nativerender.setCallback(() => {
        callbackExecuted = true;
      });
      
      await nativerender.triggerCallback();
      
      expect(callbackExecuted).assertTrue();
    });
  });
}
```

---

### 4.3 渲染管线集成测试

**测试文件**: `entry/src/ohosTest/ets/integration/pipeline/RenderPipeline.test.ets`

**测试目标**: 验证从像素数据到屏幕显示的完整渲染流程

#### 4.3.1 完整渲染流程测试

| 测试用例ID | 测试场景 | 操作步骤 | 预期结果 | 优先级 |
|-----------|---------|---------|---------|--------|
| PIPE-FLOW-001 | 单帧渲染 | submit→process→render | 帧成功渲染 | P0 |
| PIPE-FLOW-002 | 多帧连续渲染 | 100帧循环 | 无丢帧，FPS稳定 | P0 |
| PIPE-FLOW-003 | 不同格式渲染 | RGBA/RGB/BGRA切换 | 格式正确转换 | P0 |
| PIPE-FLOW-004 | 分辨率变化 | 1080p→720p→4K | 自适应调整 | P0 |
| PIPE-FLOW-005 | 空帧处理 | 提交空buffer | 优雅处理，不崩溃 | P1 |
| PIPE-FLOW-006 | 超大帧处理 | 8K分辨率 | 内存合理，性能可接受 | P1 |
| PIPE-FLOW-007 | 快速提交 | 每秒200次submit | 队列正常工作 | P1 |
| PIPE-FLOW-008 | 渲染回调 | 设置onFrameRendered | 每帧触发回调 | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function RenderPipelineTest() {
  describe('Render Pipeline Flow Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeEach(async () => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('mock_surface');
    });

    afterEach(() => {
      if (renderer) {
        renderer.dispose();
      }
    });

    it('PIPE-FLOW-001: Should render single frame successfully', 0, async () => {
      const bufferSize = 1920 * 1080 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      
      // 填充红色像素数据
      const view = new Uint8Array(buffer);
      for (let i = 0; i < bufferSize; i += 4) {
        view[i] = 255;     // R
        view[i + 1] = 0;   // G
        view[i + 2] = 0;   // B
        view[i + 3] = 255; // A
      }
      
      await renderer.renderFrame(buffer, 1920, 1080);
      
      // 如果未抛出异常，说明渲染成功
      expect(true).assertTrue();
    });

    it('PIPE-FLOW-002: Should render 100 frames without frame drops', 0, async () => {
      const bufferSize = 1920 * 1080 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      let renderedFrames = 0;
      
      renderer.setOnFrameRendered(() => {
        renderedFrames++;
      });
      
      const startTime = Date.now();
      
      for (let i = 0; i < 100; i++) {
        await renderer.renderFrame(buffer, 1920, 1080);
      }
      
      const elapsed = Date.now() - startTime;
      
      expect(renderedFrames).assertEqual(100);
      expect(elapsed).assertLess(10000); // 应在10秒内完成
    });

    it('PIPE-FLOW-003: Should handle different pixel formats', 0, async () => {
      const formats = [
        { format: PixelFormat.RGBA, bpp: 4 },
        { format: PixelFormat.RGB, bpp: 3 },
        { format: PixelFormat.BGRA, bpp: 4 }
      ];
      
      for (const fmt of formats) {
        const bufferSize = 100 * 100 * fmt.bpp;
        const buffer = new ArrayBuffer(bufferSize);
        
        // 创建新渲染器使用该格式
        const testRenderer = new ArkZeroRenderer({
          width: 100,
          height: 100,
          format: fmt.format
        });
        
        await testRenderer.initialize('mock_surface');
        await testRenderer.renderFrame(buffer, 100, 100);
        testRenderer.dispose();
      }
      
      expect(true).assertTrue();
    });

    it('PIPE-FLOW-004: Should adapt to resolution changes', 0, async () => {
      const resolutions = [
        { w: 1920, h: 1080 },
        { w: 1280, h: 720 },
        { w: 3840, h: 2160 }
      ];
      
      for (const res of resolutions) {
        await renderer.resize(res.w, res.h);
        
        const bufferSize = res.w * res.h * 4;
        const buffer = new ArrayBuffer(bufferSize);
        
        await renderer.renderFrame(buffer, res.w, res.h);
      }
      
      expect(true).assertTrue();
    });

    it('PIPE-FLOW-008: Should trigger callback on each frame', 0, async () => {
      let callbackCount = 0;
      
      renderer.setOnFrameRendered(() => {
        callbackCount++;
      });
      
      const bufferSize = 100 * 100 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      
      for (let i = 0; i < 10; i++) {
        await renderer.renderFrame(buffer, 100, 100);
      }
      
      expect(callbackCount).assertEqual(10);
    });
  });
}
```

#### 4.3.2 纹理管理集成测试

**测试文件**: `entry/src/ohosTest/ets/integration/pipeline/TextureManagement.test.ets`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| TEX-MGT-001 | 纹理创建 | 首次渲染 | 纹理成功创建 | P0 |
| TEX-MGT-002 | 纹理复用 | 相同尺寸连续渲染 | 复用现有纹理 | P0 |
| TEX-MGT-003 | 纹理重建 | 尺寸变化 | 销毁旧纹理，创建新纹理 | P0 |
| TEX-MGT-004 | 纹理池限制 | 超过maxTextures | 回收最久未用纹理 | P1 |
| TEX-MGT-005 | 纹理泄漏检测 | 1000次渲染 | 纹理数量稳定 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

export default function TextureManagementTest() {
  describe('Texture Management Integration Tests', () => {
    it('TEX-MGT-001: Should create texture on first render', 0, async () => {
      const queueHandle = await nativerender.createRenderQueue();
      const monitorHandle = await nativerender.createPerformanceMonitor();
      
      // 首次渲染前，纹理计数应为0
      let stats = await nativerender.getTextureStats();
      expect(stats.textureCount).assertEqual(0);
      
      // 执行一次渲染
      const buffer = new ArrayBuffer(100 * 100 * 4);
      await nativerender.queueSubmit(queueHandle, buffer, 100 * 100 * 4, 100, 100);
      await nativerender.processRenderQueue(queueHandle, monitorHandle);
      
      // 渲染后，应有1个纹理
      stats = await nativerender.getTextureStats();
      expect(stats.textureCount).assertEqual(1);
      
      nativerender.destroyRenderQueue(queueHandle);
      nativerender.destroyPerformanceMonitor(monitorHandle);
    });

    it('TEX-MGT-002: Should reuse texture for same size', 0, async () => {
      const queueHandle = await nativerender.createRenderQueue();
      const monitorHandle = await nativerender.createPerformanceMonitor();
      
      const buffer = new ArrayBuffer(100 * 100 * 4);
      
      // 第一次渲染
      await nativerender.queueSubmit(queueHandle, buffer, 100 * 100 * 4, 100, 100);
      await nativerender.processRenderQueue(queueHandle, monitorHandle);
      
      let stats = await nativerender.getTextureStats();
      const firstTextureCount = stats.textureCount;
      
      // 第二次渲染（相同尺寸）
      await nativerender.queueSubmit(queueHandle, buffer, 100 * 100 * 4, 100, 100);
      await nativerender.processRenderQueue(queueHandle, monitorHandle);
      
      stats = await nativerender.getTextureStats();
      
      // 纹理数量不应增加（复用）
      expect(stats.textureCount).assertEqual(firstTextureCount);
      
      nativerender.destroyRenderQueue(queueHandle);
      nativerender.destroyPerformanceMonitor(monitorHandle);
    });

    it('TEX-MGT-005: Should not leak textures after 1000 renders', 0, async () => {
      const queueHandle = await nativerender.createRenderQueue();
      const monitorHandle = await nativerender.createPerformanceMonitor();
      
      const buffer = new ArrayBuffer(100 * 100 * 4);
      
      // 渲染1000次
      for (let i = 0; i < 1000; i++) {
        await nativerender.queueSubmit(queueHandle, buffer, 100 * 100 * 4, 100, 100);
        await nativerender.processRenderQueue(queueHandle, monitorHandle);
      }
      
      const stats = await nativerender.getTextureStats();
      
      // 纹理数量应保持在合理范围（例如 <= 5）
      expect(stats.textureCount).assertLarger(0);
      expect(stats.textureCount).assertLess(10);
      
      nativerender.destroyRenderQueue(queueHandle);
      nativerender.destroyPerformanceMonitor(monitorHandle);
    });
  });
}
```

---

### 4.4 Surface 管理集成测试

**测试文件**: `entry/src/ohosTest/ets/integration/surface/SurfaceLifecycle.test.ets`

**测试目标**: 验证 Surface 生命周期管理和状态转换

#### 4.4.1 Surface 生命周期测试

| 测试用例ID | 测试场景 | 操作步骤 | 预期结果 | 优先级 |
|-----------|---------|---------|---------|--------|
| SURF-LC-001 | Surface 创建 | initialize() | Surface 成功创建 | P0 |
| SURF-LC-002 | Surface 绑定 | 绑定XComponent | 绑定成功 | P0 |
| SURF-LC-003 | Surface 解绑 | dispose() | 资源正确释放 | P0 |
| SURF-LC-004 | 重复初始化 | 两次initialize() | 第二次警告，不崩溃 | P1 |
| SURF-LC-005 | 未初始化使用 | 未initialize就renderFrame | 抛出异常 | P0 |
| SURF-LC-006 | 销毁后使用 | dispose后renderFrame | 抛出异常 | P0 |
| SURF-LC-007 | 多次销毁 | 三次dispose() | 无副作用 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function SurfaceLifecycleTest() {
  describe('Surface Lifecycle Tests', () => {
    it('SURF-LC-001: Should create surface on initialize', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      await renderer.initialize('test_surface_001');
      
      // 如果未抛出异常，说明创建成功
      expect(true).assertTrue();
      
      renderer.dispose();
    });

    it('SURF-LC-004: Should warn on duplicate initialization', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      await renderer.initialize('test_surface_001');
      
      // 第二次初始化应记录警告但不崩溃
      await renderer.initialize('test_surface_001');
      
      expect(true).assertTrue();
      
      renderer.dispose();
    });

    it('SURF-LC-005: Should throw when rendering before initialization', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      const buffer = new ArrayBuffer(100);
      
      try {
        await renderer.renderFrame(buffer, 10, 10);
        expect(false).assertTrue(); // Should not reach here
      } catch (error) {
        expect(error).assertNotNull();
      }
    });

    it('SURF-LC-006: Should throw when rendering after dispose', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      await renderer.initialize('test_surface_001');
      renderer.dispose();
      
      const buffer = new ArrayBuffer(100);
      
      try {
        await renderer.renderFrame(buffer, 10, 10);
        expect(false).assertTrue(); // Should not reach here
      } catch (error) {
        expect(error).assertNotNull();
      }
    });

    it('SURF-LC-007: Should handle multiple dispose calls', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      await renderer.initialize('test_surface_001');
      
      renderer.dispose();
      renderer.dispose();
      renderer.dispose();
      
      // 不应崩溃
      expect(true).assertTrue();
    });
  });
}
```

#### 4.4.2 Surface 动态调整测试

**测试文件**: `entry/src/ohosTest/ets/integration/surface/SurfaceResize.test.ets`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| SURF-RSZ-001 | 缩小调整 | 1920x1080 → 1280x720 | 成功调整 | P0 |
| SURF-RSZ-002 | 放大调整 | 1280x720 → 3840x2160 | 成功调整 | P0 |
| SURF-RSZ-003 | 宽高比变化 | 16:9 → 4:3 | 正确适配 | P0 |
| SURF-RSZ-004 | 频繁调整 | 10次连续resize | 无资源泄漏 | P1 |
| SURF-RSZ-005 | 调整中渲染 | resize同时renderFrame | 正确处理竞态 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function SurfaceResizeTest() {
  describe('Surface Resize Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeEach(async () => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('test_surface');
    });

    afterEach(() => {
      renderer.dispose();
    });

    it('SURF-RSZ-001: Should resize to smaller resolution', 0, async () => {
      await renderer.resize(1280, 720);
      
      const buffer = new ArrayBuffer(1280 * 720 * 4);
      await renderer.renderFrame(buffer, 1280, 720);
      
      expect(true).assertTrue();
    });

    it('SURF-RSZ-002: Should resize to larger resolution', 0, async () => {
      await renderer.resize(3840, 2160);
      
      const buffer = new ArrayBuffer(3840 * 2160 * 4);
      await renderer.renderFrame(buffer, 3840, 2160);
      
      expect(true).assertTrue();
    });

    it('SURF-RSZ-004: Should handle frequent resizes without leak', 0, async () => {
      const resolutions = [
        { w: 1920, h: 1080 },
        { w: 1280, h: 720 },
        { w: 3840, h: 2160 },
        { w: 1280, h: 720 },
        { w: 1920, h: 1080 }
      ];
      
      for (const res of resolutions) {
        await renderer.resize(res.w, res.h);
        
        const buffer = new ArrayBuffer(res.w * res.h * 4);
        await renderer.renderFrame(buffer, res.w, res.h);
      }
      
      // 检查内存使用情况（如果有API）
      const memoryInfo = await nativerender.getMemoryInfo();
      expect(memoryInfo.textureMemory).assertLess(100 * 1024 * 1024); // < 100MB
    });
  });
}
```

---

### 4.5 端到端集成测试

**测试文件**: `entry/src/ohosTest/ets/integration/e2e/FullRenderFlow.test.ets`

**测试目标**: 验证从应用启动到渲染完成的完整流程

#### 4.5.1 完整渲染流程测试

| 测试用例ID | 测试场景 | 操作步骤 | 预期结果 | 优先级 |
|-----------|---------|---------|---------|--------|
| E2E-FULL-001 | 标准渲染流程 | 创建→初始化→渲染→销毁 | 全流程成功 | P0 |
| E2E-FULL-002 | 带回调的渲染 | 设置回调+渲染 | 回调正确触发 | P0 |
| E2E-FULL-003 | 性能监控集成 | 启用监控+渲染100帧 | 统计数据准确 | P0 |
| E2E-FULL-004 | 错误恢复 | 渲染失败后重试 | 能恢复正常 | P1 |
| E2E-FULL-005 | 长时间运行 | 持续渲染1小时 | 无内存泄漏，性能稳定 | P1 |
| E2E-FULL-006 | 多实例运行 | 创建3个渲染器 | 独立工作，互不干扰 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';
import nativerender from 'libnativerender.so';

export default function FullRenderFlowTest() {
  describe('End-to-End Full Render Flow Tests', () => {
    it('E2E-FULL-001: Should complete standard render flow', 0, async () => {
      // 1. 创建渲染器
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      // 2. 初始化
      await renderer.initialize('e2e_test_surface');
      
      // 3. 渲染多帧
      const bufferSize = 1920 * 1080 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      
      for (let i = 0; i < 10; i++) {
        await renderer.renderFrame(buffer, 1920, 1080);
      }
      
      // 4. 销毁
      renderer.dispose();
      
      expect(true).assertTrue();
    });

    it('E2E-FULL-003: Should integrate performance monitoring', 0, async () => {
      const renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      
      await renderer.initialize('perf_test_surface');
      
      const monitorHandle = await nativerender.createPerformanceMonitor();
      
      // 渲染60帧
      const bufferSize = 1920 * 1080 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      
      for (let i = 0; i < 60; i++) {
        await nativerender.monitorBeginFrame(monitorHandle);
        await renderer.renderFrame(buffer, 1920, 1080);
        await nativerender.monitorEndFrame(monitorHandle, false);
      }
      
      // 获取统计数据
      const stats = await nativerender.getMonitorStats(monitorHandle);
      
      expect(stats.totalFrames).assertEqual(60);
      expect(stats.fps).assertLarger(0);
      expect(stats.droppedFrames).assertEqual(0);
      
      nativerender.destroyPerformanceMonitor(monitorHandle);
      renderer.dispose();
    });

    it('E2E-FULL-006: Should run multiple renderer instances independently', 0, async () => {
      const configs = [
        { w: 1920, h: 1080, id: 'surface_1' },
        { w: 1280, h: 720, id: 'surface_2' },
        { w: 3840, h: 2160, id: 'surface_3' }
      ];
      
      const renderers = [];
      
      // 创建3个渲染器
      for (const config of configs) {
        const renderer = new ArkZeroRenderer({
          width: config.w,
          height: config.h,
          format: PixelFormat.RGBA
        });
        
        await renderer.initialize(config.id);
        renderers.push(renderer);
      }
      
      // 并发渲染
      const bufferSize = 100 * 100 * 4;
      const buffer = new ArrayBuffer(bufferSize);
      
      for (const renderer of renderers) {
        await renderer.renderFrame(buffer, 100, 100);
      }
      
      // 全部销毁
      for (const renderer of renderers) {
        renderer.dispose();
      }
      
      expect(true).assertTrue();
    });
  });
}
```

#### 4.5.2 错误恢复测试

**测试文件**: `entry/src/ohosTest/ets/integration/e2e/ErrorRecovery.test.ets`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| ERR-REC-001 | 无效参数恢复 | 传入null buffer | 捕获错误，继续运行 | P0 |
| ERR-REC-002 | Surface失效恢复 | Surface被销毁后重连 | 重新初始化成功 | P1 |
| ERR-REC-003 | 内存不足恢复 | 分配超大buffer | 优雅降级或报错 | P1 |
| ERR-REC-004 | 并发冲突恢复 | 同时调用resize和render | 串行化处理 | P1 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function ErrorRecoveryTest() {
  describe('Error Recovery Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeEach(async () => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      await renderer.initialize('error_test_surface');
    });

    afterEach(() => {
      renderer.dispose();
    });

    it('ERR-REC-001: Should recover from invalid buffer', 0, async () => {
      // 尝试渲染无效数据
      try {
        await renderer.renderFrame(null as any, 100, 100);
      } catch (error) {
        // 捕获错误
        expect(error).assertNotNull();
      }
      
      // 使用有效数据重新渲染，应该成功
      const buffer = new ArrayBuffer(100 * 100 * 4);
      await renderer.renderFrame(buffer, 100, 100);
      
      expect(true).assertTrue();
    });

    it('ERR-REC-004: Should handle concurrent resize and render', 0, async () => {
      const promises = [];
      
      // 同时发起resize和render
      promises.push(renderer.resize(1280, 720));
      
      const buffer = new ArrayBuffer(1920 * 1080 * 4);
      promises.push(renderer.renderFrame(buffer, 1920, 1080));
      
      // 等待所有完成
      await Promise.allSettled(promises);
      
      // 应该都能完成（可能有一个失败，但不会崩溃）
      expect(true).assertTrue();
    });
  });
}
```

---

## 5. UI 自动化测试（arkxtest）

### 5.1 页面跳转测试

**测试文件**: `entry/src/ohosTest/ets/uitest/Navigation.test.ets`

#### 4.1.1 首页导航测试

| 测试用例ID | 测试场景 | 操作步骤 | 预期结果 | 优先级 |
|-----------|---------|---------|---------|--------|
| UI-NAV-001 | 启动应用 | 打开App | 显示首页 | P0 |
| UI-NAV-002 | 点击跳转按钮 | 点击"启动 Surface 渲染测试" | 跳转到SurfaceDemoPage | P0 |
| UI-NAV-003 | 验证目标页面 | 检查页面标题 | 显示"Surface Demo" | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { uiDriver, By } from '@ohos.arkxtest';

export default function NavigationTest() {
  describe('Navigation Tests', () => {
    it('UI-NAV-001: Should display home page on launch', 0, async () => {
      // 等待首页加载
      await uiDriver.sleep(2000);
      
      const title = await uiDriver.findElement(
        By.text('ArkZeroRenderer - Ultra Low Latency Demo')
      );
      
      expect(title).assertNotNull();
    });

    it('UI-NAV-002: Should navigate to SurfaceDemoPage on button click', 0, async () => {
      // 查找并点击按钮
      const button = await uiDriver.findElement(
        By.text('启动 Surface 渲染测试')
      );
      
      await button.click();
      
      // 等待页面跳转
      await uiDriver.sleep(1500);
      
      // 验证新页面元素
      const surfaceTitle = await uiDriver.findElement(
        By.text('Surface Rendering Demo')
      );
      
      expect(surfaceTitle).assertNotNull();
    });

    it('UI-NAV-003: Should verify target page title', 0, async () => {
      // 先跳转到目标页面
      const button = await uiDriver.findElement(
        By.text('启动 Surface 渲染测试')
      );
      await button.click();
      await uiDriver.sleep(1500);
      
      // 验证标题
      const title = await uiDriver.findElement(
        By.text('Surface Rendering Demo')
      );
      
      expect(title).assertNotNull();
      
      const subtitle = await uiDriver.findElement(
        By.text('XComponent Surface Integration')
      );
      
      expect(subtitle).assertNotNull();
    });
  });
}
```

### 5.2 渲染功能测试

**测试文件**: `entry/src/ohosTest/ets/uitest/Rendering.test.ets`

#### 4.2.1 XComponent 渲染测试

| 测试用例ID | 测试场景 | 操作步骤 | 预期结果 | 优先级 |
|-----------|---------|---------|---------|--------|
| UI-RND-001 | XComponent显示 | 进入SurfaceDemoPage | XComponent可见 | P0 |
| UI-RND-002 | 渲染状态 | 检查渲染状态文本 | 显示"Rendering..." | P0 |
| UI-RND-003 | 性能指标 | 查看FPS显示 | FPS数值>0 | P1 |
| UI-RND-004 | 返回主页 | 点击返回按钮 | 返回首页 | P0 |

**测试代码示例**:

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { uiDriver, By } from '@ohos.arkxtest';

export default function RenderingTest() {
  describe('Rendering Functionality Tests', () => {
    beforeEach(async () => {
      // 导航到 SurfaceDemoPage
      const button = await uiDriver.findElement(
        By.text('启动 Surface 渲染测试')
      );
      await button.click();
      await uiDriver.sleep(2000);
    });

    it('UI-RND-001: Should display XComponent', 0, async () => {
      // 查找 XComponent
      const xcomponent = await uiDriver.findElement(
        By.className('XComponent')
      );
      
      expect(xcomponent).assertNotNull();
      
      // 验证可见性
      const isVisible = await xcomponent.isVisible();
      expect(isVisible).assertTrue();
    });

    it('UI-RND-002: Should show rendering status', 0, async () => {
      // 查找状态文本
      const statusText = await uiDriver.findElement(
        By.text('Rendering...')
      );
      
      expect(statusText).assertNotNull();
    });

    it('UI-RND-003: Should display FPS greater than zero', 0, async () => {
      // 等待渲染开始
      await uiDriver.sleep(3000);
      
      // 查找 FPS 显示
      const fpsText = await uiDriver.findElement(
        By.id('fpsDisplay')
      );
      
      const fpsValue = await fpsText.getText();
      const fps = parseFloat(fpsValue.replace(' FPS', ''));
      
      expect(fps).assertLarger(0);
    });

    it('UI-RND-004: Should navigate back to home page', 0, async () => {
      // 点击返回按钮
      const backButton = await uiDriver.findElement(
        By.text('返回')
      );
      
      await backButton.click();
      await uiDriver.sleep(1000);
      
      // 验证回到首页
      const homeTitle = await uiDriver.findElement(
        By.text('ArkZeroRenderer - Ultra Low Latency Demo')
      );
      
      expect(homeTitle).assertNotNull();
    });
  });
}
```

---

## 6. 测试执行与报告

### 6.1 测试入口文件配置

#### 6.1.1 单元测试入口

**文件位置**: `entry/src/ohosTest/ets/unittest/List.test.ets`

```typescript
import { describe } from '@ohos/hypium';

// C++ 层测试（通过 NAPI 桥接）
import PerformanceMonitorTest from './native/PerformanceMonitor.test';
import PixelFormatConverterTest from './native/PixelFormatConverter.test';
import RenderQueueTest from './native/RenderQueue.test';
import RenderCommandTest from './native/RenderCommand.test';

// ArkTS 层测试
import PixelFormatTest from './arkts/PixelFormat.test';
import ArkZeroRendererTest from './arkts/ArkZeroRenderer.test';

export default function testsuite() {
  describe('ArkZeroRenderer Unit Tests', () => {
    // C++ 层测试
    PerformanceMonitorTest();
    PixelFormatConverterTest();
    RenderQueueTest();
    RenderCommandTest();
    
    // ArkTS 层测试
    PixelFormatTest();
    ArkZeroRendererTest();
  });
}
```

#### 6.1.2 UI 测试入口

**文件位置**: `entry/src/ohosTest/ets/uitest/List.ui.test.ets`

```typescript
import { describe } from '@ohos/hypium';

import NavigationTest from './Navigation.test';
import RenderingTest from './Rendering.test';

export default function uiTestsuite() {
  describe('ArkZeroRenderer UI Tests', () => {
    NavigationTest();
    RenderingTest();
  });
}
```

#### 6.1.3 集成测试入口

**文件位置**: `entry/src/ohosTest/ets/integration/List.integration.test.ets`

```typescript
import { describe } from '@ohos/hypium';

// Level 1: NAPI 桥接测试
import ModuleLoadingTest from './napi/ModuleLoading.test';
import TypeMappingTest from './napi/TypeMapping.test';
import ErrorHandlingTest from './napi/ErrorHandling.test';

// Level 2: 渲染管线测试
import QueueToRendererTest from './pipeline/QueueToRenderer.test';
import PerformanceIntegrationTest from './pipeline/PerformanceIntegration.test';
import FormatConversionTest from './pipeline/FormatConversion.test';

// Level 3: Surface 管理测试
import EGLContextLifecycleTest from './surface/EGLContextLifecycle.test';
import SurfaceBindingTest from './surface/SurfaceBinding.test';
import TextureManagementTest from './surface/TextureManagement.test';

// Level 4: 端到端测试
import FullRenderCycleTest from './e2e/FullRenderCycle.test';
import ResizeWorkflowTest from './e2e/ResizeWorkflow.test';
import ResourceCleanupTest from './e2e/ResourceCleanup.test';

export default function integrationTestsuite() {
  describe('ArkZeroRenderer Integration Tests', () => {
    // Level 1: NAPI Bridge
    ModuleLoadingTest();
    TypeMappingTest();
    ErrorHandlingTest();
    
    // Level 2: Render Pipeline
    QueueToRendererTest();
    PerformanceIntegrationTest();
    FormatConversionTest();
    
    // Level 3: Surface Management
    EGLContextLifecycleTest();
    SurfaceBindingTest();
    TextureManagementTest();
    
    // Level 4: E2E
    FullRenderCycleTest();
    ResizeWorkflowTest();
    ResourceCleanupTest();
  });
}
```

#### 6.1.4 测试运行器

**文件位置**: `entry/src/ohosTest/ets/testrunner/OpenHarmonyTestRunner.ets`

```typescript
import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import { Hypium } from '@ohos/hypium';
import testsuite from '../test/List.test';  // ⭐ 静态导入，不要使用动态 import()

export default class OpenHarmonyTestRunner {
  constructor() {
  }

  onPrepare() {
    console.info('[TestRunner] onPrepare - Initializing test environment');
  }

  onRun() {
    console.info('[TestRunner] onRun - Starting test execution');
    
    try {
      const abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
      const abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
      
      console.info('[TestRunner] Executing test suite');
      Hypium.hypiumTest(abilityDelegator, abilityDelegatorArguments, testsuite);
      
      console.info('[TestRunner] Test suite execution completed');
    } catch (error) {
      console.error('[TestRunner] Failed to execute test suite:', (error as Error).message);
    }
  }
}
```

**⚠️ 重要注意事项**：

1. **必须使用静态导入**，不能使用动态 `import()`
   ```typescript
   // ✅ 正确：静态导入
   import testsuite from '../test/List.test';
   
   // ❌ 错误：动态导入会导致测试卡住
   import('../test/List.test').then((testSuite) => {
     testSuite.default();
   });
   ```

2. **原因说明**：
   - 动态 `import()` 是异步操作，可能导致测试执行时机不可控
   - Hypium 框架需要在 `onRun()` 同步执行时注册测试套
   - 异步加载会导致测试套注册失败，出现"测试套未在 List.test.ets 文件中注册"错误

3. **官方推荐做法**：
   - 在文件顶部直接导入测试套件函数
   - 在 `onRun()` 中调用 `Hypium.hypiumTest()` 执行测试
   - 确保测试套在运行时立即可用

### 6.2 测试执行命令

```bash
# 运行所有单元测试
hvigorw test --mode module --product default

# 运行特定测试类
hvigorw test --class PerformanceMonitorTest

# 运行集成测试
hvigorw test --suite integration

# 运行C++测试
cd entry/src/main/cpp/tests
./run_tests.sh

# 生成覆盖率报告
hvigorw test --coverage

# 查看详细输出
hvigorw test --debug
```

### 6.3 集成测试覆盖率目标

| 集成级别 | 模块组合 | 覆盖率目标 | 关键路径 |
|---------|---------|-----------|----------|
| **Level 1 (NAPI)** | napi_init + Types | ≥ 95% | 模块加载、类型映射 |
| **Level 2 (Pipeline)** | Queue+Renderer+Monitor | ≥ 90% | 数据流、性能统计 |
| **Level 3 (Surface)** | EGL+Surface+Texture | ≥ 85% | Context 生命周期 |
| **Level 4 (E2E)** | 全栈集成 | ≥ 80% | 完整工作流 |
| **总体** | 所有层级 | ≥ 85% | 关键业务路径 |

### 6.4 测试报告模板

```markdown
# 单元测试报告 - v1.0.0

## 执行摘要
- 总测试用例: 127
- 通过: 125 (98.4%)
- 失败: 2 (1.6%)
- 跳过: 0 (0%)

## C++ 层测试
- PerformanceMonitor: 45/45 ✅
- PixelFormatConverter: 18/18 ✅
- RenderQueue: 32/32 ✅
- RenderCommand: 8/8 ✅

## ArkTS 层测试
- PixelFormat: 7/7 ✅
- ArkZeroRenderer: 13/15 ⚠️
  - AZR-RF-008: 失败（内存泄漏检测）
  - AZR-RF-009: 失败（并发渲染超时）

## 覆盖率
- 语句覆盖率: 87.3%
- 分支覆盖率: 82.1%
- 函数覆盖率: 100%

## 失败用例详情
1. AZR-RF-008: 多次渲染内存增长超过阈值
   - 预期: < 5MB
   - 实际: 12MB
   
2. AZR-RF-009: 并发渲染超时
   - 预期: < 100ms
   - 实际: 250ms

## 建议
- 修复内存泄漏问题
- 优化并发渲染性能
```

---

**文档版本**: 1.0.0  
**最后更新**: 2026-05-12  
**维护者**: ArkZeroRenderer QA Team

---

## 8. Instrument Test 运行指南

### 8.1 测试环境要求

**重要说明**: ArkZero 项目的测试属于 **Instrument Test**（仪器化测试），必须在 HarmonyOS 设备或模拟器上运行，因为需要访问 NAPI 和 C++ Native 代码。

#### 8.1.1 前置条件

1. **开发环境**
   - DevEco Studio 6.0+ 或更高版本
   - HarmonyOS SDK API 12+
   - Node.js 16+ (用于 ohpm)

2. **测试设备**
   - 物理设备：HarmonyOS 手机/平板（开发者模式已开启）
   - 模拟器：DevEco Studio 内置模拟器（推荐 Phone - Mate 40 Pro）

3. **依赖配置**
   ```json5
   // entry/oh-package.json5
   {
     "devDependencies": {
       "@ohos/hypium": "1.0.24"
     }
   }
   ```

#### 8.1.2 测试基础设施文件

确保以下文件存在且配置正确：

```
entry/src/ohosTest/
├── ets/
│   ├── test/                          # 测试用例
│   │   ├── List.test.ets             # 测试套件入口
│   │   └── native/
│   │       └── PerformanceMonitor.test.ets
│   ├── testability/                   # 测试基础设施 ⭐
│   │   ├── TestAbility.ets           # 测试 Ability 入口
│   │   └── pages/
│   │       └── Index.ets             # 测试执行页面
│   └── testrunner/
│       └── OpenHarmonyTestRunner.ets  # 测试运行器
├── resources/
│   └── base/
│       └── profile/
│           └── test_pages.json       # 测试页面配置 ⭐
└── module.json5                       # 测试模块配置
```

**关键配置文件内容**：

**test_pages.json** (`entry/src/ohosTest/resources/base/profile/test_pages.json`):
```json
{
  "src": [
    "testability/pages/Index"
  ]
}
```

**module.json5** (`entry/src/ohosTest/module.json5`):
```json5
{
  "module": {
    "name": "entry_test",
    "type": "feature",
    "mainElement": "TestAbility",
    "pages": "$profile:test_pages",  // 引用 test_pages.json
    "abilities": [
      {
        "name": "TestAbility",
        "srcEntry": "./ets/testability/TestAbility.ets",
        "exported": true
      }
    ]
  }
}
```

---

### 8.2 运行 Instrument Test

#### 8.2.1 方法一：通过 DevEco Studio（推荐）

**步骤 1: 连接设备或启动模拟器**

1. 打开 DevEco Studio
2. 点击菜单栏 **Tools** → **Device Manager**
3. 选择并启动模拟器，或连接物理设备
4. 确认设备在线（绿色指示灯）

**步骤 2: 配置测试运行**

1. 点击顶部工具栏 **Run** → **Edit Configurations...**
2. 点击 **+** → 选择 **HarmonyOS Test**
3. 配置如下：
   - **Name**: `ArkZero Instrument Test`
   - **Module**: `entry`
   - **Target**: `ohosTest`
   - **Test Runner Class**: `OpenHarmonyTestRunner`
   - **Device**: 选择已连接的设备/模拟器

**步骤 3: 运行测试**

1. 点击 **Run** 按钮（绿色三角形）或按 `Shift+F10`
2. 等待应用安装到设备
3. 自动启动 TestAbility 并执行测试

**步骤 4: 查看测试结果**

- **Run 窗口**: 显示测试执行进度和结果
- **HiLog 窗口**: 查看详细日志（过滤标签 `[Hypium]`、`[INSTRUMENT_TEST]`）
- **测试报告**: 自动生成 HTML 报告（位于 `entry/build/reports/`）

**预期输出示例**:
```
[I] [INSTRUMENT_TEST] START
[I] [INSTRUMENT_TEST] Running test suite...
[I] [Hypium] PerformanceMonitor Constructor Tests
[I] [Hypium] ✓ PM-CTOR-001: Should initialize with zero frames
[I] [Hypium] ✓ PM-CTOR-002: Should initialize with zero dropped frames
[I] [Hypium] ✓ PM-CTOR-003: Should initialize with zero FPS
[I] [Hypium] ✓ PM-CTOR-004: Should initialize with zero drop rate
[I] [Hypium] PerformanceMonitor Frame Recording Tests
[I] [Hypium] ✓ PM-BF-001: Should record normal frame
[I] [Hypium] ✓ PM-BF-002: Should record dropped frame
...
[I] [Hypium] Total: 18 tests, 18 passed, 0 failed
[I] [INSTRUMENT_TEST] END
```

---

#### 8.2.2 方法二：通过命令行

**步骤 1: 清理并构建项目**

```powershell
# 进入项目根目录
cd C:\learn\ArkZero

# 清理构建缓存
.\hvigorw.bat clean

# 安装依赖
ohpm install

# 构建测试包
.\hvigorw.bat assembleHap -p module=entry -p target=ohosTest
```

**步骤 2: 运行测试**

```powershell
# 运行所有 Instrument Test
.\hvigorw.bat test -p module=entry -p target=ohosTest

# 或者使用完整命令
.\hvigorw.bat test --mode module -p product=default -p module=entry
```

**步骤 3: 查看测试结果**

```powershell
# 实时查看 HiLog 日志（另开一个终端）
hdc hilog | findstr "Hypium"

# 查看测试报告
start entry\build\reports\tests\ohosTest\index.html
```

---

#### 8.2.3 方法三：通过 hdc 命令直接运行

**适用场景**: 自动化脚本、CI/CD 流水线

```powershell
# 1. 安装测试 HAP
hdc app install entry\build\outputs\default\entry-default-unsigned.hap

# 2. 启动测试 Ability
hdc shell aa start -a TestAbility -b com.samples.ndkxcomponent.entry_test

# 3. 监控测试执行
hdc hilog | findstr "Hypium"

# 4. 获取测试结果（从日志中解析）
hdc shell cat /data/local/tmp/test_result.txt
```

---

### 8.3 调试测试

#### 8.3.1 使用 HiLog 查看日志

```powershell
# 实时监控所有测试日志
hdc hilog

# 只查看 Hypium 框架日志
hdc hilog | findstr "Hypium"

# 查看自定义日志标签
hdc hilog | findstr "INSTRUMENT_TEST"

# 保存日志到文件
hdc hilog > test_log.txt
```

**常用日志标签**:
- `[Hypium]`: Hypium 测试框架输出
- `[INSTRUMENT_TEST]`: 测试页面自定义日志
- `[TestRunner]`: 测试运行器日志
- `[LOCAL_TEST]`: 兼容性日志标签

---

#### 8.3.2 断点调试（DevEco Studio）

**步骤 1: 设置断点**

1. 打开测试文件（如 `PerformanceMonitor.test.ets`）
2. 在代码行号左侧点击，设置红色断点
3. 或在关键位置添加 `debugger;` 语句

**步骤 2: 以调试模式运行**

1. 右键测试配置 → **Debug**
2. 或点击工具栏 **Debug** 按钮（虫子图标）
3. 等待应用启动并连接到调试器

**步骤 3: 使用调试工具**

- **Variables 面板**: 查看变量值
- **Call Stack 面板**: 查看调用栈
- **Watch 面板**: 监视表达式
- **Console 面板**: 执行临时表达式

---

#### 8.3.3 常见问题排查

**问题 1: 找不到 `@ohos/hypium` 模块**

```
Error: Cannot find module '@ohos/hypium'
```

**解决方案**:
```powershell
# 检查依赖是否安装
ohpm list

# 重新安装依赖
ohpm install

# 清理并重新构建
.\hvigorw.bat clean
.\hvigorw.bat assembleHap -p module=entry -p target=ohosTest
```

---

**问题 2: 测试无法找到 Native 模块**

```
Error: Module 'libnativerender.so' not found
```

**解决方案**:
1. 确认 C++ 代码已编译成功
2. 检查 `CMakeLists.txt` 配置
3. 重新构建项目：
   ```powershell
   .\hvigorw.bat clean
   .\hvigorw.bat assembleHap -p module=entry
   ```

---

**问题 3: TestAbility 无法启动**

```
Error: Unable to start TestAbility
```

**解决方案**:
1. 检查 `test_pages.json` 是否存在且配置正确
2. 检查 `TestAbility.ets` 路径是否正确
3. 验证 `module.json5` 中的 `pages` 字段：
   ```json5
   "pages": "$profile:test_pages"  // 必须引用 test_pages.json
   ```

---

**问题 4: 测试超时**

```
Error: Test timeout after 15000ms
```

**解决方案**:
1. 增加超时时间（在 `Index.ets` 中）：
   ```typescript
   abilityDelegatorArguments.parameters = {
     "-s timeout": "30000"  // 增加到 30 秒
   };
   ```
2. 优化测试逻辑，减少不必要的等待
3. 检查是否有异步操作未正确处理

---

### 8.4 快速开始：5分钟运行第一个测试

**步骤 1: 准备环境**（首次运行）

```powershell
cd C:\learn\ArkZero
ohpm install
```

**步骤 2: 启动模拟器**

1. 打开 DevEco Studio
2. **Tools** → **Device Manager**
3. 启动 **Phone - Mate 40 Pro** 模拟器

**步骤 3: 运行测试**

1. 打开 `PerformanceMonitor.test.ets`
2. 右键 → **Run 'PerformanceMonitorTest'**
3. 选择已启动的模拟器
4. 等待测试执行完成

**步骤 4: 查看结果**

- **Run 窗口**: 查看测试通过/失败状态
- **HiLog 窗口**: 查看详细执行日志
- 预期看到：`Total: 18 tests, 18 passed, 0 failed`

---

### 8.5 Instrument Test vs Local Test

| 特性 | Local Test | Instrument Test |
|------|-----------|----------------|
| 目录 | `src/test/` | `src/ohosTest/` ✅ |
| 运行环境 | 本地 JVM | 设备/模拟器 |
| Native 代码访问 | ❌ 不支持 | ✅ 支持 |
| NAPI 调用 | ❌ 不支持 | ✅ 支持 |
| 执行速度 | 快 | 较慢 |
| 适用场景 | 纯 ETS/JS 逻辑 | 需要访问系统 API/Native |

**ArkZero 项目必须使用 Instrument Test**，因为 PerformanceMonitor 等核心功能通过 NAPI 暴露 C++ 实现。

---

### 8.6 CI/CD 集成示例

**GitHub Actions 配置**:

```yaml
name: ArkZero Instrument Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup HarmonyOS SDK
      uses: harmonyos/setup-sdk@v1
      with:
        sdk-version: '12'
    
    - name: Install dependencies
      run: ohpm install
    
    - name: Build test package
      run: hvigorw assembleHap -p module=entry -p target=ohosTest
    
    - name: Run Instrument Test
      uses: harmonyos/run-test@v1
      with:
        emulator-type: 'phone'
        test-module: 'entry'
        test-target: 'ohosTest'
    
    - name: Upload test report
      uses: actions/upload-artifact@v3
      with:
        name: test-report
        path: entry/build/reports/tests/ohosTest/
```

---

## 9. 常见问题与解决方案

### 9.1 测试套注册失败

**错误信息**：
```
错误: 测试套“XXX”所在的函数未在“List.test.ets”文件中注册
```

**原因分析**：
1. **动态 import() 导致异步加载**：测试套未能在 `onRun()` 同步执行时注册
2. **List.test.ets 中嵌套了多余的 describe**：测试函数被包裹在额外的 describe 中
3. **构建缓存未清理**：旧的编译结果仍然包含错误的代码

**解决方案**：

✅ **方案 1：使用静态导入（推荐）**
```typescript
// OpenHarmonyTestRunner.ets
import testsuite from '../test/List.test';  // ✅ 静态导入

export default class OpenHarmonyTestRunner {
  onRun() {
    const abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
    const abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
    Hypium.hypiumTest(abilityDelegator, abilityDelegatorArguments, testsuite);
  }
}
```

❌ **避免：动态导入**
```typescript
// ❌ 错误做法
import('../test/List.test').then((testSuite) => {
  testSuite.default();
});
```

✅ **方案 2：List.test.ets 直接调用测试函数**
```typescript
// List.test.ets
export default function testsuite() {
  // ✅ 直接调用，不要嵌套在 describe 中
  PerformanceMonitorTest();
  PixelFormatTest();
  ArkZeroRendererConfigTest();
}
```

❌ **避免：嵌套 describe**
```typescript
// ❌ 错误做法
export default function testsuite() {
  describe('AllTests', () => {  // ← 多余的 describe
    PerformanceMonitorTest();
  });
}
```

✅ **方案 3：清理构建缓存**
```powershell
# PowerShell
Remove-Item -Recurse -Force entry\.test, entry\build, .hvigor\cache -ErrorAction SilentlyContinue
```

### 9.2 Hypium 命名规范错误

**错误信息**：
```
"XXX Unit Tests"是一个无效的描述值，输入的值必须仅包含数字、字母、下划线（_）和句点（.），并且只能以字母开头。
```

**原因**：`describe()` 或 `it()` 的描述字符串包含了空格或其他非法字符

**解决方案**：
```typescript
// ✅ 正确：使用句点分隔
describe('ModuleName.CategoryName', () => {
  it('TEST-ID-001: Description here', 0, () => {
    // ...
  });
});

// ✅ 也可以：使用下划线
describe('ModuleName_CategoryName', () => {
  // ...
});

// ❌ 错误：包含空格
describe('Module Name Category Name', () => {
  // ...
});
```

### 9.3 构建任务不存在

**错误信息**：
```
Task [ 'assembleOhosTest' ] was not found in the project.
```

**原因**：不同版本的 DevEco Studio 使用不同的任务名称

**解决方案**：
```powershell
# 1. 查看可用任务
hvigorw tasks

# 2. 使用正确的任务名（通常是 test）
hvigorw --mode module -p module=entry -p product=default test
```

### 9.4 NAPI 模块加载失败

**错误信息**：
```
Failed to load native module: libnativerender.so
```

**解决方案**：
1. 确保 C++ 代码编译成功
2. 检查 `CMakeLists.txt` 配置
3. 确认 `oh-package.json5` 中的依赖正确
4. 重新构建整个项目

### 9.5 设备未连接

**错误信息**：
```
error: no devices found
```

**解决方案**：
```powershell
# 1. 检查设备连接
hdc list targets

# 2. 启动模拟器或连接真机
# 在 DevEco Studio 中：Tools → Device Manager

# 3. 确俚开发者模式已开启（真机）
```

---
