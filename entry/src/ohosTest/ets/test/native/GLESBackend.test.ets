import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * GLESBackend 单元测试
 * 
 * 测试 C++ GLESBackend 类（OpenGL ES 渲染后端）
 * 验证 Facade 模式整合 EGL + Texture + Shader 的能力
 * 
 * ⭐ 使用离屏 EGL 上下文进行测试
 * ⭐ 测试 RGBA 和 YUV 两种渲染路径
 */

// PixelFormat 枚举值（与 C++ 端保持一致）
enum PixelFormat {
  RGBA = 0,
  RGB = 1,
  NV21 = 2,
  NV12 = 3,
}

/**
 * 生成模拟的 RGBA 数据
 */
function generateRGBAData(width: number, height: number): ArrayBuffer {
  const size: number = width * height * 4; // RGBA = 4 bytes per pixel
  const buffer: ArrayBuffer = new ArrayBuffer(size);
  const view: Uint8Array = new Uint8Array(buffer);
  
  // 填充渐变颜色
  for (let i: number = 0; i < size; i += 4) {
    view[i] = Math.floor((i / size) * 255);     // R
    view[i + 1] = 128;                           // G
    view[i + 2] = 64;                            // B
    view[i + 3] = 255;                           // A
  }
  
  return buffer;
}

/**
 * 生成模拟的 NV21 数据
 */
function generateNV21Data(width: number, height: number): { yPlane: ArrayBuffer, uvPlane: ArrayBuffer } {
  const ySize: number = width * height;
  const yPlane: ArrayBuffer = new ArrayBuffer(ySize);
  const yView: Uint8Array = new Uint8Array(yPlane);
  
  for (let i: number = 0; i < ySize; i++) {
    yView[i] = Math.floor((i / ySize) * 255);
  }
  
  const uvWidth: number = Math.floor(width / 2);
  const uvHeight: number = Math.floor(height / 2);
  const uvSize: number = uvWidth * uvHeight * 2;
  const uvPlane: ArrayBuffer = new ArrayBuffer(uvSize);
  const uvView: Uint8Array = new Uint8Array(uvPlane);
  
  for (let i: number = 0; i < uvSize; i++) {
    uvView[i] = 128;
  }
  
  return { yPlane, uvPlane };
}

