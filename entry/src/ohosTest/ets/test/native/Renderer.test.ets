import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * Renderer 单元测试
 * 
 * 测试 C++ Renderer 类通过 NAPI 桥接的功能
 * 覆盖异步渲染、性能监控、后端管理等核心功能
 * 
 * ⭐ 使用离屏模式（Pbuffer）进行测试，验证核心逻辑
 */

/**
 * 分辨率结构
 */
interface Resolution {
  width: number;
  height: number;
}

export default function RendererTest() {
  describe('Renderer.ConstructorTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('R-CTOR-001: Should create renderer with async enabled', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, true);
      expect(handle >= 0).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-CTOR-002: Should create renderer with sync mode', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, false);
      expect(handle >= 0).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-CTOR-003: Should support multiple instances', 0, async () => {
      const handle1: number = await nativerender.createTestRenderer(320, 240, 0, true);
      const handle2: number = await nativerender.createTestRenderer(640, 480, 0, true);
      
      expect(handle1 >= 0).assertTrue();
      expect(handle2 >= 0).assertTrue();
      expect(handle1 !== handle2).assertTrue();
      
      nativerender.destroyTestRenderer(handle1);
      nativerender.destroyTestRenderer(handle2);
    });
  });

  describe('Renderer.InitializeTests', () => {
    
    // ===== 初始化测试 =====
    
    it('R-INIT-001: Should initialize offscreen successfully', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, true);
      
      const success: boolean = await nativerender.rendererInitialize(handle, 640, 480);
      expect(success).assertTrue();
      
      const initialized: boolean = await nativerender.isRendererInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-INIT-002: Should initialize with different resolutions', 0, async () => {
      const resolutions: Resolution[] = [
        { width: 320, height: 240 },
        { width: 640, height: 480 },
        { width: 1280, height: 720 }
      ];
      
      for (const res of resolutions) {
        const handle: number = await nativerender.createTestRenderer(res.width, res.height, 0, true);
        const success: boolean = await nativerender.rendererInitialize(handle, res.width, res.height);
        
        expect(success).assertTrue();
        
        nativerender.destroyTestRenderer(handle);
      }
    });

    it('R-INIT-003: Should return backend name after initialization', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, true);
      await nativerender.rendererInitialize(handle, 640, 480);
      
      const backendName: string = await nativerender.getRendererBackendName(handle);
      expect(backendName.length > 0).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });
  });

  describe('Renderer.RenderTests', () => {
    
    // ===== 渲染测试 =====
    
    it('R-RND-001: Should render small frame in async mode', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(64, 64, 0, true);
      await nativerender.rendererInitialize(handle, 64, 64);
      
      // 生成模拟 RGBA 数据
      const size = 64 * 64 * 4;
      const buffer = new ArrayBuffer(size);
      const view = new Uint8Array(buffer);
      for (let i = 0; i < size; i++) {
        view[i] = 128;
      }
      
      const success: boolean = await nativerender.rendererRenderFrame(handle, buffer, 64, 64);
      expect(success).assertTrue();
      
      // 等待一小段时间让异步线程处理
      await new Promise<void>((resolve: () => void) => { setTimeout(resolve, 100); });
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-RND-002: Should render 720p frame', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(1280, 720, 0, true);
      await nativerender.rendererInitialize(handle, 1280, 720);
      
      const size = 1280 * 720 * 4;
      const buffer = new ArrayBuffer(size);
      
      const success: boolean = await nativerender.rendererRenderFrame(handle, buffer, 1280, 720);
      expect(success).assertTrue();
      
      await new Promise<void>((resolve: () => void) => { setTimeout(resolve, 100); });
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-RND-003: Should render multiple frames sequentially', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(64, 64, 0, true);
      await nativerender.rendererInitialize(handle, 64, 64);
      
      const size = 64 * 64 * 4;
      const buffer = new ArrayBuffer(size);
      
      // 连续渲染 5 帧
      for (let i = 0; i < 5; i++) {
        const success: boolean = await nativerender.rendererRenderFrame(handle, buffer, 64, 64);
        expect(success).assertTrue();
      }
      
      // 等待异步线程处理
      await new Promise<void>((resolve: () => void) => { setTimeout(resolve, 200); });
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-RND-004: Should fail rendering before initialization', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(64, 64, 0, true);
      
      const size = 64 * 64 * 4;
      const buffer = new ArrayBuffer(size);
      
      const success: boolean = await nativerender.rendererRenderFrame(handle, buffer, 64, 64);
      expect(success).assertFalse();
      
      nativerender.destroyTestRenderer(handle);
    });
  });

  describe('Renderer.ResizeTests', () => {
    
    // ===== 尺寸调整测试 =====
    
    it('R-RES-001: Should resize after initialization', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, true);
      await nativerender.rendererInitialize(handle, 640, 480);
      
      const success: boolean = await nativerender.rendererResize(handle, 1280, 720);
      expect(success).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-RES-002: Should fail resize before initialization', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, true);
      
      const success: boolean = await nativerender.rendererResize(handle, 1280, 720);
      expect(success).assertFalse();
      
      nativerender.destroyTestRenderer(handle);
    });
  });

  describe('Renderer.PerformanceTests', () => {
    
    // ===== 性能统计测试 =====
    
    it('R-PERF-001: Should return performance stats', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(64, 64, 0, true);
      await nativerender.rendererInitialize(handle, 64, 64);
      
      const stats: string = await nativerender.getRendererPerformanceStats(handle);
      expect(stats.length > 0).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-PERF-002: Should track frame count after rendering', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(64, 64, 0, true);
      await nativerender.rendererInitialize(handle, 64, 64);
      
      const size = 64 * 64 * 4;
      const buffer = new ArrayBuffer(size);
      
      // 渲染 10 帧
      for (let i = 0; i < 10; i++) {
        await nativerender.rendererRenderFrame(handle, buffer, 64, 64);
      }
      
      // 等待处理
      await new Promise<void>((resolve: () => void) => { setTimeout(resolve, 200); });
      
      const stats: string = await nativerender.getRendererPerformanceStats(handle);
      expect(stats.length > 0).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });
  });

  describe('Renderer.IntegrationTests', () => {
    
    // ===== 集成测试 =====
    
    it('R-INT-001: Should complete full lifecycle', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(640, 480, 0, true);
      
      // 1. 初始化
      const initSuccess: boolean = await nativerender.rendererInitialize(handle, 640, 480);
      expect(initSuccess).assertTrue();
      
      // 2. 渲染多帧
      const size = 640 * 480 * 4;
      const buffer = new ArrayBuffer(size);
      
      for (let i = 0; i < 3; i++) {
        const renderSuccess: boolean = await nativerender.rendererRenderFrame(handle, buffer, 640, 480);
        expect(renderSuccess).assertTrue();
      }
      
      // 3. 调整尺寸
      const resizeSuccess: boolean = await nativerender.rendererResize(handle, 1280, 720);
      expect(resizeSuccess).assertTrue();
      
      // 4. 获取统计
      const stats: string = await nativerender.getRendererPerformanceStats(handle);
      expect(stats.length > 0).assertTrue();
      
      // 5. 销毁
      nativerender.destroyTestRenderer(handle);
      
      // 6. 验证已销毁
      const initialized: boolean = await nativerender.isRendererInitialized(handle);
      expect(initialized).assertFalse();
    });

    it('R-INT-002: Should handle multiple independent instances', 0, async () => {
      const handle1: number = await nativerender.createTestRenderer(320, 240, 0, true);
      const handle2: number = await nativerender.createTestRenderer(640, 480, 0, true);
      
      await nativerender.rendererInitialize(handle1, 320, 240);
      await nativerender.rendererInitialize(handle2, 640, 480);
      
      const size1 = 320 * 240 * 4;
      const size2 = 640 * 480 * 4;
      const buffer1 = new ArrayBuffer(size1);
      const buffer2 = new ArrayBuffer(size2);
      
      await nativerender.rendererRenderFrame(handle1, buffer1, 320, 240);
      await nativerender.rendererRenderFrame(handle2, buffer2, 640, 480);
      
      await new Promise<void>((resolve: () => void) => { setTimeout(resolve, 100); });
      
      nativerender.destroyTestRenderer(handle1);
      nativerender.destroyTestRenderer(handle2);
    });

    it('R-INT-003: Should handle rapid create-destroy cycles', 0, async () => {
      // 快速循环 5 次：创建-初始化-销毁
      for (let i = 0; i < 5; i++) {
        const handle: number = await nativerender.createTestRenderer(64, 64, 0, true);
        await nativerender.rendererInitialize(handle, 64, 64);
        nativerender.destroyTestRenderer(handle);
      }
    });
  });

  describe('Renderer.BoundaryTests', () => {
    
    // ===== 边界测试 =====
    
    it('R-EDGE-001: Should handle minimum resolution', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(1, 1, 0, true);
      await nativerender.rendererInitialize(handle, 1, 1);
      
      const size = 1 * 1 * 4;
      const buffer = new ArrayBuffer(size);
      
      const success: boolean = await nativerender.rendererRenderFrame(handle, buffer, 1, 1);
      expect(success).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-EDGE-002: Should handle non-square resolutions', 0, async () => {
      const handle: number = await nativerender.createTestRenderer(1920, 1080, 0, true);
      await nativerender.rendererInitialize(handle, 1920, 1080);
      
      const size = 1920 * 1080 * 4;
      const buffer = new ArrayBuffer(size);
      
      const success: boolean = await nativerender.rendererRenderFrame(handle, buffer, 1920, 1080);
      expect(success).assertTrue();
      
      nativerender.destroyTestRenderer(handle);
    });

    it('R-EDGE-003: Should handle invalid handle gracefully', 0, async () => {
      const invalidHandle: number = 99999;
      
      const initialized: boolean = await nativerender.isRendererInitialized(invalidHandle);
      expect(initialized).assertFalse();
      
      const backendName: string | null = await nativerender.getRendererBackendName(invalidHandle);
      expect(backendName === null || backendName === undefined).assertTrue();
    });
  });
}
