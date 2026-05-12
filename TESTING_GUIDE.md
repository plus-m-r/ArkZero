# ArkZeroRenderer 测试指南

本文档提供 ArkZeroRenderer 项目的完整测试方案，包括环境准备、功能测试、性能测试和故障排查。

---

## 📋 目录

- [1. 测试环境准备](#1-测试环境准备)
- [2. 编译与构建测试](#2-编译与构建测试)
- [3. 自动化集成测试（Hypium + arkxtest）](#3-自动化集成测试hypium--arkxtest)
- [4. 手动功能测试](#4-手动功能测试)
- [5. 性能测试](#5-性能测试)
- [6. 压力测试](#6-压力测试)
- [7. 兼容性测试](#7-兼容性测试)
- [8. 日志监控](#8-日志监控)
- [9. 常见问题排查](#9-常见问题排查)

---

## 1. 测试环境准备

### 1.1 硬件要求

| 组件 | 最低配置 | 推荐配置 |
|------|---------|---------|
| **设备** | HarmonyOS 5.0+ 真机或模拟器 | HarmonyOS 5.0+ 真机 |
| **GPU** | 支持 OpenGL ES 3.0 | 支持 OpenGL ES 3.2+ |
| **内存** | 2GB RAM | 4GB+ RAM |
| **存储** | 100MB 可用空间 | 500MB+ 可用空间 |
| **屏幕分辨率** | 720p (1280x720) | 1080p (1920x1080) 或 4K |

### 1.2 软件要求

| 软件 | 版本要求 |
|------|---------|
| **DevEco Studio** | 6.0+ (推荐 6.0.2 Release) |
| **HarmonyOS SDK** | API 12+ (HarmonyOS 5.0+) |
| **Node.js** | 16.x+ (用于 ohpm) |
| **CMake** | 3.5.0+ (内置于 DevEco) |
| **NDK** | 随 SDK 自动安装 |

### 1.3 环境验证

在开始测试前，请确认以下配置：

```bash
# 1. 检查 DevEco Studio 版本
# 打开 DevEco Studio → Help → About

# 2. 检查 SDK 版本
# 打开 DevEco Studio → Settings → HarmonyOS SDK
# 确认已安装 API 12+ SDK

# 3. 检查 NDK 工具链
# 打开 DevEco Studio → Settings → HarmonyOS SDK → Native
# 确认 CMake、LLDB、Native 工具链已安装

# 4. 验证 Git 配置（用于 SSH 推送）
git remote -v
# 应显示: origin  git@github.com:plus-m-r/ArkZero.git
```

---

## 2. 编译与构建测试

### 2.1 清理构建缓存

```bash
# 在 DevEco Studio 中
Build → Clean Project

# 或使用命令行
hvigorw clean
```

### 2.2 编译项目

```bash
# 在 DevEco Studio 中
Build → Rebuild Project

# 预期结果：
# ✅ BUILD SUCCESSFUL
# ❌ 如有错误，查看错误日志并修复
```

### 2.3 验证编译产物

编译成功后，检查以下文件是否生成：

```
entry/build/default/outputs/default/
├── entry-default-signed.hap    # 应用安装包
└── libs/
    └── arm64-v8a/
        └── libnativerender.so  # Native 渲染库
```

### 2.4 常见编译错误及解决

| 错误信息 | 原因 | 解决方案 |
|---------|------|---------|
| `Cannot find module 'libnativerender.so'` | 导入路径错误 | 确认使用 `'libnativerender.so'` 而非 `'nativerender'` |
| `'PixelFormat' cannot be used as a value` | 枚举导出方式错误 | 使用 `export enum` 语法 |
| `napi_define_properties failed` | NAPI 注册失败 | 检查 `napi_init.cpp` 中的模块名配置 |
| `CMake configuration failed` | CMake 配置错误 | 检查 `CMakeLists.txt` 中的依赖库路径 |

---

## 3. 自动化集成测试（Hypium + arkxtest）

### 3.1 测试框架介绍

**Hypium** 是 HarmonyOS 官方的单元测试框架，**arkxtest** 是用于 UI 自动化测试的工具。本方案结合两者实现完整的自动化测试。

#### 测试层级：

| 层级 | 工具 | 测试内容 |
|------|------|---------|
| **单元测试** | Hypium | C++ 核心组件、ArkTS 工具类 |
| **集成测试** | Hypium + NAPI Mock | Native 模块接口、渲染器 API |
| **UI 自动化** | arkxtest | 页面跳转、渲染效果、交互流程 |

### 3.2 环境配置

#### 步骤 1：安装测试依赖

在 `entry/oh-package.json5` 中添加测试依赖：

```json5
{
  "license": "",
  "devDependencies": {
    "@ohos/hypium": "1.0.6"
  },
  "author": "",
  "name": "entry",
  "description": "Please describe the basic information.",
  "main": "",
  "version": "1.0.0",
  "dependencies": {}
}
```

#### 步骤 2：配置测试目录结构

创建以下目录结构：

```
entry/src/
├── main/                    # 主代码
└── ohosTest/               # 测试代码
    ├── ets/
    │   ├── testrunner/
    │   │   └── OpenHarmonyTestRunner.ts
    │   ├── unittest/
    │   │   ├── renderer/
    │   │   │   ├── ArkZeroRenderer.test.ets
    │   │   │   └── PixelFormat.test.ets
    │   │   └── utils/
    │   │       └── BufferPool.test.ets
    │   └── uitest/
    │       ├── pages/
    │       │   └── SurfaceDemoPage.test.ets
    │       └── common/
    │           └── TestHelper.ets
    └── resources/
        └── base/
            └── profile/
                └── test_pages.json
```

#### 步骤 3：配置测试运行器

创建 `entry/src/ohosTest/ets/testrunner/OpenHarmonyTestRunner.ts`：

```typescript
import { TestRunner } from '@ohos/hypium';

export default class OpenHarmonyTestRunner implements TestRunner {
  constructor() {
  }

  onPrepare() {
    console.info('OpenHarmonyTestRunner OnPrepare');
  }

  async onExecute() {
    console.info('OpenHarmonyTestRunner OnExecute');
    // Hypium 会自动执行所有测试用例
  }
}
```

#### 步骤 4：配置 build-profile.json5

在 `entry/build-profile.json5` 中添加测试配置：

```json5
{
  "apiType": "stageMode",
  "buildOption": {
  },
  "targets": [
    {
      "name": "default"
    },
    {
      "name": "ohosTest",
      "runtimeOS": "HarmonyOS"
    }
  ]
}
```

### 3.3 编写单元测试

#### 测试用例 1：PixelFormat 枚举测试

文件：`entry/src/ohosTest/ets/unittest/renderer/PixelFormat.test.ets`

```typescript
import { describe, it, expect } from '@ohos/hypium';
import { PixelFormat } from '../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function PixelFormatTest() {
  describe('PixelFormat Enum Tests', () => {
    it('should have correct RGBA value', 0, () => {
      expect(PixelFormat.RGBA).assertEqual(0);
    });

    it('should have correct RGB value', 0, () => {
      expect(PixelFormat.RGB).assertEqual(1);
    });

    it('should have correct BGRA value', 0, () => {
      expect(PixelFormat.BGRA).assertEqual(2);
    });

    it('should have correct NV21 value', 0, () => {
      expect(PixelFormat.NV21).assertEqual(3);
    });

    it('should have correct NV12 value', 0, () => {
      expect(PixelFormat.NV12).assertEqual(4);
    });

    it('should have all 5 format types', 0, () => {
      const formats = [PixelFormat.RGBA, PixelFormat.RGB, PixelFormat.BGRA, PixelFormat.NV21, PixelFormat.NV12];
      expect(formats.length).assertEqual(5);
    });
  });
}
```

#### 测试用例 2：ArkZeroRenderer 初始化测试

文件：`entry/src/ohosTest/ets/unittest/renderer/ArkZeroRenderer.test.ets`

```typescript
import { describe, it, expect, beforeAll, afterAll } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../main/ets/components/rendering/ArkZeroRenderer';

export default function ArkZeroRendererTest() {
  describe('ArkZeroRenderer Unit Tests', () => {
    let renderer: ArkZeroRenderer;

    beforeAll(() => {
      console.info('BeforeAll: Create renderer instance');
    });

    it('should create renderer instance', 0, () => {
      renderer = new ArkZeroRenderer({
        width: 1920,
        height: 1080,
        format: PixelFormat.RGBA
      });
      expect(renderer).assertNotNull();
    });

    it('should have correct initial state', 0, () => {
      // 注意：由于无法访问私有属性，这里测试公共 API
      expect(renderer).toBeDefined();
    });

    it('should throw error when renderFrame called before initialize', 0, async () => {
      const buffer = new ArrayBuffer(100);
      try {
        await renderer.renderFrame(buffer, 10, 10);
        // 如果没有抛出错误，测试失败
        expect(false).assertTrue();
      } catch (error) {
        // 预期会抛出错误
        expect(true).assertTrue();
      }
    });

    afterAll(() => {
      console.info('AfterAll: Cleanup renderer');
      if (renderer) {
        renderer.dispose();
      }
    });
  });
}
```

### 3.4 编写 UI 自动化测试

#### 测试用例 3：页面跳转与渲染测试

文件：`entry/src/ohosTest/ets/uitest/pages/SurfaceDemoPage.test.ets`

```typescript
import { describe, it, expect, beforeEach, afterEach } from '@ohos/hypium';
import { UiDriver, By, Key } from '@ohos/uiAutomator';

export default function SurfaceDemoPageUITest() {
  let driver: UiDriver;

  describe('SurfaceDemoPage UI Automation Tests', () => {
    beforeEach(async () => {
      // 启动应用
      driver = await UiDriver.create();
      await driver.launchApp('com.samples.ndkxcomponent');
      // 等待应用启动
      await driver.sleep(2000);
    });

    it('should display home page with title', 0, async () => {
      // 验证标题存在
      const title = await driver.findElement(By.text('ArkZeroRenderer - Ultra Low Latency Demo'));
      expect(title).not.toBeNull();
    });

    it('should display jump button', 0, async () => {
      // 验证按钮存在
      const button = await driver.findElement(By.text('启动 Surface 渲染测试'));
      expect(button).not.toBeNull();
    });

    it('should navigate to SurfaceDemoPage on button click', 0, async () => {
      // 点击按钮
      const button = await driver.findElement(By.text('启动 Surface 渲染测试'));
      await button.click();
      
      // 等待页面跳转
      await driver.sleep(1000);
      
      // 验证 XComponent 已加载（通过检查状态文本）
      const statusText = await driver.findElement(By.textContains('Status:'));
      expect(statusText).not.toBeNull();
    });

    it('should show frame counter increasing', 0, async () => {
      // 先跳转到渲染页面
      const button = await driver.findElement(By.text('启动 Surface 渲染测试'));
      await button.click();
      await driver.sleep(2000);
      
      // 获取初始帧数
      const frames1 = await driver.findElement(By.textContains('Frames:'));
      const text1 = await frames1.getText();
      const count1 = parseInt(text1.replace('Frames: ', ''));
      
      // 等待 2 秒
      await driver.sleep(2000);
      
      // 获取更新后的帧数
      const frames2 = await driver.findElement(By.textContains('Frames:'));
      const text2 = await frames2.getText();
      const count2 = parseInt(text2.replace('Frames: ', ''));
      
      // 验证帧数增加
      expect(count2).toBeGreaterThan(count1);
    });

    afterEach(async () => {
      // 清理资源
      if (driver) {
        await driver.dispose();
      }
    });
  });
}
```

### 3.5 运行自动化测试

#### 方法 1：通过 DevEco Studio

```
1. 选择测试配置：Run → Edit Configurations
2. 添加新配置：HarmonyOS Test
3. 选择 Module：entry
4. 选择测试类型：Unit Test / UI Test
5. 点击 Run 按钮执行测试
```

#### 方法 2：通过命令行

```bash
# 运行所有单元测试
hvigorw test --mode module --product default

# 运行特定测试类
hvigorw test --mode module --product default --class ArkZeroRendererTest

# 运行 UI 自动化测试
hvigorw uiTest --mode module --product default
```

#### 方法 3：通过 hdc 命令（真机/模拟器）

```bash
# 连接设备
hdc list targets

# 安装测试包
hdc install entry/build/default/outputs/default/entry-default-signed.hap

# 运行测试
hdc shell aa start -a TestAbility -b com.samples.ndkxcomponent

# 查看测试结果
hdc shell hilog | grep -E "Hypium|PASS|FAIL"
```

### 3.6 测试报告生成

#### 查看测试结果

测试完成后，结果会保存在：

```
entry/build/default/test/reports/
├── index.html          # HTML 格式报告
├── result.json         # JSON 格式结果
└── coverage/           # 代码覆盖率报告
```

#### 测试输出示例

```
========== Test Results ==========
Total: 15 tests
Passed: 14
Failed: 1
Skipped: 0

✅ PixelFormat Enum Tests (6/6 passed)
✅ ArkZeroRenderer Unit Tests (3/3 passed)
⚠️  SurfaceDemoPage UI Tests (5/6 passed)
   ❌ should show frame counter increasing - Timeout

Report saved to: entry/build/default/test/reports/index.html
```

### 3.7 CI/CD 集成

#### GitHub Actions 配置示例

创建 `.github/workflows/test.yml`：

```yaml
name: ArkZeroRenderer Tests

on:
  push:
    branches: [ master ]
  pull_request:
    branches: [ master ]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup HarmonyOS SDK
      uses: actions/setup-java@v3
      with:
        java-version: '17'
    
    - name: Install Dependencies
      run: |
        cd entry
        ohpm install
    
    - name: Build Project
      run: hvigorw assembleHap
    
    - name: Run Unit Tests
      run: hvigorw test --mode module --product default
    
    - name: Upload Test Report
      uses: actions/upload-artifact@v3
      with:
        name: test-report
        path: entry/build/default/test/reports/
```

---

## 4. 手动功能测试

### 4.1 基础功能测试流程

#### 测试步骤：

1. **部署应用**
   ```
   - 连接 HarmonyOS 设备或启动模拟器
   - 点击 DevEco Studio 的 Run 按钮
   - 等待应用安装并启动
   ```

2. **首页验证**
   ```
   预期界面：
   - 标题："ArkZeroRenderer - Ultra Low Latency Demo"
   - 副标题："Ready for XComponent Surface Integration"
   - 蓝色按钮："启动 Surface 渲染测试"
   ```

3. **跳转测试**
   ```
   - 点击"启动 Surface 渲染测试"按钮
   - 预期：成功跳转到 SurfaceDemoPage
   - 不应出现崩溃或白屏
   ```

4. **渲染初始化测试**
   ```
   观察 HiLog 日志，应看到：
   ✅ XComponent loaded, surfaceId: xxx
   ✅ Renderer initialized
   ✅ Rendering started at 60fps
   ```

5. **渲染效果验证**
   ```
   预期视觉效果：
   - 全屏动态渐变动画（红绿蓝三色正弦波）
   - 动画流畅，无明显卡顿
   - 左上角显示状态信息：
     * Status: ✅ Running
     * Frames: [持续增长的数字]
   ```

### 4.2 核心 API 测试

#### 测试用例 1：渲染器初始化

```typescript
// 测试代码位置：SurfaceDemoPage.ets aboutToAppear()
const renderer = new ArkZeroRenderer({
  width: 1920,
  height: 1080,
  format: PixelFormat.RGBA
});

await renderer.initialize(surfaceId);

// 验证点：
// ✅ nativeHandle > 0
// ✅ isInitialized === true
// ✅ 无异常抛出
```

#### 测试用例 2：帧渲染

```typescript
// 测试代码位置：SurfaceDemoPage.ets startRendering()
const pixelBuffer = new ArrayBuffer(1920 * 1080 * 4);
fillPixelData(pixelBuffer);

await renderer.renderFrame(pixelBuffer, 1920, 1080);

// 验证点：
// ✅ Promise resolve（无 reject）
// ✅ onFrameRenderedCallback 被调用
// ✅ frameCount 增加
```

#### 测试用例 3：尺寸调整

```typescript
// 手动测试：旋转设备或修改窗口大小
await renderer.resize(newWidth, newHeight);

// 验证点：
// ✅ 画面自适应新尺寸
// ✅ 无拉伸或变形
// ✅ 性能无明显下降
```

#### 测试用例 4：资源清理

```typescript
// 测试代码位置：SurfaceDemoPage.ets aboutToDisappear()
renderer.dispose();

// 验证点：
// ✅ nativeHandle === 0
// ✅ isInitialized === false
// ✅ 无内存泄漏（通过 Profiler 验证）
```

### 4.3 多格式支持测试

修改 `SurfaceDemoPage.ets` 中的 `PixelFormat` 进行测试：

| 格式 | 配置 | 预期结果 |
|------|------|---------|
| **RGBA** | `format: PixelFormat.RGBA` | ✅ 正常显示彩色动画 |
| **BGRA** | `format: PixelFormat.BGRA` | ✅ 正常显示（颜色可能偏移） |
| **RGB** | `format: PixelFormat.RGB` | ⚠️ 需要调整缓冲区大小（3字节/像素） |
| **NV21** | `format: PixelFormat.NV21` | ⚠️ 需要 YUV 数据源 |
| **NV12** | `format: PixelFormat.NV12` | ⚠️ 需要 YUV 数据源 |

---

## 5. 性能测试

### 5.1 帧率测试

#### 测试方法：

1. **目测法**
   ```
   - 观察 Frames 计数器增长速度
   - 60fps ≈ 每秒增加 60 帧
   - 记录 10 秒内的帧数增长
   ```

2. **HiLog 分析法**
   ```bash
   # 过滤渲染日志
   hilog | grep "Frame rendered"
   
   # 计算帧间隔时间
   # 理想值：~16ms (60fps)
   # 可接受：16-20ms (50-60fps)
   # 需优化：>20ms (<50fps)
   ```

3. **Profiler 精确测量**
   ```
   - 打开 DevEco Studio → Profiler → Performance
   - 选择 "Frame Rate" 监控项
   - 运行应用 30 秒
   - 记录平均帧率和帧时间分布
   ```

#### 性能指标标准：

| 指标 | 优秀 | 良好 | 需优化 |
|------|------|------|--------|
| **帧率 (FPS)** | ≥ 58 | 50-57 | < 50 |
| **帧时间 (ms)** | ≤ 17 | 17-20 | > 20 |
| **丢帧率 (%)** | < 1% | 1-5% | > 5% |

### 5.2 CPU 占用测试

#### 测试步骤：

```
1. 打开 DevEco Studio → Profiler → CPU
2. 启动应用并开始渲染
3. 运行 60 秒
4. 记录 CPU 占用曲线
```

#### 预期结果：

| 场景 | CPU 占用（单核） | 说明 |
|------|-----------------|------|
| **空闲状态** | < 1% | 未开始渲染 |
| **渲染中** | < 3% | 零拷贝优势体现 |
| **峰值** | < 5% | 初始化或 resize 时 |

### 5.3 内存占用测试

#### 测试步骤：

```
1. 打开 DevEco Studio → Profiler → Memory
2. 启动应用并开始渲染
3. 运行 5 分钟
4. 观察内存曲线趋势
```

#### 验证点：

- ✅ **稳定状态**：内存占用波动 < 10MB
- ✅ **无泄漏**：长时间运行后内存不持续增长
- ✅ **GC 正常**：定期垃圾回收，无内存尖峰

#### 内存预算：

| 组件 | 预期占用 |
|------|---------|
| **ArkTS Heap** | ~50MB |
| **Native Heap** | ~30MB |
| **Graphics Buffer** | ~8MB (1080p RGBA) |
| **总计** | < 100MB |

### 5.4 延迟测试

#### 测试方法：

由于无法直接测量端到端延迟，采用间接方法：

1. **帧时间测量**
   ```typescript
   // 在 renderFrame 前后添加时间戳
   const startTime = Date.now();
   await renderer.renderFrame(buffer, w, h);
   const endTime = Date.now();
   console.log(`Render latency: ${endTime - startTime}ms`);
   ```

2. **预期结果**：
   - 渲染延迟：< 5ms（GPU 上传时间）
   - 总延迟：< 10ms（含 VSync 等待）

---

## 6. 压力测试

### 6.1 长时间运行测试

#### 测试场景：

```
- 连续渲染 1 小时
- 分辨率：1920x1080 @ 60fps
- 记录关键指标变化
```

#### 监控指标：

| 时间点 | 帧率 | CPU% | 内存(MB) | 温度(°C) |
|--------|------|------|----------|----------|
| 0 min | 60 | 2.5 | 85 | 35 |
| 15 min | 60 | 2.6 | 86 | 37 |
| 30 min | 59 | 2.7 | 87 | 38 |
| 60 min | 59 | 2.8 | 88 | 39 |

#### 通过标准：

- ✅ 帧率下降 < 5%
- ✅ 内存增长 < 20MB
- ✅ 无崩溃或异常
- ✅ 设备温度 < 45°C

### 6.2 高分辨率测试

#### 测试配置：

```typescript
// 修改 SurfaceDemoPage.ets
const renderer = new ArkZeroRenderer({
  width: 3840,   // 4K 宽度
  height: 2160,  // 4K 高度
  format: PixelFormat.RGBA
});
```

#### 预期结果：

| 分辨率 | 目标帧率 | 可接受帧率 | 内存增量 |
|--------|---------|-----------|---------|
| 720p (1280x720) | 60fps | ≥ 58fps | +3MB |
| 1080p (1920x1080) | 60fps | ≥ 55fps | +8MB |
| 4K (3840x2160) | 30fps | ≥ 25fps | +32MB |

### 6.3 快速切换测试

#### 测试步骤：

```
1. 快速进入/退出渲染页面 50 次
2. 每次停留 2-3 秒
3. 观察是否有资源泄漏或崩溃
```

#### 验证点：

- ✅ 无崩溃
- ✅ 内存稳定（无累积增长）
- ✅ 每次初始化成功
- ✅ 句柄正确释放

---

## 7. 兼容性测试

### 7.1 设备兼容性

| 设备类型 | 测试重点 | 预期结果 |
|---------|---------|---------|
| **旗舰机** (Mate 60 Pro) | 高性能模式 | ✅ 60fps 稳定 |
| **中端机** (Nova 11) | 平衡模式 | ✅ 55-60fps |
| **入门机** (畅享系列) | 节能模式 | ⚠️ 可能降至 30-45fps |
| **平板** (MatePad Pro) | 大屏适配 | ✅ 正常渲染，需测试 rotate |
| **模拟器** | 基础功能 | ✅ 功能正常，性能较低 |

### 7.2 系统版本兼容性

| HarmonyOS 版本 | API Level | 支持状态 |
|---------------|-----------|---------|
| 5.0 | API 12 | ✅ 完全支持 |
| 5.1 | API 13 | ✅ 完全支持 |
| 6.0 | API 14+ | ⚠️ 需测试验证 |

### 7.3 GPU 后端兼容性

#### 测试方法：

修改 `GLESBackend.cpp` 中的 OpenGL ES 版本要求：

```cpp
// 测试不同 GLES 版本
glGetString(GL_VERSION);  // 查询实际版本
```

| OpenGL ES 版本 | 支持状态 | 备注 |
|---------------|---------|------|
| 3.0 | ✅ 最低要求 | 基础功能正常 |
| 3.1 | ✅ 推荐 | 支持更多特性 |
| 3.2 | ✅ 最佳 | 完整功能集 |

---

## 8. 日志监控

### 8.1 HiLog 标签说明

| 标签 | 级别 | 用途 |
|------|------|------|
| `ArkZeroRenderer` | INFO/DEBUG/ERROR | 渲染器生命周期、错误信息 |
| `SurfaceDemoPage` | INFO/DEBUG | 页面状态、帧计数 |
| `Init` | INFO/ERROR | NAPI 模块初始化 |

### 8.2 关键日志示例

#### 成功初始化：

```
05-12 19:51:44.100 24720 24720 I ArkZeroRenderer: ✅ ArkZeroRenderer initialized: 1920x1080
05-12 19:51:44.150 24720 24720 I SurfaceDemoPage: ✅ XComponent loaded, surfaceId: 0x7f8a9b0c
05-12 19:51:44.200 24720 24720 I SurfaceDemoPage: ✅ Rendering started at 60fps
```

#### 渲染循环：

```
05-12 19:51:44.220 24720 24720 D ArkZeroRenderer: 🎨 Frame rendered: 1920x1080
05-12 19:51:44.237 24720 24720 D SurfaceDemoPage: 🎨 Frame rendered: 1
05-12 19:51:44.254 24720 24720 D SurfaceDemoPage: 🎨 Frame rendered: 2
...
```

#### 错误日志：

```
05-12 19:51:44.100 24720 24720 E ArkZeroRenderer: ❌ ArkZeroRenderer initialization failed: Surface creation error
05-12 19:51:44.150 24720 24720 E ArkZeroRenderer: ❌ renderFrame failed: Invalid native handle
```

### 8.3 日志过滤命令

```bash
# 查看所有 ArkZero 相关日志
hilog | grep -E "ArkZeroRenderer|SurfaceDemoPage"

# 仅查看错误日志
hilog | grep "❌"

# 实时监控帧率
hilog | grep "Frame rendered" | awk '{print $1, $2, $3}'

# 导出日志到文件
hilog > arkzero_test.log
```

---

## 9. 常见问题排查

### 9.1 编译阶段问题

#### Q1: `Cannot find module 'libnativerender.so'`

**原因**：导入路径不正确

**解决**：
```typescript
// ❌ 错误
import nativerender from 'nativerender';

// ✅ 正确
import nativerender from 'libnativerender.so';
```

#### Q2: `'PixelFormat' cannot be used as a value`

**原因**：枚举导出方式不符合 ArkTS 规范

**解决**：
```typescript
// ✅ 使用标准 enum 语法
export enum PixelFormat {
  RGBA = 0,
  RGB = 1,
  BGRA = 2,
  NV21 = 3,
  NV12 = 4
}
```

#### Q3: `napi_define_properties failed`

**原因**：NAPI 模块注册失败

**检查清单**：
- [ ] `napi_init.cpp` 中 `.nm_modname = "nativerender"`
- [ ] `types/libnativerender/oh-package.json5` 中 `"name": "libnativerender.so"`
- [ ] CMakeLists.txt 中 `add_library(nativerender SHARED ...)`

### 9.2 运行时问题

#### Q4: 应用启动后白屏或崩溃

**排查步骤**：

1. 检查 HiLog 错误信息
2. 确认 surfaceId 获取成功
3. 验证 Native 库加载：
   ```bash
   adb shell ls /data/app/el1/bundle/public/com.samples.ndkxcomponent/libs/arm64-v8a/
   # 应看到 libnativerender.so
   ```

#### Q5: 渲染画面黑屏

**可能原因**：

1. **Surface 未正确绑定**
   ```typescript
   // 确认 XComponent type 为 SURFACE
   XComponent({
     type: XComponentType.SURFACE,  // ✅
     // type: XComponentType.TEXTURE, // ❌
   })
   ```

2. **像素数据为空**
   ```typescript
   // 检查 buffer 是否正确填充
   console.log('Buffer size:', pixelBuffer.byteLength);
   // 应为: 1920 * 1080 * 4 = 8294400
   ```

3. **OpenGL 上下文丢失**
   ```
   查看日志：EGL context error
   解决：重新初始化渲染器
   ```

#### Q6: 帧率低或卡顿

**优化建议**：

1. **降低分辨率**
   ```typescript
   // 从 4K 降到 1080p
   width: 1920, height: 1080
   ```

2. **检查 CPU 占用**
   ```
   - 如果 CPU > 10%，检查是否有额外计算
   - 确保像素数据生成在后台线程
   ```

3. **启用 GPU 调试**
   ```bash
   # 查看 GPU 使用情况
   adb shell dumpsys SurfaceFlinger --latency-clear
   ```

### 9.3 性能问题

#### Q7: 内存持续增长

**排查方法**：

1. 使用 Profiler 拍摄堆快照
2. 对比不同时间点的内存分布
3. 检查是否有未释放的 ArrayBuffer

**常见泄漏点**：

```typescript
// ❌ 错误：每次渲染创建新 buffer
async renderFrame() {
  const buffer = new ArrayBuffer(size);  // 泄漏！
  await renderer.renderFrame(buffer, w, h);
}

// ✅ 正确：复用 buffer
private pixelBuffer: ArrayBuffer;

async renderFrame() {
  if (!this.pixelBuffer) {
    this.pixelBuffer = new ArrayBuffer(size);
  }
  await renderer.renderFrame(this.pixelBuffer, w, h);
}
```

#### Q8: 设备过热

**原因分析**：

- GPU 持续高负载
- CPU 频繁唤醒
- 屏幕亮度过高

**缓解措施**：

1. 降低帧率目标（60fps → 30fps）
2. 降低分辨率
3. 实现帧跳过逻辑：
   ```typescript
   if (Date.now() - lastRenderTime < 33) {
     return;  // 跳过这一帧（目标 30fps）
   }
   ```

---

## 📊 测试报告模板

### 测试执行记录

| 测试项 | 测试结果 | 备注 |
|--------|---------|------|
| 编译构建 | ✅ / ❌ | |
| 应用启动 | ✅ / ❌ | |
| 页面跳转 | ✅ / ❌ | |
| 渲染初始化 | ✅ / ❌ | |
| 帧渲染 | ✅ / ❌ | |
| 资源清理 | ✅ / ❌ | |
| 帧率 (FPS) | ___ | 目标: ≥ 55 |
| CPU 占用 (%) | ___ | 目标: < 3 |
| 内存占用 (MB) | ___ | 目标: < 100 |
| 长时间运行 | ✅ / ❌ | 时长: ___ min |
| 高分辨率 | ✅ / ❌ | 分辨率: ___ |

### 问题汇总

| 序号 | 问题描述 | 严重程度 | 状态 |
|------|---------|---------|------|
| 1 | | 高/中/低 | 待修复/已修复 |
| 2 | | 高/中/低 | 待修复/已修复 |

---

## 🔗 相关文档

- [README.md](./README.md) - 项目概述
- [ARCHITECTURE.md](./ARCHITECTURE.md) - 架构设计
- [API_REFERENCE.md](./API_REFERENCE.md) - API 参考
- [HIGH_PERFORMANCE_ROADMAP.md](./HIGH_PERFORMANCE_ROADMAP.md) - 性能优化路线
- [Hypium 官方文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/using-hypium) - Hypium 测试框架指南
- [arkxtest 官方文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/ui-testing-overview) - UI 自动化测试指南

---

## 📝 更新日志

| 版本 | 日期 | 更新内容 |
|------|------|---------|
| 1.1.0 | 2026-05-12 | 添加 Hypium + arkxtest 自动化集成测试方案 |
| 1.0.0 | 2026-05-12 | 初始版本，包含完整测试方案 |

---

**最后更新**: 2026-05-12  
**维护者**: ArkZeroRenderer Team
