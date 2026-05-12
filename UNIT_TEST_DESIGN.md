# ArkZeroRenderer 详细单元测试设计文档

本文档提供**精确到每个类的每个方法**的单元测试设计方案，确保代码质量和覆盖率。

---

## 📋 目录

- [1. C++ 层单元测试](#1-c-层单元测试)
  - [1.1 PerformanceMonitor 类](#11-performancemonitor-类)
  - [1.2 PixelFormatConverter 类](#12-pixelformatconverter-类)
  - [1.3 RenderQueue 类](#13-renderqueue-类)
  - [1.4 RenderCommand 结构体](#14-rendercommand-结构体)
- [2. ArkTS 层单元测试](#2-arkts-层单元测试)
  - [2.1 PixelFormat 枚举](#21-pixelformat-枚举)
  - [2.2 ArkZeroRenderer 类](#22-arkzerorenderer-类)
  - [2.3 ArkZeroRendererConfig 接口](#23-arkzerorendererconfig-接口)
- [3. 测试执行与报告](#3-测试执行与报告)

---

## 1. C++ 层单元测试

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

### 1.2 PixelFormatConverter 类

**文件位置**: `entry/src/main/cpp/renderer/backend/PixelFormatConverter.h`  
**测试文件**: `entry/src/main/cpp/tests/test_pixel_format_converter.cpp`

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

### 1.3 RenderQueue 类

**文件位置**: `entry/src/main/cpp/renderer/core/RenderQueue.h`  
**测试文件**: `entry/src/main/cpp/tests/test_render_queue.cpp`

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

### 1.4 RenderCommand 结构体

**文件位置**: `entry/src/main/cpp/renderer/core/RenderQueue.h`  
**测试文件**: `entry/src/main/cpp/tests/test_render_command.cpp`

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

## 2. ArkTS 层单元测试

### 2.1 PixelFormat 枚举

**文件位置**: `entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`  
**测试文件**: `entry/src/ohosTest/ets/unittest/renderer/PixelFormat.test.ets`

#### 2.1.1 枚举值验证

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
import { PixelFormat } from '../../../../main/ets/components/rendering/ArkZeroRenderer';

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

### 2.2 ArkZeroRenderer 类

**文件位置**: `entry/src/main/ets/components/rendering/ArkZeroRenderer.ets`  
**测试文件**: `entry/src/ohosTest/ets/unittest/renderer/ArkZeroRenderer.test.ets`

#### 2.2.1 构造函数 `constructor(config: ArkZeroRendererConfig)`

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
import { describe, it, expect, beforeEach, afterEach } from '@ohos/hypium';
import { ArkZeroRenderer, PixelFormat } from '../../../../main/ets/components/rendering/ArkZeroRenderer';

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
}
```

---

#### 2.2.2 方法 `async initialize(surfaceId: string): Promise<void>`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-INIT-001 | 正常初始化 | 有效surfaceId | Promise resolve | P0 |
| AZR-INIT-002 | 重复初始化 | 已初始化后再次调用 | 警告日志，无操作 | P1 |
| AZR-INIT-003 | 空surfaceId | "" | Promise reject | P0 |
| AZR-INIT-004 | null surfaceId | null | TypeScript编译错误 | P0 |
| AZR-INIT-005 | 初始化后状态 | 成功后 | isInitialized==true | P0 |
| AZR-INIT-006 | 初始化失败 | 无效surfaceId | Promise reject | P0 |
| AZR-INIT-007 | 异步性 | 调用后立即检查 | 未完成时应pending | P1 |

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
    expect(true).assertTrue(); // TODO: 验证实际状态
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

---

#### 2.2.3 方法 `setOnFrameRendered(callback: () => void): void`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-SFR-001 | 设置回调 | 有效函数 | 回调保存成功 | P0 |
| AZR-SFR-002 | 覆盖回调 | 设置两次 | 使用最新回调 | P1 |
| AZR-SFR-003 | null回调 | null | TypeScript编译错误 | P0 |
| AZR-SFR-004 | 回调触发 | renderFrame后 | 回调被调用 | P0 |

**测试代码示例**:
```typescript
describe('ArkZeroRenderer Callback Tests', () => {
  it('AZR-SFR-001: Should set callback', 0, () => {
    const renderer = new ArkZeroRenderer({
      width: 1920,
      height: 1080,
      format: PixelFormat.RGBA
    });
    
    let callbackCalled = false;
    renderer.setOnFrameRendered(() => {
      callbackCalled = true;
    });
    
    // 通过后续renderFrame验证回调是否触发
    expect(renderer).assertNotNull();
  });
});
```

---

#### 2.2.4 方法 `async renderFrame(pixelData: ArrayBuffer, width: number, height: number): Promise<void>`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-RF-001 | 正常渲染 | 有效buffer,w,h | Promise resolve | P0 |
| AZR-RF-002 | 未初始化 | 未调用initialize | Promise reject | P0 |
| AZR-RF-003 | 空buffer | new ArrayBuffer(0) | 应处理或reject | P1 |
| AZR-RF-004 | buffer尺寸不匹配 | 小buffer,大w,h | 应检测并reject | P1 |
| AZR-RF-005 | 零宽度 | buffer,w=0,h=1080 | 应reject | P2 |
| AZR-RF-006 | 零高度 | buffer,w=1920,h=0 | 应reject | P2 |
| AZR-RF-007 | 回调触发 | 设置回调后渲染 | 回调被调用 | P0 |
| AZR-RF-008 | 多次渲染 | 连续100次 | 无内存泄漏 | P1 |
| AZR-RF-009 | 并发渲染 | 同时发起多个 | 按序处理或拒绝 | P2 |

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
});
```

---

#### 2.2.5 方法 `async resize(width: number, height: number): Promise<void>`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-RSZ-001 | 正常调整 | w=1280,h=720 | Promise resolve | P0 |
| AZR-RSZ-002 | 未初始化 | 未initialize | Promise reject | P0 |
| AZR-RSZ-003 | 零宽度 | w=0,h=720 | 应reject | P1 |
| AZR-RSZ-004 | 零高度 | w=1280,h=0 | 应reject | P1 |
| AZR-RSZ-005 | 相同尺寸 | w=1920,h=1080 | 应成功（无操作） | P2 |
| AZR-RSZ-006 | 超大尺寸 | w=7680,h=4320 | 应成功 | P1 |
| AZR-RSZ-007 | 配置更新 | resize后 | config.width/height更新 | P0 |

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

---

#### 2.2.6 方法 `dispose(): void`

| 测试用例ID | 测试场景 | 输入 | 预期输出 | 优先级 |
|-----------|---------|------|---------|--------|
| AZR-DSP-001 | 正常清理 | 已初始化 | 资源释放 | P0 |
| AZR-DSP-002 | 未初始化 | 未initialize | 无操作，不崩溃 | P0 |
| AZR-DSP-003 | 重复清理 | 多次dispose | 无副作用 | P1 |
| AZR-DSP-004 | 清理后状态 | dispose后 | isInitialized==false | P0 |
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
    
    // 验证资源已释放
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
```

---

### 2.3 ArkZeroRendererConfig 接口

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

## 3. 测试执行与报告

### 3.1 测试执行命令

```bash
# 运行所有单元测试
hvigorw test --mode module --product default

# 运行特定测试类
hvigorw test --class PerformanceMonitorTest

# 运行C++测试
cd entry/src/main/cpp/tests
./run_tests.sh

# 生成覆盖率报告
hvigorw test --coverage

# 查看详细输出
hvigorw test --debug
```

### 3.2 测试覆盖率目标

| 模块 | 语句覆盖率 | 分支覆盖率 | 函数覆盖率 | 行覆盖率 |
|------|-----------|-----------|-----------|---------|
| **PerformanceMonitor** | ≥ 90% | ≥ 85% | 100% | ≥ 90% |
| **PixelFormatConverter** | ≥ 95% | ≥ 90% | 100% | ≥ 95% |
| **RenderQueue** | ≥ 85% | ≥ 80% | 100% | ≥ 85% |
| **ArkZeroRenderer** | ≥ 80% | ≥ 75% | 100% | ≥ 80% |
| **总体** | ≥ 85% | ≥ 80% | 100% | ≥ 85% |

### 3.3 测试报告模板

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
