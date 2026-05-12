# C++ 层测试

本目录包含 ArkZero 渲染引擎的 C++ 层单元测试和性能测试。

## 测试文件

### test_performance.cpp
**功能**: 性能监控和核心组件测试

**测试内容**:
1. **PerformanceMonitor** - FPS、FrameTime、DropRate 统计验证
2. **PixelFormatConverter** - 像素格式转换逻辑测试
3. **YUV 格式检测** - NV21/NV12 与 RGBA 区分验证

**特点**:
- ✅ 纯 C++ 实现，不依赖 HarmonyOS NDK 库
- ✅ 可在标准 Linux 环境下编译运行
- ✅ 模拟 100 帧渲染场景
- ✅ 包含断言验证确保逻辑正确性

## 编译和运行

### 在 HarmonyOS 环境中
```bash
cd build
cmake ..
make test_performance
./test_performance
```

### 在标准 Linux 环境中
由于测试代码不依赖 HarmonyOS NDK 特定库，可以直接编译：
```bash
g++ -std=c++17 -I.. \
    test_performance.cpp \
    ../renderer/core/PerformanceMonitor.cpp \
    ../renderer/backend/PixelFormatConverter.cpp \
    -o test_performance

./test_performance
```

## 预期输出

```
========================================
  ArkZero C++ Layer Performance Tests
========================================

========== Test 1: PerformanceMonitor ==========
📊 Performance Stats:
   FPS: 58.5
   FrameTime: 16.2 ms
   DropRate: 10.0%
   TotalFrames: 100
   DroppedFrames: 10
   JSON: {"fps":58.5,"frameTime":16.2,...}
✅ TotalFrames 正确
✅ DroppedFrames 正确

========== Test 2: PixelFormatConverter ==========
RGBA Format:
   InternalFormat: 6408
   GLFormat: 6408
   BytesPerPixel: 4
✅ RGBA BPP 正确
...

========================================
  ✅ All Tests Completed!
========================================
```

## 注意事项

⚠️ **限制**: 
- 完整渲染测试需要真实的 NativeWindow（HarmonyOS XComponent Surface）
- OpenGL 相关测试需要 EGL/GLES 上下文
- 当前测试仅验证纯 C++ 逻辑部分

🔧 **未来扩展**:
- 添加 Google Test 框架支持
- 集成到 CI/CD 流程
- 添加更多边界条件测试
