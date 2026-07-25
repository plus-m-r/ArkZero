import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import { ArkZeroRenderer, ArkZeroRendererConfig, PixelFormat } from '@plusml/arkzero';
import hilog from '@ohos.hilog';

interface FormatInfo {
  format: PixelFormat;
  name: string;
}

const DOMAIN = 0x0001;
const TAG = 'PixelFormatIntegrationTest';

export class PixelFormatIntegrationTest {
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
    hilog.info(DOMAIN, TAG, 'Starting PixelFormat Integration Tests');
    
    await this.testRGBAFormat();
    await this.testRGBFormat();
    await this.testBGRAFormat();
    await this.testNV21Format();
    await this.testNV12Format();
    await this.testFormatSwitching();
    
    hilog.info(DOMAIN, TAG, `All PixelFormat tests completed: ${this.results.length} cases`);
    
    if (this.onComplete) {
      this.onComplete(this.results);
    }
    
    return this.results;
  }

  private async testRGBAFormat(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = 'RGBA8888格式渲染';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGBA
    };
    
    const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: RGBA8888格式渲染');
      
      await renderer.initialize(this.surfaceId);
      
      const bufferSize: number = this.width * this.height * 4;
      const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      const data: Uint8Array = new Uint8Array(pixelBuffer);
      
      let i: number;
      for (i = 0; i < bufferSize; i += 4) {
        data[i] = 255;
        data[i + 1] = 0;
        data[i + 2] = 0;
        data[i + 3] = 255;
      }
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `RGBA8888格式渲染成功 (${duration}ms)`
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
        message: `RGBA8888格式失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testRGBFormat(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = 'RGB888格式渲染';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGB
    };
    
    const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: RGB888格式渲染');
      
      await renderer.initialize(this.surfaceId);
      
      const bufferSize: number = this.width * this.height * 3;
      const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      const data: Uint8Array = new Uint8Array(pixelBuffer);
      
      let i: number;
      for (i = 0; i < bufferSize; i += 3) {
        data[i] = 0;
        data[i + 1] = 255;
        data[i + 2] = 0;
      }
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `RGB888格式渲染成功 (${duration}ms)`
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
        message: `RGB888格式失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testBGRAFormat(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = 'BGRA8888格式渲染';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.BGRA
    };
    
    const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: BGRA8888格式渲染');
      
      await renderer.initialize(this.surfaceId);
      
      const bufferSize: number = this.width * this.height * 4;
      const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
      const data: Uint8Array = new Uint8Array(pixelBuffer);
      
      let i: number;
      for (i = 0; i < bufferSize; i += 4) {
        data[i] = 0;
        data[i + 1] = 0;
        data[i + 2] = 255;
        data[i + 3] = 255;
      }
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `BGRA8888格式渲染成功 (${duration}ms)`
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
        message: `BGRA8888格式失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testNV21Format(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = 'NV21格式渲染';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.NV21
    };
    
    const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: NV21格式渲染');
      
      await renderer.initialize(this.surfaceId);
      
      const ySize: number = this.width * this.height;
      const uvSize: number = ySize / 2;
      const totalSize: number = ySize + uvSize;
      const pixelBuffer: ArrayBuffer = new ArrayBuffer(totalSize);
      const data: Uint8Array = new Uint8Array(pixelBuffer);
      
      let i: number;
      for (i = 0; i < ySize; i++) {
        data[i] = 128;
      }
      for (i = ySize; i < totalSize; i += 2) {
        data[i] = 128;
        data[i + 1] = 128;
      }
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `NV21格式渲染成功 (${duration}ms)`
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
        message: `NV21格式失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testNV12Format(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = 'NV12格式渲染';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.NV12
    };
    
    const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: NV12格式渲染');
      
      await renderer.initialize(this.surfaceId);
      
      const ySize: number = this.width * this.height;
      const uvSize: number = ySize / 2;
      const totalSize: number = ySize + uvSize;
      const pixelBuffer: ArrayBuffer = new ArrayBuffer(totalSize);
      const data: Uint8Array = new Uint8Array(pixelBuffer);
      
      let i: number;
      for (i = 0; i < ySize; i++) {
        data[i] = 64;
      }
      for (i = ySize; i < totalSize; i += 2) {
        data[i] = 192;
        data[i + 1] = 192;
      }
      
      await renderer.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `NV12格式渲染成功 (${duration}ms)`
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
        message: `NV12格式失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testFormatSwitching(): Promise<void> {
    const startTime: number = Date.now();
    const testName: string = '格式切换性能';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 格式切换性能');
      
      const formats: FormatInfo[] = [
        { format: PixelFormat.RGBA, name: 'RGBA' },
        { format: PixelFormat.RGB, name: 'RGB' },
        { format: PixelFormat.BGRA, name: 'BGRA' }
      ];
      
      let idx: number;
      for (idx = 0; idx < formats.length; idx++) {
        const fmt: FormatInfo = formats[idx];
        const config: ArkZeroRendererConfig = {
          width: this.width,
          height: this.height,
          format: fmt.format
        };
        
        const renderer: ArkZeroRenderer = new ArkZeroRenderer(config);
        
        try {
          await renderer.initialize(this.surfaceId);
          
          const bytesPerPixel: number = fmt.format === PixelFormat.RGB ? 3 : 4;
          const bufferSize: number = this.width * this.height * bytesPerPixel;
          const pixelBuffer: ArrayBuffer = new ArrayBuffer(bufferSize);
          const data: Uint8Array = new Uint8Array(pixelBuffer);
          data.fill(128);
          
          await renderer.renderFrame(pixelBuffer, this.width, this.height);
        } finally {
          renderer.dispose();
        }
      }
      
      const duration: number = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功切换${formats.length}种格式 (${duration}ms)`
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
        message: `格式切换失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }
}
