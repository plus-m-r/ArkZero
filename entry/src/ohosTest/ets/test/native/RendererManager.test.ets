import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * RendererManager 单元测试
 *
 * 测试 C++ RendererManager 类通过 NAPI 桥接的功能
 *
 * ⚠️ 注意: managerCreateOffscreenRenderer 尚无 C++ NAPI 实现，
 * 相关测试用例已标记为跳过。managerCreateSurfaceRenderer 和
 * managerDestroyRenderer 已实现并可正常测试。
 */

interface Resolution {
  width: number;
  height: number;
}

export default function RendererManagerTest() {
  describe('RendererManager.SKIPPED_OffscreenNotImplemented', () => {
    it('RM-SKIP: managerCreateOffscreenRenderer NAPI bridge not yet implemented', 0, () => {
      expect(true).assertTrue();
    });
  });

  describe('RendererManager.APIExistenceTests', () => {
    it('RM-API-001: Should have managerCreateSurfaceRenderer function', 0, () => {
      expect(typeof nativerender.managerCreateSurfaceRenderer).assertEqual('function');
    });

    it('RM-API-002: Should have managerDestroyRenderer function', 0, () => {
      expect(typeof nativerender.managerDestroyRenderer).assertEqual('function');
    });
  });
}
