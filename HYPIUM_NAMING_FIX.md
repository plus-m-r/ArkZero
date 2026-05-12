# Hypium 测试命名规范修复报告

**修复时间**: 2026-05-12  
**问题**: describe 描述字符串包含空格，违反 Hypium 命名规范  
**状态**: ✅ 已修复

---

## 🔍 问题描述

**错误信息**：
```
"ArkZeroRenderer Unit Tests"是一个无效的描述值，输入的值必须仅包含数字、字母、下划线（_）和句点（.），并且只能以字母开头。
```

**根本原因**：
Hypium 测试框架对 `describe()` 和 `it()` 的描述字符串有严格的命名规范，不允许使用空格。

---

## ✅ 修复内容

### 修改的文件清单

| 文件 | 修改数量 | 状态 |
|------|---------|------|
| `entry/src/ohosTest/ets/test/List.test.ets` | 1 处 | ✅ 已修复 |
| `entry/src/ohosTest/ets/test/native/PerformanceMonitor.test.ets` | 4 处 | ✅ 已修复 |
| `entry/src/ohosTest/ets/test/arkts/PixelFormat.test.ets` | 1 处 | ✅ 已修复 |
| `entry/src/ohosTest/ets/test/arkts/ArkZeroRendererConfig.test.ets` | 1 处 | ✅ 已修复 |
| `run_instrument_test.ps1` | 1 处 | ✅ 已更新 |

**总计**: 7 处 describe 描述 + 1 处构建命令 = **8 处修改**

---

## 📝 具体修改对照表

### 1. List.test.ets

```typescript
// ❌ 修改前
describe('ArkZeroRenderer Unit Tests', () => {

// ✅ 修改后
describe('ArkZeroRenderer.UnitTests', () => {
```

### 2. PerformanceMonitor.test.ets

```typescript
// ❌ 修改前
describe('PerformanceMonitor Constructor Tests', () => {
describe('PerformanceMonitor Frame Recording Tests', () => {
describe('PerformanceMonitor Reset Tests', () => {
describe('PerformanceMonitor Advanced Tests', () => {

// ✅ 修改后
describe('PerformanceMonitor.ConstructorTests', () => {
describe('PerformanceMonitor.FrameRecordingTests', () => {
describe('PerformanceMonitor.ResetTests', () => {
describe('PerformanceMonitor.AdvancedTests', () => {
```

### 3. PixelFormat.test.ets

```typescript
// ❌ 修改前
describe('PixelFormat Enum Tests', () => {

// ✅ 修改后
describe('PixelFormat.EnumTests', () => {
```

### 4. ArkZeroRendererConfig.test.ets

```typescript
// ❌ 修改前
describe('ArkZeroRendererConfig Interface Tests', () => {

// ✅ 修改后
describe('ArkZeroRendererConfig.InterfaceTests', () => {
```

### 5. run_instrument_test.ps1

```powershell
# ❌ 修改前
& $HVIGOR $HVIGOR_SCRIPT --mode module -p module=entry -p product=default assembleOhosTest

# ✅ 修改后
& $HVIGOR $HVIGOR_SCRIPT --mode module -p module=entry -p product=default test
```

---

## 📋 Hypium 命名规范详解

### ✅ 允许的字符

| 字符类型 | 示例 | 说明 |
|---------|------|------|
| 字母 | `A-Z`, `a-z` | 必须以字母开头 |
| 数字 | `0-9` | 不能单独作为开头 |
| 下划线 | `_` | 可用于分隔单词 |
| 句点 | `.` | 推荐用于层级分隔 |

### ❌ 禁止的字符

| 字符 | 示例 | 原因 |
|------|------|------|
| 空格 | `'Unit Tests'` | 不被解析器接受 |
| 横线 | `'unit-tests'` | 特殊含义 |
| 其他符号 | `!@#$%^&*()` | 保留字符 |

### 🎯 最佳实践

**推荐格式**：使用句点（`.`）分隔层级

```typescript
// ✅ 推荐：清晰的层级结构
describe('ModuleName.CategoryName', () => {
  it('TEST-ID-001: Description here', 0, () => {
    // ...
  });
});

// ✅ 也可以：使用下划线
describe('ModuleName_CategoryName', () => {
  // ...
});

// ❌ 避免：使用空格
describe('Module Name Category Name', () => {
  // ...
});
```

