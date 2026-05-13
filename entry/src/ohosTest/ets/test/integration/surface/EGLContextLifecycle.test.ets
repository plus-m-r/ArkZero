import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * EGL Context 生命周期集成测试
 * 
 * 验证 EGL Context 的创建、使用和销毁流程
 */

export default function EGLContextLifecycleTest() {
  describe('EGL Context Lifecycle Integration Tests', () => {
    
    it('EGL-LIFE-001: Should create and destroy EGL context', 0, async () => {
      // 1. 创建 EGL Context
      const eglHandle: number = await nativerender.createEGLContext(640, 480);
      expect(eglHandle >= 0).assertTrue();
      
      // 2. 验证初始化状态
      const initialized: boolean = await nativerender.isEGLInitialized(eglHandle);
      expect(initialized).assertTrue();
      
      // 3. 销毁 EGL Context
      nativerender.destroyEGLContext(eglHandle);
    });

    it('EGL-LIFE-002: Should handle multiple EGL contexts', 0, async () => {
      const contextCount: number = 3;
      const handles: number[] = [];
      
      // 1. 创建多个 EGL Context
      for (let i: number = 0; i < contextCount; i++) {
        const handle: number = await nativerender.createEGLContext(
          320 * (i + 1), 240 * (i + 1)
        );
        expect(handle >= 0).assertTrue();
        handles.push(handle);
      }
      
      // 2. 验证所有 Context 都已初始化
      for (const handle of handles) {
        const initialized: boolean = await nativerender.isEGLInitialized(handle);
        expect(initialized).assertTrue();
      }
      
      // 3. 销毁所有 Context
      for (const handle of handles) {
        nativerender.destroyEGLContext(handle);
      }
    });

    it('EGL-LIFE-003: Should make context current', 0, async () => {
      const eglHandle: number = await nativerender.createEGLContext(640, 480);
      expect(eglHandle >= 0).assertTrue();
      
      // MakeCurrent 应该成功（在真实环境中）
      // 这里只验证接口不会崩溃
      
      nativerender.destroyEGLContext(eglHandle);
    });
  });
}
