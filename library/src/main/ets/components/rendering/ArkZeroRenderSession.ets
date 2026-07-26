import { ArkZeroRenderer, PixelFormat, ArkZeroRendererConfig, DirtyRect, TileRegion } from './ArkZeroRenderer';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'ArkZeroRenderSession';

export interface RenderSessionConfig {
  width: number;
  height: number;
  format: PixelFormat;
  onError?: (errorMsg: string) => void;
}

export class ArkZeroRenderSession {
  private renderer?: ArkZeroRenderer;
  private config: RenderSessionConfig;
  private surfaceId: string = '';
  private isReady: boolean = false;
  private pendingFullFrame: ArrayBuffer | null = null;
  private pendingFullFrameW: number = 0;
  private pendingFullFrameH: number = 0;

  constructor(config: RenderSessionConfig) {
    this.config = config;
  }

  async attach(surfaceId: string): Promise<void> {
    this.surfaceId = surfaceId;
    const rendererConfig: ArkZeroRendererConfig = {
      width: this.config.width,
      height: this.config.height,
      format: this.config.format
    };
    this.renderer = new ArkZeroRenderer(rendererConfig);

    try {
      await this.renderer.initialize(surfaceId);
      this.isReady = true;

      if (this.pendingFullFrame) {
        const buffer = this.pendingFullFrame;
        const w = this.pendingFullFrameW;
        const h = this.pendingFullFrameH;
        this.pendingFullFrame = null;
        await this.renderFullFrame(buffer, w, h);
      }

      hilog.info(DOMAIN, TAG, 'Session attached: surfaceId=%{public}s', surfaceId);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, 'attach failed: %{public}s', errorMsg);
      this.notifyError(errorMsg);
    }
  }

  async renderFullFrame(pixelData: ArrayBuffer, width: number, height: number): Promise<void> {
    if (!this.isReady || !this.renderer) {
      this.pendingFullFrame = pixelData;
      this.pendingFullFrameW = width;
      this.pendingFullFrameH = height;
      hilog.info(DOMAIN, TAG, 'Buffering first frame (surface not ready)');
      return;
    }

    try {
      await this.renderer.renderFrame(pixelData, width, height);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.notifyError(errorMsg);
    }
  }

  async renderDirtyRegions(
    pixelData: ArrayBuffer,
    frameWidth: number,
    frameHeight: number,
    regions: DirtyRect[]
  ): Promise<void> {
    if (!this.isReady || !this.renderer) {
      return;
    }

    try {
      await this.renderer.updateDirtyRegions(pixelData, frameWidth, frameHeight, regions);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.notifyError(errorMsg);
    }
  }

  async renderTileRegions(
    tiles: TileRegion[],
    frameWidth: number,
    frameHeight: number,
    swap: boolean = true
  ): Promise<void> {
    if (!this.isReady || !this.renderer) {
      return;
    }

    try {
      await this.renderer.renderTileRegions(tiles, frameWidth, frameHeight, swap);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.notifyError(errorMsg);
    }
  }

  async resize(width: number, height: number): Promise<void> {
    if (!this.isReady || !this.renderer) {
      return;
    }

    try {
      await this.renderer.resize(width, height);
      this.config.width = width;
      this.config.height = height;
      hilog.info(DOMAIN, TAG, 'Session resized: %{public}dx%{public}d', width, height);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.notifyError(errorMsg);
    }
  }

  async setVSync(enabled: boolean): Promise<void> {
    if (!this.isReady || !this.renderer) {
      return;
    }

    try {
      await this.renderer.setVSync(enabled);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.notifyError(errorMsg);
    }
  }

  async presentFrame(): Promise<void> {
    if (!this.isReady || !this.renderer) {
      return;
    }

    try {
      await this.renderer.presentFrame();
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.notifyError(errorMsg);
    }
  }

  detach(): void {
    if (this.renderer) {
      this.renderer.dispose();
      this.renderer = undefined;
      this.isReady = false;
      this.pendingFullFrame = null;
      hilog.info(DOMAIN, TAG, 'Session detached');
    }
  }

  get ready(): boolean {
    return this.isReady;
  }

  get width(): number {
    return this.config.width;
  }

  get height(): number {
    return this.config.height;
  }

  private notifyError(errorMsg: string): void {
    hilog.error(DOMAIN, TAG, '%{public}s', errorMsg);
    if (this.config.onError) {
      this.config.onError(errorMsg);
    }
  }
}
