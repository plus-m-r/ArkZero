import { describe, it, expect } from '@ohos/hypium';
import nativerender from 'libnativerender.so';

/**
 * 渲染管线集成测试
 * 
 * 验证从像素数据到屏幕显示的完整渲染流程
 */

export default function RenderPipelineTest() {
  describe('Render Pipeline Integration Tests', () => {
    
    it('PIPE-FLOW-001: Should render single frame through pipeline', 0, async () => {
      // 1. 创建渲染器
      const rendererHandle: number = await nativerender.managerCreateOffscreenRenderer(
        640, 480, 0, true
      );
      expect(rendererHandle > 0).assertTrue();
      
      // 2. 准备像素数据
      const bufferSize: number = 640 * 480 * 4; // RGBA
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      const view: Uint8Array = new Uint8Array(buffer);
      
      // 填充红色
      for (let i: number = 0; i < bufferSize; i += 4) {
        view[i] = 255;     // R
        view[i + 1] = 0;   // G
        view[i + 2] = 0;   // B
        view[i + 3] = 255; // A
      }
      
      // 3. 渲染帧
      const success: boolean = await nativerender.rendererRenderFrame(
        rendererHandle, buffer, 640, 480
      );
      expect(success).assertTrue();
      
      // 4. 清理
      await nativerender.managerDestroyRenderer(rendererHandle);
    });

    it('PIPE-FLOW-002: Should handle multiple consecutive frames', 0, async () => {
      const rendererHandle: number = await nativerender.managerCreateOffscreenRenderer(
        640, 480, 0, true
      );
      expect(rendererHandle > 0).assertTrue();
      
      const bufferSize: number = 640 * 480 * 4;
      const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      
      // 渲染 100 帧
      let successCount: number = 0;
      for (let i: number = 0; i < 100; i++) {
        const success: boolean = await nativerender.rendererRenderFrame(
          rendererHandle, buffer, 640, 480
        );
        if (success) {
          successCount++;
        }
      }
      
      expect(successCount).assertEqual(100);
      
      await nativerender.managerDestroyRenderer(rendererHandle);
    });

    it('PIPE-FLOW-003: Should handle different pixel formats', 0, async () => {
      const formats: number[] = [0, 1, 2]; // RGBA, RGB, BGRA
      
      for (const format of formats) {
        const rendererHandle: number = await nativerender.managerCreateOffscreenRenderer(
          640, 480, format, true
        );
        expect(rendererHandle > 0).assertTrue();
        
        const bufferSize: number = 640 * 480 * 4;
        const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
        
        const success: boolean = await nativerender.rendererRenderFrame(
          rendererHandle, buffer, 640, 480
        );
        expect(success).assertTrue();
        
        await nativerender.managerDestroyRenderer(rendererHandle);
      }
    });
  });
}
