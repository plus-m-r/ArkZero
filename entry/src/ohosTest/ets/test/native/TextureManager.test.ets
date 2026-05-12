import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * TextureManager 单元测试
 * 
 * 测试 C++ TextureManager 类通过 NAPI 桥接的功能
 * 覆盖纹理创建、更新、销毁等核心功能
 * 
 * ⭐ 使用离屏 EGL 上下文（Pbuffer）进行真正的 GL 功能测试
 */

// 定义接口类型
interface TextureInfo {
  textureId: number;
  isCreated: boolean;
}

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

export default function TextureManagerTest() {
  describe('TextureManager.ConstructorTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('TM-CTOR-001: Should create manager instance', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      expect(handle >= 0).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-CTOR-002: Should initialize with no texture', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      const created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertFalse();
      
      const textureId: number = await nativerender.getTextureId(handle);
      expect(textureId).assertEqual(0);
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-CTOR-003: Should support multiple instances', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle1: number = await nativerender.createTextureManager();
      const handle2: number = await nativerender.createTextureManager();
      const handle3: number = await nativerender.createTextureManager();
      
      expect(handle1 !== handle2).assertTrue();
      expect(handle2 !== handle3).assertTrue();
      expect(handle1 !== handle3).assertTrue();
      
      nativerender.destroyTextureManager(handle1);
      nativerender.destroyTextureManager(handle2);
      nativerender.destroyTextureManager(handle3);
      await eglHelper.teardown();
    });
  });

  describe('TextureManager.CreateTests', () => {
    
    // ===== 纹理创建测试 =====
    
    it('TM-CRT-001: Should create RGBA texture', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // GL_RGBA = 0x1908
      const success: boolean = await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      expect(success).assertTrue();
      
      // 验证纹理已创建
      const created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertTrue();
      
      const textureId: number = await nativerender.getTextureId(handle);
      expect(textureId > 0).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-CRT-002: Should create RGB texture', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(128, 128);
      
      const handle: number = await nativerender.createTextureManager();
      
      // GL_RGB = 0x1907
      const success: boolean = await nativerender.textureCreate(handle, 128, 128, 0x1907, 0x1907);
      expect(success).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-CRT-003: Should create different sizes', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(512, 512);
      
      const handle: number = await nativerender.createTextureManager();
      
      interface Size {
        width: number;
        height: number;
      }
      
      const sizes: Size[] = [
        { width: 32, height: 32 },
        { width: 256, height: 256 },
        { width: 512, height: 512 },
      ];
      
      for (let i: number = 0; i < sizes.length; i++) {
        await nativerender.textureCreate(
          handle, 
          sizes[i].width, 
          sizes[i].height, 
          0x1908, 
          0x1908
        );
      }
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-CRT-004: Should handle zero dimensions', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 零尺寸应该失败或返回 false
      const success: boolean = await nativerender.textureCreate(handle, 0, 0, 0x1908, 0x1908);
      // 预期会失败
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-CRT-005: Should handle large dimensions', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(3840, 2160);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 大尺寸（4K）
      const success: boolean = await nativerender.textureCreate(handle, 3840, 2160, 0x1908, 0x1908);
      // 可能成功或失败（取决于 GPU 支持）
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('TextureManager.UpdateTests', () => {
    
    // ===== 纹理更新测试 =====
    
    it('TM-UPD-001: Should update texture with valid data', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 先创建纹理
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      // 准备像素数据（RGBA，64x64 = 16384 字节）
      const bufferSize: number = 64 * 64 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      
      // 更新纹理
      const success: boolean = await nativerender.textureUpdate(handle, buffer, 64, 64, 0x1908);
      expect(success).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-UPD-002: Should reject empty data', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 先创建纹理
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      // 尝试用空缓冲区更新
      const emptyBuffer: ArrayBuffer = new ArrayBuffer(0);
      const success: boolean = await nativerender.textureUpdate(handle, emptyBuffer, 64, 64, 0x1908);
      // 预期会失败
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-UPD-003: Should handle different formats', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 创建 RGBA 纹理
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      const bufferSize: number = 64 * 64 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      
      // 用不同格式更新（可能失败，但不应崩溃）
      await nativerender.textureUpdate(handle, buffer, 64, 64, 0x1907); // GL_RGB
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-UPD-004: Should update multiple times', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 先创建纹理
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      const bufferSize: number = 64 * 64 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      
      // 多次更新
      for (let i: number = 0; i < 10; i++) {
        const success: boolean = await nativerender.textureUpdate(handle, buffer, 64, 64, 0x1908);
        expect(success).assertTrue();
      }
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('TextureManager.DestroyTests', () => {
    
    // ===== 纹理销毁测试 =====
    
    it('TM-DST-001: Should destroy texture', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 先创建纹理
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      // 销毁纹理
      await nativerender.textureDestroy(handle);
      
      const created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertFalse();
      
      const textureId: number = await nativerender.getTextureId(handle);
      expect(textureId).assertEqual(0);
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-DST-002: Should allow multiple destroy calls', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 先创建纹理
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      // 多次销毁（不应崩溃）
      await nativerender.textureDestroy(handle);
      await nativerender.textureDestroy(handle);
      await nativerender.textureDestroy(handle);
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-DST-003: Should destroy without creating', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 未创建就销毁（不应崩溃）
      await nativerender.textureDestroy(handle);
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('TextureManager.StateTests', () => {
    
    // ===== 状态查询测试 =====
    
    it('TM-STT-001: Should return correct texture ID after create', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 初始状态
      let textureId: number = await nativerender.getTextureId(handle);
      expect(textureId).assertEqual(0);
      
      // 创建后
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      textureId = await nativerender.getTextureId(handle);
      expect(textureId > 0).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-STT-002: Should track creation state correctly', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 初始状态
      let created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertFalse();
      
      // 创建后
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      created = await nativerender.isTextureCreated(handle);
      expect(created).assertTrue();
      
      // 销毁后
      await nativerender.textureDestroy(handle);
      created = await nativerender.isTextureCreated(handle);
      expect(created).assertFalse();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-STT-003: Should maintain state across operations', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(64, 64);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 创建
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      
      // 更新
      const bufferSize: number = 64 * 64 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      await nativerender.textureUpdate(handle, buffer, 64, 64, 0x1908);
      
      // 状态应该保持不变
      const created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertTrue();
      
      const textureId: number = await nativerender.getTextureId(handle);
      expect(textureId > 0).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });
  });

  describe('TextureManager.IntegrationTests', () => {
    
    // ===== 集成场景测试 =====
    
    it('TM-INT-001: Should support create-update-destroy lifecycle', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(128, 128);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 1. 创建
      const createSuccess: boolean = await nativerender.textureCreate(handle, 128, 128, 0x1908, 0x1908);
      expect(createSuccess).assertTrue();
      
      // 2. 更新
      const bufferSize: number = 128 * 128 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      const updateSuccess: boolean = await nativerender.textureUpdate(handle, buffer, 128, 128, 0x1908);
      expect(updateSuccess).assertTrue();
      
      // 3. 销毁
      await nativerender.textureDestroy(handle);
      
      // 验证状态
      const created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertFalse();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-INT-002: Should support recreate after destroy', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(128, 128);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 第一次创建-销毁
      await nativerender.textureCreate(handle, 64, 64, 0x1908, 0x1908);
      await nativerender.textureDestroy(handle);
      
      // 第二次创建
      const success: boolean = await nativerender.textureCreate(handle, 128, 128, 0x1908, 0x1908);
      expect(success).assertTrue();
      
      const created: boolean = await nativerender.isTextureCreated(handle);
      expect(created).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-INT-003: Should handle multiple managers independently', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(128, 128);
      
      const handle1: number = await nativerender.createTextureManager();
      const handle2: number = await nativerender.createTextureManager();
      
      // 分别操作
      await nativerender.textureCreate(handle1, 64, 64, 0x1908, 0x1908);
      await nativerender.textureCreate(handle2, 128, 128, 0x1908, 0x1908);
      
      const id1: number = await nativerender.getTextureId(handle1);
      const id2: number = await nativerender.getTextureId(handle2);
      
      // 两个纹理ID都应该有效且不同
      expect(id1 > 0).assertTrue();
      expect(id2 > 0).assertTrue();
      expect(id1 !== id2).assertTrue();
      
      nativerender.destroyTextureManager(handle1);
      nativerender.destroyTextureManager(handle2);
      await eglHelper.teardown();
    });
  });

  describe('TextureManager.EdgeCaseTests', () => {
    
    // ===== 边界情况测试 =====
    
    it('TM-EDGE-001: Should handle very small texture', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(1, 1);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 1x1 纹理
      const success: boolean = await nativerender.textureCreate(handle, 1, 1, 0x1908, 0x1908);
      expect(success).assertTrue();
      
      const bufferSize: number = 1 * 1 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      await nativerender.textureUpdate(handle, buffer, 1, 1, 0x1908);
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-EDGE-002: Should handle non-square texture', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(1920, 1080);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 宽屏 1920x1080
      const success: boolean = await nativerender.textureCreate(handle, 1920, 1080, 0x1908, 0x1908);
      expect(success).assertTrue();
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });

    it('TM-EDGE-003: Should handle rapid create-destroy cycles', 0, async () => {
      const eglHelper = new EGLContextHelper();
      await eglHelper.setup(32, 32);
      
      const handle: number = await nativerender.createTextureManager();
      
      // 快速循环 20 次
      for (let i: number = 0; i < 20; i++) {
        await nativerender.textureCreate(handle, 32, 32, 0x1908, 0x1908);
        await nativerender.textureDestroy(handle);
      }
      
      nativerender.destroyTextureManager(handle);
      await eglHelper.teardown();
    });
  });
}
