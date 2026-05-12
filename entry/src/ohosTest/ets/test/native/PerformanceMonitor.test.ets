import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * PerformanceMonitor 统计数据接口
 */
interface MonitorStats {
  totalFrames: number;
  droppedFrames: number;
  fps: number;
  dropRate: number;
  frameTimeMs: number;
}

/**
 * PerformanceMonitor 单元测试
 * 
 * 测试 C++ PerformanceMonitor 类通过 NAPI 桥接的功能
 * 覆盖构造函数、BeginFrame/EndFrame、Reset 等核心功能
 */
export default function PerformanceMonitorTest() {
  describe('PerformanceMonitor.ConstructorTests', () => {
    it('PM-CTOR-001: Should initialize with zero frames', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      
      expect(stats.totalFrames).assertEqual(0);
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-002: Should initialize with zero dropped frames', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      
      expect(stats.droppedFrames).assertEqual(0);
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-003: Should initialize with zero FPS', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      
      expect(stats.fps).assertEqual(0.0);
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-004: Should initialize with zero drop rate', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      
      expect(stats.dropRate).assertEqual(0.0);
      nativerender.destroyPerformanceMonitor(handle);
    });
  });

  describe('PerformanceMonitor.FrameRecordingTests', () => {
    it('PM-BF-001: Should record normal frame', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      await nativerender.monitorBeginFrame(handle);
      await nativerender.monitorEndFrame(handle, false);
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(1);
      expect(stats.droppedFrames).assertEqual(0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-002: Should record dropped frame', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      await nativerender.monitorBeginFrame(handle);
      await nativerender.monitorEndFrame(handle, true);
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(1);
      expect(stats.droppedFrames).assertEqual(1);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-003: Should count multiple frames correctly', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      for (let i = 0; i < 100; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, i % 10 === 0);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(100);
      expect(stats.droppedFrames).assertEqual(10);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-004: Should calculate FPS accurately', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 模拟 60fps，每帧间隔 16ms
      for (let i = 0; i < 60; i++) {
        await nativerender.monitorBeginFrame(handle);
        await new Promise<void>((resolve: Function) => setTimeout(resolve, 16));
        await nativerender.monitorEndFrame(handle, false);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.fps).assertLarger(55.0);
      expect(stats.fps).assertLess(65.0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-BF-005: Should calculate drop rate accurately', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 90 正常帧 + 10 丢帧
      for (let i = 0; i < 90; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      for (let i = 0; i < 10; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, true);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.dropRate).assertLarger(0.09);
      expect(stats.dropRate).assertLess(0.11);
      
      nativerender.destroyPerformanceMonitor(handle);
    });
  });

  describe('PerformanceMonitor.ResetTests', () => {
    it('PM-RST-001: Should reset all statistics', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 先积累数据
      for (let i = 0; i < 50; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, i < 10);
      }
      
      // 重置
      await nativerender.monitorReset(handle);
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      
      expect(stats.totalFrames).assertEqual(0);
      expect(stats.droppedFrames).assertEqual(0);
      expect(stats.fps).assertEqual(0.0);
      expect(stats.dropRate).assertEqual(0.0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-RST-002: Should allow reuse after reset', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 第一轮
      for (let i = 0; i < 30; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      
      // 重置
      await nativerender.monitorReset(handle);
      
      // 第二轮
      for (let i = 0; i < 20; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(20); // 应该是20，不是50
      
      nativerender.destroyPerformanceMonitor(handle);
    });
  });

  describe('PerformanceMonitor.AdvancedTests', () => {
    it('PM-ADV-001: Should handle rapid frame recording', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 快速记录 200 帧
      for (let i = 0; i < 200; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, false);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(200);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-ADV-002: Should calculate average frame time', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 模拟稳定 16ms 帧时间
      for (let i = 0; i < 60; i++) {
        await nativerender.monitorBeginFrame(handle);
        await new Promise<void>((resolve: Function) => setTimeout(resolve, 16));
        await nativerender.monitorEndFrame(handle, false);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.frameTimeMs).assertLarger(15.0);
      expect(stats.frameTimeMs).assertLess(17.0);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-ADV-003: Should handle alternating dropped frames', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 交替正常帧和丢帧
      for (let i = 0; i < 100; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, i % 2 === 0);
      }
      
      const stats: MonitorStats = await nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(100);
      expect(stats.droppedFrames).assertEqual(50);
      expect(stats.dropRate).assertLarger(0.45);
      expect(stats.dropRate).assertLess(0.55);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-ADV-004: Should maintain stats consistency', 0, async () => {
      const handle: number = await nativerender.createPerformanceMonitor();
      
      // 记录一些帧
      for (let i = 0; i < 50; i++) {
        await nativerender.monitorBeginFrame(handle);
        await nativerender.monitorEndFrame(handle, i < 5);
      }
      
      // 多次获取统计，应该一致
      const stats1: MonitorStats = await nativerender.getMonitorStats(handle);
      const stats2: MonitorStats = await nativerender.getMonitorStats(handle);
      const stats3: MonitorStats = await nativerender.getMonitorStats(handle);
      
      expect(stats1.totalFrames).assertEqual(stats2.totalFrames);
      expect(stats2.totalFrames).assertEqual(stats3.totalFrames);
      expect(stats1.droppedFrames).assertEqual(stats2.droppedFrames);
      expect(stats2.droppedFrames).assertEqual(stats3.droppedFrames);
      
      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-ADV-005: Should handle multiple monitor instances', 0, async () => {
      // 创建多个监控器实例
      const handle1: number = await nativerender.createPerformanceMonitor();
      const handle2: number = await nativerender.createPerformanceMonitor();
      const handle3: number = await nativerender.createPerformanceMonitor();
      
      // 分别记录不同数量的帧
      for (let i = 0; i < 10; i++) {
        await nativerender.monitorBeginFrame(handle1);
        await nativerender.monitorEndFrame(handle1, false);
      }
      
      for (let i = 0; i < 20; i++) {
        await nativerender.monitorBeginFrame(handle2);
        await nativerender.monitorEndFrame(handle2, false);
      }
      
      for (let i = 0; i < 30; i++) {
        await nativerender.monitorBeginFrame(handle3);
        await nativerender.monitorEndFrame(handle3, false);
      }
      
      // 验证每个监控器的统计数据独立
      const stats1: MonitorStats = await nativerender.getMonitorStats(handle1);
      const stats2: MonitorStats = await nativerender.getMonitorStats(handle2);
      const stats3: MonitorStats = await nativerender.getMonitorStats(handle3);
      
      expect(stats1.totalFrames).assertEqual(10);
      expect(stats2.totalFrames).assertEqual(20);
      expect(stats3.totalFrames).assertEqual(30);
      
      nativerender.destroyPerformanceMonitor(handle1);
      nativerender.destroyPerformanceMonitor(handle2);
      nativerender.destroyPerformanceMonitor(handle3);
    });
  });
}
