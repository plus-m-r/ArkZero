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

import { ArkZeroRenderer, PixelFormat } from '../../components/rendering/ArkZeroRenderer';
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
        width: 1920,
        height: 1080,
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

  /**
   * 测试 2: 渲染单帧
   */
  private async testRenderFrame(): Promise<void> {
    const startTime = Date.now();
    const testName = '渲染单帧';
    
    try {
      hilog.info(DOMAIN, TAG, '▶️ Test: 渲染单帧');
      
      if (!this.renderer) {
        throw new Error('渲染器未初始化');
      }
      
      // 创建测试图案
      const bufferSize = 1920 * 1080 * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data = new Uint8Array(pixelBuffer);
      
      // 填充红色
      for (let i = 0; i < bufferSize; i += 4) {
        data[i] = 255;     // R
        data[i + 1] = 0;   // G
        data[i + 2] = 0;   // B
        data[i + 3] = 255; // A
      }
      
      // 渲染
      await this.renderer.renderFrame(pixelBuffer, 1920, 1080);
      
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
      const data = new Uint8Array(pixelBuffer);
      
      // 填充绿色
      for (let i = 0; i < bufferSize; i += 4) {
        data[i] = 0;       // R
        data[i + 1] = 255; // G
        data[i + 2] = 0;   // B
        data[i + 3] = 255; // A
      }
      
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
      
      // 恢复到原始尺寸
      await this.renderer.resize(1920, 1080);
      
      const frameCount = 10;
      const bufferSize = 1920 * 1080 * 4;
      
      for (let i = 0; i < frameCount; i++) {
        const pixelBuffer = new ArrayBuffer(bufferSize);
        const data = new Uint8Array(pixelBuffer);
        
        // 不同颜色
        const color = (i * 25) % 256;
        for (let j = 0; j < bufferSize; j += 4) {
          data[j] = color;
          data[j + 1] = 128;
          data[j + 2] = 255 - color;
          data[j + 3] = 255;
        }
        
        await this.renderer.renderFrame(pixelBuffer, 1920, 1080);
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
