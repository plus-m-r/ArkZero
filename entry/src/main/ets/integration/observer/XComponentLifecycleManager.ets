import { XComponentLifecycleObserver } from './XComponentLifecycleObserver';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'LifecycleManager';

export class XComponentLifecycleManager {
  private observers: XComponentLifecycleObserver[] = [];
  
  addObserver(observer: XComponentLifecycleObserver): void {
    if (!this.observers.includes(observer)) {
      this.observers.push(observer);
      hilog.info(DOMAIN, TAG, 'Observer added: %{public}s', observer.getObserverName());
    } else {
      hilog.warn(DOMAIN, TAG, 'Observer already exists: %{public}s', observer.getObserverName());
    }
  }
  
  removeObserver(observer: XComponentLifecycleObserver): void {
    const index = this.observers.indexOf(observer);
    if (index !== -1) {
      this.observers.splice(index, 1);
      hilog.info(DOMAIN, TAG, 'Observer removed: %{public}s', observer.getObserverName());
    }
  }
  
  notifySurfaceCreated(surfaceId: string, width: number, height: number): void {
    hilog.info(DOMAIN, TAG, 'Notifying surface created: %{public}s (%{public}dx%{public}d)', surfaceId, width, height);
    
    this.observers.forEach(observer => {
      try {
        observer.onSurfaceCreated(surfaceId, width, height);
      } catch (error) {
        const errMsg = error instanceof Error ? error.message : String(error);
        hilog.error(DOMAIN, TAG, 'Error in observer %{public}s: %{public}s', observer.getObserverName(), errMsg);
      }
    });
  }
  
  notifySurfaceChanged(surfaceId: string, width: number, height: number): void {
    hilog.info(DOMAIN, TAG, 'Notifying surface changed: %{public}s (%{public}dx%{public}d)', surfaceId, width, height);
    
    this.observers.forEach(observer => {
      try {
        observer.onSurfaceChanged(surfaceId, width, height);
      } catch (error) {
        const errMsg = error instanceof Error ? error.message : String(error);
        hilog.error(DOMAIN, TAG, 'Error in observer %{public}s: %{public}s', observer.getObserverName(), errMsg);
      }
    });
  }
  
  notifySurfaceDestroyed(surfaceId: string): void {
    hilog.info(DOMAIN, TAG, 'Notifying surface destroyed: %{public}s', surfaceId);
    
    this.observers.forEach(observer => {
      try {
        observer.onSurfaceDestroyed(surfaceId);
      } catch (error) {
        const errMsg = error instanceof Error ? error.message : String(error);
        hilog.error(DOMAIN, TAG, 'Error in observer %{public}s: %{public}s', observer.getObserverName(), errMsg);
      }
    });
  }
  
  getObserverCount(): number {
    return this.observers.length;
  }
  
  clearObservers(): void {
    this.observers = [];
    hilog.info(DOMAIN, TAG, 'All observers cleared');
  }
}
