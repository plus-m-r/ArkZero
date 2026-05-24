# ArkZero 文档中心

欢迎来到 ArkZero 项目的文档中心！这里包含了项目的所有技术文档。

## 📚 文档导航

### 🚀 快速开始
- **[README.md](./README.md)** - 项目概述和快速入门指南
  - 项目简介
  - 主要功能
  - 快速开始步骤

### 🏗️ 架构设计
- **[ARCHITECTURE.md](./ARCHITECTURE.md)** - 系统架构设计文档
  - 整体架构说明
  - 模块设计
  - 数据流分析

### 🧪 测试文档
- **[UNIT_TEST_DESIGN.md](./UNIT_TEST_DESIGN.md)** ⭐ **核心文档**
  - 完整的测试架构设计
  - 单元测试（C++ NAPI + ArkTS）
  - 集成测试（main 模块）
  - UI 自动化测试
  - 测试最佳实践
  - 常见问题与解决方案
  
  **推荐阅读顺序**：
  1. 第 1 章 - 了解测试项目结构
  2. 第 4.2 章 - Main 模块集成测试（新增）
  3. 第 8 章 - Instrument Test 运行指南
  4. 第 9 章 - 常见问题与解决方案
  5. 第 10 章 - 更新日志

### 📖 API 参考
- **[API_REFERENCE.md](./API_REFERENCE.md)** - 完整的 API 参考文档
  - 类和方法说明
  - 使用示例
  - 参数详解

### 📋 其他文档
- **[DOCUMENT_CLEANUP_SUMMARY.md](./DOCUMENT_CLEANUP_SUMMARY.md)** - 文档清理总结
  - 文档整理记录
  - 清理统计信息

## 🎯 根据角色选择文档

### 👨‍💻 新开发者
1. 阅读 [README.md](./README.md) 了解项目
2. 阅读 [ARCHITECTURE.md](./ARCHITECTURE.md) 理解架构
3. 阅读 [UNIT_TEST_DESIGN.md](./UNIT_TEST_DESIGN.md) 学习测试

### 🧪 测试工程师
1. 重点阅读 [UNIT_TEST_DESIGN.md](./UNIT_TEST_DESIGN.md)
   - 第 4 章 - 集成测试
   - 第 5 章 - UI 自动化测试
   - 第 8 章 - 测试运行指南
2. 参考 [API_REFERENCE.md](./API_REFERENCE.md) 了解接口

### 🔧 日常开发
- 查阅 [API_REFERENCE.md](./API_REFERENCE.md) 了解 API 用法
- 参考 [UNIT_TEST_DESIGN.md](./UNIT_TEST_DESIGN.md) 编写测试

## 📂 代码位置

### 主代码
- **ArkTS 代码**: `entry/src/main/ets/`
- **C++ 代码**: `entry/src/main/cpp/`
- **集成测试**: `entry/src/main/ets/integration/`

### 测试代码
- **单元测试**: `entry/src/ohosTest/ets/test/`
- **Native 测试**: `entry/src/ohosTest/ets/test/native/`
- **ArkTS 测试**: `entry/src/ohosTest/ets/test/arkts/`

## 🔗 相关资源

- **测试运行脚本**: `run_instrument_test.ps1`
- **构建配置**: `build-profile.json5`
- **依赖配置**: `oh-package.json5`

## 💡 使用提示

### 搜索文档
- 使用 IDE 的全局搜索功能查找特定内容
- 在 UNIT_TEST_DESIGN.md 中使用 Ctrl+F 搜索关键词

### 最新版本
- 所有文档都保存在 Git 版本控制中
- 查看历史变更：`git log -- docs/`

### 贡献文档
- 发现错误？请提交 Issue
- 改进建议？欢迎 Pull Request

## 📅 文档更新

最后更新时间：**2026-05-13**

主要更新：
- ✅ 集成测试迁移到 main 模块
- ✅ 文档结构优化（统一到 docs 目录）
- ✅ 删除冗余文档，保留核心内容

---

**维护者**: ArkZeroRenderer Team  
**文档版本**: v2.0  
**许可证**: Apache-2.0