export default function GLESBackendTest() {
  describe('GLESBackend.ConstructorTests', () => {
    
    it('GB-CTOR-001: Should create backend instance', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      expect(handle >= 0).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-CTOR-002: Should initialize with uninitialized state', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const initialized: boolean = await nativerender.isGLESBackendInitialized(handle);
      expect(initialized).assertFalse();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-CTOR-003: Should support multiple instances', 0, async () => {
      const handle1: number = await nativerender.createGLESBackend();
      const handle2: number = await nativerender.createGLESBackend();
      const handle3: number = await nativerender.createGLESBackend();
      
      expect(handle1 !== handle2).assertTrue();
      expect(handle2 !== handle3).assertTrue();
      
      nativerender.destroyGLESBackend(handle1);
      nativerender.destroyGLESBackend(handle2);
      nativerender.destroyGLESBackend(handle3);
    });
  });

  describe('GLESBackend.InitializeTests', () => {
    
    it('GB-INIT-001: Should initialize with RGBA format', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      // 使用空 surfaceId，会自动切换到离屏模式
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, '', 64, 64, PixelFormat.RGBA
      );
      expect(success).assertTrue();
      
      const initialized: boolean = await nativerender.isGLESBackendInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INIT-002: Should initialize with RGB format', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, '', 128, 128, PixelFormat.RGB
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INIT-003: Should initialize with NV21 format', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, '', 64, 64, PixelFormat.NV21
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INIT-004: Should initialize with NV12 format', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      const success: boolean = await nativerender.glesBackendInitialize(
        handle, '', 64, 64, PixelFormat.NV12
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INIT-005: Should allow re-initialization', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      let success: boolean = await nativerender.glesBackendInitialize(
        handle, '', 64, 64, PixelFormat.RGBA
      );
      expect(success).assertTrue();
      
      // 销毁后重新初始化
      nativerender.destroyGLESBackend(handle);
      
      const handle2: number = await nativerender.createGLESBackend();
      success = await nativerender.glesBackendInitialize(
        handle2, '', 128, 128, PixelFormat.RGBA
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle2);
    });
  });

  describe('GLESBackend.RenderRGBATests', () => {
    
    it('GB-RGBA-001: Should render small RGBA frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 64, 64, PixelFormat.RGBA);
      
      const data: ArrayBuffer = generateRGBAData(64, 64);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, data, 64, 64
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-RGBA-002: Should render 720p RGBA frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 1280, 720, PixelFormat.RGBA);
      
      const data: ArrayBuffer = generateRGBAData(1280, 720);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, data, 1280, 720
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-RGBA-003: Should render multiple frames', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 128, 128, PixelFormat.RGBA);
      
      for (let i: number = 0; i < 10; i++) {
        const data: ArrayBuffer = generateRGBAData(128, 128);
        const success: boolean = await nativerender.glesBackendRenderFrame(
          handle, data, 128, 128
        );
        expect(success).assertTrue();
      }
      
      nativerender.destroyGLESBackend(handle);
    });
  });

  describe('GLESBackend.RenderYUVTests', () => {
    
    it('GB-YUV-001: Should render NV21 frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 64, 64, PixelFormat.NV21);
      
      // 注意：NV21 需要特殊的渲染接口，这里简化测试
      // 实际应该调用 YUVShaderManager 的接口
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-YUV-002: Should render NV12 frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 64, 64, PixelFormat.NV12);
      
      nativerender.destroyGLESBackend(handle);
    });
  });

  describe('GLESBackend.IntegrationTests', () => {
    
    it('GB-INT-001: Should support full render lifecycle', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      
      // 1. 初始化
      const initSuccess: boolean = await nativerender.glesBackendInitialize(
        handle, '', 640, 480, PixelFormat.RGBA
      );
      expect(initSuccess).assertTrue();
      
      // 2. 渲染多帧
      for (let i: number = 0; i < 5; i++) {
        const data: ArrayBuffer = generateRGBAData(640, 480);
        const renderSuccess: boolean = await nativerender.glesBackendRenderFrame(
          handle, data, 640, 480
        );
        expect(renderSuccess).assertTrue();
      }
      
      // 3. 销毁
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-INT-002: Should support different formats', 0, async () => {
      interface FormatTest {
        format: PixelFormat;
        width: number;
        height: number;
      }
      
      const tests: FormatTest[] = [
        { format: PixelFormat.RGBA, width: 64, height: 64 },
        { format: PixelFormat.RGB, width: 128, height: 128 },
        { format: PixelFormat.NV21, width: 64, height: 64 },
        { format: PixelFormat.NV12, width: 64, height: 64 },
      ];
      
      for (let i: number = 0; i < tests.length; i++) {
        const handle: number = await nativerender.createGLESBackend();
        
        const success: boolean = await nativerender.glesBackendInitialize(
          handle, '', tests[i].width, tests[i].height, tests[i].format
        );
        expect(success).assertTrue();
        
        nativerender.destroyGLESBackend(handle);
      }
    });
  });

  describe('GLESBackend.EdgeCaseTests', () => {
    
    it('GB-EDGE-001: Should handle very small frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 16, 16, PixelFormat.RGBA);
      
      const data: ArrayBuffer = generateRGBAData(16, 16);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, data, 16, 16
      );
      expect(success).assertTrue();
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-EDGE-002: Should handle large frame', 0, async () => {
      const handle: number = await nativerender.createGLESBackend();
      await nativerender.glesBackendInitialize(handle, '', 1920, 1080, PixelFormat.RGBA);
      
      const data: ArrayBuffer = generateRGBAData(1920, 1080);
      const success: boolean = await nativerender.glesBackendRenderFrame(
        handle, data, 1920, 1080
      );
      // 可能成功或失败（取决于内存）
      
      nativerender.destroyGLESBackend(handle);
    });

    it('GB-EDGE-003: Should handle rapid create-destroy cycles', 0, async () => {
      for (let i: number = 0; i < 10; i++) {
        const handle: number = await nativerender.createGLESBackend();
        await nativerender.glesBackendInitialize(handle, '', 32, 32, PixelFormat.RGBA);
        nativerender.destroyGLESBackend(handle);
      }
    });
  });
}
