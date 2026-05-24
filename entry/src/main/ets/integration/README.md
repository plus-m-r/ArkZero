# ArkZeroRenderer 集成测试

## 📁 目录结构

```
integration/
├── components/
│   └── TestResultPanel.ets          # 测试结果面板组件
├── pages/
│   └── IntegrationTestPage.ets      # 集成测试主页面
├── tests/
│   └── RendererIntegration.test.ets # 渲染器集成测试逻辑
└── README.md                         # 本文档
```

## 🎯 功能说明

### 1. TestResultPanel.ets
**测试结果面板组件**，用于实时显示测试进度和结果。

**特性**：
- ✅ 实时显示测试进度条
- ✅ 颜色编码状态（绿色=通过，红色=失败，橙色=运行中）
- ✅ 统计通过率
- ✅ 显示每个测试用例的耗时

### 2. RendererIntegration.test.ets
**渲染器集成测试类**，包含 5 个核心测试用例。

**测试用例**：
1. **初始化渲染器** - 验证渲染器创建和 Surface 绑定
2. **渲染单帧** - 测试基本渲染功能
3. **调整渲染尺寸** - 验证动态 resize 功能
4. **连续渲染多帧** - 测试多帧渲染性能和稳定性
5. **资源清理** - 验证 dispose 正确释放资源

### 3. IntegrationTestPage.ets
**集成测试主页面**，提供用户交互界面。

**功能**：
- XComponent Surface 初始化
- 测试启动控制
- 测试结果可视化展示

## 🚀 使用方法

### 方式 1：通过应用界面运行（推荐）

1. **构建并安装应用**：
   ```powershell
   hvigorw clean
   hvigorw --mode module -p module=entry -p product=default assembleHap
   hdc install entry/build/default/outputs/default/entry-default-signed.hap
   ```

2. **启动应用**：
   ```powershell
   hdc shell aa start -a EntryAbility -b com.samples.ndkxcomponent
   ```

3. **运行测试**：
   - 在首页点击 **"🧪 运行集成测试"** 按钮
   - 等待 Surface 初始化完成
   - 点击 **"开始集成测试"** 按钮
   - 查看实时测试结果

### 方式 2：DevEco Studio 调试

1. 打开 `IntegrationTestPage.ets`
2. 设置断点（可选）
3. 运行应用到设备/模拟器
4. 导航到集成测试页面
5. 观察调试信息和日志输出

## 📊 测试结果解读

### 状态图标
- ✅ **通过** - 测试成功完成
- ❌ **失败** - 测试出现错误
- ⏳ **运行中** - 测试正在执行
- ⏸️ **待执行** - 测试尚未开始

### 统计信息
- **总计**: 测试用例总数
- **通过**: 成功的测试数量
- **失败**: 失败的测试数量
- **通过率**: 百分比形式

## 🔧 扩展新的测试模块

### 步骤 1: 创建测试类

在 `tests/` 目录下创建新的测试文件：

```typescript
// 示例：EGLContextLifecycle.test.ets
import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

export class EGLContextLifecycleTest {
  private surfaceId: string = '';
  private results: TestCaseResult[] = [];
  
  constructor(surfaceId: string) {
    this.surfaceId = surfaceId;
  }
  
  async runAllTests(): Promise<TestCaseResult[]> {
    await this.testContextCreation();
    await this.testContextSharing();
    await this.testContextDestruction();
    return this.results;
  }
  
  private async testContextCreation(): Promise<void> {
    // 实现测试逻辑
  }
}
```

### 步骤 2: 在页面中调用

修改 `IntegrationTestPage.ets`：

```typescript
import { EGLContextLifecycleTest } from '../tests/EGLContextLifecycle.test';

// 在 startIntegrationTests 方法中添加
const eglTest = new EGLContextLifecycleTest(this.surfaceId);
await eglTest.runAllTests();
```

### 步骤 3: 更新测试计数

```typescript
@State totalTests: number = 8; // 更新为总测试数
```

## 💡 最佳实践

### 1. 测试命名规范
- 使用清晰的中文名称
- 反映测试的核心功能
- 示例：`初始化渲染器`、`渲染单帧`

### 2. 错误处理
- 捕获所有异常
- 记录详细的错误信息
- 提供有意义的错误消息

### 3. 性能考虑
- 避免在测试中进行大量内存分配
- 及时释放资源
- 记录每个测试的耗时

### 4. 日志输出
- 使用 `hilog` 记录关键步骤
- 区分 info/warn/error 级别
- 便于问题排查

## 🐛 常见问题

### Q1: Surface 初始化失败
**A**: 检查 XComponent 是否正确配置，确保 device 支持 SURFACE 类型。

### Q2: 测试执行缓慢
**A**: 检查是否有不必要的等待或循环，优化测试逻辑。

### Q3: 内存泄漏警告
**A**: 确保在测试完成后调用 `dispose()` 释放资源。

## 📝 下一步计划

- [ ] 添加 EGL 上下文生命周期测试
- [ ] 添加纹理管理器测试
- [ ] 添加性能基准测试（FPS、延迟）
- [ ] 添加 Surface 生命周期测试
- [ ] 添加后台化/前台化恢复测试
- [ ] 导出测试结果为 JSON/XML
- [ ] 添加性能图表可视化

## 📞 技术支持

如有问题，请参考：
- 项目主文档：`README.md`
- 测试设计文档：`UNIT_TEST_DESIGN.md`
- WindowSurface 测试状态：`WINDOWSURFACE_TEST_STATUS.md`

---

**最后更新**: 2026-05-13  
**维护者**: ArkZeroRenderer Team
