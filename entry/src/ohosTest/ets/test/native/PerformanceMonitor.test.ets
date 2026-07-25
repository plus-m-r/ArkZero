import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

interface MonitorStats {
  totalFrames: number;
  droppedFrames: number;
  fps: number;
  dropRate: number;
  frameTimeMs: number;
}

export default function PerformanceMonitorTest() {
  describe('PerformanceMonitor.ConstructorTests', () => {

    it('PM-CTOR-001: Should create PerformanceMonitor instance', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();
      expect(handle > 0).assertTrue();

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-CTOR-002: Should create multiple instances', 0, () => {
      const handle1: number = nativerender.createPerformanceMonitor();
      const handle2: number = nativerender.createPerformanceMonitor();

      expect(handle1 > 0).assertTrue();
      expect(handle2 > 0).assertTrue();
      expect(handle1 !== handle2).assertTrue();

      nativerender.destroyPerformanceMonitor(handle1);
      nativerender.destroyPerformanceMonitor(handle2);
    });
  });

  describe('PerformanceMonitor.FrameTrackingTests', () => {

    it('PM-FT-001: Should track begin and end frame', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      nativerender.monitorBeginFrame(handle);
      nativerender.monitorEndFrame(handle, false);

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames > 0).assertTrue();

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-FT-002: Should track multiple frames', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      const frameCount: number = 10;
      for (let i: number = 0; i < frameCount; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, false);
      }

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(frameCount);

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-FT-003: Should track dropped frames', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      nativerender.monitorBeginFrame(handle);
      nativerender.monitorEndFrame(handle, true);

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.droppedFrames > 0).assertTrue();

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-FT-004: Should distinguish rendered vs dropped frames', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      for (let i: number = 0; i < 5; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, false);
      }

      for (let i: number = 0; i < 3; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, true);
      }

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(8);
      expect(stats.droppedFrames).assertEqual(3);

      nativerender.destroyPerformanceMonitor(handle);
    });
  });

  describe('PerformanceMonitor.ResetTests', () => {

    it('PM-RST-001: Should reset statistics', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      for (let i: number = 0; i < 5; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, false);
      }

      nativerender.monitorReset(handle);

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(0);

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-RST-002: Should allow tracking after reset', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      for (let i: number = 0; i < 5; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, false);
      }

      nativerender.monitorReset(handle);

      for (let i: number = 0; i < 3; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, false);
      }

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(3);

      nativerender.destroyPerformanceMonitor(handle);
    });
  });

  describe('PerformanceMonitor.StatsTests', () => {

    it('PM-STAT-001: Should return stats object with correct fields', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      nativerender.monitorBeginFrame(handle);
      nativerender.monitorEndFrame(handle, false);

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(typeof stats.totalFrames).assertEqual('number');
      expect(typeof stats.droppedFrames).assertEqual('number');
      expect(typeof stats.fps).assertEqual('number');
      expect(typeof stats.dropRate).assertEqual('number');
      expect(typeof stats.frameTimeMs).assertEqual('number');

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-STAT-002: Should report zero stats initially', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(0);
      expect(stats.droppedFrames).assertEqual(0);

      nativerender.destroyPerformanceMonitor(handle);
    });
  });

  describe('PerformanceMonitor.DestroyTests', () => {

    it('PM-DST-001: Should destroy cleanly', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      nativerender.monitorBeginFrame(handle);
      nativerender.monitorEndFrame(handle, false);

      nativerender.destroyPerformanceMonitor(handle);

      expect(true).assertTrue();
    });

    it('PM-DST-002: Should handle destroy without any frames tracked', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();

      nativerender.destroyPerformanceMonitor(handle);

      expect(true).assertTrue();
    });
  });

  describe('PerformanceMonitor.IntegrationTests', () => {

    it('PM-INT-001: Should support full lifecycle', 0, () => {
      const handle: number = nativerender.createPerformanceMonitor();
      expect(handle > 0).assertTrue();

      for (let i: number = 0; i < 30; i++) {
        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, i % 5 === 0);
      }

      const stats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(stats.totalFrames).assertEqual(30);
      expect(stats.droppedFrames).assertEqual(6);

      nativerender.monitorReset(handle);

      const resetStats: MonitorStats = nativerender.getMonitorStats(handle);
      expect(resetStats.totalFrames).assertEqual(0);

      nativerender.destroyPerformanceMonitor(handle);
    });

    it('PM-INT-002: Should handle rapid create-destroy cycles', 0, () => {
      for (let i: number = 0; i < 5; i++) {
        const handle: number = nativerender.createPerformanceMonitor();
        expect(handle > 0).assertTrue();

        nativerender.monitorBeginFrame(handle);
        nativerender.monitorEndFrame(handle, false);

        nativerender.destroyPerformanceMonitor(handle);
      }
    });
  });
}
