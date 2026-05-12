import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * PixelFormatConverter 单元测试
 * 
 * 测试 C++ PixelFormatConverter 类通过 NAPI 桥接的功能
 * 覆盖 GetGLInternalFormat、GetGLFormat、GetBytesPerPixel 三个静态方法
 */
export default function PixelFormatConverterTest() {
  describe('PixelFormatConverter.InternalFormatTests', () => {
    
    // ===== GL 内部格式映射测试 =====
    
    it('PFC-IF-001: Should map RGBA to GL_RGBA', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(0); // RGBA=0
      expect(internalFormat).assertEqual(0x1908); // GL_RGBA
    });

    it('PFC-IF-002: Should map RGB to GL_RGB', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(1); // RGB=1
      expect(internalFormat).assertEqual(0x1907); // GL_RGB
    });

    it('PFC-IF-003: Should map BGRA to GL_RGBA', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(2); // BGRA=2
      expect(internalFormat).assertEqual(0x1908); // GL_RGBA (fallback)
    });

    it('PFC-IF-004: Should handle NV21 format', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(3); // NV21=3
      // NV21 不是标准 OpenGL 格式，应该返回默认值
      expect(internalFormat).assertEqual(0x1908); // GL_RGBA (default)
    });

    it('PFC-IF-005: Should handle NV12 format', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(4); // NV12=4
      expect(internalFormat).assertEqual(0x1908); // GL_RGBA (default)
    });
  });

  describe('PixelFormatConverter.FormatTests', () => {
    
    // ===== GL 像素格式映射测试 =====
    
    it('PFC-FMT-001: Should map RGBA to GL_RGBA', 0, () => {
      const format: number = nativerender.getGLFormat(0); // RGBA=0
      expect(format).assertEqual(0x1908); // GL_RGBA
    });

    it('PFC-FMT-002: Should map RGB to GL_RGB', 0, () => {
      const format: number = nativerender.getGLFormat(1); // RGB=1
      expect(format).assertEqual(0x1907); // GL_RGB
    });

    it('PFC-FMT-003: Should map BGRA to GL_BGRA_EXT or GL_RGBA', 0, () => {
      const format: number = nativerender.getGLFormat(2); // BGRA=2
      // BGRA 可能映射到 GL_BGRA_EXT (0x80E1) 或 fallback 到 GL_RGBA (0x1908)
      const isBGRA: boolean = (format === 0x80E1) || (format === 0x1908);
      expect(isBGRA).assertTrue();
    });

    it('PFC-FMT-004: Should handle NV21 format', 0, () => {
      const format: number = nativerender.getGLFormat(3); // NV21=3
      expect(format).assertEqual(0x1908); // GL_RGBA (default)
    });

    it('PFC-FMT-005: Should handle invalid format', 0, () => {
      const format: number = nativerender.getGLFormat(99); // 无效格式
      expect(format).assertEqual(0x1908); // GL_RGBA (default)
    });
  });

  describe('PixelFormatConverter.BytesPerPixelTests', () => {
    
    // ===== 每像素字节数测试 =====
    
    it('PFC-BPP-001: RGBA should be 4 bytes per pixel', 0, () => {
      const bytes: number = nativerender.getBytesPerPixel(0); // RGBA=0
      expect(bytes).assertEqual(4);
    });

    it('PFC-BPP-002: RGB should be 3 bytes per pixel', 0, () => {
      const bytes: number = nativerender.getBytesPerPixel(1); // RGB=1
      expect(bytes).assertEqual(3);
    });

    it('PFC-BPP-003: BGRA should be 4 bytes per pixel', 0, () => {
      const bytes: number = nativerender.getBytesPerPixel(2); // BGRA=2
      expect(bytes).assertEqual(4);
    });

    it('PFC-BPP-004: NV21 should return default 4 bytes', 0, () => {
      const bytes: number = nativerender.getBytesPerPixel(3); // NV21=3
      // NV21 实际是 1.5 字节/像素，但当前实现返回默认值 4
      expect(bytes).assertEqual(4);
    });

    it('PFC-BPP-005: NV12 should return default 4 bytes', 0, () => {
      const bytes: number = nativerender.getBytesPerPixel(4); // NV12=4
      expect(bytes).assertEqual(4);
    });

    it('PFC-BPP-006: Invalid format should return default 4 bytes', 0, () => {
      const bytes: number = nativerender.getBytesPerPixel(99); // 无效格式
      expect(bytes).assertEqual(4);
    });
  });

  describe('PixelFormatConverter.ConsistencyTests', () => {
    
    // ===== 一致性测试 =====
    
    it('PFC-CON-001: RGBA and BGRA should have same bytes per pixel', 0, () => {
      const rgbaBytes: number = nativerender.getBytesPerPixel(0);
      const bgraBytes: number = nativerender.getBytesPerPixel(2);
      expect(rgbaBytes).assertEqual(bgraBytes);
    });

    it('PFC-CON-002: RGB should have fewer bytes than RGBA', 0, () => {
      const rgbBytes: number = nativerender.getBytesPerPixel(1);
      const rgbaBytes: number = nativerender.getBytesPerPixel(0);
      expect(rgbBytes).assertLess(rgbaBytes);
    });

    it('PFC-CON-003: All formats should return valid OpenGL constants', 0, () => {
      const formats: number[] = [0, 1, 2, 3, 4];
      
      formats.forEach((format: number) => {
        const internalFormat: number = nativerender.getGLInternalFormat(format);
        const glFormat: number = nativerender.getGLFormat(format);
        
        // OpenGL 常量应该是正数
        expect(internalFormat > 0).assertTrue();
        expect(glFormat > 0).assertTrue();
      });
    });

    it('PFC-CON-004: Format mapping should be deterministic', 0, () => {
      // 多次调用应该返回相同结果
      const results1: number = nativerender.getGLInternalFormat(0);
      const results2: number = nativerender.getGLInternalFormat(0);
      const results3: number = nativerender.getGLInternalFormat(0);
      
      expect(results1).assertEqual(results2);
      expect(results2).assertEqual(results3);
    });
  });

  describe('PixelFormatConverter.EdgeCaseTests', () => {
    
    // ===== 边界情况测试 =====
    
    it('PFC-EDGE-001: Should handle zero format', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(0);
      const glFormat: number = nativerender.getGLFormat(0);
      const bytes: number = nativerender.getBytesPerPixel(0);
      
      expect(internalFormat > 0).assertTrue();
      expect(glFormat > 0).assertTrue();
      expect(bytes > 0).assertTrue();
    });

    it('PFC-EDGE-002: Should handle negative format gracefully', 0, () => {
      // 负数格式应该返回默认值
      const internalFormat: number = nativerender.getGLInternalFormat(-1);
      expect(internalFormat).assertEqual(0x1908); // GL_RGBA (default)
    });

    it('PFC-EDGE-003: Should handle very large format value', 0, () => {
      const internalFormat: number = nativerender.getGLInternalFormat(9999);
      expect(internalFormat).assertEqual(0x1908); // GL_RGBA (default)
    });
  });
}
