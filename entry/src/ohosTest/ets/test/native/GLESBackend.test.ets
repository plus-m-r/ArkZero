import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * GLESBackend 单元测试
 * 
 * 测试 C++ GLESBackend 类通过 NAPI 桥接的功能
 * 覆盖 EGL 初始化、纹理管理、Shader 渲染等核心功能
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

/**
 * 辅助类：生成模拟的 RGBA 像素数据
 */
function generateRGBAData(width: number, height: number): ArrayBuffer {
  const size = width * height * 4; // RGBA = 4 bytes per pixel
  const buffer = new ArrayBuffer(size);
  const view = new Uint8Array(buffer);
  
  // 填充渐变颜色
  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const idx = (y * width + x) * 4;
      view[idx] = Math.floor((x / width) * 255);     // R
      view[idx + 1] = Math.floor((y / height) * 255); // G
      view[idx + 2] = 128;                            // B
      view[idx + 3] = 255;                            // A
    }
  }
  
  return buffer;
}

export default function GLESBackendTest() {
  describe('GLESBackend.ConstructorTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('GB-CTOR-001: Should create backend instance', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      expect(handle >= 0).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-CTOR-002: Should initialize in uninitialized state', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const initialized: boolean = await nativerender.isGLESBackendInitialized(handle);
      expect(initialized).assertFalse();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-CTOR-003: Should support multiple instances', 0, async () => {
      const handle1: number = await nativerender.createGLESBackend();
      const handle2: number = await nativerender.createGLESBackend();
      const handle3: number = await nativerender.createGLESBackend();
      
      expect(handle1 >= 0).assertTrue();
      expect(handle2 >= 0).assertTrue();
      expect(handle3 >= 0).assertTrue();
      expect(handle1 !== handle2).assertTrue();
      expect(handle2 !== handle3).assertTrue();
      
      nativerender.destroyGLESBackend(handle1);
      nativerender.destroyGLESBackend(handle2);
      nativerender.destroyGLESBackend(handle3);
    });
  });

  describe('GLESBackend.InitializationTests', () => {
    
    // ===== 初始化测试 =====
    
    it('GB-INIT-001: Should initialize with offscreen mode', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, 640, 480, 0  // PixelFormat.RGBA_8888 = 0
      );
      
      expect(success).assertTrue();
      
      const initialized: boolean = await nativerender.isGLESBackendInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INIT-002: Should initialize with different resolutions', 0, async () => {
      const resolutions: Resolution[] = [
        { width: 320, height: 240 },
        { width: 640, height: 480 },
        { width: 1280, height: 720 }
      ];
      
      for (const res of resolutions) {
        const handle: number = await nativerender.createGLESBackend();
        
        const success: boolean = await nativerender.glesBackendInitialize(
          handle, res.width, res.height, 0
        );
        
        expect(success).assertTrue();
        
        nativerender.destroyGLESBackend(handle);
      }
    });

    it('GB-INIT-003: Should handle zero dimensions gracefully', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      // 零尺寸可能被内部处理为默认值或失败
      // 这里只验证不会崩溃
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, 0, 0, 0
      );
      
      // 注意：具体行为取决于 GLESBackend::InitializeOffscreen 的实现
      // 如果成功，说明内部有默认值处理
      // 如果失败，说明有严格的参数验证
      
      nativerender.destroyGLESBackend(handle);
    });
  });

  describe('GLESBackend.RenderTests', () => {
    
    // ===== 渲染测试 =====
    
    it('GB-RND-001: Should render small frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      await nativerender.glesBackendInitialize(handle, 64, 64, 0);
      
      const pixelData: ArrayBuffer = generateRGBAData(64, 64);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, pixelData, 64, 64
      );
      
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-RND-002: Should render 720p frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      await nativerender.glesBackendInitialize(handle, 1280, 720, 0);
      
      const pixelData: ArrayBuffer = generateRGBAData(1280, 720);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, pixelData, 1280, 720
      );
      
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-RND-003: Should render multiple frames', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      await nativerender.glesBackendInitialize(handle, 640, 480, 0);
      
      // 连续渲染 10 帧
      for (let i = 0; i < 10; i++) {
        const pixelData: ArrayBuffer = generateRGBAData(640, 480);
        const success: boolean = await nativerender.glesBackendRenderFrame(
          handle, pixelData, 640, 480
        );
        expect(success).assertTrue();
      }
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-RND-004: Should fail render before initialization', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const pixelData: ArrayBuffer = generateRGBAData(64, 64);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, pixelData, 64, 64
      );
      
      // 未初始化时渲染应该失败
      expect(success).assertFalse();
      
      nativerender.destroyGLESBackend(handle);
    });
  });

  describe('GLESBackend.IntegrationTests', () => {
    
    // ===== 集成测试 =====
    
    it('GB-INT-001: Should complete full lifecycle', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      // 1. 检查初始状态
      const init1: boolean = await nativerender.isGLESBackendInitialized(handle);
      expect(init1).assertFalse();
      
      // 2. 初始化
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, 640, 480, 0
      );
      expect(success).assertTrue();
      
      // 3. 检查初始化状态
      const init2: boolean = await nativerender.isGLESBackendInitialized(handle);
      expect(init2).assertTrue();
      
      // 4. 渲染多帧
      for (let i = 0; i < 5; i++) {
        const pixelData: ArrayBuffer = generateRGBAData(640, 480);
        const renderSuccess: boolean = await nativerender.glesBackendRenderFrame(
          handle, pixelData, 640, 480
        );
        expect(renderSuccess).assertTrue();
      }
      
      // 5. 销毁
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INT-002: Should handle different resolutions sequentially', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const resolutions: Resolution[] = [
        { width: 320, height: 240 },
        { width: 640, height: 480 },
        { width: 1280, height: 720 }
      ];
      
      for (const res of resolutions) {
        // 每次重新初始化为不同分辨率
        const initHandle: number = await nativerender.createGLESBackend();
        
        await nativerender.glesBackendInitialize(
          initHandle, res.width, res.height, 0
        );
        
        const pixelData: ArrayBuffer = generateRGBAData(res.width, res.height);
        const success: boolean = await nativerender.glesBackendRenderFrame(
          initHandle, pixelData, res.width, res.height
        );
        
        expect(success).assertTrue();
        
        nativerender.destroyGLESBackend(initHandle);
      }
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INT-003: Should maintain independent state across instances', 0, async () => {
      const handle1: number = await nativerender.createGLESBackend();
      const handle2: number = await nativerender.createGLESBackend();
      
      // 只初始化 handle1
      await nativerender.glesBackendInitialize(handle1, 640, 480, 0);
      
      const init1: boolean = await nativerender.isGLESBackendInitialized(handle1);
      const init2: boolean = await nativerender.isGLESBackendInitialized(handle2);
      
      expect(init1).assertTrue();
      expect(init2).assertFalse();
      
      nativerender.destroyGLESBackend(handle1);
      nativerender.destroyGLESBackend(handle2);
    });
  });

  describe('GLESBackend.EdgeCaseTests', () => {
    
    // ===== 边界测试 =====
    
    it('GB-EDGE-001: Should handle minimal frame size', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      await nativerender.glesBackendInitialize(handle, 1, 1, 0);
      
      const pixelData: ArrayBuffer = generateRGBAData(1, 1);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, pixelData, 1, 1
      );
      
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-EDGE-002: Should handle non-square resolution', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      await nativerender.glesBackendInitialize(handle, 1920, 1080, 0);
      
      const pixelData: ArrayBuffer = generateRGBAData(1920, 1080);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, pixelData, 1920, 1080
      );
      
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-EDGE-003: Should handle rapid initialization cycles', 0, async () => {
      // 快速循环 10 次：创建-初始化-销毁
      for (let i = 0; i < 10; i++) {
        const handle: number = await nativerender.createGLESBackend();
        await nativerender.glesBackendInitialize(handle, 64, 64, 0);
        nativerender.destroyGLESBackend(handle);
      }
    });
  });
}
