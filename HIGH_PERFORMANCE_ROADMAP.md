# ArkZeroRenderer 高性能优化路线图

## 🎯 愿景
将 ArkZeroRenderer 打造为 HarmonyOS 平台上**最高性能的零拷贝渲染组件**

**核心指标：**
- ✅ 4K (3840x2160) @ 60fps 稳定渲染
- ✅ 端到端延迟 < 16ms（一帧时间）
- ✅ 零内存拷贝（ArkTS → Native → GPU）
- ✅ GPU 显存占用 < 100MB（4K RGBA）
- ✅ CPU 占用 < 5%（单核）

---

## 🔴 Phase 1: 核心性能优化（必须实现）

### 1.1 帧同步与 VSync 支持
**优先级：** 🔴 P0  
**预计工作量：** 2-3 天  
**影响范围：** GLESBackend.cpp

#### 问题分析
当前实现每次 `renderFrame` 都立即调用 `eglSwapBuffers`，导致：
- ❌ **画面撕裂**：GPU 渲染速度与屏幕刷新不同步
- ❌ **功耗过高**：无限制渲染，CPU/GPU 持续满载
- ❌ **帧率不稳定**：可能在 30-120fps 之间波动

#### 实施方案

**步骤 1：启用 VSync**
```cpp
// renderer/backend/GLESBackend.cpp
bool GLESBackend::InitEGLContext() {
    // ... 现有 EGL 初始化代码 ...
    
    // ✅ 启用 VSync（匹配屏幕刷新率）
    if (!eglSwapInterval(m_eglDisplay, 1)) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Failed to set swap interval, using default");
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "GLESBackend", "✅ VSync enabled (swap interval = 1)");
    
    return true;
}
```

**步骤 2：添加帧时间监控**
```cpp
// renderer/backend/GLESBackend.h
#include <chrono>

class GLESBackend : public IRenderBackend {
private:
    std::chrono::steady_clock::time_point m_lastFrameTime;
    double m_avgFrameTimeMs = 0.0;
    
    void recordFrameTime();
};

// renderer/backend/GLESBackend.cpp
void GLESBackend::recordFrameTime() {
    auto now = std::chrono::steady_clock::now();
    if (m_lastFrameTime.time_since_epoch().count() != 0) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            now - m_lastFrameTime);
        
        // 指数移动平均（EMA）
        double frameTimeMs = duration.count() / 1000.0;
        m_avgFrameTimeMs = 0.9 * m_avgFrameTimeMs + 0.1 * frameTimeMs;
        
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
            "GLESBackend", "Frame time: %{public}.2f ms (avg: %{public}.2f ms)", 
            frameTimeMs, m_avgFrameTimeMs);
    }
    m_lastFrameTime = now;
}

bool GLESBackend::RenderFrame(...) {
    // ... 渲染代码 ...
    
    eglSwapBuffers(m_eglDisplay, m_eglSurface);
    
    recordFrameTime();  // ✅ 记录帧时间
    
    return true;
}
```

#### 验收标准
- ✅ 无画面撕裂（肉眼观察）
- ✅ 稳定 60fps（或设备最大刷新率）
- ✅ 帧时间波动 < ±2ms
- ✅ 功耗降低 20%+（使用 DevEco Profiler 验证）

---

### 1.2 异步渲染架构
**优先级：** 🔴 P0  
**预计工作量：** 4-5 天  
**影响范围：** Renderer.h/cpp, RendererApi.cpp

#### 问题分析
当前是**同步渲染**，ArkTS 主线程阻塞等待 Native 渲染完成，导致：
- ❌ UI 线程阻塞
- ❌ 无法并行处理
- ❌ 延迟增加

#### 实施方案

**核心设计：**
1. 创建线程安全的渲染命令队列（RenderQueue）
2. 启动后台渲染线程
3. ArkTS 层 submitFrame 立即返回（< 1ms）
4. 后台线程从队列取出命令并执行渲染

**关键文件：**
- `renderer/core/RenderCommand.h` - 渲染命令结构
- `renderer/core/RenderQueue.h/cpp` - 线程安全队列
- 更新 `Renderer.h/cpp` - 集成异步渲染
- 更新 `RendererApi.cpp` - NAPI 桥接层适配

#### 验收标准
- ✅ `renderFrame()` 返回时间 < 1ms（原 5-15ms）
- ✅ UI 线程完全不阻塞
- ✅ 支持 3 帧缓冲，自动丢弃过时帧
- ✅ 后台渲染线程稳定运行

---

### 1.3 纹理池与预分配
**优先级：** 🔴 P0  
**预计工作量：** 2-3 天  
**影响范围：** 新增 TexturePool.h/cpp

#### 问题分析
当前 `Resize` 操作销毁并重新创建纹理，导致：
- ❌ GPU 显存碎片化
- ❌ Resize 耗时长（4K ~50ms）
- ❌ 无法复用相同尺寸的纹理

#### 实施方案

**核心设计：**
1. 实现 TexturePool 类，管理纹理生命周期
2. 预分配常用分辨率（1080p, 4K, 720p）
3. Resize 时从池中获取/归还纹理，而非销毁重建

**关键文件：**
- `renderer/backend/TexturePool.h/cpp` - 纹理池实现
- 更新 `GLESBackend.h/cpp` - 集成纹理池

#### 验收标准
- ✅ Resize 耗时从 ~50ms 降至 < 1ms
- ✅ 内存碎片减少 50%+
- ✅ 预分配命中率达到 80%+

---

### 1.4 多格式直接渲染支持
**优先级：** 🔴 P0  
**预计工作量：** 1-2 天  
**影响范围：** GLESBackend.cpp

