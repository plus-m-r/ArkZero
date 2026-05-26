import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * Renderer 简化单元测试
 * 只测试当前保留的核心 API
 */

export default function RendererTest() {
  describe('Renderer.BasicTests', () => {
    it('R-001: Should have core API functions defined', 0, async () => {
      // 检查核心渲染 API
      expect(typeof nativerender.create).assertEqual('function');
      expect(typeof nativerender.renderFrame).assertEqual('function');
      expect(typeof nativerender.resize).assertEqual('function');
      expect(typeof nativerender.destroy).assertEqual('function');
      
      // 检查 Manager API
      expect(typeof nativerender.managerCreateSurfaceRenderer).assertEqual('function');
      expect(typeof nativerender.managerDestroyRenderer).assertEqual('function');
      
      // 检查 SurfaceManager API
      expect(typeof nativerender.surfaceManagerCreateNativeWindow).assertEqual('function');
      expect(typeof nativerender.surfaceManagerDestroyNativeWindow).assertEqual('function');
    });
  });
}
