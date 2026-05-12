import { describe } from '@ohos/hypium';

// C++ 层测试（通过 NAPI 桥接）
import PerformanceMonitorTest from './native/PerformanceMonitor.test';
import PixelFormatConverterTest from './native/PixelFormatConverter.test';

// ArkTS 层测试
import PixelFormatTest from './arkts/PixelFormat.test';
import ArkZeroRendererConfigTest from './arkts/ArkZeroRendererConfig.test';
import ArkZeroRendererTest from './arkts/ArkZeroRenderer.test';

export default function testsuite() {
  // C++ 层测试
  PerformanceMonitorTest();
  PixelFormatConverterTest();
  
  // ArkTS 层测试
  PixelFormatTest();
  ArkZeroRendererConfigTest();
  ArkZeroRendererTest();
}
