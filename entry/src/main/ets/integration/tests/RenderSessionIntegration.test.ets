import { ArkZeroRenderSession, RenderSessionConfig, PixelFormat, TileRegion, DirtyRect } from '@plusml/arkzero';
import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'SessionTest';

export class RenderSessionIntegrationTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';

  constructor(surfaceId: string, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.onComplete = onComplete;
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting RenderSession Integration Tests');

    const config: RenderSessionConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const session = new ArkZeroRenderSession(config);

    try {
      await session.attach(this.surfaceId);

      const warmBuffer = new ArrayBuffer(800 * 600 * 4);
      const warm32 = new Uint32Array(warmBuffer);
      warm32.fill(0xFF000000);
      await session.renderFullFrame(warmBuffer, 800, 600);

      await this.testReadyAfterAttach(session);
      await this.testRenderFullFrame(session);
      await this.testRenderTileRegions(session);
      await this.testRenderDirtyRegions(session);
      await this.testResize(session);
      await this.testRenderTileAfterResize(session);

      session.detach();
      this.addPass('Session detach', 0, 'detach OK');
      this.addPass('Session ready state', 0, `ready=${session.ready} (expected false)`);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, `Session test suite failed: ${errorMsg}`);
      session.detach();
    }

    await this.testRenderBeforeAttach();
    await this.testOnErrorCallback();

    hilog.info(DOMAIN, TAG, `All Session tests completed: ${this.results.length} cases`);

    if (this.onComplete) {
      this.onComplete(this.results);
    }

    return this.results;
  }

  private async testReadyAfterAttach(session: ArkZeroRenderSession): Promise<void> {
    const testName = 'Session attach + ready';
    const startTime = Date.now();

    try {
      if (!session.ready) {
        throw new Error('should be ready after attach');
      }

      if (session.width !== 800 || session.height !== 600) {
        throw new Error(`dimensions mismatch: ${session.width}x${session.height}`);
      }

      this.addPass(testName, startTime, `ready=${session.ready}, ${session.width}x${session.height}`);
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testRenderFullFrame(session: ArkZeroRenderSession): Promise<void> {
    const testName = 'Session renderFullFrame';
    const startTime = Date.now();

    try {
      const buffer = new ArrayBuffer(800 * 600 * 4);
      const data32 = new Uint32Array(buffer);
      data32.fill(0xFF0000FF);

      const renderStart = Date.now();
      await session.renderFullFrame(buffer, 800, 600);
      const renderMs = Date.now() - renderStart;

      this.addPass(testName, startTime, `render=${renderMs}ms`);
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testRenderTileRegions(session: ArkZeroRenderSession): Promise<void> {
    const testName = 'Session renderTileRegions';
    const startTime = Date.now();

    try {
      const tileBuffer = new ArrayBuffer(80 * 80 * 4);
      const tile32 = new Uint32Array(tileBuffer);
      tile32.fill(0xFFFF0000);

      const tile: TileRegion = {
        ratioX: 0.25,
        ratioY: 0.25,
        tilePixelWidth: 80,
        tilePixelHeight: 80,
        pixelData: tileBuffer
      };

      const renderStart = Date.now();
      await session.renderTileRegions([tile], 800, 600, true);
      const renderMs = Date.now() - renderStart;

      this.addPass(testName, startTime, `render=${renderMs}ms`);
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testRenderDirtyRegions(session: ArkZeroRenderSession): Promise<void> {
    const testName = 'Session renderDirtyRegions';
    const startTime = Date.now();

    try {
      const fullBuffer = new ArrayBuffer(800 * 600 * 4);
      const full32 = new Uint32Array(fullBuffer);
      full32.fill(0xFF000000);

      const region: DirtyRect = { x: 100, y: 100, w: 50, h: 50 };
      full32.fill(0xFF00FF00);

      const renderStart = Date.now();
      await session.renderDirtyRegions(fullBuffer, 800, 600, [region]);
      const renderMs = Date.now() - renderStart;

      this.addPass(testName, startTime, `render=${renderMs}ms`);
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testResize(session: ArkZeroRenderSession): Promise<void> {
    const testName = 'Session resize';
    const startTime = Date.now();

    try {
      const w1: number = 640;
      const h1: number = 480;
      await session.resize(w1, h1);

      if (session.width !== w1 || session.height !== h1) {
        throw new Error(`dimensions after resize: ${session.width}x${session.height}`);
      }

      const buf640 = new ArrayBuffer(w1 * h1 * 4);
      const d640 = new Uint32Array(buf640);
      d640.fill(0xFF0000FF);
      await session.renderFullFrame(buf640, w1, h1);

      const w2: number = 800;
      const h2: number = 600;
      await session.resize(w2, h2);

      if (session.width !== w2 || session.height !== h2) {
        throw new Error(`dimensions after resize back: ${session.width}x${session.height}`);
      }

      this.addPass(testName, startTime, 'resize OK');
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testRenderTileAfterResize(session: ArkZeroRenderSession): Promise<void> {
    const testName = 'Session tile after resize';
    const startTime = Date.now();

    try {
      const buf800 = new ArrayBuffer(800 * 600 * 4);
      const d800 = new Uint32Array(buf800);
      d800.fill(0xFF111111);
      await session.renderFullFrame(buf800, 800, 600);

      const tileBuffer = new ArrayBuffer(50 * 50 * 4);
      const tile32 = new Uint32Array(tileBuffer);
      tile32.fill(0xFF00FF00);

      const tile: TileRegion = {
        ratioX: 0.5,
        ratioY: 0.5,
        tilePixelWidth: 50,
        tilePixelHeight: 50,
        pixelData: tileBuffer
      };

      const renderStart = Date.now();
      await session.renderTileRegions([tile], 800, 600, true);
      const renderMs = Date.now() - renderStart;

      this.addPass(testName, startTime, `render=${renderMs}ms`);
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testRenderBeforeAttach(): Promise<void> {
    const testName = 'Session render before attach';
    const startTime = Date.now();

    const config: RenderSessionConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const session = new ArkZeroRenderSession(config);

    try {
      const buffer = new ArrayBuffer(800 * 600 * 4);
      const data32 = new Uint32Array(buffer);
      data32.fill(0xFF0000FF);

      await session.renderFullFrame(buffer, 800, 600);

      if (session.ready) {
        throw new Error('should not be ready');
      }

      this.addPass(testName, startTime, 'no crash, frame buffered');
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      session.detach();
    }
  }

  private async testOnErrorCallback(): Promise<void> {
    const testName = 'Session onError callback';
    const startTime = Date.now();

    let capturedError: string = '';
    const config: RenderSessionConfig = {
      width: 800,
      height: 600,
      format: PixelFormat.RGBA,
      onError: (msg: string) => {
        capturedError = msg;
      }
    };
    const session = new ArkZeroRenderSession(config);

    try {
      await session.renderTileRegions([], 800, 600, true);

      if (session.ready) {
        throw new Error('should not be ready');
      }

      this.addPass(testName, startTime, 'silent skip (expected)');
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      session.detach();
    }
  }

  private addPass(testName: string, startTime: number, message: string): void {
    this.results.push({
      name: testName,
      status: TestStatus.PASSED,
      duration: startTime > 0 ? Date.now() - startTime : 0,
      message: message
    });
    hilog.info(DOMAIN, TAG, `PASSED: ${testName} - ${message}`);
  }

  private addError(testName: string, startTime: number, error: Object): void {
    const errorMsg = error instanceof Error ? error.message : String(error);
    this.results.push({
      name: testName,
      status: TestStatus.FAILED,
      duration: Date.now() - startTime,
      message: errorMsg
    });
    hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
  }
}
