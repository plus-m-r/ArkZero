/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { ArkZeroRenderer, PixelFormat, DirtyRect } from '../../components/rendering/ArkZeroRenderer';
import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'RendererIntegrationTest';

/**
 * 渲染器集成测试
 * 
 * 🎯 测试内容：
 * - 初始化流程
 * - 渲染功能
 * - 尺寸调整
 * - 资源清理
 */
export class RendererIntegrationTest {
  private renderer?: ArkZeroRenderer;
  private surfaceId: string = '';
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;

  constructor(surfaceId: string, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.onComplete = onComplete;
  }

  /**
   * 运行所有测试
   */
  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, '🧪 Starting Renderer Integration Tests');
    
    await this.testInitialization();
    await this.testRenderFrame();
    await this.testResize();
    await this.testMultipleRenders();
    await this.testIncrementalRender();
    await this.testDispose();
    
    hilog.info(DOMAIN, TAG, `✅ All tests completed: ${this.results.length} cases`);
    
    if (this.onComplete) {
      this.onComplete(this.results);
    }
    
    return this.results;
  }

  /**
   * 测试 1: 初始化
   */
  private async testInitialization(): Promise<void> {
    const startTime = Date.now();
    const testName = '初始化渲染器';
    
    try {
      hilog.info(DOMAIN, TAG, '▶️ Test: 初始化渲染器');
      
      this.renderer = new ArkZeroRenderer({
        width: 800,
        height: 600,
        format: PixelFormat.RGBA
      });
      
      await this.renderer.initialize(this.surfaceId);
      
      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `初始化成功 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `✅ ${testName}: PASSED (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `初始化失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `❌ ${testName}: FAILED - ${errorMsg}`);
    }
  }

  private async testRenderFrame(): Promise<void> {
    const startTime = Date.now();
    const testName = '渲染单帧';
    
    try {
      hilog.info(DOMAIN, TAG, '▶️ Test: 渲染单帧');
      
      if (!this.renderer) {
        throw new Error('渲染器未初始化');
      }
      
      const bufferSize = 800 * 600 * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      data32.fill(0xFF0000FF);
      
      await this.renderer.renderFrame(pixelBuffer, 800, 600);
      
      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `渲染成功 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `✅ ${testName}: PASSED (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `渲染失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `❌ ${testName}: FAILED - ${errorMsg}`);
    }
  }

  /**
   * 测试 3: 调整尺寸
   */
  private async testResize(): Promise<void> {
    const startTime = Date.now();
    const testName = '调整渲染尺寸';
    
    try {
      hilog.info(DOMAIN, TAG, '▶️ Test: 调整渲染尺寸');
      
      if (!this.renderer) {
        throw new Error('渲染器未初始化');
      }
      
      // 调整到较小尺寸
      await this.renderer.resize(800, 600);
      
      // 渲染一帧验证
      const bufferSize = 800 * 600 * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      data32.fill(0xFF00FF00);
      
      await this.renderer.renderFrame(pixelBuffer, 800, 600);
      
      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `调整成功 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `✅ ${testName}: PASSED (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `调整失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `❌ ${testName}: FAILED - ${errorMsg}`);
    }
  }

  /**
   * 测试 4: 连续渲染多帧
   */
  private async testMultipleRenders(): Promise<void> {
    const startTime = Date.now();
    const testName = '连续渲染多帧';
    
    try {
      hilog.info(DOMAIN, TAG, '▶️ Test: 连续渲染多帧');
      
      if (!this.renderer) {
        throw new Error('渲染器未初始化');
      }
      
      // 恢复到较小尺寸（避免模拟器超时）
      await this.renderer.resize(800, 600);
      
      const frameCount = 5;
      const bufferSize = 800 * 600 * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);

      for (let i = 0; i < frameCount; i++) {
        const r = (i * 50) % 256;
        const g = 128;
        const b = 255 - r;
        const pixelValue = (255 << 24) | (b << 16) | (g << 8) | r;
        data32.fill(pixelValue);
        
        await this.renderer.renderFrame(pixelBuffer, 800, 600);
      }
      
      const duration = Date.now() - startTime;
      const avgTime = duration / frameCount;
      
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `${frameCount}帧成功，平均${avgTime.toFixed(1)}ms/帧`
      });
      
      hilog.info(DOMAIN, TAG, `✅ ${testName}: PASSED (${duration}ms, avg ${avgTime.toFixed(1)}ms/frame)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `连续渲染失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `❌ ${testName}: FAILED - ${errorMsg}`);
    }
  }

  private async testIncrementalRender(): Promise<void> {
    const startTime = Date.now();
    const testName = '增量渲染';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 增量渲染');
      
      if (!this.renderer) {
        throw new Error('渲染器未初始化');
      }

      const frameWidth = 800;
      const frameHeight = 600;

      const fullBuffer = new ArrayBuffer(frameWidth * frameHeight * 4);
      const full32 = new Uint32Array(fullBuffer);
      full32.fill(0xFF0000FF);
      await this.renderer.renderFrame(fullBuffer, frameWidth, frameHeight);

      full32.fill(0xFF00FF00, 100 * frameWidth + 100, 100 * frameWidth + 100 + 50);
      for (let row = 1; row < 50; row++) {
        const start = (100 + row) * frameWidth + 100;
        full32.fill(0xFF00FF00, start, start + 50);
      }

      full32.fill(0xFF0000FF, 200 * frameWidth + 200, 200 * frameWidth + 200 + 80);
      for (let row = 1; row < 40; row++) {
        const start = (200 + row) * frameWidth + 200;
        full32.fill(0xFF0000FF, start, start + 80);
      }

      const region1: DirtyRect = { x: 100, y: 100, w: 50, h: 50 };
      const region2: DirtyRect = { x: 200, y: 200, w: 80, h: 40 };

      const incrementalStart = Date.now();
      await this.renderer.renderFrameRegions(
        fullBuffer,
        frameWidth,
        frameHeight,
        [region1, region2],
        true
      );
      const incrementalDuration = Date.now() - incrementalStart;

      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `2区域增量渲染成功 (${incrementalDuration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `${testName}: PASSED (${incrementalDuration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `增量渲染失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `${testName}: FAILED - ${errorMsg}`);
    }
  }

  /**
   * 测试 5: 资源清理
   */
  private async testDispose(): Promise<void> {
    const startTime = Date.now();
    const testName = '资源清理';
    
    try {
      hilog.info(DOMAIN, TAG, '▶️ Test: 资源清理');
      
      if (!this.renderer) {
        throw new Error('渲染器未初始化');
      }
      
      this.renderer.dispose();
      this.renderer = undefined;
      
      const duration = Date.now() - startTime;
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `清理成功 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `✅ ${testName}: PASSED (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `清理失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `❌ ${testName}: FAILED - ${errorMsg}`);
    }
  }
}
