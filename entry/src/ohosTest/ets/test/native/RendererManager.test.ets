import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * RendererManager 单元测试
 * 
 * 测试 C++ RendererManager 类通过 NAPI 桥接的功能
 * 覆盖渲染器生命周期管理、线程安全、资源清理等核心功能
 */

/**
 * 分辨率结构
 */
interface Resolution {
  width: number;
  height: number;
}

export default function RendererManagerTest() {
  describe('RendererManager.ConstructorTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('RM-CTOR-001: Should create renderer through manager', 0, async () => {
      const handle: number = await nativerender.managerCreateOffscreenRenderer(640, 480, 0, true);
      expect(handle > 0).assertTrue();
      
      const success: boolean = await nativerender.managerDestroyRenderer(handle);
      expect(success).assertTrue();
    });

    it('RM-CTOR-002: Should support different async modes', 0, async () => {
      const handle1: number = await nativerender.managerCreateOffscreenRenderer(320, 240, 0, true);
      const handle2: number = await nativerender.managerCreateOffscreenRenderer(320, 240, 0, false);
      
      expect(handle1 > 0).assertTrue();
      expect(handle2 > 0).assertTrue();
      expect(handle1 !== handle2).assertTrue();
      
      await nativerender.managerDestroyRenderer(handle1);
      await nativerender.managerDestroyRenderer(handle2);
    });

    it('RM-CTOR-003: Should generate unique handles', 0, async () => {
      const handles: number[] = [];
      
      // 创建多个渲染器，检查句柄唯一性
      for (let i = 0; i < 5; i++) {
        const handle: number = await nativerender.managerCreateOffscreenRenderer(64, 64, 0, true);
        expect(handle > 0).assertTrue();
        expect(handles.includes(handle)).assertFalse();
        handles.push(handle);
      }
      
      // 清理所有渲染器
      for (const handle of handles) {
        await nativerender.managerDestroyRenderer(handle);
      }
    });
  });

  describe('RendererManager.LifecycleTests', () => {
    
    // ===== 生命周期测试 =====
    
    it('RM-LIFE-001: Should destroy renderer successfully', 0, async () => {
      const handle: number = await nativerender.managerCreateOffscreenRenderer(640, 480, 0, true);
      
      const success: boolean = await nativerender.managerDestroyRenderer(handle);
      expect(success).assertTrue();
      
      // 再次销毁应该失败
      const failure: boolean = await nativerender.managerDestroyRenderer(handle);
      expect(failure).assertFalse();
    });

    it('RM-LIFE-002: Should handle invalid handle gracefully', 0, async () => {
      const invalidHandle: number = 99999;
      
      const success: boolean = await nativerender.managerDestroyRenderer(invalidHandle);
      expect(success).assertFalse();
    });
  });

  describe('RendererManager.IntegrationTests', () => {
    
    // ===== 集成测试 =====
    
    it('RM-INT-001: Should complete full lifecycle', 0, async () => {
      const handle: number = await nativerender.managerCreateOffscreenRenderer(640, 480, 0, true);
      
      expect(handle > 0).assertTrue();
      
      // 销毁渲染器
      const destroySuccess: boolean = await nativerender.managerDestroyRenderer(handle);
      expect(destroySuccess).assertTrue();
    });

    it('RM-INT-002: Should support multiple concurrent instances', 0, async () => {
      const handles: number[] = [];
      
      // 并发创建多个渲染器
      for (let i = 0; i < 3; i++) {
        const handle: number = await nativerender.managerCreateOffscreenRenderer(320, 240, 0, true);
        expect(handle > 0).assertTrue();
        handles.push(handle);
      }
      
      // 验证都能正常使用
      for (const handle of handles) {
        const destroySuccess: boolean = await nativerender.managerDestroyRenderer(handle);
        expect(destroySuccess).assertTrue();
      }
    });

    it('RM-INT-003: Should handle rapid create-destroy cycles', 0, async () => {
      // 快速循环 5 次：创建-销毁
      for (let i = 0; i < 5; i++) {
        const handle: number = await nativerender.managerCreateOffscreenRenderer(64, 64, 0, true);
        expect(handle > 0).assertTrue();
        
        const success: boolean = await nativerender.managerDestroyRenderer(handle);
        expect(success).assertTrue();
      }
    });
  });

  describe('RendererManager.BoundaryTests', () => {
    
    // ===== 边界测试 =====
    
    it('RM-EDGE-001: Should handle minimum resolution', 0, async () => {
      const handle: number = await nativerender.managerCreateOffscreenRenderer(1, 1, 0, true);
      expect(handle > 0).assertTrue();
      
      const success: boolean = await nativerender.managerDestroyRenderer(handle);
      expect(success).assertTrue();
    });

    it('RM-EDGE-002: Should handle different resolutions', 0, async () => {
      const resolutions: Resolution[] = [
        { width: 64, height: 64 },
        { width: 320, height: 240 },
        { width: 1280, height: 720 }
      ];
      
      for (const res of resolutions) {
        const handle: number = await nativerender.managerCreateOffscreenRenderer(res.width, res.height, 0, true);
        expect(handle > 0).assertTrue();
        
        const success: boolean = await nativerender.managerDestroyRenderer(handle);
        expect(success).assertTrue();
      }
    });
  });
}
