import nativerender from 'libnativerender.so';
import hilog from '@ohos.hilog';

export enum PixelFormat {
  RGBA = 0,
  RGB = 1,
  BGRA = 2,
  NV21 = 3,
  NV12 = 4
}

const DOMAIN = 0x0001;
const TAG = 'ArkZeroRenderer';

export interface ArkZeroRendererConfig {
  width: number;
  height: number;
  format: PixelFormat;
  surfaceId?: string;
}

export interface DirtyRect {
  x: number;
  y: number;
  w: number;
  h: number;
}

export interface TileRegion {
  ratioX: number;
  ratioY: number;
  tilePixelWidth: number;
  tilePixelHeight: number;
  pixelData: ArrayBuffer;
}

export interface DisposeAsyncOptions {
  wait?: boolean;
  timeoutMs?: number;
}

export class ArkZeroRenderer {
  private nativeHandle: number = 0;
  private config: ArkZeroRendererConfig;
  private isInitialized: boolean = false;
  private onFrameRenderedCallback?: () => void;

  constructor(config: ArkZeroRendererConfig) {
    this.config = config;
  }

  async initialize(surfaceId: string): Promise<void> {
    if (this.isInitialized) {
      hilog.warn(DOMAIN, TAG, 'ArkZeroRenderer already initialized');
      return;
    }

    try {
      hilog.info(DOMAIN, TAG, 'initialize START: surfaceId=%{public}s, w=%{public}d, h=%{public}d, fmt=%{public}d',
        surfaceId, this.config.width, this.config.height, this.config.format);

      this.nativeHandle = await nativerender.create(
        surfaceId,
        this.config.width,
        this.config.height,
        this.config.format
      );

      this.isInitialized = true;

      hilog.info(DOMAIN, TAG, 'initialize SUCCESS: nativeHandle=%{public}d', this.nativeHandle);

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, 'ArkZeroRenderer initialization failed: %{public}s', errorMsg);
      throw new Error(`Failed to initialize ArkZeroRenderer: ${errorMsg}`);
    }
  }

  setOnFrameRendered(callback: () => void): void {
    this.onFrameRenderedCallback = callback;
  }

  async renderFrame(
    pixelData: ArrayBuffer,
    width: number,
    height: number
  ): Promise<void> {
    if (!this.isInitialized) {
      hilog.error(DOMAIN, TAG, 'renderFrame called but NOT initialized');
      throw new Error('ArkZeroRenderer not initialized');
    }

    try {
      await nativerender.renderFrame(
        this.nativeHandle,
        pixelData,
        width,
        height
      );

      if (this.onFrameRenderedCallback) {
        this.onFrameRenderedCallback();
      }

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      throw new Error(errorMsg);
    }
  }

  async renderFrameRegions(
    pixelData: ArrayBuffer,
    frameWidth: number,
    frameHeight: number,
    regions: DirtyRect[],
    swap: boolean = true
  ): Promise<void> {
    if (!this.isInitialized) {
      throw new Error('ArkZeroRenderer not initialized');
    }

    try {
      await nativerender.renderFrameRegions(
        this.nativeHandle,
        pixelData,
        frameWidth,
        frameHeight,
        regions,
        swap
      );

      if (swap && this.onFrameRenderedCallback) {
        this.onFrameRenderedCallback();
      }

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      throw new Error(errorMsg);
    }
  }

  async renderTileRegions(
    tiles: TileRegion[],
    frameWidth: number,
    frameHeight: number,
    swap: boolean = true
  ): Promise<void> {
    if (!this.isInitialized) {
      throw new Error('ArkZeroRenderer not initialized');
    }

    try {
      await nativerender.renderTileRegions(
        this.nativeHandle,
        tiles,
        frameWidth,
        frameHeight,
        swap
      );

      if (swap && this.onFrameRenderedCallback) {
        this.onFrameRenderedCallback();
      }

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      throw new Error(errorMsg);
    }
  }

  async updateDirtyRegions(
    pixelData: ArrayBuffer,
    frameWidth: number,
    frameHeight: number,
    regions: DirtyRect[]
  ): Promise<void> {
    if (!this.isInitialized) {
      throw new Error('ArkZeroRenderer not initialized');
    }

    try {
      await nativerender.updateDirtyRegions(
        this.nativeHandle,
        pixelData,
        frameWidth,
        frameHeight,
        regions
      );
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      throw new Error(errorMsg);
    }
  }

  async presentFrame(): Promise<void> {
    if (!this.isInitialized) {
      throw new Error('ArkZeroRenderer not initialized');
    }

    try {
      await nativerender.presentFrame(this.nativeHandle);

      if (this.onFrameRenderedCallback) {
        this.onFrameRenderedCallback();
      }

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      throw new Error(errorMsg);
    }
  }

  async resize(width: number, height: number): Promise<void> {
    if (!this.isInitialized) {
      throw new Error('ArkZeroRenderer not initialized');
    }

    try {
      await nativerender.resize(this.nativeHandle, width, height);

      this.config.width = width;
      this.config.height = height;

      hilog.info(DOMAIN, TAG, 'ArkZeroRenderer resized: %{public}dx%{public}d', width, height);

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, 'resize failed: %{public}s', errorMsg);
      throw new Error(errorMsg);
    }
  }

  dispose(): void {
    if (!this.isInitialized) {
      return;
    }

    try {
      nativerender.destroy(this.nativeHandle);
    } finally {
      this.nativeHandle = 0;
      this.isInitialized = false;
      hilog.info(DOMAIN, TAG, 'ArkZeroRenderer disposed');
    }
  }

  async disposeAsync(options?: DisposeAsyncOptions): Promise<void> {
    this.dispose();
  }
}
