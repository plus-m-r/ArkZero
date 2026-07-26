import { ArkZeroRenderer, ArkZeroRendererConfig, PixelFormat, TileRegion } from '@plusml/arkzero';
import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'TileRenderTest';

export class TileRenderIntegrationTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';

  constructor(surfaceId: string, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.onComplete = onComplete;
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting Tile Render Integration Tests');

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const warmBuffer = new ArrayBuffer(800 * 600 * 4);
      const warm32 = new Uint32Array(warmBuffer);
      warm32.fill(0xFF000000);
      await renderer.renderFrame(warmBuffer, 800, 600);

      await this.testSingleTileRender(renderer);
      await this.testMultipleTilesRender(renderer);
      await this.testTileRenderThenFullFrame(renderer);
      await this.testTileRenderNoSwap(renderer);
      await this.testTileRenderAfterResize(renderer);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, `Test setup failed: ${errorMsg}`);
    } finally {
      renderer.dispose();
    }

    hilog.info(DOMAIN, TAG, `All Tile Render tests completed: ${this.results.length} cases`);

    if (this.onComplete) {
      this.onComplete(this.results);
    }

    return this.results;
  }

  private async testSingleTileRender(renderer: ArkZeroRenderer): Promise<void> {
    const testName = '单Tile渲染';
    const startTime = Date.now();

    try {
      const tileW = 120;
      const tileH = 80;
      const tileBuffer = new ArrayBuffer(tileW * tileH * 4);
      const tile32 = new Uint32Array(tileBuffer);
      tile32.fill(0xFF0000FF);

      const tile: TileRegion = {
        ratioX: 0.1,
        ratioY: 0.2,
        tilePixelWidth: tileW,
        tilePixelHeight: tileH,
        pixelData: tileBuffer
      };

      const renderStart = Date.now();
      await renderer.renderTileRegions([tile], 800, 600, true);
      const renderMs = Date.now() - renderStart;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `120x80 tile at (10%,20%) OK, render=${renderMs}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testMultipleTilesRender(renderer: ArkZeroRenderer): Promise<void> {
    const testName = '多Tile渲染(4个)';
    const startTime = Date.now();

    try {
      const tileW = 80;
      const tileH = 60;
      const tiles: TileRegion[] = [];

      const positions: number[][] = [
        [0.0, 0.0],
        [0.5, 0.0],
        [0.0, 0.5],
        [0.5, 0.5]
      ];

      const colors: number[] = [0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00];

      for (let i = 0; i < 4; i++) {
        const tileBuffer = new ArrayBuffer(tileW * tileH * 4);
        const tile32 = new Uint32Array(tileBuffer);
        tile32.fill(colors[i]);

        const tile: TileRegion = {
          ratioX: positions[i][0],
          ratioY: positions[i][1],
          tilePixelWidth: tileW,
          tilePixelHeight: tileH,
          pixelData: tileBuffer
        };
        tiles.push(tile);
      }

      const renderStart = Date.now();
      await renderer.renderTileRegions(tiles, 800, 600, true);
      const renderMs = Date.now() - renderStart;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `4x 80x60 tiles OK, render=${renderMs}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testTileRenderThenFullFrame(renderer: ArkZeroRenderer): Promise<void> {
    const testName = 'Tile渲染后接全帧';
    const startTime = Date.now();

    try {
      const tileBuffer = new ArrayBuffer(100 * 100 * 4);
      const tile32 = new Uint32Array(tileBuffer);
      tile32.fill(0xFF00FF00);

      const tile: TileRegion = {
        ratioX: 0.25,
        ratioY: 0.25,
        tilePixelWidth: 100,
        tilePixelHeight: 100,
        pixelData: tileBuffer
      };

      const renderStart = Date.now();
      await renderer.renderTileRegions([tile], 800, 600, false);

      const fullBuffer = new ArrayBuffer(800 * 600 * 4);
      const full32 = new Uint32Array(fullBuffer);
      full32.fill(0xFF0000FF);
      await renderer.renderFrame(fullBuffer, 800, 600);
      const renderMs = Date.now() - renderStart;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `tile(no-swap) + fullFrame OK, render=${renderMs}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testTileRenderNoSwap(renderer: ArkZeroRenderer): Promise<void> {
    const testName = 'Tile渲染no-swap+present';
    const startTime = Date.now();

    try {
      const tile1Buffer = new ArrayBuffer(60 * 60 * 4);
      const tile1_32 = new Uint32Array(tile1Buffer);
      tile1_32.fill(0xFFFF0000);

      const tile1: TileRegion = {
        ratioX: 0.0,
        ratioY: 0.0,
        tilePixelWidth: 60,
        tilePixelHeight: 60,
        pixelData: tile1Buffer
      };

      const renderStart = Date.now();
      await renderer.renderTileRegions([tile1], 800, 600, false);

      const tile2Buffer = new ArrayBuffer(60 * 60 * 4);
      const tile2_32 = new Uint32Array(tile2Buffer);
      tile2_32.fill(0xFF0000FF);

      const tile2: TileRegion = {
        ratioX: 0.1,
        ratioY: 0.1,
        tilePixelWidth: 60,
        tilePixelHeight: 60,
        pixelData: tile2Buffer
      };

      await renderer.renderTileRegions([tile2], 800, 600, false);

      await renderer.presentFrame();
      const renderMs = Date.now() - renderStart;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `2 tiles (no-swap) + present OK, render=${renderMs}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    }
  }

  private async testTileRenderAfterResize(renderer: ArkZeroRenderer): Promise<void> {
    const testName = 'resize后Tile渲染';
    const startTime = Date.now();

    try {
      await renderer.resize(640, 480);

      const buf640 = new ArrayBuffer(640 * 480 * 4);
      const d640 = new Uint32Array(buf640);
      d640.fill(0xFF000000);
      await renderer.renderFrame(buf640, 640, 480);

      await renderer.resize(800, 600);

      const buf800 = new ArrayBuffer(800 * 600 * 4);
      const d800 = new Uint32Array(buf800);
      d800.fill(0xFF111111);
      await renderer.renderFrame(buf800, 800, 600);

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
      await renderer.renderTileRegions([tile], 800, 600, true);
      const renderMs = Date.now() - renderStart;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `resize + tile OK, render=${renderMs}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    }
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
