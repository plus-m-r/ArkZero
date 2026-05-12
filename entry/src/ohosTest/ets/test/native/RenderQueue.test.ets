import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * RenderQueue 单元测试
 * 
 * 测试 C++ RenderQueue 类通过 NAPI 桥接的功能
 * 覆盖构造、提交、消费、停止等核心功能
 */

// 定义接口类型
interface QueueInfo {
  size: number;
  isRunning: boolean;
}

interface RenderCommandData {
  dataSize: number;
  width: number;
  height: number;
}

export default function RenderQueueTest() {
  describe('RenderQueue.InitializationTests', () => {
    
    // ===== 构造函数测试 =====
    
    it('RQ-INIT-001: Should initialize with default size', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(0);
      expect(info.isRunning).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-INIT-002: Should initialize with custom max size', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(5);
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(0);
      expect(info.isRunning).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-INIT-003: Should start in running state', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue.SubmitTests', () => {
    
    // ===== 提交命令测试 =====
    
    it('RQ-SUB-001: Should submit command successfully', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      const result: boolean = await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      expect(result).assertTrue();
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(1);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-SUB-002: Should submit multiple commands', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(5);
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 提交 3 个命令
      for (let i: number = 0; i < 3; i++) {
        const result: boolean = await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
        expect(result).assertTrue();
      }
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(3);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-SUB-003: Should drop oldest when queue is full', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(3);
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 填满队列
      for (let i: number = 0; i < 3; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      }
      
      let info: any = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(3);
      
      // 提交第4个，应该丢弃最旧的
      await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      
      info = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(3); // 仍然是3
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-SUB-004: Should reject submit after stop', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      await nativerender.queueStop(handle);
      
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      const result: boolean = await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      
      expect(result).assertFalse();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-SUB-005: Should accept empty buffer', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(0);
      
      const result: boolean = await nativerender.queueSubmit(handle, buffer, 0, 10, 10);
      expect(result).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue.DequeueTests', () => {
    
    // ===== 消费命令测试 =====
    
    it('RQ-DEQ-001: Should dequeue command', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      await nativerender.queueSubmit(handle, buffer, 100, 1920, 1080);
      
      const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
      expect(cmd !== null).assertTrue();
      expect(cmd.dataSize).assertEqual(100);
      expect(cmd.width).assertEqual(1920);
      expect(cmd.height).assertEqual(1080);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-DEQ-002: Should reduce queue size after dequeue', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 提交 3 个命令
      for (let i: number = 0; i < 3; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      }
      
      let info: any = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(3);
      
      // 消费 1 个
      await nativerender.queueDequeue(handle);
      
      info = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(2);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-DEQ-003: Should return null after stop', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      await nativerender.queueStop(handle);
      
      const cmd: any = await nativerender.queueDequeue(handle);
      expect(cmd).assertNull();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-DEQ-004: Should consume all commands', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 提交 5 个命令
      for (let i: number = 0; i < 5; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      }
      
      // 消费所有命令
      for (let i: number = 0; i < 5; i++) {
        const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
        expect(cmd !== null).assertTrue();
      }
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(0);
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue.StopTests', () => {
    
    // ===== 停止队列测试 =====
    
    it('RQ-STP-001: Should set running flag to false', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      
      let info: any = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertTrue();
      
      await nativerender.queueStop(handle);
      
      info = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertFalse();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-STP-002: Should allow multiple stop calls', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      
      await nativerender.queueStop(handle);
      await nativerender.queueStop(handle); // 第二次调用不应崩溃
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertFalse();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-STP-003: Should prevent submit after stop', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 先提交一个
      await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      
      // 停止队列
      await nativerender.queueStop(handle);
      
      // 再提交应该失败
      const result: boolean = await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      expect(result).assertFalse();
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue.IntegrationTests', () => {
    
    // ===== 集成场景测试 =====
    
    it('RQ-INT-001: Should support producer-consumer pattern', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(3);
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 生产者：提交 10 个命令
      for (let i: number = 0; i < 10; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
      }
      
      // 消费者：消费 10 个命令
      for (let i: number = 0; i < 10; i++) {
        const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
        expect(cmd !== null).assertTrue();
      }
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(0);
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-INT-002: Should handle different frame sizes', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      
      // 提交不同大小的帧
      const sizes: number[] = [100, 1000, 10000];
      
      for (let i: number = 0; i < sizes.length; i++) {
        const buffer: ArrayBuffer = new ArrayBuffer(sizes[i]);
        await nativerender.queueSubmit(handle, buffer, sizes[i], 10, 10);
      }
      
      // 验证都能正确消费
      for (let i: number = 0; i < sizes.length; i++) {
        const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
        expect(cmd !== null).assertTrue();
        expect(cmd.dataSize).assertEqual(sizes[i]);
      }
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-INT-003: Should handle different resolutions', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      interface Resolution {
        width: number;
        height: number;
      }
      
      const resolutions: Resolution[] = [
        { width: 640, height: 480 },
        { width: 1280, height: 720 },
        { width: 1920, height: 1080 },
      ];
      
      // 提交不同分辨率的帧
      for (let i: number = 0; i < resolutions.length; i++) {
        await nativerender.queueSubmit(
          handle, 
          buffer, 
          100, 
          resolutions[i].width, 
          resolutions[i].height
        );
      }
      
      // 验证都能正确消费
      for (let i: number = 0; i < resolutions.length; i++) {
        const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
        expect(cmd !== null).assertTrue();
        expect(cmd.width).assertEqual(resolutions[i].width);
        expect(cmd.height).assertEqual(resolutions[i].height);
      }
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-INT-004: Should maintain FIFO order', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(10);
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 提交带有顺序标识的命令（通过宽度字段）
      for (let i: number = 0; i < 5; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, i + 1, 10);
      }
      
      // 验证消费顺序与提交顺序一致
      for (let i: number = 0; i < 5; i++) {
        const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
        expect(cmd !== null).assertTrue();
        expect(cmd.width).assertEqual(i + 1);
      }
      
      nativerender.destroyRenderQueue(handle);
    });
  });

  describe('RenderQueue.EdgeCaseTests', () => {
    
    // ===== 边界情况测试 =====
    
    it('RQ-EDGE-001: Should handle zero max size gracefully', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(0);
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-EDGE-002: Should handle very large max size', 0, async () => {
      const handle: number = await nativerender.createRenderQueue(1000);
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.isRunning).assertTrue();
      
      nativerender.destroyRenderQueue(handle);
    });

    it('RQ-EDGE-003: Should handle rapid submit-dequeue cycles', 0, async () => {
      const handle: number = await nativerender.createRenderQueue();
      const buffer: ArrayBuffer = new ArrayBuffer(100);
      
      // 快速提交和消费 50 次
      for (let i: number = 0; i < 50; i++) {
        await nativerender.queueSubmit(handle, buffer, 100, 10, 10);
        const cmd: RenderCommandData | null = await nativerender.queueDequeue(handle);
        expect(cmd !== null).assertTrue();
      }
      
      const info: QueueInfo = await nativerender.getQueueInfo(handle);
      expect(info.size).assertEqual(0);
      
      nativerender.destroyRenderQueue(handle);
    });
  });
}
