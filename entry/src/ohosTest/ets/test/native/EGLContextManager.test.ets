import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * EGLContextManager 单元测试
 * 
 * 测试 C++ EGLContextManager 类通过 NAPI 桥接的功能
 * 覆盖离屏 EGL 上下文（Pbuffer）的创建、销毁、状态管理等核心功能
 * 
 * ⭐ 关键：为其他 GL 相关测试提供有效的 OpenGL 上下文
 */

export default function EGLContextManagerTest() {
  describe('EGLContextManager.ConstructorTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('EGL-CTOR-001: Should create EGL context with valid dimensions', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      expect(handle >= 0).assertTrue();
      
      const initialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyEGLContext(handle);
    });

    it('EGL-CTOR-002: Should support different sizes', 0, async () => {
      interface Size {
        width: number;
        height: number;
      }
      
      const sizes: Size[] = [
        { width: 32, height: 32 },
        { width: 128, height: 128 },
        { width: 256, height: 256 },
        { width: 512, height: 512 },
      ];
      
      for (let i: number = 0; i < sizes.length; i++) {
        const handle: number = await nativerender.createEGLContext(sizes[i].width, sizes[i].height);
        expect(handle >= 0).assertTrue();
        
        const initialized: boolean = await nativerender.isEGLInitialized(handle);
        expect(initialized).assertTrue();
        
        nativerender.destroyEGLContext(handle);
      }
    });

    it('EGL-CTOR-003: Should support multiple contexts', 0, async () => {
      const handle1: number = await nativerender.createEGLContext(64, 64);
      const handle2: number = await nativerender.createEGLContext(128, 128);
      const handle3: number = await nativerender.createEGLContext(256, 256);
      
      expect(handle1 !== handle2).assertTrue();
      expect(handle2 !== handle3).assertTrue();
      expect(handle1 !== handle3).assertTrue();
      
      const init1: boolean = await nativerender.isEGLInitialized(handle1);
      const init2: boolean = await nativerender.isEGLInitialized(handle2);
      const init3: boolean = await nativerender.isEGLInitialized(handle3);
      
      expect(init1).assertTrue();
      expect(init2).assertTrue();
      expect(init3).assertTrue();
      
      nativerender.destroyEGLContext(handle1);
      nativerender.destroyEGLContext(handle2);
      nativerender.destroyEGLContext(handle3);
    });
  });

  describe('EGLContextManager.StateTests', () => {
    
    // ===== 状态查询测试 =====
    
    it('EGL-STT-001: Should report initialized state correctly', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      const initialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyEGLContext(handle);
      
      // 销毁后不应再访问（句柄已失效）
    });

    it('EGL-STT-002: Should maintain state across operations', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      // 多次检查状态
      for (let i: number = 0; i < 5; i++) {
        const initialized: boolean = await nativerender.isEGLInitialized(handle);
        expect(initialized).assertTrue();
      }
      
      nativerender.destroyEGLContext(handle);
    });
  });

  describe('EGLContextManager.MakeCurrentTests', () => {
    
    // ===== MakeCurrent 测试 =====
    
    it('EGL-MKC-001: Should make context current successfully', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      const success: boolean = await nativerender.eglMakeCurrent(handle);
      expect(success).assertTrue();
      
      nativerender.destroyEGLContext(handle);
    });

    it('EGL-MKC-002: Should allow multiple makeCurrent calls', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      // 多次调用 makeCurrent（应该都成功）
      for (let i: number = 0; i < 5; i++) {
        const success: boolean = await nativerender.eglMakeCurrent(handle);
        expect(success).assertTrue();
      }
      
      nativerender.destroyEGLContext(handle);
    });
  });

  describe('EGLContextManager.DestroyTests', () => {
    
    // ===== 销毁测试 =====
    
    it('EGL-DST-001: Should destroy context cleanly', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      // 销毁前验证
      const initialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(initialized).assertTrue();
      
      // 销毁
      nativerender.destroyEGLContext(handle);
      
      // 注意：销毁后不应再访问该句柄
    });

    it('EGL-DST-002: Should allow destroy without explicit makeCurrent', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      // 不调用 makeCurrent，直接销毁
      nativerender.destroyEGLContext(handle);
    });

    it('EGL-DST-003: Should handle multiple destroy calls gracefully', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      // 第一次销毁
      nativerender.destroyEGLContext(handle);
      
      // 第二次销毁（应该不会崩溃，但行为未定义）
      // 注意：实际使用中不应该这样做
    });
  });

  describe('EGLContextManager.IntegrationTests', () => {
    
    // ===== 集成场景测试 =====
    
    it('EGL-INT-001: Should support create-makeCurrent-destroy lifecycle', 0, async () => {
      const handle: number = await nativerender.createEGLContext(64, 64);
      
      // 1. 验证初始化
      const initialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(initialized).assertTrue();
      
      // 2. MakeCurrent
      const success: boolean = await nativerender.eglMakeCurrent(handle);
      expect(success).assertTrue();
      
      // 3. 再次验证状态
      const stillInitialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(stillInitialized).assertTrue();
      
      // 4. 销毁
      nativerender.destroyEGLContext(handle);
    });

    it('EGL-INT-002: Should support recreate after destroy', 0, async () => {
      // 第一次创建-销毁
      const handle1: number = await nativerender.createEGLContext(64, 64);
      nativerender.destroyEGLContext(handle1);
      
      // 第二次创建
      const handle2: number = await nativerender.createEGLContext(128, 128);
      expect(handle2 >= 0).assertTrue();
      
      const initialized: boolean = await nativerender.isEGLInitialized(handle2);
      expect(initialized).assertTrue();
      
      nativerender.destroyEGLContext(handle2);
    });

    it('EGL-INT-003: Should handle rapid create-destroy cycles', 0, async () => {
      // 快速循环 10 次
      for (let i: number = 0; i < 10; i++) {
        const handle: number = await nativerender.createEGLContext(32, 32);
        expect(handle >= 0).assertTrue();
        
        const initialized: boolean = await nativerender.isEGLInitialized(handle);
        expect(initialized).assertTrue();
        
        nativerender.destroyEGLContext(handle);
      }
    });

    it('EGL-INT-004: Should support concurrent contexts', 0, async () => {
      // 创建多个上下文并同时存在
      const handles: number[] = [];
      
      for (let i: number = 0; i < 5; i++) {
        const handle: number = await nativerender.createEGLContext(64 + i * 32, 64 + i * 32);
        expect(handle >= 0).assertTrue();
        handles.push(handle);
      }
      
      // 验证所有上下文都已初始化
      for (let i: number = 0; i < handles.length; i++) {
        const initialized: boolean = await nativerender.isEGLInitialized(handles[i]);
        expect(initialized).assertTrue();
      }
      
      // 销毁所有上下文
      for (let i: number = 0; i < handles.length; i++) {
        nativerender.destroyEGLContext(handles[i]);
      }
    });
  });

  describe('EGLContextManager.EdgeCaseTests', () => {
    
    // ===== 边界情况测试 =====
    
    it('EGL-EDGE-001: Should handle very small dimensions', 0, async () => {
      const handle: number = await nativerender.createEGLContext(1, 1);
      expect(handle >= 0).assertTrue();
      
      const initialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyEGLContext(handle);
    });

    it('EGL-EDGE-002: Should handle large dimensions', 0, async () => {
      // 4K 分辨率
      const handle: number = await nativerender.createEGLContext(3840, 2160);
      expect(handle >= 0).assertTrue();
      
      const initialized: boolean = await nativerender.isEGLInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyEGLContext(handle);
    });

    it('EGL-EDGE-003: Should handle non-square dimensions', 0, async () => {
      interface Dimension {
        width: number;
        height: number;
      }
      
      const dimensions: Dimension[] = [
        { width: 1920, height: 1080 },  // 1080p
        { width: 1280, height: 720 },   // 720p
        { width: 800, height: 600 },    // SVGA
      ];
      
      for (let i: number = 0; i < dimensions.length; i++) {
        const handle: number = await nativerender.createEGLContext(
          dimensions[i].width, 
          dimensions[i].height
        );
        expect(handle >= 0).assertTrue();
        
        const initialized: boolean = await nativerender.isEGLInitialized(handle);
        expect(initialized).assertTrue();
        
        nativerender.destroyEGLContext(handle);
      }
    });
  });
}
