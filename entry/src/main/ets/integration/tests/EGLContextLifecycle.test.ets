import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import { ArkZeroRenderer, ArkZeroRendererConfig, PixelFormat } from '../../components/rendering/ArkZeroRenderer';
import hilog from '@ohos.hilog';

interface Resolution {
  width: number;
  height: number;
}

const DOMAIN = 0x0001;
const TAG = 'EGLContextLifecycleTest';

export class EGLContextLifecycleTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';
  private width: number = 1920;
  private height: number = 1080;

  constructor(surfaceId: string, width: number, height: number, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.width = width;
    this.height = height;
    this.onComplete = onComplete;
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting EGL Context Lifecycle Tests');
    
    await this.testContextCreation();
    await this.testContextReuse();
    await this.testContextResize();
    await this.testContextDestruction();
    await this.testRecreateAfterDestroy();
    
    hilog.info(DOMAIN, TAG, `All EGL Context tests completed: ${this.results.length} cases`);
    
    if (this.onComplete) {
      this.onComplete(this.results);
    }
    
    return this.results;
  }

  private async testContextCreation(): Promise<void> {
    const startTime = Date.now();
    const testName = '创建EGL上下文';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGBA
    };
    
    const renderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, `Test: 创建EGL上下文 with surfaceId: ${this.surfaceId}`);
      
      await renderer.initialize(this.surfaceId);
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `上下文创建成功 (${duration}ms)`
      };
      this.results.push(result);
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `创建失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testContextReuse(): Promise<void> {
    const startTime = Date.now();
    const testName = '复用EGL上下文';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGBA
    };
    
    const renderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 复用EGL上下文');
      
      await renderer.initialize(this.surfaceId);
      
      const bufferSize = this.width * this.height * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data = new Uint8Array(pixelBuffer);
      data.fill(255);
      
      for (let i = 0; i < 3; i++) {
        await renderer.renderFrame(pixelBuffer, this.width, this.height);
      }
      
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `上下文复用3次渲染成功 (${duration}ms)`
      };
      this.results.push(result);
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `复用失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testContextResize(): Promise<void> {
    const startTime = Date.now();
    const testName = 'EGL上下文调整尺寸';
    
    const config: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGBA
    };
    
    const renderer = new ArkZeroRenderer(config);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: EGL上下文调整尺寸');
      
      await renderer.initialize(this.surfaceId);
      
      const testResolutions: Resolution[] = [
        { width: 1280, height: 720 },
        { width: 800, height: 600 },
        { width: 1920, height: 1080 }
      ];
      
      for (let i = 0; i < testResolutions.length; i++) {
        const res = testResolutions[i];
        await renderer.resize(res.width, res.height);
        
        const bufferSize = res.width * res.height * 4;
        const pixelBuffer = new ArrayBuffer(bufferSize);
        await renderer.renderFrame(pixelBuffer, res.width, res.height);
      }
      
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功调整${testResolutions.length}次分辨率 (${duration}ms)`
      };
      this.results.push(result);
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `调整尺寸失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer.dispose();
    }
  }

  private async testContextDestruction(): Promise<void> {
    const startTime = Date.now();
    const testName = '销毁EGL上下文';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 销毁EGL上下文');
      
      const config: ArkZeroRendererConfig = {
        width: this.width,
        height: this.height,
        format: PixelFormat.RGBA
      };
      
      const renderer = new ArkZeroRenderer(config);
      
      await renderer.initialize(this.surfaceId);
      
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
          status: TestStatus.PASSED,
          duration: duration,
          message: `上下文正确销毁，无法再渲染 (${duration}ms)`
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
        message: `销毁失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testRecreateAfterDestroy(): Promise<void> {
    const startTime = Date.now();
    const testName = '销毁后重新创建EGL上下文';
    
    const config1: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGBA
    };
    
    const renderer1 = new ArkZeroRenderer(config1);
    
    const config2: ArkZeroRendererConfig = {
      width: this.width,
      height: this.height,
      format: PixelFormat.RGBA
    };
    
    const renderer2 = new ArkZeroRenderer(config2);
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 销毁后重新创建EGL上下文');
      
      await renderer1.initialize(this.surfaceId);
      renderer1.dispose();
      
      await renderer2.initialize(this.surfaceId);
      
      const bufferSize = this.width * this.height * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      await renderer2.renderFrame(pixelBuffer, this.width, this.height);
      
      const duration = Date.now() - startTime;
      
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功销毁并重新创建 (${duration}ms)`
      };
      this.results.push(result);
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      const result: TestCaseResult = {
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `重新创建失败: ${errorMsg}`
      };
      this.results.push(result);
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    } finally {
      renderer2.dispose();
    }
  }
}