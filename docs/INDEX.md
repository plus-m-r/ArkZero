# ArkZero 文档中心

## 文档导航

### 快速开始
- [README.md](./README.md) - 项目概述、核心特性、性能基准、快速代码示例

### 架构设计
- [ARCHITECTURE.md](./ARCHITECTURE.md) - 四层架构、目录结构、核心设计决策、关键约束

### API 参考
- [API_REFERENCE.md](./API_REFERENCE.md) - ArkTS 封装层 API、NAPI 模块层 API、C++ 关键类、使用注意事项

### 测试文档
- [UNIT_TEST_DESIGN.md](./UNIT_TEST_DESIGN.md) - 集成测试设计、增量基准测试、测试运行指南

## 代码位置

### 主代码
- **ArkTS**: `entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`
- **C++**: `entry/src/main/cpp/renderer/`
- **NAPI 绑定**: `entry/src/main/cpp/napi_bridge/`
- **类型声明**: `entry/src/main/cpp/types/libnativerender/Index.d.ts`

### 页面
- **首页**: `entry/src/main/ets/pages/Index.ets`
- **Surface 演示**: `entry/src/main/ets/pages/SurfaceDemoPage.ets`
- **集成测试**: `entry/src/main/ets/integration/pages/IntegrationTestPage.ets`

### 测试
- **集成测试**: `entry/src/main/ets/integration/tests/`
- **增量基准测试**: `entry/src/main/ets/integration/tests/IncrementalBenchmark.test.ets`
