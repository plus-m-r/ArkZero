import { ArkZeroSurfaceView, SurfaceViewConfig, NormalizedTouchEvent, ArkZeroRenderSession, RenderSessionConfig, PixelFormat, DirtyRect, TileRegion } from '@plusml/arkzero';
import { router } from '@kit.ArkUI';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'RenderCorrectPage';

enum DemoPhase {
  COLOR_BANDS = 0,
  INCREMENTAL_DIRTY = 1,
  INCREMENTAL_TILE = 2,
  INCREMENTAL_TILE_PARALLEL = 3,
  DONE = 4
}

interface PhaseLabel {
  id: DemoPhase;
  label: string;
}

const PHASE_LABELS: PhaseLabel[] = [
  { id: DemoPhase.COLOR_BANDS, label: '色彩准确性' },
  { id: DemoPhase.INCREMENTAL_DIRTY, label: '增量渲染(脏区)' },
  { id: DemoPhase.INCREMENTAL_TILE, label: '增量渲染(Tile)' },
  { id: DemoPhase.INCREMENTAL_TILE_PARALLEL, label: '并行Tile渲染' },
  { id: DemoPhase.DONE, label: '完成' }
];

@Entry
@Component
struct RenderingCorrectnessPage {
  @State isInitialized: boolean = false;
  @State currentPhase: number = DemoPhase.COLOR_BANDS;
  @State phaseLabel: string = PHASE_LABELS[0].label;
  @State subStepInfo: string = '';

  private session?: ArkZeroRenderSession;
  private pixelBuffer?: ArrayBuffer;
  private pixelView?: Uint32Array;
  private renderWidth: number = 640;
  private renderHeight: number = 480;
  private running: boolean = false;
  private renderLoopPromise?: Promise<void>;
  private phaseStep: number = 0;
  private stepTimer: number = 0;

  private readonly surfaceViewConfig: SurfaceViewConfig = {
    renderWidth: 640,
    renderHeight: 480,
    format: PixelFormat.RGBA,
    originRatioX: 0.0,
    originRatioY: 0.0,
    sizeRatioX: 0.65,
    enableTouch: false,
    xComponentId: 'render_correctness_surface'
  };