**测试 ID 格式**：
```typescript
// ✅ 推荐格式
it('PM-CTOR-001: Should initialize with zero frames', 0, async () => {
  // PM = PerformanceMonitor
  // CTOR = Constructor
  // 001 = 序号
});

it('PF-VAL-002: RGB should equal 1', 0, () => {
  // PF = PixelFormat
  // VAL = Value
  // 002 = 序号
});
```

---

## 🚀 运行测试

### 方法一：使用自动化脚本（推荐）

```powershell
cd C:\learn\ArkZero
.\run_instrument_test.ps1
```

### 方法二：手动执行

```powershell
cd C:\learn\ArkZero
$env:DEVECO_SDK_HOME = "C:\Program Files\Huawei\DevEco Studio\sdk"

# 运行测试
& "C:\Program Files\Huawei\DevEco Studio\tools\node\node.exe" `
  "C:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js" `
  --mode module -p module=entry -p product=default test
```

### 方法三：通过 DevEco Studio

1. 右键点击 `List.test.ets`
2. 选择 **Run 'List.test'**
3. 选择设备/模拟器
4. 查看测试结果

---

## 📊 预期测试结果

### 成功的输出示例

```
[TestRunner] onPrepare - Initializing test environment
[TestRunner] onRun - Starting test execution
[TestRunner] Test suite loaded successfully

Hypium: ArkZeroRenderer.UnitTests
  Hypium: PerformanceMonitor.ConstructorTests
    ✓ PM-CTOR-001: Should initialize with zero frames
    ✓ PM-CTOR-002: Should initialize with zero dropped frames
    ...
  Hypium: PerformanceMonitor.FrameRecordingTests
    ✓ PM-BF-001: Should record normal frame
    ...
  Hypium: PixelFormat.EnumTests
    ✓ PF-VAL-001: RGBA should equal 0
    ...
  Hypium: ArkZeroRendererConfig.InterfaceTests
    ✓ AZRC-REQ-001: Config should have width field
    ...

Hypium Summary:
Total: 73, Passed: 73, Failed: 0, Skipped: 0
Test Duration: 3.456s

✅ All tests passed!
```

---

## ⚠️ 注意事项

### 1. it() 描述也需遵守规范

虽然 `it()` 的描述可以包含更多字符（如冒号、空格），但建议保持一致性：

```typescript
// ✅ 推荐
it('PM-CTOR-001: Should initialize with zero frames', 0, async () => {

// ⚠️ 也可以（但不推荐混合使用）
it('PM CTOR 001 Should initialize', 0, async () => {
```

### 2. 未来新增测试文件

创建新的测试文件时，请遵循以下模板：

```typescript
import { describe, it, expect } from '@ohos/hypium';

export default function NewModuleTest() {
  describe('NewModule.CategoryName', () => {
    it('NM-CAT-001: Test description here', 0, () => {
      expect(true).assertTrue();
    });
  });
}
```

### 3. 在 UNIT_TEST_DESIGN.md 中更新

记得在测试设计文档中也使用正确的命名格式：

```markdown
## 测试用例列表

- `PM-CTOR-001`: Should initialize with zero frames
- `PF-VAL-001`: RGBA should equal 0
- `AZRC-REQ-001`: Config should have width field
```

---

## 📈 修复验证

### 验证步骤

1. **检查所有 describe 描述**
   ```powershell
   # 查找是否还有包含空格的 describe
   Get-ChildItem entry\src\ohosTest\ets\test -Recurse -Filter "*.test.ets" | 
     Select-String "describe\('.* .*'\s*,"
   ```
   
   **预期输出**：无结果（表示已全部修复）

2. **编译检查**
   ```powershell
   & "C:\Program Files\Huawei\DevEco Studio\tools\node\node.exe" `
     "C:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js" `
     --mode module -p module=entry -p product=default compileOhosTest
   ```

3. **运行测试**
   ```powershell
   .\run_instrument_test.ps1
   ```

---

## 🎯 总结

✅ **已完成**：
- 修复了 7 个 `describe` 描述字符串
- 更新了自动化脚本的构建命令
- 符合 Hypium 命名规范
- 所有测试文件已准备就绪

🚀 **下一步**：
- 运行第一阶段测试（73 个用例）
- 验证测试结果
- 根据测试结果进行第二阶段开发

---

**修复完成时间**: 2026-05-12 23:00  
**修复人员**: ArkZero QA Team  
**下次更新**: 测试运行完成后
