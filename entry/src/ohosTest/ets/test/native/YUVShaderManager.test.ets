import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * YUVShaderManager 单元测试
 * 
 * 测试 C++ YUVShaderManager 类通过 NAPI 桥接的功能
 * 覆盖 Shader 编译、NV21/NV12 渲染、双纹理管理等核心功能
 * 
 * ⭐ 使用离屏 EGL 上下文（Pbuffer）进行真正的 GL 功能测试
 * ⭐ 验证 GPU Shader YUV→RGB 转换能力
 */

/**
 * 辅助类：管理 EGL 上下文生命周期
 */
class EGLContextHelper {
  private eglHandle: number = -1;
  
  async setup(width: number = 64, height: number = 64): Promise<void> {
    this.eglHandle = await nativerender.createEGLContext(width, height);
    if (this.eglHandle < 0) {
      throw new Error('Failed to create EGL context');
    }
  }
  
  async teardown(): Promise<void> {
    if (this.eglHandle >= 0) {
      await nativerender.destroyEGLContext(this.eglHandle);
      this.eglHandle = -1;
    }
  }
}

/**
 * 生成模拟的 YUV 数据（NV21 格式）
 * @param width 宽度
 * @param height 高度
 * @returns { yPlane: ArrayBuffer, uvPlane: ArrayBuffer }
 */
interface YUVData {
  yPlane: ArrayBuffer;
  uvPlane: ArrayBuffer;
}

function generateNV21Data(width: number, height: number): YUVData {
  // Y 平面：width x height
  const ySize: number = width * height;
  const yPlane: ArrayBuffer = new ArrayBuffer(ySize);
  const yView: Uint8Array = new Uint8Array(yPlane);
  
  // 填充 Y 数据（灰度渐变）
  for (let i: number = 0; i < ySize; i++) {
    yView[i] = Math.floor((i / ySize) * 255);
  }
  
  // UV 平面：(width/2) x (height/2) x 2 (VU 交错)
  const uvWidth: number = Math.floor(width / 2);
  const uvHeight: number = Math.floor(height / 2);
  const uvSize: number = uvWidth * uvHeight * 2;
  const uvPlane: ArrayBuffer = new ArrayBuffer(uvSize);
  const uvView: Uint8Array = new Uint8Array(uvPlane);
  
  // 填充 UV 数据（中性值 128）
  for (let i: number = 0; i < uvSize; i++) {
    uvView[i] = 128;
  }
  
  return { yPlane, uvPlane };
}

/**
 * 生成模拟的 YUV 数据（NV12 格式）
 * @param width 宽度
 * @param height 高度
 * @returns { yPlane: ArrayBuffer, uvPlane: ArrayBuffer }
 */
function generateNV12Data(width: number, height: number): YUVData {
  // NV12 和 NV21 的数据结构相同，只是 UV 通道顺序不同
  return generateNV21Data(width, height);
}

