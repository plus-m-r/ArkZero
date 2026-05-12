import { describe } from '@ohos/hypium';

// C++ 层测试（通过 NAPI 桥接）
import PerformanceMonitorTest from './native/PerformanceMonitor.test';
import PixelFormatConverterTest from './native/PixelFormatConverter.test';
import RenderQueueTest from './native/RenderQueue.test';
import EGLContextManagerTest from './native/EGLContextManager.test';
import TextureManagerTest from './native/TextureManager.test';
import YUVShaderManagerTest from './native/YUVShaderManager.test';
import GLESBackendTest from './native/GLESBackend.test';

// ArkTS 层测试
import PixelFormatTest from './arkts/PixelFormat.test';
import ArkZeroRendererConfigTest from './arkts/ArkZeroRendererConfig.test';
import ArkZeroRendererTest from './arkts/ArkZeroRenderer.test';

export default function testsuite() {
  // C++ 层测试
  PerformanceMonitorTest();
  PixelFormatConverterTest();
  RenderQueueTest();
  EGLContextManagerTest();  // ⭐ 先测试 EGL 上下文管理
  TextureManagerTest();     // ⭐ 再测试纹理管理（依赖 EGL）
  YUVShaderManagerTest();   // ⭐ 最后测试着色器管理（依赖 EGL + 纹理）
  GLESBackendTest();        // ⭐ 测试 OpenGL ES 后端（整合所有组件）
  
  // ArkTS 层测试
  PixelFormatTest();
  ArkZeroRendererConfigTest();
  ArkZeroRendererTest();
}