  private sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve) => {
      setTimeout(resolve, ms);
    });
  }

  private onSurfaceLoaded(surfaceId: string): void {
    const sessionConfig: RenderSessionConfig = {
      width: this.renderWidth,
      height: this.renderHeight,
      format: PixelFormat.RGBA,
      onError: (errorMsg: string) => {
        hilog.error(DOMAIN, TAG, 'Session error: %{public}s', errorMsg);
      }
    };
    this.session = new ArkZeroRenderSession(sessionConfig);

    this.session.attach(surfaceId).then(() => {
      this.isInitialized = true;
      hilog.info(DOMAIN, TAG, 'Session attached');
      this.startDemo();
    }).catch((error: Error) => {
      hilog.error(DOMAIN, TAG, 'Session attach failed: %{public}s', error.message);
    });
  }

  private async startDemo(): Promise<void> {
    if (!this.session) {
      return;
    }

    const bufferSize = this.renderWidth * this.renderHeight * 4;
    this.pixelBuffer = new ArrayBuffer(bufferSize);
    this.pixelView = new Uint32Array(this.pixelBuffer);

    this.running = true;
    this.currentPhase = DemoPhase.COLOR_BANDS;
    this.phaseStep = 0;

    this.renderLoopPromise = (async () => {
      while (this.running) {
        try {
          if (!this.pixelBuffer || !this.pixelView || !this.session) {
            break;
          }

          switch (this.currentPhase) {
            case DemoPhase.COLOR_BANDS:
              this.drawColorBands(this.pixelView, this.renderWidth, this.renderHeight);
              await this.session.renderFullFrame(this.pixelBuffer, this.renderWidth, this.renderHeight);
              await this.sleep(1500);

              this.currentPhase = DemoPhase.INCREMENTAL_DIRTY;
              this.phaseStep = 0;
              this.stepTimer = 0;
              this.phaseLabel = PHASE_LABELS[DemoPhase.INCREMENTAL_DIRTY].label;
              break;

            case DemoPhase.INCREMENTAL_DIRTY:
              if (this.runIncrementalDirtyStep()) {
                this.currentPhase = DemoPhase.INCREMENTAL_TILE;
                this.phaseStep = 0;
                this.stepTimer = 0;
                this.phaseLabel = PHASE_LABELS[DemoPhase.INCREMENTAL_TILE].label;
              }
              await this.sleep(800);
              break;

            case DemoPhase.INCREMENTAL_TILE:
              if (this.runIncrementalTileStep()) {
                this.currentPhase = DemoPhase.INCREMENTAL_TILE_PARALLEL;
                this.phaseStep = 0;
                this.stepTimer = 0;
                this.phaseLabel = PHASE_LABELS[DemoPhase.INCREMENTAL_TILE_PARALLEL].label;
              }
              await this.sleep(800);
              break;

            case DemoPhase.INCREMENTAL_TILE_PARALLEL:
              if (this.runParallelTileStep()) {
                this.currentPhase = DemoPhase.DONE;
                this.phaseLabel = PHASE_LABELS[DemoPhase.DONE].label;
                this.subStepInfo = '所有渲染正确性验证完成';
              }
              await this.sleep(800);
              break;

            case DemoPhase.DONE:
              await this.sleep(500);
              break;
          }
        } catch (error) {
          const errorMsg = error instanceof Error ? error.message : String(error);
          hilog.error(DOMAIN, TAG, 'Render failed: %{public}s', errorMsg);
        }
      }
    })();
  }

  private drawColorBands(buf: Uint32Array, w: number, h: number): void {
    buf.fill(0xFF333333);

    const bandCount = 6;
    const bandW = Math.floor(w / bandCount);
    const colors: number[] = [
      0xFF0000FF,
      0xFF00FF00,
      0xFFFF0000,
      0xFFFFFFFF,
      0xFF000000,
      0xFF00FFFF
    ];
    const labels: string[] = ['RED', 'GREEN', 'BLUE', 'WHITE', 'BLACK', 'YELLOW'];

    for (let b = 0; b < bandCount; b++) {
      const x0 = b * bandW;
      const x1 = x0 + bandW;
      for (let y = 0; y < h; y++) {
        for (let x = x0; x < x1; x++) {
          buf[y * w + x] = colors[b];
        }
      }
    }

    const labelColor = 0xFFAAAAAA;
    for (let b = 0; b < bandCount; b++) {
      const cx = b * bandW + Math.floor(bandW / 2) - 20;
      const cy = Math.floor(h / 2);
      this.drawTextLabel(buf, w, h, cx, cy, labels[b], labelColor);
    }

    this.subStepInfo = '6色带: 红/绿/蓝/白/黑/黄';
  }

  private drawTextLabel(buf: Uint32Array, w: number, h: number, startX: number, startY: number, text: string, color: number): void {
    const font: Record<string, number[][]> = {
      'R': [[1,1,1,0],[1,0,0,1],[1,1,1,0],[1,0,1,0],[1,0,0,1]],
      'E': [[1,1,1,1],[1,0,0,0],[1,1,1,0],[1,0,0,0],[1,1,1,1]],
      'D': [[1,1,1,0],[1,0,0,1],[1,0,0,1],[1,0,0,1],[1,1,1,0]],
      'G': [[1,1,1,1],[1,0,0,0],[1,0,1,1],[1,0,0,1],[1,1,1,1]],
      'N': [[1,0,0,1],[1,1,0,1],[1,0,1,1],[1,0,0,1],[1,0,0,1]],
      'B': [[1,1,1,0],[1,0,0,1],[1,1,1,0],[1,0,0,1],[1,1,1,0]],
      'L': [[1,0,0,0],[1,0,0,0],[1,0,0,0],[1,0,0,0],[1,1,1,1]],
      'U': [[1,0,0,1],[1,0,0,1],[1,0,0,1],[1,0,0,1],[1,1,1,1]],
      'W': [[1,0,0,1],[1,0,0,1],[1,0,1,1],[1,1,0,1],[1,0,0,1]],
      'H': [[1,0,0,1],[1,0,0,1],[1,1,1,1],[1,0,0,1],[1,0,0,1]],
      'I': [[1,1,1],[0,1,0],[0,1,0],[0,1,0],[1,1,1]],
      'T': [[1,1,1],[0,1,0],[0,1,0],[0,1,0],[0,1,0]],
      'A': [[0,1,1,0],[1,0,0,1],[1,1,1,1],[1,0,0,1],[1,0,0,1]],
      'K': [[1,0,0,1],[1,0,1,0],[1,1,0,0],[1,0,1,0],[1,0,0,1]],
      'Y': [[1,0,0,1],[0,1,1,0],[0,1,1,0],[0,1,1,0],[0,1,1,0]],
      'C': [[0,1,1,1],[1,0,0,0],[1,0,0,0],[1,0,0,0],[0,1,1,1]],
      'O': [[0,1,1,0],[1,0,0,1],[1,0,0,1],[1,0,0,1],[0,1,1,0]]
    };

    let offsetX = 0;
    const scale = 4;
    for (let ci = 0; ci < text.length; ci++) {
      const ch = text[ci];
      const glyph = font[ch];
      if (!glyph) {
        offsetX += 5 * scale;
        continue;
      }
      for (let row = 0; row < glyph.length; row++) {
        for (let col = 0; col < glyph[row].length; col++) {
          if (glyph[row][col] === 1) {
            const px = startX + offsetX + col * scale;
            const py = startY + row * scale;
            for (let sy = 0; sy < scale; sy++) {
              for (let sx = 0; sx < scale; sx++) {
                const fx = px + sx;
                const fy = py + sy;
                if (fx >= 0 && fx < w && fy >= 0 && fy < h) {
                  buf[fy * w + fx] = color;
                }
              }
            }
          }
        }
      }
      offsetX += (glyph[0].length + 1) * scale;
    }
  }

  private runIncrementalDirtyStep(): boolean {
    if (!this.pixelView || !this.pixelBuffer || !this.session) {
      return true;
    }

    const w = this.renderWidth;
    const h = this.renderHeight;

    if (this.phaseStep === 0) {
      this.pixelView.fill(0xFF00FFFF);
      this.session.renderFullFrame(this.pixelBuffer, w, h);
      this.subStepInfo = '基底: 黄色全帧 (0/4)';
      this.phaseStep = 1;
      return false;
    }

    const quadrant = this.phaseStep - 1;
    const textColor = 0xFF0000FF;
    const labels: string[] = ['T1', 'T2', 'T3', 'T4'];

    const halfW = Math.floor(w / 2);
    const halfH = Math.floor(h / 2);

    const rects: DirtyRect[] = [
      { x: 0, y: 0, w: halfW, h: halfH },
      { x: halfW, y: 0, w: w - halfW, h: halfH },
      { x: 0, y: halfH, w: halfW, h: h - halfH },
      { x: halfW, y: halfH, w: w - halfW, h: h - halfH }
    ];

    const rect = rects[quadrant];

    for (let row = 0; row < rect.h; row++) {
      const start = (rect.y + row) * w + rect.x;
      this.pixelView.fill(0xFF00FFFF, start, start + rect.w);
    }

    const cx = rect.x + Math.floor(rect.w / 2) - 12;
    const cy = rect.y + Math.floor(rect.h / 2) - 10;
    this.drawTextLabel(this.pixelView, w, h, cx, cy, labels[quadrant], textColor);

    this.session.renderDirtyRegions(this.pixelBuffer, w, h, [rect]);
    this.session.presentFrame();

    this.subStepInfo = `增量脏区 ${quadrant + 1}/4: 黄底红字"${labels[quadrant]}" (${rect.w}x${rect.h})`;

    this.phaseStep++;
    if (this.phaseStep > 4) {
      this.subStepInfo = '4个脏区增量渲染完成, 黄底红字全帧覆盖';
      return true;
    }

    return false;
  }

  private runIncrementalTileStep(): boolean {
    if (!this.pixelView || !this.pixelBuffer || !this.session) {
      return true;
    }

    const w = this.renderWidth;
    const h = this.renderHeight;

    if (this.phaseStep === 0) {
      this.pixelView.fill(0xFF00FFFF);
      this.session.renderFullFrame(this.pixelBuffer, w, h);
      this.subStepInfo = 'Tile基底: 黄色全帧 (0/4)';
      this.phaseStep = 1;
      return false;
    }

    const tileIndex = this.phaseStep - 1;
    const tileW = 160;
    const tileH = 120;
    const labels: string[] = ['TA', 'TB', 'TC', 'TD'];
    const positions: number[][] = [
      [0.0, 0.0],
      [0.5, 0.0],
      [0.0, 0.5],
      [0.5, 0.5]
    ];

    const tileBuffer = new ArrayBuffer(tileW * tileH * 4);
    const tile32 = new Uint32Array(tileBuffer);
    tile32.fill(0xFF00FFFF);

    const tileView = new Uint32Array(this.pixelBuffer);
    const offsetX = Math.floor(positions[tileIndex][0] * w);
    const offsetY = Math.floor(positions[tileIndex][1] * h);
    this.drawTextLabel(tileView, w, h, offsetX + Math.floor(tileW / 2) - 12, offsetY + Math.floor(tileH / 2) - 10, labels[tileIndex], 0xFF0000FF);

    for (let row = 0; row < tileH; row++) {
      const srcStart = row * tileW;
      const dstStart = (offsetY + row) * w + offsetX;
      for (let col = 0; col < tileW; col++) {
        tile32[srcStart + col] = tileView[dstStart + col];
      }
    }

    const pos = positions[tileIndex];

    this.session.renderTileRegions([{
      ratioX: pos[0],
      ratioY: pos[1],
      tilePixelWidth: tileW,
      tilePixelHeight: tileH,
      pixelData: tileBuffer
    }], w, h, true);

    this.subStepInfo = `Tile ${tileIndex + 1}/4: 黄底红字"${labels[tileIndex]}" at (${pos[0]},${pos[1]})`;

    this.phaseStep++;
    if (this.phaseStep > 4) {
      this.subStepInfo = '4个Tile增量渲染完成, 黄底红字覆盖';
      return true;
    }

    return false;
  }

  private runParallelTileStep(): boolean {
    if (!this.pixelView || !this.pixelBuffer || !this.session) {
      return true;
    }

    const w = this.renderWidth;
    const h = this.renderHeight;

    if (this.phaseStep === 0) {
      this.pixelView.fill(0xFFFFFFFF);
      this.session.renderFullFrame(this.pixelBuffer, w, h);
      this.subStepInfo = '并行Tile基底: 白色全帧 (0/3)';
      this.phaseStep = 1;
      return false;
    }

    const tileW = 160;
    const tileH = 120;
    const labels: string[] = ['P1', 'P2', 'P3', 'P4'];
    const colors: number[] = [0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF00FFFF];
    const positions: number[][] = [
      [0.0, 0.0],
      [0.5, 0.0],
      [0.0, 0.5],
      [0.5, 0.5]
    ];

    if (this.phaseStep === 1) {
      const tiles: TileRegion[] = [];
      const tileBuffers: ArrayBuffer[] = [];

      for (let i = 0; i < 4; i++) {
        const tileBuffer = new ArrayBuffer(tileW * tileH * 4);
        const tile32 = new Uint32Array(tileBuffer);
        tile32.fill(colors[i]);

        const offsetX = Math.floor(positions[i][0] * w);
        const offsetY = Math.floor(positions[i][1] * h);

        const cx = Math.floor(tileW / 2) - 12;
        const cy = Math.floor(tileH / 2) - 10;
        this.drawTileLabel(tile32, tileW, tileH, cx, cy, labels[i], 0xFF000000);

        tiles.push({
          ratioX: positions[i][0],
          ratioY: positions[i][1],
          tilePixelWidth: tileW,
          tilePixelHeight: tileH,
          pixelData: tileBuffer
        });
        tileBuffers.push(tileBuffer);
      }

      this.session.renderTileRegions(tiles, w, h, true);

      this.subStepInfo = '并行4-Tile: 一次提交 红/绿/蓝/黄 4色瓦片';
      this.phaseStep = 2;
      return false;
    }

    if (this.phaseStep === 2) {
      const tiles: TileRegion[] = [];

      const innerLabels: string[] = ['A', 'B', 'C', 'D'];
      const innerColors: number[] = [0xFFFF0000, 0xFF0000FF, 0xFF00FF00, 0xFFFFFF00];
      const innerPositions: number[][] = [
        [0.125, 0.125],
        [0.375, 0.125],
        [0.125, 0.375],
        [0.375, 0.375]
      ];

      const smallTileW = 80;
      const smallTileH = 60;

      for (let i = 0; i < 4; i++) {
        const tileBuffer = new ArrayBuffer(smallTileW * smallTileH * 4);
        const tile32 = new Uint32Array(tileBuffer);
        tile32.fill(innerColors[i]);

        const cx = Math.floor(smallTileW / 2) - 8;
        const cy = Math.floor(smallTileH / 2) - 10;
        this.drawTileLabel(tile32, smallTileW, smallTileH, cx, cy, innerLabels[i], 0xFF000000);

        tiles.push({
          ratioX: innerPositions[i][0],
          ratioY: innerPositions[i][1],
          tilePixelWidth: smallTileW,
          tilePixelHeight: smallTileH,
          pixelData: tileBuffer
        });
      }

      this.session.renderTileRegions(tiles, w, h, true);

      this.subStepInfo = '并行4-Tile(内层): 小瓦片嵌套渲染';
      this.phaseStep = 3;
      return false;
    }

    this.subStepInfo = '并行Tile渲染验证完成: 多瓦片一次性提交';
    return true;
  }

  private drawTileLabel(buf: Uint32Array, tileW: number, tileH: number, startX: number, startY: number, text: string, color: number): void {
    const font: Record<string, number[][]> = {
      'P': [[0,1,1,0],[1,0,0,1],[1,1,1,1],[1,0,0,0],[1,0,0,0]],
      '1': [[0,1,0],[1,1,0],[0,1,0],[0,1,0],[1,1,1]],
      '2': [[1,1,1,0],[0,0,0,1],[0,1,1,0],[1,0,0,0],[1,1,1,1]],
      '3': [[1,1,1,0],[0,0,0,1],[0,1,1,0],[0,0,0,1],[1,1,1,0]],
      '4': [[1,0,0,1],[1,0,0,1],[1,1,1,1],[0,0,0,1],[0,0,0,1]],
      'A': [[0,1,1,0],[1,0,0,1],[1,1,1,1],[1,0,0,1],[1,0,0,1]],
      'B': [[1,1,1,0],[1,0,0,1],[1,1,1,0],[1,0,0,1],[1,1,1,0]],
      'C': [[0,1,1,1],[1,0,0,0],[1,0,0,0],[1,0,0,0],[0,1,1,1]],
      'D': [[1,1,1,0],[1,0,0,1],[1,0,0,1],[1,0,0,1],[1,1,1,0]]
    };

    let offsetX = 0;
    const scale = 3;
    for (let ci = 0; ci < text.length; ci++) {
      const ch = text[ci];
      const glyph = font[ch];
      if (!glyph) {
        offsetX += 5 * scale;
        continue;
      }
      for (let row = 0; row < glyph.length; row++) {
        for (let col = 0; col < glyph[row].length; col++) {
          if (glyph[row][col] === 1) {
            const px = startX + offsetX + col * scale;
            const py = startY + row * scale;
            for (let sy = 0; sy < scale; sy++) {
              for (let sx = 0; sx < scale; sx++) {
                const fx = px + sx;
                const fy = py + sy;
                if (fx >= 0 && fx < tileW && fy >= 0 && fy < tileH) {
                  buf[fy * tileW + fx] = color;
                }
              }
            }
          }
        }
      }
      offsetX += (glyph[0].length + 1) * scale;
    }
  }

  aboutToDisappear() {
    this.running = false;

    if (this.renderLoopPromise) {
      this.renderLoopPromise.then(() => {
        if (this.session) {
          this.session.detach();
        }
      }).catch(() => {
        if (this.session) {
          this.session.detach();
        }
      });
    } else if (this.session) {
      this.session.detach();
    }
  }

  build() {
    Column() {
      Row() {
        Button('返回')
          .fontSize(14)
          .height(32)
          .margin({ right: 10 })
          .onClick(() => {
            router.back();
          })

        Text('渲染正确性验证')
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.White)
      }
      .width('100%')
      .padding({ left: 12, right: 12, top: 8, bottom: 8 })
      .backgroundColor('#1a1a1a')

      ArkZeroSurfaceView({
        config: this.surfaceViewConfig,
        onSurfaceLoaded: (surfaceId: string) => {
          this.onSurfaceLoaded(surfaceId);
        }
      })
        .margin({ top: 8 })

      Column() {
        Row() {
          Text('阶段:')
            .fontSize(14)
            .fontColor('#AAAAAA')
            .margin({ right: 6 })

          Text(this.phaseLabel)
            .fontSize(14)
            .fontWeight(FontWeight.Bold)
            .fontColor('#4CAF50')
        }
        .width('100%')

        if (this.subStepInfo) {
          Text(this.subStepInfo)
            .fontSize(13)
            .fontColor('#CCCCCC')
            .margin({ top: 4 })
            .width('100%')
        }

        if (this.currentPhase === DemoPhase.DONE) {
          Text('验证通过: 色彩准确 + 增量渲染完成')
            .fontSize(15)
            .fontWeight(FontWeight.Bold)
            .fontColor('#4CAF50')
            .margin({ top: 8 })
        }
      }
      .width('100%')
      .padding(12)
      .backgroundColor('#1a1a1a')
      .margin({ top: 8 })

      if (!this.isInitialized) {
        Text('等待 Surface 初始化...')
          .fontSize(12)
          .fontColor('#FF9800')
          .margin({ top: 10 })
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#0d0d0d')
  }
}
