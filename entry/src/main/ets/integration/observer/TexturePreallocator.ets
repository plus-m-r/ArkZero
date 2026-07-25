import { XComponentLifecycleObserver } from './XComponentLifecycleObserver';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'TexturePreallocator';

interface Resolution {
  width: number;
  height: number;
}

export class TexturePreallocator implements XComponentLifecycleObserver {
  private preallocatedResolutions: Resolution[] = [];
  
  constructor() {
    this.preallocatedResolutions = [
      { width: 1920, height: 1080 },
      { width: 1280, height: 720 },
      { width: 3840, height: 2160 },
      { width: 800, height: 600 },
    ];
  }
  
  onSurfaceCreated(surfaceId: string, width: number, height: number): void {
    hilog.info(DOMAIN, TAG, 'Surface created: %{public}s (%{public}dx%{public}d)', surfaceId, width, height);
    hilog.info(DOMAIN, TAG, 'Will preallocate %{public}d resolutions', this.preallocatedResolutions.length);
    this.preallocateTextures(width, height);
  }
  
  onSurfaceChanged(surfaceId: string, width: number, height: number): void {
    hilog.info(DOMAIN, TAG, 'Surface resized: %{public}s -> %{public}dx%{public}d', surfaceId, width, height);
    this.preallocateTextures(width, height);
  }
  
  onSurfaceDestroyed(surfaceId: string): void {
    hilog.info(DOMAIN, TAG, 'Surface destroyed: %{public}s', surfaceId);
  }
  
  getObserverName(): string {
    return 'TexturePreallocator';
  }
  
  private preallocateTextures(currentWidth: number, currentHeight: number): void {
    hilog.info(DOMAIN, TAG, 'Preallocating textures...');
    
    const currentInList = this.preallocatedResolutions.some(
      res => res.width === currentWidth && res.height === currentHeight
    );
    
    if (!currentInList) {
      hilog.info(DOMAIN, TAG, 'Adding current resolution: %{public}dx%{public}d', currentWidth, currentHeight);
    }
    
    this.preallocatedResolutions.forEach((res: Resolution, index: number) => {
      hilog.info(DOMAIN, TAG, '[%{public}d] %{public}dx%{public}d', index + 1, res.width, res.height);
    });
    
    hilog.info(DOMAIN, TAG, 'Preallocation plan ready');
  }
  
  addResolution(width: number, height: number): void {
    const exists = this.preallocatedResolutions.some(
      res => res.width === width && res.height === height
    );
    
    if (!exists) {
      this.preallocatedResolutions.push({ width, height });
      hilog.info(DOMAIN, TAG, 'Added resolution: %{public}dx%{public}d', width, height);
    }
  }
  
  getPreallocatedResolutions(): Resolution[] {
    return [...this.preallocatedResolutions];
  }
}
