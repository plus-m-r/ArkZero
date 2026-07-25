import { ArkZeroRenderer, ArkZeroRendererConfig, PixelFormat, DirtyRect } from '../../components/rendering/ArkZeroRenderer';
import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'IncrBenchmark';

export class IncrementalBenchmarkTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';

  constructor(surfaceId: string, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.onComplete = onComplete;
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting Incremental Benchmark Tests');

    await this.baselineFullFrame();
    await this.separatedSmallDirty();
    await this.separatedMediumDirty();
    await this.separatedAccumulate3();
    await this.separatedAccumulate10();
    await this.separatedVaryingRatio();
    await this.separatedSustained100();
    await this.fullFrameSustained100();

    hilog.info(DOMAIN, TAG, `All Incremental Benchmark tests completed: ${this.results.length} cases`);

    if (this.onComplete) {
      this.onComplete(this.results);
    }

    return this.results;
  }

  private async baselineFullFrame(): Promise<void> {
    const testName = '基线: 全帧50帧';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const frameCount = 50;

      const t0 = Date.now();
      for (let i = 0; i < frameCount; i++) {
        data32.fill(0xFF000000 | ((i * 5) % 256));
        await renderer.renderFrame(pixelBuffer, w, h);
      }
      const elapsed = Date.now() - t0;
      const avg = elapsed / frameCount;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `${frameCount}帧 总${elapsed}ms 平均${avg.toFixed(1)}ms/帧`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedSmallDirty(): Promise<void> {
    const testName = '分离: 1%脏区 update+present';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const frameCount = 50;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const regions: DirtyRect[] = [
        { x: 10, y: 10, w: 80, h: 60 }
      ];

      const t0 = Date.now();
      let uploadTime = 0;
      let presentTime = 0;

      for (let i = 0; i < frameCount; i++) {
        const color = 0xFF000000 | ((i * 7) % 256);
        for (let row = 0; row < 60; row++) {
          const start = (10 + row) * w + 10;
          data32.fill(color, start, start + 80);
        }

        const u0 = Date.now();
        await renderer.updateDirtyRegions(pixelBuffer, w, h, regions);
        uploadTime += Date.now() - u0;

        const p0 = Date.now();
        await renderer.presentFrame();
        presentTime += Date.now() - p0;
      }
      const elapsed = Date.now() - t0;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `总${elapsed}ms 上传${(uploadTime / frameCount).toFixed(1)}ms/帧 呈现${(presentTime / frameCount).toFixed(1)}ms/帧`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedMediumDirty(): Promise<void> {
    const testName = '分离: 10%脏区 update+present';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const frameCount = 50;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const regions: DirtyRect[] = [
        { x: 100, y: 100, w: 250, h: 190 }
      ];

      const t0 = Date.now();
      let uploadTime = 0;
      let presentTime = 0;

      for (let i = 0; i < frameCount; i++) {
        const color = 0xFF000000 | ((i * 11) % 256);
        for (let row = 0; row < 190; row++) {
          const start = (100 + row) * w + 100;
          data32.fill(color, start, start + 250);
        }

        const u0 = Date.now();
        await renderer.updateDirtyRegions(pixelBuffer, w, h, regions);
        uploadTime += Date.now() - u0;

        const p0 = Date.now();
        await renderer.presentFrame();
        presentTime += Date.now() - p0;
      }
      const elapsed = Date.now() - t0;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `250x190(10%) 总${elapsed}ms 上传${(uploadTime / frameCount).toFixed(1)}ms 呈现${(presentTime / frameCount).toFixed(1)}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedAccumulate3(): Promise<void> {
    const testName = '累积: 3帧脏区1次present';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const roundCount = 30;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const regions: DirtyRect[] = [
        { x: 100, y: 100, w: 120, h: 80 }
      ];

      const t0 = Date.now();
      let uploadTime = 0;
      let presentTime = 0;

      for (let round = 0; round < roundCount; round++) {
        for (let sub = 0; sub < 3; sub++) {
          const color = 0xFF000000 | ((round * 3 + sub) % 256);
          for (let row = 0; row < 80; row++) {
            const start = (100 + row) * w + 100;
            data32.fill(color, start, start + 120);
          }

          const u0 = Date.now();
          await renderer.updateDirtyRegions(pixelBuffer, w, h, regions);
          uploadTime += Date.now() - u0;
        }

        const p0 = Date.now();
        await renderer.presentFrame();
        presentTime += Date.now() - p0;
      }
      const elapsed = Date.now() - t0;
      const totalOps = roundCount * 3;
      const avgUpload = uploadTime / totalOps;
      const totalPresentMs = presentTime;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `${roundCount}轮(3update+1present) 总${elapsed}ms 上传${avgUpload.toFixed(1)}ms/次 呈现${(totalPresentMs / roundCount).toFixed(1)}ms/次`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedAccumulate10(): Promise<void> {
    const testName = '累积: 10帧脏区1次present';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const roundCount = 10;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const regions: DirtyRect[] = [
        { x: 50, y: 50, w: 80, h: 60 }
      ];

      const t0 = Date.now();
      let uploadTime = 0;
      let presentTime = 0;

      for (let round = 0; round < roundCount; round++) {
        for (let sub = 0; sub < 10; sub++) {
          const color = 0xFF000000 | ((round * 10 + sub) % 256);
          for (let row = 0; row < 60; row++) {
            const start = (50 + row) * w + 50;
            data32.fill(color, start, start + 80);
          }

          const u0 = Date.now();
          await renderer.updateDirtyRegions(pixelBuffer, w, h, regions);
          uploadTime += Date.now() - u0;
        }

        const p0 = Date.now();
        await renderer.presentFrame();
        presentTime += Date.now() - p0;
      }
      const elapsed = Date.now() - t0;
      const totalUpdates = roundCount * 10;
      const avgUpload = uploadTime / totalUpdates;
      const avgPresent = presentTime / roundCount;
      const effectivePerFrame = elapsed / (roundCount * 10);

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `${roundCount}轮(10update+1present) 上传${avgUpload.toFixed(1)}ms/次 呈现${avgPresent.toFixed(1)}ms/次 等效${effectivePerFrame.toFixed(1)}ms/帧`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedVaryingRatio(): Promise<void> {
    const testName = '分离: 1%/5%/10%/25%/50%脏区对比';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const framesPerRatio = 20;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const ratios: DirtyRect[] = [
        { x: 0, y: 0, w: 80, h: 60 },
        { x: 0, y: 0, w: 178, h: 170 },
        { x: 0, y: 0, w: 253, h: 190 },
        { x: 0, y: 0, w: 400, h: 300 },
        { x: 0, y: 0, w: 566, h: 424 }
      ];

      const labels: string[] = ['1%', '5%', '10%', '25%', '50%'];
      const uploadAvgs: number[] = [];
      const presentAvgs: number[] = [];

      for (let r = 0; r < ratios.length; r++) {
        const rect = ratios[r];
        let uploadTotal = 0;
        let presentTotal = 0;

        for (let i = 0; i < framesPerRatio; i++) {
          const color = 0xFF000000 | ((i * 19 + r * 40) % 256);
          for (let row = 0; row < rect.h; row++) {
            const start = (rect.y + row) * w + rect.x;
            data32.fill(color, start, start + rect.w);
          }

          const u0 = Date.now();
          await renderer.updateDirtyRegions(pixelBuffer, w, h, [rect]);
          uploadTotal += Date.now() - u0;

          const p0 = Date.now();
          await renderer.presentFrame();
          presentTotal += Date.now() - p0;
        }

        uploadAvgs.push(uploadTotal / framesPerRatio);
        presentAvgs.push(presentTotal / framesPerRatio);
      }

      const parts: string[] = labels.map((label: string, idx: number) =>
        `${label}:U=${uploadAvgs[idx].toFixed(1)}ms+P=${presentAvgs[idx].toFixed(1)}ms`
      );

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: parts.join(' | ')
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedScattered20(): Promise<void> {
    const testName = '分离: 20个分散小区域 update+present';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const frameCount = 30;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const regions: DirtyRect[] = [];
      for (let i = 0; i < 20; i++) {
        regions.push({
          x: (i % 10) * 75 + 10,
          y: Math.floor(i / 10) * 280 + 20,
          w: 20,
          h: 20
        });
      }

      const t0 = Date.now();
      let uploadTime = 0;
      let presentTime = 0;

      for (let i = 0; i < frameCount; i++) {
        const color = 0xFF000000 | ((i * 23) % 256);
        for (let r = 0; r < regions.length; r++) {
          const rect = regions[r];
          for (let row = 0; row < rect.h; row++) {
            const start = (rect.y + row) * w + rect.x;
            data32.fill(color, start, start + rect.w);
          }
        }

        const u0 = Date.now();
        await renderer.updateDirtyRegions(pixelBuffer, w, h, regions);
        uploadTime += Date.now() - u0;

        const p0 = Date.now();
        await renderer.presentFrame();
        presentTime += Date.now() - p0;
      }
      const elapsed = Date.now() - t0;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `20区(8000px) 总${elapsed}ms 上传${(uploadTime / frameCount).toFixed(1)}ms 呈现${(presentTime / frameCount).toFixed(1)}ms`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async separatedSustained100(): Promise<void> {
    const testName = '分离: 增量100帧耐力';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const frameCount = 100;

      data32.fill(0xFF0000FF);
      await renderer.renderFrame(pixelBuffer, w, h);

      const regions: DirtyRect[] = [
        { x: 100, y: 100, w: 120, h: 80 }
      ];

      let minTime = Number.MAX_VALUE;
      let maxTime = 0;
      let totalTime = 0;

      for (let i = 0; i < frameCount; i++) {
        const color = 0xFF000000 | ((i * 3) % 256);
        for (let row = 0; row < 80; row++) {
          const start = (100 + row) * w + 100;
          data32.fill(color, start, start + 120);
        }

        const f0 = Date.now();
        await renderer.updateDirtyRegions(pixelBuffer, w, h, regions);
        await renderer.presentFrame();
        const fElapsed = Date.now() - f0;

        if (fElapsed < minTime) minTime = fElapsed;
        if (fElapsed > maxTime) maxTime = fElapsed;
        totalTime += fElapsed;
      }

      const avg = totalTime / frameCount;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `120x80(2%) min=${minTime}ms max=${maxTime}ms avg=${avg.toFixed(1)}ms/帧`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
    }
  }

  private async fullFrameSustained100(): Promise<void> {
    const testName = '耐力: 全帧100帧';
    const startTime = Date.now();

    const config: ArkZeroRendererConfig = { width: 800, height: 600, format: PixelFormat.RGBA };
    const renderer = new ArkZeroRenderer(config);

    try {
      await renderer.initialize(this.surfaceId);

      const w = 800;
      const h = 600;
      const bufferSize = w * h * 4;
      const pixelBuffer = new ArrayBuffer(bufferSize);
      const data32 = new Uint32Array(pixelBuffer);
      const frameCount = 100;

      let minTime = Number.MAX_VALUE;
      let maxTime = 0;
      let totalTime = 0;

      for (let i = 0; i < frameCount; i++) {
        data32.fill(0xFF000000 | ((i * 3) % 256));

        const f0 = Date.now();
        await renderer.renderFrame(pixelBuffer, w, h);
        const fElapsed = Date.now() - f0;

        if (fElapsed < minTime) minTime = fElapsed;
        if (fElapsed > maxTime) maxTime = fElapsed;
        totalTime += fElapsed;
      }

      const avg = totalTime / frameCount;

      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: Date.now() - startTime,
        message: `全帧 min=${minTime}ms max=${maxTime}ms avg=${avg.toFixed(1)}ms/帧`
      });
    } catch (error) {
      this.addError(testName, startTime, error);
    } finally {
      renderer.dispose();
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