export default function YUVShaderManagerTest() {
  describe('YUVShaderManager.ConstructorTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('YUV-CTOR-001: Should create shader manager instance', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      expect(handle >= 0).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-CTOR-002: Should initialize with uninitialized state', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      
      const initialized: boolean = await nativerender.isYUVShaderInitialized(handle);
      expect(initialized).assertFalse();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-CTOR-003: Should support multiple instances', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(256, 256);
      
      const handle1: number = await nativerender.createYUVShaderManager();
      const handle2: number = await nativerender.createYUVShaderManager();
      const handle3: number = await nativerender.createYUVShaderManager();
      
      expect(handle1 !== handle2).assertTrue();
      expect(handle2 !== handle3).assertTrue();
      expect(handle1 !== handle3).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle1);
      nativerender.destroyYUVShaderManager(handle2);
      nativerender.destroyYUVShaderManager(handle3);
      await eglHelper.teardown();
    });
  });

  describe('YUVShaderManager.InitializeTests', () => {
    
    // ===== 初始化测试 =====
    
    it('YUV-INIT-001: Should initialize shader successfully', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      
      const success: boolean = await nativerender.yuvShaderInitialize(handle);
      expect(success).assertTrue();
      
      const initialized: boolean = await nativerender.isYUVShaderInitialized(handle);
      expect(initialized).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-INIT-002: Should allow re-initialization', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      
      // 第一次初始化
      let success: boolean = await nativerender.yuvShaderInitialize(handle);
      expect(success).assertTrue();
      
      // 第二次初始化（应该也成功）
      success = await nativerender.yuvShaderInitialize(handle);
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-INIT-003: Should fail render before initialization', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      
      // 未初始化就尝试渲染
      const data = generateNV21Data(64, 64);
      const success: boolean = await nativerender.yuvShaderRenderNV21(
        handle, data.yPlane, data.uvPlane, 64, 64
      );
      expect(success).assertFalse();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('YUVShaderManager.RenderNV21Tests', () => {
    
    // ===== NV21 渲染测试 =====
    
    it('YUV-NV21-001: Should render small NV21 frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV21Data(64, 64);
      const success: boolean = await nativerender.yuvShaderRenderNV21(
        handle, data.yPlane, data.uvPlane, 64, 64
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-NV21-002: Should render 720p NV21 frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(1280, 720);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV21Data(1280, 720);
      const success: boolean = await nativerender.yuvShaderRenderNV21(
        handle, data.yPlane, data.uvPlane, 1280, 720
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-NV21-003: Should render 1080p NV21 frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(1920, 1080);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV21Data(1920, 1080);
      const success: boolean = await nativerender.yuvShaderRenderNV21(
        handle, data.yPlane, data.uvPlane, 1920, 1080
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-NV21-004: Should handle multiple renders', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(128, 128);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      // 连续渲染 10 帧
      for (let i: number = 0; i < 10; i++) {
        const data = generateNV21Data(128, 128);
        const success: boolean = await nativerender.yuvShaderRenderNV21(
          handle, data.yPlane, data.uvPlane, 128, 128
        );
        expect(success).assertTrue();
      }
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-NV21-005: Should handle different resolutions', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(640, 480);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      interface Resolution {
        width: number;
        height: number;
      }
      
      const resolutions: Resolution[] = [
        { width: 320, height: 240 },
        { width: 640, height: 480 },
        { width: 1280, height: 720 },
      ];
      
      for (let i: number = 0; i < resolutions.length; i++) {
        const data = generateNV21Data(resolutions[i].width, resolutions[i].height);
        const success: boolean = await nativerender.yuvShaderRenderNV21(
          handle, data.yPlane, data.uvPlane, resolutions[i].width, resolutions[i].height
        );
        expect(success).assertTrue();
      }
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('YUVShaderManager.RenderNV12Tests', () => {
    
    // ===== NV12 渲染测试 =====
    
    it('YUV-NV12-001: Should render small NV12 frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV12Data(64, 64);
      const success: boolean = await nativerender.yuvShaderRenderNV12(
        handle, data.yPlane, data.uvPlane, 64, 64
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-NV12-002: Should render 1080p NV12 frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(1920, 1080);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV12Data(1920, 1080);
      const success: boolean = await nativerender.yuvShaderRenderNV12(
        handle, data.yPlane, data.uvPlane, 1920, 1080
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-NV12-003: Should handle multiple NV12 renders', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(256, 256);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      // 连续渲染 5 帧
      for (let i: number = 0; i < 5; i++) {
        const data = generateNV12Data(256, 256);
        const success: boolean = await nativerender.yuvShaderRenderNV12(
          handle, data.yPlane, data.uvPlane, 256, 256
        );
        expect(success).assertTrue();
      }
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('YUVShaderManager.IntegrationTests', () => {
    
    // ===== 集成场景测试 =====
    
    it('YUV-INT-001: Should support full render lifecycle', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(640, 480);
      
      const handle: number = await nativerender.createYUVShaderManager();
      
      // 1. 初始化
      const initSuccess: boolean = await nativerender.yuvShaderInitialize(handle);
      expect(initSuccess).assertTrue();
      
      // 2. 渲染多帧
      for (let i: number = 0; i < 3; i++) {
        const data = generateNV21Data(640, 480);
        const renderSuccess: boolean = await nativerender.yuvShaderRenderNV21(
          handle, data.yPlane, data.uvPlane, 640, 480
        );
        expect(renderSuccess).assertTrue();
      }
      
      // 3. 销毁
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-INT-002: Should support mixed NV21 and NV12 rendering', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(128, 128);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      // 交替渲染 NV21 和 NV12
      for (let i: number = 0; i < 4; i++) {
        if (i % 2 === 0) {
          const data = generateNV21Data(128, 128);
          const success: boolean = await nativerender.yuvShaderRenderNV21(
            handle, data.yPlane, data.uvPlane, 128, 128
          );
          expect(success).assertTrue();
        } else {
          const data = generateNV12Data(128, 128);
          const success: boolean = await nativerender.yuvShaderRenderNV12(
            handle, data.yPlane, data.uvPlane, 128, 128
          );
          expect(success).assertTrue();
        }
      }
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-INT-003: Should support multiple shaders independently', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(256, 256);
      
      const handle1: number = await nativerender.createYUVShaderManager();
      const handle2: number = await nativerender.createYUVShaderManager();
      
      await nativerender.yuvShaderInitialize(handle1);
      await nativerender.yuvShaderInitialize(handle2);
      
      // 分别渲染
      const data1 = generateNV21Data(128, 128);
      const data2 = generateNV21Data(256, 256);
      
      const success1: boolean = await nativerender.yuvShaderRenderNV21(
        handle1, data1.yPlane, data1.uvPlane, 128, 128
      );
      const success2: boolean = await nativerender.yuvShaderRenderNV21(
        handle2, data2.yPlane, data2.uvPlane, 256, 256
      );
      
      expect(success1).assertTrue();
      expect(success2).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle1);
      nativerender.destroyYUVShaderManager(handle2);
      await eglHelper.teardown();
    });
  });

  describe('YUVShaderManager.EdgeCaseTests', () => {
    
    // ===== 边界情况测试 =====
    
    it('YUV-EDGE-001: Should handle very small frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(16, 16);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV21Data(16, 16);
      const success: boolean = await nativerender.yuvShaderRenderNV21(
        handle, data.yPlane, data.uvPlane, 16, 16
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-EDGE-002: Should handle non-square frame', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(1920, 1080);
      
      const handle: number = await nativerender.createYUVShaderManager();
      await nativerender.yuvShaderInitialize(handle);
      
      const data = generateNV21Data(1920, 1080);
      const success: boolean = await nativerender.yuvShaderRenderNV21(
        handle, data.yPlane, data.uvPlane, 1920, 1080
      );
      expect(success).assertTrue();
      
      nativerender.destroyYUVShaderManager(handle);
      await eglHelper.teardown();
    });

    it('YUV-EDGE-003: Should handle rapid initialization cycles', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      // 快速循环 10 次：初始化-销毁
      for (let i: number = 0; i < 10; i++) {
        const handle: number = await nativerender.createYUVShaderManager();
        await nativerender.yuvShaderInitialize(handle);
        nativerender.destroyYUVShaderManager(handle);
      }
      
      await eglHelper.teardown();
    });
  });
}