#### 问题分析
当前实现只支持 RGBA 格式，如果输入是 BGRA/RGB/YUV，需要：
- ❌ **CPU 格式转换**：浪费 CPU 周期，增加延迟
- ❌ **内存拷贝**：额外的缓冲区分配
- ❌ **性能损失**：4K 图像转换耗时 ~5-15ms

#### 核心理念
**不转换格式，而是根据输入格式选择对应的 OpenGL API 直接渲染！**

#### 实施方案

**步骤 1：支持 RGBA/BGRA/RGB 直接上传**
```cpp
// renderer/backend/GLESBackend.cpp
GLenum GLESBackend::GetGLFormat(PixelFormat format) const {
    switch (format) {
        case PixelFormat::RGBA:
            return GL_RGBA;           // ✅ 直接匹配
        case PixelFormat::BGRA:
#ifdef GL_BGRA_EXT
            return GL_BGRA_EXT;       // ✅ 直接匹配（扩展支持）
#else
            return GL_RGBA;           // 降级：需调用方预转换
#endif
        case PixelFormat::RGB:
            return GL_RGB;            // ✅ 直接匹配
        default:
            return GL_RGBA;
    }
}

bool GLESBackend::RenderFrame(...) {
    // ... 现有代码 ...
    
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    
    // ✅ 根据格式选择正确的 OpenGL 格式，零转换！
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GetGLFormat(m_format), GL_UNSIGNED_BYTE, pixelData);
    
    // ... 其余代码 ...
}
```

**步骤 2（可选）：支持 YUV 原生渲染**

对于相机/视频场景，支持 YUV 格式直接渲染（非转换，是 GPU 实时合成）：

```cpp
// 新增方法：YUV 直接渲染
bool GLESBackend::RenderYUVFrame(const void* yData, const void* uvData,
                                  int32_t width, int32_t height,
                                  PixelFormat yuvFormat);

// 实现原理：
// 1. Y 数据上传到 Texture Y (GL_LUMINANCE)
// 2. UV 数据上传到 Texture UV (GL_LUMINANCE_ALPHA)
// 3. GPU Shader 实时采样并合成（每帧在 GPU 完成）
// 4. CPU 零开销，无格式转换
```

#### 验收标准
- ✅ RGBA/BGRA/RGB 格式零转换直接渲染
- ✅ 4K @ 60fps 稳定运行
- ✅ BGRA 在不支持的平台上给出明确警告
- ✅ （可选）YUV 格式 GPU 实时合成，CPU 零开销

---

## 🟡 Phase 2: 可观测性与自适应（强烈建议）

### 2.1 性能监控系统
**优先级：** 🟡 P1  
**预计工作量：** 2-3 天

#### 实施方案
实现 PerformanceMonitor 类，统计：
- FPS（帧率）
- 平均帧耗时
- P95/P99 帧耗时
- 丢帧率
- GPU 显存占用

**ArkTS 层暴露接口：**
```typescript
interface RenderStats {
  fps: number;
  avgFrameTimeMs: number;
  p95FrameTimeMs: number;
  dropRate: number;
  gpuMemoryMB: number;
}

ArkZeroRenderer.getStats(): RenderStats;
```

---

### 2.2 自适应质量降级
**优先级：** 🟡 P1  
**预计工作量：** 2-3 天

#### 实施方案
根据性能监控数据，动态调整渲染质量：
- LOW: 540p @ 30fps
- MEDIUM: 720p @ 60fps
- HIGH: 1080p @ 60fps
- ULTRA: 4K @ 60fps

当检测到帧耗时超过阈值时，自动降级到下一档。

---

## 🟢 Phase 3: 高级特性（可选）

### 3.1 HDR 支持
**优先级：** 🟢 P2  
**预计工作量：** 5-7 天

- 10-bit/12-bit 色彩深度
- PQ/HLG HDR 曲线
- HDR10/Dolby Vision

### 3.2 广色域支持
**优先级：** 🟢 P2  
**预计工作量：** 3-4 天

- Display P3
- Adobe RGB

---

## 📊 实施时间表

| Phase | 任务 | 工作量 | 累计时间 |
|-------|------|--------|----------|
| **Phase 1** | 1.1 VSync 支持 | 2-3 天 | 2-3 天 |
| | 1.2 异步渲染 | 4-5 天 | 6-8 天 |
| | 1.3 纹理池 | 2-3 天 | 8-11 天 |
| | 1.4 多格式直接渲染 | 1-2 天 | 9-13 天 |
| **Phase 2** | 2.1 性能监控 | 2-3 天 | 11-16 天 |
| | 2.2 自适应质量 | 2-3 天 | 13-19 天 |
| **Phase 3** | 3.1 HDR 支持 | 5-7 天 | 18-26 天 |
| | 3.2 广色域 | 3-4 天 | 21-30 天 |

**总计：** 约 1-1.5 个月（全职开发）

---

## 🎯 成功指标

完成 Phase 1 后应达到：
- ✅ 4K @ 60fps 稳定运行
- ✅ 端到端延迟 < 16ms
- ✅ CPU 占用 < 5%
- ✅ GPU 显存 < 100MB
- ✅ 零内存拷贝

完成 Phase 2 后应达到：
- ✅ 实时性能监控可视化
- ✅ 自动适应不同性能设备
- ✅ 低端设备也能流畅运行

---

## 📝 注意事项

1. **测试覆盖**：每个功能都需要在多种设备上测试（高端、中端、低端）
2. **性能基准**：建立性能基准测试套件，确保每次优化都有量化收益
3. **向后兼容**：保持 API 兼容性，避免破坏现有用户代码
4. **文档更新**：每个新功能都要更新 API 文档和使用示例
