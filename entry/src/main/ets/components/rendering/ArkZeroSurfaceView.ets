import { PixelFormat } from './ArkZeroRenderer';
import { display } from '@kit.ArkUI';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'ArkZeroSurfaceView';

export interface TouchPoint {
  id: number;
  ratioX: number;
  ratioY: number;
  pixelX: number;
  pixelY: number;
}

export interface NormalizedTouchEvent {
  type: TouchType;
  touches: TouchPoint[];
}

export interface SurfaceViewConfig {
  renderWidth: number;
  renderHeight: number;
  format: PixelFormat;
  originRatioX?: number;
  originRatioY?: number;
  sizeRatioX?: number;
  sizeRatioY?: number;
  enableTouch?: boolean;
  xComponentId?: string;
}

@Component
export struct ArkZeroSurfaceView {
  @Prop config: SurfaceViewConfig = {
    renderWidth: 640,
    renderHeight: 480,
    format: PixelFormat.RGBA,
    originRatioX: 0.0,
    originRatioY: 0.0,
    sizeRatioX: 1.0,
    enableTouch: true,
    xComponentId: 'arkzero_surface'
  };
  touchHandler?: (event: NormalizedTouchEvent) => void;
  onSurfaceLoaded?: (surfaceId: string) => void;

  private areaWidth: number = 0;
  private areaHeight: number = 0;
  private scaleX: number = 1;
  private scaleY: number = 1;
  private areaMeasured: boolean = false;
  private componentVpW: number = 0;
  private componentVpH: number = 0;
  private posX: number = 0;
  private posY: number = 0;

  private xComponentController: XComponentController = new XComponentController();

  aboutToAppear() {
    try {
      const d = display.getDefaultDisplaySync();
      const screenVpW = d.width / d.densityPixels;
      const screenVpH = d.height / d.densityPixels;

      const originX = this.config.originRatioX ?? 0.0;
      const originY = this.config.originRatioY ?? 0.0;
      const sizeX = this.config.sizeRatioX ?? 1.0;

      this.posX = screenVpW * originX;
      this.posY = screenVpH * originY;

      const aspect = this.config.renderHeight / this.config.renderWidth;

      if (this.config.sizeRatioY != null) {
        this.componentVpW = screenVpW * sizeX;
        this.componentVpH = screenVpH * this.config.sizeRatioY;
      } else {
        this.componentVpW = screenVpW * sizeX;
        this.componentVpH = this.componentVpW * aspect;
        if (this.componentVpH > screenVpH) {
          this.componentVpH = screenVpH;
          this.componentVpW = this.componentVpH / aspect;
        }
      }

      hilog.info(DOMAIN, TAG,
        'Viewport: screen=%{public}dx%{public}d vp, component=%{public}dx%{public}d vp, pos=(%{public}d,%{public}d)',
        Math.floor(screenVpW), Math.floor(screenVpH),
        Math.floor(this.componentVpW), Math.floor(this.componentVpH),
        Math.floor(this.posX), Math.floor(this.posY));
    } catch (e) {
      hilog.warn(DOMAIN, TAG, 'display.getDefaultDisplaySync failed, using fallback 100%% layout');
      this.componentVpW = 0;
      this.componentVpH = 0;
      this.posX = 0;
      this.posY = 0;
    }
  }

  build() {
    Stack() {
      XComponent({
        id: this.config.xComponentId ?? 'arkzero_surface',
        type: XComponentType.SURFACE,
        controller: this.xComponentController
      })
        .onLoad(() => {
          const surfaceId = this.xComponentController.getXComponentSurfaceId();
          hilog.info(DOMAIN, TAG, 'XComponent loaded, surfaceId: %{public}s', surfaceId);
          if (this.onSurfaceLoaded) {
            this.onSurfaceLoaded(surfaceId);
          }
        })
        .onDestroy(() => {
          hilog.info(DOMAIN, TAG, 'XComponent destroyed');
        })
        .width('100%')
        .height('100%')

      if (this.config.enableTouch ?? true) {
        Column()
          .width('100%')
          .height('100%')
          .hitTestBehavior(HitTestMode.Default)
          .onTouch((event: TouchEvent) => {
            if (this.touchHandler && this.areaWidth > 0 && this.areaHeight > 0) {
              const touches: TouchPoint[] = [];
              for (let i = 0; i < event.touches.length; i++) {
                const t = event.touches[i];
                const pixelX = Math.floor(t.x * this.scaleX);
                const pixelY = Math.floor(t.y * this.scaleY);
                const tp: TouchPoint = {
                  id: t.id,
                  ratioX: t.x / this.areaWidth,
                  ratioY: t.y / this.areaHeight,
                  pixelX: pixelX,
                  pixelY: pixelY
                };
                touches.push(tp);
              }
              const mappedEvent: NormalizedTouchEvent = {
                type: event.type,
                touches: touches
              };
              this.touchHandler(mappedEvent);
            }
          })
      }
    }
    .width(this.componentVpW > 0 ? this.componentVpW : '100%')
    .height(this.componentVpH > 0 ? this.componentVpH : '100%')
    .position(this.componentVpW > 0 ? { x: this.posX, y: this.posY } : undefined)
    .onAreaChange((_oldValue: Area, newValue: Area) => {
      const w = Number(newValue.width);
      const h = Number(newValue.height);
      if (w > 0 && h > 0) {
        this.areaWidth = w;
        this.areaHeight = h;
        this.scaleX = this.config.renderWidth / w;
        this.scaleY = this.config.renderHeight / h;
        if (!this.areaMeasured) {
          this.areaMeasured = true;
          hilog.info(DOMAIN, TAG, 'Area measured: %{public}dx%{public}d vp, scale=(%{public}d%%,%{public}d%%)',
            Math.floor(w), Math.floor(h),
            Math.floor(this.scaleX * 100), Math.floor(this.scaleY * 100));
        }
      }
    })
  }
}
