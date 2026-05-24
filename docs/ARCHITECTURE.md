# ScreenRenderer 模块架构说明

## 📐 目录结构

```
entry/src/main/cpp/
├── napi_init.cpp                    # NAPI模块注册入口（只负责导出接口）
├── screen_renderer/                 # ScreenRenderer独立模块 ⭐
│   ├── screen_renderer.h           # C++核心类声明（纯C++，无NAPI依赖）
│   ├── screen_renderer.cpp         # C++核心类实现（OpenGL/EGL逻辑）
│   └── screen_renderer_napi.cpp    # NAPI桥接层（只负责类型转换）
├── manager/                         # XComponent管理器（保持原有功能）
│   ├── plugin_manager.h
│   └── plugin_manager.cpp
├── render/                          # 原有的XComponent渲染逻辑
│   ├── plugin_render.h
│   ├── plugin_render.cpp
│   └── egl_core.*
├── common/                          # 公共头文件
│   └── common.h
└── types/                           # TypeScript类型定义
    └── native_screen_renderer.d.ts
```

## 🎯 设计原则

### 1. 职责分离（Separation of Concerns）

#### ❌ 之前的错误做法
```cpp
// plugin_manager.cpp - 混乱的实现
class PluginManager {
    // XComponent管理逻辑
    void SetNativeXComponent(...);
    
    // ScreenRenderer的NAPI接口（不应该在这里！）
    static napi_value CreateRenderer(...);
    static napi_value RenderFrame(...);
};
```

**问题**：
- PluginManager既管理XComponent，又处理ScreenRenderer
- NAPI桥接代码和业务逻辑混在一起
- 违反单一职责原则

#### ✅ 正确的架构

```
┌─────────────────────────────────────┐
│   ArkTS Layer (ScreenRenderer.ets)  │  ← UI层调用
└──────────────┬──────────────────────┘
               │ NAPI调用
┌──────────────▼──────────────────────┐
│  NAPI Bridge (screen_renderer_      │  ← 只负责类型转换
│       napi.cpp)                     │     ArkTS ↔ C++
└──────────────┬──────────────────────┘
               │ 纯C++调用
┌──────────────▼──────────────────────┐
│  Core Class (screen_renderer.cpp)   │  ← 业务逻辑实现
│                                     │     OpenGL/EGL管理
└─────────────────────────────────────┘
```

### 2. 三层架构

#### 第一层：ArkTS封装层
**文件**: `entry/src/main/ets/components/rendering/ScreenRenderer.ets`

**职责**:
- 提供友好的TypeScript API
- 管理renderer生命周期
- 处理回调和Promise

**特点**:
- 不直接操作Native资源
- 通过NAPI调用C++层
- 符合ArkTS编程规范

```typescript
export enum PixelFormat {
  RGBA = 0,   // RGBA8888 (4字节/像素)
  RGB = 1,    // RGB888 (3字节/像素)
  BGRA = 2,   // BGRA8888 (4字节/像素)
  NV21 = 3,   // YUV420半平面 (1.5字节/像素)
  NV12 = 4    // YUV420半平面 (1.5字节/像素)
}

export class ScreenRenderer {
  async initialize(): Promise<void> {
    this.nativeHandle = await nativeScreenRenderer.create(
      this.width,
      this.height,
      this.format
    );
  }
  
  async renderFrame(pixelData: ArrayBuffer, ...): Promise<void> {
    await nativeScreenRenderer.renderFrame(
      this.nativeHandle,
      pixelData,  // ArrayBuffer零拷贝传递
      width,
      height
    );
  }
}
```

#### 第二层：NAPI桥接层
**文件**: `entry/src/main/cpp/screen_renderer/screen_renderer_napi.cpp`

**职责**:
- **仅负责类型转换**：ArkTS类型 ↔ C++类型
- 参数验证
- 错误处理
- Promise封装

**特点**:
- 不包含业务逻辑
- 不管理OpenGL/EGL资源
- 纯粹的"翻译层"

```cpp
// 只做类型转换，不做业务逻辑
napi_value RenderFrame(napi_env env, napi_callback_info info) {
    // 1. 提取参数（类型转换）
    int32_t handle = ...;
    void* data = ...;  // napi_get_arraybuffer_info
    
    // 2. 获取C++对象
    ScreenRenderer* renderer = manager.GetRenderer(handle);
    
    // 3. 调用纯C++方法
    renderer->RenderFrame(data, size, width, height);
    
    // 4. 返回Promise
    return promise;
}
```

#### 第三层：C++核心层
**文件**: `entry/src/main/cpp/screen_renderer/screen_renderer.cpp`

**职责**:
- OpenGL ES纹理管理
- EGL上下文管理
- 实际的渲染逻辑
- GPU资源管理

**特点**:
- **纯C++实现，无NAPI依赖**
- 可以被其他C++代码复用
- 易于单元测试

```cpp
class ScreenRenderer {
public:
    bool Initialize();  // 初始化EGL和OpenGL
    bool RenderFrame(const void* pixelData, ...);  // 渲染到GPU
    uint32_t GetTextureId() const;  // 获取纹理ID
    void Destroy();  // 释放资源
    
private:
    uint32_t m_textureId;  // OpenGL纹理
    void* m_eglContext;    // EGL上下文
    // ... 纯C++成员变量
};
```

