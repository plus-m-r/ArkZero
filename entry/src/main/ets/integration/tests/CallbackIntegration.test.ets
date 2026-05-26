import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import { ArkZeroRenderer, ArkZeroRendererConfig, PixelFormat } from '../../components/rendering/ArkZeroRenderer';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'CallbackIntegrationTest';

export class CallbackIntegrationTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';
  private width: number = 800;
  private height: number = 600;

  constructor(surfaceId: string, width: number, height: number, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.width = width;
    this.height = height;
    this.onComplete = onComplete;
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting Callback Integration Tests');
    
    await this.testFrameRenderedCallback();
    await this.testMultipleRenderWithCallback();
    await this.testCallbackOnError();
    await this.testCallbackOrder();
    await this.testCallbackWithResize();
    
    hilog.info(DOMAIN, TAG, `All Callback tests completed: ${this.results.length} cases`);
    
    if (this.onComplete) {
      this.onComplete(this.results);
    }
    
    return this.results;
  }

  private async testFrameRenderedCallback(): Promise<void> {
    const startTime = Date.now();
    const testName = '单帧渲染完成回调';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 单帧渲染完成回调');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      let callbackInvoked = false;
      const callbackStartTime = Date.now();
      
      renderer.setOnFrameRendered(() => {
        callbackInvoked = true;
        const callbackDuration = Date.now() - callbackStartTime;
        hilog.info(DOMAIN, TAG, `Callback invoked after ${callbackDuration}ms`);
      });
      
      const bufferSize = this.width * this.height * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: callbackInvoked ? TestStatus.PASSED : TestStatus.FAILED,
        duration: duration,
        message: callbackInvoked ? `回调成功触发 (${duration}ms)` : '回调未被触发'
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `单帧回调失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testMultipleRenderWithCallback(): Promise<void> {
    const startTime = Date.now();
    const testName = '多帧渲染回调计数';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 多帧渲染回调计数');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      let callbackCount = 0;
      
      renderer.setOnFrameRendered(() => {
        callbackCount++;
        hilog.info(DOMAIN, TAG, `Callback invoked: count=${callbackCount}`);
      });
      
      const frameCount = 5;
      const bufferSize = this.width * this.height * 4;
      
      for (let i = 0; i < frameCount; i++) {
        const pixelBuffer = new ArrayBuffer(bufferSize);
        const data = new Uint8Array(pixelBuffer);
        data.fill(i * 50 % 256);
        
        await renderer.renderFrame(pixelBuffer, this.width, this.height);
      }
      
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: callbackCount === frameCount ? TestStatus.PASSED : TestStatus.FAILED,
        duration: duration,
        message: callbackCount === frameCount ? `${frameCount}帧渲染，回调触发${callbackCount}次 (${duration}ms)` : `期望回调${frameCount}次，实际${callbackCount}次`
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `多帧回调计数失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testCallbackOnError(): Promise<void> {
    const startTime = Date.now();
    const testName = '错误情况下回调行为';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 错误情况下回调行为');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      let callbackInvoked = false;
      
      renderer.setOnFrameRendered(() => {
        callbackInvoked = true;
      });
      
      renderer.dispose();
      
      try {
        const bufferSize = this.width * this.height * 4;
        const pixelBuffer = new ArrayBuffer(bufferSize);
        await renderer.renderFrame(pixelBuffer, this.width, this.height);
        
        const result: TestCaseResult = {
          name: testName,
          status: TestStatus.FAILED,
          duration: Date.now() - startTime,
          message: '销毁后仍能渲染，不符合预期'
        };
        this.results.push(result);
        
      } catch (error) {
        const duration = Date.now() - startTime;
        const result: TestCaseResult = {
          name: testName,
          status: callbackInvoked ? TestStatus.FAILED : TestStatus.PASSED,
          duration: duration,
          message: callbackInvoked ? '错误情况下回调被意外触发' : `错误时回调未触发，行为正确 (${duration}ms)`
        };
        this.results.push(result);
      }
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName}`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `错误回调测试失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testCallbackOrder(): Promise<void> {
    const startTime = Date.now();
    const testName = '回调执行顺序';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 回调执行顺序');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      const order: number[] = [];
      
      renderer.setOnFrameRendered(() => {
        order.push(1);
      });
      
      const bufferSize = this.width * this.height * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      order.push(2);
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      order.push(3);
      
      const duration = Date.now() - startTime;
      
      const expectedOrder: number[] = [1, 2, 1, 3];
      const orderCorrect = JSON.stringify(order) === JSON.stringify(expectedOrder);
      
      const result: TestCaseResult = {
        name: testName,
        status: orderCorrect ? TestStatus.PASSED : TestStatus.FAILED,
        duration: duration,
        message: orderCorrect 
          ? `回调顺序正确: [${order.join(', ')}] (${duration}ms)`
          : `回调顺序错误: 期望[${expectedOrder.join(', ')}], 实际[${order.join(', ')}]`
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `回调顺序测试失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testCallbackWithResize(): Promise<void> {
    const startTime = Date.now();
    const testName = 'resize后回调行为';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: resize后回调行为');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      let callbackCount = 0;
      
      renderer.setOnFrameRendered(() => {
        callbackCount++;
      });
      
      const bufferSize1 = this.width * this.height * 4;
      const pixelBuffer1 = new ArrayBuffer(bufferSize1);
      await renderer.renderFrame(pixelBuffer1, this.width, this.height);
      
      await renderer.resize(640, 480);
      
      const bufferSize2 = 640 * 480 * 4;
      const pixelBuffer2 = new ArrayBuffer(bufferSize2);
      await renderer.renderFrame(pixelBuffer2, 640, 480);
      
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: callbackCount === 2 ? TestStatus.PASSED : TestStatus.FAILED,
        duration: duration,
        message: callbackCount === 2 ? `resize前后回调各触发1次，共${callbackCount}次 (${duration}ms)` : `resize后回调次数异常: 期望2次，实际${callbackCount}次`
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `resize回调测试失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }
}
