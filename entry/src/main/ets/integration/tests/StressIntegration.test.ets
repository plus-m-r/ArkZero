import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import { ArkZeroRenderer, ArkZeroRendererConfig, PixelFormat } from '../../components/rendering/ArkZeroRenderer';
import hilog from '@ohos.hilog';

interface ResizeOperation {
  w: number;
  h: number;
}

interface Resolution {
  w: number;
  h: number;
}

const DOMAIN = 0x0001;
const TAG = 'StressIntegrationTest';

export class StressIntegrationTest {
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
    hilog.info(DOMAIN, TAG, 'Starting Stress Integration Tests');
    
    await this.testHighFrequencyRendering();
    await this.testRapidResize();
    await this.testLargeResolution();
    await this.testConsecutiveDisposeCreate();
    await this.testMemoryPressure();
    
    hilog.info(DOMAIN, TAG, `All Stress tests completed: ${this.results.length} cases`);
    
    if (this.onComplete) {
      this.onComplete(this.results);
    }
    
    return this.results;
  }

  private async testHighFrequencyRendering(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = '高频渲染测试';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 高频渲染测试');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      const frameCount: number = 30;
      const bufferSize: number = this.width * this.height * 4;
      
      for (let i: number = 0; i < frameCount; i++) {
        const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
        const data: Uint8Array = new Uint8Array(pixelBuffer);
        data.fill(i * 8 % 256);
        
        await renderer.renderFrame(pixelBuffer, this.width, this.height);
      }
      
      const duration: number = Date.now() - startTime;
      const avgTime: number = duration / frameCount;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `${frameCount}帧渲染完成，平均${avgTime.toFixed(1)}ms/帧`
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms, avg ${avgTime.toFixed(1)}ms/frame)`);
      
    } catch (error) {
      const duration: number = Date.now() - startTime;
      const errorMsg: string = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `高频渲染失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testRapidResize(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = '快速尺寸切换测试';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 快速尺寸切换测试');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      const resizeOperations: ResizeOperation[] = [
        { w: 640, h: 480 },
        { w: 1280, h: 720 },
        { w: 800, h: 600 },
        { w: 1024, h: 768 },
        { w: this.width, h: this.height }
      ];
      
      for (let i: number = 0; i < resizeOperations.length; i++) {
        const res: ResizeOperation = resizeOperations[i];
        await renderer.resize(res.w, res.h);
        
        const bufferSize: number = res.w * res.h * 4;
        const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
        await renderer.renderFrame(pixelBuffer, res.w, res.h);
      }
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功执行${resizeOperations.length}次resize (${duration}ms)`
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration: number = Date.now() - startTime;
      const errorMsg: string = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `快速切换失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testLargeResolution(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = '大分辨率渲染测试';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 大分辨率渲染测试');
      
      const testResolutions: Resolution[] = [
        { w: 1920, h: 1080 },
        { w: 2560, h: 1440 },
        { w: 3840, h: 2160 }
      ];
      
      for (let i: number = 0; i < testResolutions.length; i++) {
        const res: Resolution = testResolutions[i];
        const config: ArkZeroRendererConfig = {
          width: res.w,
          height: res.h,
          format: PixelFormat.RGBA
        };
        
        const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
        
        await renderer.initialize(this.surfaceId);
        
        const bufferSize: number = res.w * res.h * 4;
        const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
        const data: Uint8Array = new Uint8Array(pixelBuffer);
        data.fill(64);
        
        await renderer.renderFrame(pixelBuffer, res.w, res.h);
        
        renderer.dispose();
      }
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功渲染${testResolutions.length}个大分辨率 (${duration}ms)`
      };
      this.results.push(result);
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration: number = Date.now() - startTime;
      const errorMsg: string = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `大分辨率渲染失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testConsecutiveDisposeCreate(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = '连续创建销毁测试';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 连续创建销毁测试');
      
      const loopCount: number = 5;
      
      for (let i: number = 0; i < loopCount; i++) {
        const config: ArkZeroRendererConfig = {
          width: this.width,
          height: this.height,
          format: PixelFormat.RGBA
        };
        
        const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
        
        await renderer.initialize(this.surfaceId);
        
        const bufferSize: number = this.width * this.height * 4;
        const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
        await renderer.renderFrame(pixelBuffer, this.width, this.height);
        
        renderer.dispose();
      }
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `连续${loopCount}次创建/销毁成功 (${duration}ms)`
      };
      this.results.push(result);
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration: number = Date.now() - startTime;
      const errorMsg: string = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `连续创建/销毁失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testMemoryPressure(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = '内存压力测试';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 内存压力测试');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
      const bufferCount: number = 10;
      const bufferSize: number = this.width * this.height * 4;
      const buffers: ArrayBuffer[] = [];
      
      for (let i: number = 0; i < bufferCount; i++) {
        const buffer: ArrayBuffer = new ArrayBuffer(bufferSize);
        const data: Uint8Array = new Uint8Array(buffer);
        data.fill(i * 25 % 256);
        buffers.push(buffer);
      }
      
      for (let i: number = 0; i < bufferCount; i++) {
        await renderer.renderFrame(buffers[i], this.width, this.height);
      }
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功处理${bufferCount}个缓冲区，内存压力测试通过 (${duration}ms)`
      };
      this.results.push(result);
      
      renderer.dispose();
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration: number = Date.now() - startTime;
      const errorMsg: string = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `内存压力测试失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }
}
