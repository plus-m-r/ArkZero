# ArkZeroRenderer 集成测试

## 📁 目录结构

```
integration/
├── components/
│   └── TestResultPanel.ets          # 测试结果面板组件
├── observer/
│   ├── XComponentLifecycleObserver.ts # 生命周期观察者接口
│   ├── XComponentLifecycleManager.ts  # 生命周期管理器
│   └── TexturePreallocator.ts         # 纹理预分配器
├── pages/
│   └── IntegrationTestPage.ets      # 集成测试主页面
├── tests/
│   ├── RendererIntegration.test.ets  # 渲染器基础集成测试
│   ├── EGLContextLifecycle.test.ets  # EGL上下文生命周期测试
│   ├── PixelFormatIntegration.test.ets # 像素格式兼容性测试
│   ├── StressIntegration.test.ets    # 压力测试
│   ├── CallbackIntegration.test.ets  # 回调机制测试
│   └── LifecycleObserverIntegration.test.ets # 生命周期观察者测试
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

### 2. 测试模块

#### 2.1 RendererIntegration.test.ets
**渲染器基础集成测试**，包含 5 个核心测试用例。

**测试用例**：
1. **初始化渲染器** - 验证渲染器创建和 Surface 绑定
2. **渲染单帧** - 测试基本渲染功能
3. **调整渲染尺寸** - 验证动态 resize 功能
4. **连续渲染多帧** - 测试多帧渲染性能和稳定性
5. **资源清理** - 验证 dispose 正确释放资源

#### 2.2 EGLContextLifecycle.test.ets
**EGL上下文生命周期测试**，包含 5 个测试用例。

**测试用例**：
1. **创建EGL上下文** - 验证上下文创建和初始化
2. **复用EGL上下文** - 测试上下文复用能力
3. **EGL上下文调整尺寸** - 验证resize后上下文状态
4. **销毁EGL上下文** - 测试销毁后状态
5. **销毁后重新创建EGL上下文** - 验证重建流程

#### 2.3 PixelFormatIntegration.test.ets
**像素格式兼容性测试**，包含 6 个测试用例。

**测试用例**：
1. **RGBA8888格式渲染** - 测试RGBA格式支持
2. **RGB888格式渲染** - 测试RGB格式支持
3. **BGRA8888格式渲染** - 测试BGRA格式支持
4. **NV21格式渲染** - 测试YUV420半平面NV21格式
5. **NV12格式渲染** - 测试YUV420半平面NV12格式
6. **格式切换性能** - 测试不同格式间切换

#### 2.4 StressIntegration.test.ets
**压力测试**，包含 5 个测试用例。

**测试用例**：
1. **高频渲染测试** - 30帧连续渲染性能
2. **快速尺寸切换测试** - 频繁resize操作
3. **大分辨率渲染测试** - 1080p/2K/4K分辨率
4. **连续创建销毁测试** - 资源泄漏检测
5. **内存压力测试** - 多缓冲区并发渲染

#### 2.5 CallbackIntegration.test.ets
**回调机制测试**，包含 5 个测试用例。

**测试用例**：
1. **单帧渲染完成回调** - 验证回调触发
2. **多帧渲染回调计数** - 验证回调次数
3. **错误情况下回调行为** - 测试异常情况
4. **回调执行顺序** - 验证回调时序
5. **resize后回调行为** - 测试resize后回调状态

#### 2.6 LifecycleObserverIntegration.test.ets
**生命周期观察者测试**，包含 6 个测试用例。

**测试用例**：
1. **观察者添加与移除** - 验证观察者管理
2. **观察者通知顺序** - 测试通知机制
3. **纹理预分配功能** - 验证预分配器
4. **多观察者管理** - 测试多观察者并发
5. **观察者错误处理** - 测试异常隔离
6. **完整生命周期序列** - 端到端测试

### 3. IntegrationTestPage.ets
**集成测试主页面**，提供用户交互界面。

**功能**：
- 选择测试模块
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
   - 选择要运行的测试模块
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
// 示例：NewFeature.test.ets
import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

export class NewFeatureTest {
  private surfaceId: string = '';
  private results: TestCaseResult[] = [];
  
  constructor(surfaceId: string) {
    this.surfaceId = surfaceId;
  }
  
  async runAllTests(): Promise<TestCaseResult[]> {
    await this.testFeature1();
    await this.testFeature2();
    return this.results;
  }
  
  private async testFeature1(): Promise<void> {
    const startTime = Date.now();
    const testName = '测试功能1';
    
    try {
      // 实现测试逻辑
      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: '测试通过'
      });
    } catch (error) {
      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: String(error)
      });
    }
  }
}
```

### 步骤 2: 在页面中调用

修改 `IntegrationTestPage.ets`：

```typescript
import { NewFeatureTest } from '../tests/NewFeature.test';

// 在TestModule枚举中添加
enum TestModule {
  // ... 现有模块
  NEW_FEATURE = '新功能测试'
}

// 在runTestSwitch中添加case
case TestModule.NEW_FEATURE:
  this.totalTests = 2;
  await this.runNewFeatureTest();
  break;

// 添加运行方法
private async runNewFeatureTest(): Promise<void> {
  const test = new NewFeatureTest(this.surfaceId, (results) => {
    this.testResults = results;
    this.completedTests = results.length;
  });
  await test.runAllTests();
}

// 在build()方法中添加按钮
Button(TestModule.NEW_FEATURE)
  .onClick(() => {
    this.runTestModule(TestModule.NEW_FEATURE);
  })
```

### 步骤 3: 更新测试计数

```typescript
this.totalTests = 2; // 更新为实际的测试用例数
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

## 📝 测试统计

| 测试模块 | 测试用例数 | 状态 |
|---------|-----------|------|
| 渲染器基础测试 | 5 | ✅ 可用 |
| EGL上下文生命周期 | 5 | ✅ 可用 |
| 像素格式兼容性 | 6 | ✅ 可用 |
| 压力测试 | 5 | ✅ 可用 |
| 回调机制测试 | 5 | ✅ 可用 |
| 生命周期观察者 | 6 | ✅ 可用 |
| **总计** | **32** | - |

## 📞 技术支持

如有问题，请参考：
- 项目主文档：`README.md`
- 测试设计文档：`UNIT_TEST_DESIGN.md`

---

**最后更新**: 2026-05-24  
**维护者**: ArkZeroRenderer Team