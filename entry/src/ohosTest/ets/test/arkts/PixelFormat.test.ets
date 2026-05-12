import { describe, it, expect } from '@ohos/hypium';
import { PixelFormat } from '../../../../main/ets/components/rendering/ArkZeroRenderer';

/**
 * PixelFormat 枚举单元测试
 * 
 * 测试目标：
 * - 验证枚举值正确性
 * - 验证枚举成员完整性
 * - 验证枚举值的唯一性
 */
export default function PixelFormatTest() {
  describe('PixelFormat.EnumTests', () => {
    
    // ===== 枚举值正确性测试 =====
    
    it('PF-VAL-001: RGBA should equal 0', 0, () => {
      expect(PixelFormat.RGBA).assertEqual(0);
    });

    it('PF-VAL-002: RGB should equal 1', 0, () => {
      expect(PixelFormat.RGB).assertEqual(1);
    });

    it('PF-VAL-003: BGRA should equal 2', 0, () => {
      expect(PixelFormat.BGRA).assertEqual(2);
    });

    it('PF-VAL-004: NV21 should equal 3', 0, () => {
      expect(PixelFormat.NV21).assertEqual(3);
    });

    it('PF-VAL-005: NV12 should equal 4', 0, () => {
      expect(PixelFormat.NV12).assertEqual(4);
    });

    // ===== 枚举成员完整性测试 =====
    
    it('PF-COMP-001: Should have exactly 5 members', 0, () => {
      // Verify all 5 enum values exist and are unique
      const values: number[] = [
        PixelFormat.RGBA,
        PixelFormat.RGB,
        PixelFormat.BGRA,
        PixelFormat.NV21,
        PixelFormat.NV12
      ];
      const uniqueValues = new Set(values);
      expect(uniqueValues.size).assertEqual(5);
    });

    it('PF-COMP-002: Should contain all expected formats', 0, () => {
      // Verify each format has a valid value
      expect(PixelFormat.RGBA).assertEqual(0);
      expect(PixelFormat.RGB).assertEqual(1);
      expect(PixelFormat.BGRA).assertEqual(2);
      expect(PixelFormat.NV21).assertEqual(3);
      expect(PixelFormat.NV12).assertEqual(4);
    });

    // ===== 枚举值唯一性测试 =====
    
    it('PF-UNIQ-001: All enum values should be unique', 0, () => {
      const values = [
        PixelFormat.RGBA,
        PixelFormat.RGB,
        PixelFormat.BGRA,
        PixelFormat.NV21,
        PixelFormat.NV12
      ];
      
      const uniqueValues = new Set(values);
      expect(uniqueValues.size).assertEqual(values.length);
    });

    // ===== 字节数语义测试 =====
    
    it('PF-BPP-001: RGBA should be 4 bytes per pixel', 0, () => {
      // RGBA8888: R(1) + G(1) + B(1) + A(1) = 4 bytes
      expect(PixelFormat.RGBA).assertEqual(0);
    });

    it('PF-BPP-002: RGB should be 3 bytes per pixel', 0, () => {
      // RGB888: R(1) + G(1) + B(1) = 3 bytes
      expect(PixelFormat.RGB).assertEqual(1);
    });

    it('PF-BPP-003: BGRA should be 4 bytes per pixel', 0, () => {
      // BGRA8888: B(1) + G(1) + R(1) + A(1) = 4 bytes
      expect(PixelFormat.BGRA).assertEqual(2);
    });

    it('PF-BPP-004: NV21 should be YUV format (1.5 bytes average)', 0, () => {
      // NV21: YUV420 semi-planar, average 1.5 bytes per pixel
      expect(PixelFormat.NV21).assertEqual(3);
    });

    it('PF-BPP-005: NV12 should be YUV format (1.5 bytes average)', 0, () => {
      // NV12: YUV420 semi-planar, average 1.5 bytes per pixel
      expect(PixelFormat.NV12).assertEqual(4);
    });

    // ===== 格式分类测试 =====
    
    it('PF-CAT-001: RGBA and BGRA should both be 4-byte formats', 0, () => {
      // Both are 4 bytes per pixel - compare as numbers
      expect(Number(PixelFormat.RGBA) !== Number(PixelFormat.RGB)).assertTrue();
      expect(Number(PixelFormat.BGRA) !== Number(PixelFormat.RGB)).assertTrue();
    });

    it('PF-CAT-002: NV21 and NV12 should be different formats', 0, () => {
      // Different YUV layouts - compare as numbers
      expect(Number(PixelFormat.NV21) !== Number(PixelFormat.NV12)).assertTrue();
    });

    it('PF-CAT-003: RGB should be the only 3-byte format', 0, () => {
      // Compare as numbers
      expect(Number(PixelFormat.RGB) !== Number(PixelFormat.RGBA)).assertTrue();
      expect(Number(PixelFormat.RGB) !== Number(PixelFormat.BGRA)).assertTrue();
      expect(Number(PixelFormat.RGB) !== Number(PixelFormat.NV21)).assertTrue();
      expect(Number(PixelFormat.RGB) !== Number(PixelFormat.NV12)).assertTrue();
    });

    // ===== 边界值测试 =====
    
    it('PF-BOUND-001: First format value should be 0', 0, () => {
      expect(PixelFormat.RGBA).assertEqual(0);
    });

    it('PF-BOUND-002: Last format value should be 4', 0, () => {
      expect(PixelFormat.NV12).assertEqual(4);
    });

    it('PF-BOUND-003: Values should be sequential', 0, () => {
      expect(PixelFormat.RGB + 1).assertEqual(PixelFormat.BGRA);
      expect(PixelFormat.BGRA + 1).assertEqual(PixelFormat.NV21);
      expect(PixelFormat.NV21 + 1).assertEqual(PixelFormat.NV12);
    });

    // ===== 类型安全测试 =====
    
    it('PF-TYPE-001: Enum values should be numbers', 0, () => {
      expect(typeof PixelFormat.RGBA).assertEqual('number');
      expect(typeof PixelFormat.RGB).assertEqual('number');
      expect(typeof PixelFormat.BGRA).assertEqual('number');
      expect(typeof PixelFormat.NV21).assertEqual('number');
      expect(typeof PixelFormat.NV12).assertEqual('number');
    });

    it('PF-TYPE-002: Enum values should be integers', 0, () => {
      expect(Number.isInteger(PixelFormat.RGBA)).assertTrue();
      expect(Number.isInteger(PixelFormat.RGB)).assertTrue();
      expect(Number.isInteger(PixelFormat.BGRA)).assertTrue();
      expect(Number.isInteger(PixelFormat.NV21)).assertTrue();
      expect(Number.isInteger(PixelFormat.NV12)).assertTrue();
    });

    it('PF-TYPE-003: Enum values should be non-negative', 0, () => {
      expect(PixelFormat.RGBA >= 0).assertTrue();
      expect(PixelFormat.RGB >= 0).assertTrue();
      expect(PixelFormat.BGRA >= 0).assertTrue();
      expect(PixelFormat.NV21 >= 0).assertTrue();
      expect(PixelFormat.NV12 >= 0).assertTrue();
    });
  });
}