### 3. 管理器模式

**文件**: `screen_renderer_napi.cpp` 中的 `ScreenRendererManager`

**职责**:
- 管理所有ScreenRenderer实例的生命周期
- 通过handle映射到C++对象
- 线程安全访问（std::mutex）

```cpp
class ScreenRendererManager {
public:
    static ScreenRendererManager& GetInstance();
    
    int32_t CreateRenderer(int32_t width, int32_t height);
    ScreenRenderer* GetRenderer(int32_t handle);
    bool DestroyRenderer(int32_t handle);
    
private:
    std::unordered_map<int32_t, std::unique_ptr<ScreenRenderer>> m_renderers;
    std::mutex m_mutex;  // 线程安全
};
```

## 🔑 关键设计决策

### 1. 为什么NAPI桥接层单独成文件？

**好处**:
- ✅ 清晰的职责边界
- ✅ 易于维护和测试
- ✅ 可以独立替换NAPI实现（如切换到其他绑定系统）
- ✅ C++核心代码可以在非HarmonyOS平台复用

**对比**:
```
❌ 混合方式:
plugin_manager.cpp (500行)
  - 200行 XComponent管理
  - 300行 ScreenRenderer NAPI

✅ 分离方式:
plugin_manager.cpp (150行) - 只管理XComponent
screen_renderer_napi.cpp (400行) - 只负责NAPI桥接
screen_renderer.cpp (200行) - 纯C++实现
```

### 2. 为什么使用handle而不是直接暴露指针？

**安全性**:
- ✅ 防止ArkTS层直接操作C++指针
- ✅ 可以通过handle验证对象有效性
- ✅ 便于实现引用计数和自动清理
- ✅ 避免悬空指针问题

```typescript
// ArkTS层只能拿到number类型的handle
const handle: number = await create(1920, 1080);

// 无法直接访问C++对象
// const renderer: ScreenRenderer* = handle;  // ❌ 不可能
```

### 3. 零拷贝如何实现？

**关键点**: `napi_get_arraybuffer_info`

```cpp
// NAPI桥接层
void* data = nullptr;
size_t byteLength = 0;
napi_get_arraybuffer_info(env, args[1], &data, &byteLength);

// 直接将指针传递给C++层（无拷贝）
renderer->RenderFrame(data, byteLength, width, height);

// C++层直接使用指针进行OpenGL上传
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
```

**数据流**:
```
ArkTS ArrayBuffer 
    ↓ (napi_get_arraybuffer_info - 获取指针，无拷贝)
C++ void* 指针
    ↓ (glTexSubImage2D - DMA传输到GPU)
GPU显存
```

## 📊 模块依赖关系

```
┌─────────────────────┐
│  ScreenRenderer.ets │  ArkTS封装层
└──────────┬──────────┘
           │ import nativerender
           ↓
┌──────────────────────────┐
│  nativerender module     │  NAPI模块
│  (napi_init.cpp)         │
└──────────┬───────────────┘
           │ 导出5个函数
           ↓
┌──────────────────────────────┐
│  screen_renderer_napi.cpp    │  NAPI桥接层
│  - CreateRenderer            │
│  - RenderFrame               │
│  - GetTextureId              │
│  - ResizeRenderer            │
│  - DestroyRenderer           │
└──────────┬───────────────────┘
           │ 调用
           ↓
┌──────────────────────────────┐
│  ScreenRendererManager       │  对象管理器
│  - 管理实例生命周期          │
│  - handle ↔ 对象映射         │
└──────────┬───────────────────┘
           │ 调用
           ↓
┌──────────────────────────────┐
│  ScreenRenderer (C++)        │  核心业务逻辑
│  - Initialize()              │
│  - RenderFrame()             │
│  - GetTextureId()            │
│  - Resize()                  │
│  - Destroy()                 │
└──────────────────────────────┘
```

## 🚀 后续开发计划

### Phase 1: 接口框架（已完成 ✅）
- [x] 创建独立的screen_renderer目录
- [x] 定义ScreenRenderer C++类接口
- [x] 实现NAPI桥接层
- [x] 创建ArkTS封装层
- [x] 更新CMakeLists.txt

### Phase 2: 核心功能实现（待开发 📋）
- [ ] 实现EGL上下文初始化
- [ ] 实现OpenGL纹理创建和管理
- [ ] 实现RenderFrame的glTexSubImage2D调用
- [ ] 实现Resize时的纹理重建
- [ ] 实现资源清理逻辑

### Phase 3: 优化和测试（待开发 📋）
- [ ] 添加性能监控
- [ ] 实现BufferPool复用机制
- [ ] 添加单元测试
- [ ] 内存泄漏检测
- [ ] 压力测试

## 📝 总结

通过这次重构，我们实现了：

1. ✅ **清晰的职责分离**: NAPI桥接、C++核心、ArkTS封装各司其职
2. ✅ **模块化设计**: ScreenRenderer独立于PluginManager
3. ✅ **可维护性**: 每层职责明确，易于理解和修改
4. ✅ **可复用性**: C++核心层可在其他项目中使用
5. ✅ **零拷贝优化**: 通过ArrayBuffer指针直接传递数据

这种架构遵循了软件工程的最佳实践，为后续的功能扩展和维护打下了坚实的基础。
