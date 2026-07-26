import { ArkZeroSurfaceView, SurfaceViewConfig, NormalizedTouchEvent, ArkZeroRenderSession, RenderSessionConfig, PixelFormat, TileRegion } from '@plusml/arkzero';
import { router } from '@kit.ArkUI';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'SurfaceDemoPage';

interface TouchFeedback {
  x: number;
  y: number;
  radius: number;
  color: number;
  ttl: number;
}

@Entry
@Component
struct SurfaceDemoPage {
  @State isInitialized: boolean = false;
  @State frameCount: number = 0;

  private session?: ArkZeroRenderSession;
  private pixelBuffer?: ArrayBuffer;
  private pixelView?: Uint32Array;
  private running: boolean = false;
  private renderLoopPromise?: Promise<void>;

  private readonly surfaceViewConfig: SurfaceViewConfig = {
    renderWidth: 640,
    renderHeight: 480,
    format: PixelFormat.RGBA,
    originRatioX: 0.056,
    originRatioY: 0.0,
    sizeRatioX: 1.0,
    enableTouch: true,
    xComponentId: 'surface_renderer'
  };

  private activeTouches: TouchFeedback[] = [];

  private drawCircle(bufferView: Uint32Array, bufWidth: number, bufHeight: number,
                     cx: number, cy: number, radius: number, color: number): void {
    const r2 = radius * radius;
    const yStart = Math.max(0, cy - radius);
    const yEnd = Math.min(bufHeight - 1, cy + radius);
    const xStart = Math.max(0, cx - radius);
    const xEnd = Math.min(bufWidth - 1, cx + radius);

    for (let y = yStart; y <= yEnd; y++) {
      const dy = y - cy;
      const rowOffset = y * bufWidth;
      for (let x = xStart; x <= xEnd; x++) {
        const dx = x - cx;
        if (dx * dx + dy * dy <= r2) {
          bufferView[rowOffset + x] = color;
        }
      }
    }
  }

  private generateTestPattern(bufferView: Uint32Array, width: number, height: number, time: number): void {
    bufferView.fill(0xFFFFFFFF);

    const barWidth = 120;
    const barHeight = 120;
    const movingX = Math.floor((time / 4) % Math.max(1, width - barWidth));
    const movingY = Math.floor((time / 6) % Math.max(1, height - barHeight));

    for (let y = 0; y < height; y++) {
      const rowOffset = y * width;
      for (let x = movingX; x < movingX + barWidth; x++) {
        if (x >= 0 && x < width) {
          bufferView[rowOffset + x] = 0xFFFF0000;
        }
      }
    }

    for (let y = movingY; y < movingY + barHeight; y++) {
      if (y >= 0 && y < height) {
        const rowOffset = y * width;
        for (let x = 0; x < width; x += 2) {
          bufferView[rowOffset + x] = 0xFF0000FF;
        }
      }
    }

    for (let i = 0; i < this.activeTouches.length; i++) {
      const t = this.activeTouches[i];
      this.drawCircle(bufferView, width, height, t.x, t.y, t.radius, t.color);
    }
  }

  private async sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve) => {
      setTimeout(resolve, ms);
    });
  }

  private onSurfaceLoaded(surfaceId: string): void {
    const rw = this.surfaceViewConfig.renderWidth;
    const rh = this.surfaceViewConfig.renderHeight;
    const sessionConfig: RenderSessionConfig = {
      width: rw,
      height: rh,
      format: PixelFormat.RGBA,
      onError: (errorMsg: string) => {
        hilog.error(DOMAIN, TAG, 'Session error: %{public}s', errorMsg);
      }
    };
    this.session = new ArkZeroRenderSession(sessionConfig);

    this.session.attach(surfaceId).then(() => {
      this.isInitialized = true;
      hilog.info(DOMAIN, TAG, 'Session attached');
      this.startRendering();
    }).catch((error: Error) => {
      hilog.error(DOMAIN, TAG, 'Session attach failed: %{public}s', error.message);
    });
  }

  private async startRendering(): Promise<void> {
    if (!this.session || !this.isInitialized) {
      hilog.error(DOMAIN, TAG, 'Session not initialized');
      return;
    }

    const rw = this.surfaceViewConfig.renderWidth;
    const rh = this.surfaceViewConfig.renderHeight;
    const bufferSize = rw * rh * 4;
    this.pixelBuffer = new ArrayBuffer(bufferSize);
    this.pixelView = new Uint32Array(this.pixelBuffer);

    let frameTime = 0;
    this.running = true;

    this.renderLoopPromise = (async () => {
      while (this.running) {
        try {
          this.generateTestPattern(this.pixelView!, rw, rh, frameTime);

          await this.session!.renderFullFrame(this.pixelBuffer!, rw, rh);

          if (!this.running) break;

          frameTime += 8;

          let writeIdx = 0;
          for (let i = 0; i < this.activeTouches.length; i++) {
            this.activeTouches[i].ttl--;
            if (this.activeTouches[i].ttl > 0) {
              this.activeTouches[writeIdx] = this.activeTouches[i];
              writeIdx++;
            }
          }
          this.activeTouches.length = writeIdx;

        } catch (error) {
          const errorMsg = error instanceof Error ? error.message : String(error);
          hilog.error(DOMAIN, TAG, 'Render failed: %{public}s', errorMsg);
          if (!this.running) break;
        }

        await this.sleep(100);
      }
    })();

    hilog.info(DOMAIN, TAG, 'Rendering started: %{public}dx%{public}d', rw, rh);
  }

  private handleTouch(event: NormalizedTouchEvent): void {
    if (event.type === TouchType.Down || event.type === TouchType.Up || event.type === TouchType.Cancel) {
      for (let i = 0; i < event.touches.length; i++) {
        const touch = event.touches[i];
        hilog.info(DOMAIN, TAG,
          'Touch: id=%{public}d ratio=(%{public}d%%,%{public}d%%) pixel=(%{public}d,%{public}d)',
          touch.id, Math.floor(touch.ratioX * 100), Math.floor(touch.ratioY * 100),
          touch.pixelX, touch.pixelY);
      }
    }

    if (event.type === TouchType.Down || event.type === TouchType.Move) {
      for (let i = 0; i < event.touches.length; i++) {
        const touch = event.touches[i];
        this.activeTouches.push({
          x: touch.pixelX,
          y: touch.pixelY,
          radius: 30,
          color: 0xFF00FF00,
          ttl: 3
        });
      }
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
    Stack() {
      ArkZeroSurfaceView({
        config: this.surfaceViewConfig,
        touchHandler: (event: NormalizedTouchEvent) => {
          this.handleTouch(event);
        },
        onSurfaceLoaded: (surfaceId: string) => {
          this.onSurfaceLoaded(surfaceId);
        }
      })

      Row() {
        Button('Back')
          .fontSize(12)
          .height(30)
          .margin({ right: 10 })
          .onClick(() => {
            router.back();
          })

        Text(`Status: ${this.isInitialized ? 'Running' : 'Initializing'}`)
          .fontSize(14)
          .fontColor(Color.White)
          .margin({ right: 20 });

        Text(`Frames: ${this.frameCount}`)
          .fontSize(14)
          .fontColor(Color.White);
      }
      .padding(10)
      .backgroundColor('rgba(0, 0, 0, 0.6)')
      .borderRadius(8)
      .position({ x: 10, y: 10 })
      .hitTestBehavior(HitTestMode.Transparent)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.Black)
  }
}
