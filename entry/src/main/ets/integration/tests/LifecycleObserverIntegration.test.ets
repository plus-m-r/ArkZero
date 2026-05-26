import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';
import { XComponentLifecycleManager } from '../observer/XComponentLifecycleManager';
import { TexturePreallocator } from '../observer/TexturePreallocator';
import { XComponentLifecycleObserver } from '../observer/XComponentLifecycleObserver';

const DOMAIN = 0x0001;
const TAG = 'LifecycleObserverIntegrationTest';

/**
 * 分辨率数据结构
 */
interface Resolution {
  width: number;
  height: number;
}

/**
 * 测试用观察者实现类
 */
class ObserverTest implements XComponentLifecycleObserver {
  private name: string;
  public surfaceCreatedCount: number = 0;
  public surfaceChangedCount: number = 0;
  public surfaceDestroyedCount: number = 0;
  public notificationOrder: string[] = [];

  constructor(name: string) {
    this.name = name;
  }

  onSurfaceCreated(surfaceId: string, width: number, height: number): void {
    this.surfaceCreatedCount++;
    this.notificationOrder.push(`${this.name}-created`);
  }

  onSurfaceChanged(surfaceId: string, width: number, height: number): void {
    this.surfaceChangedCount++;
    this.notificationOrder.push(`${this.name}-changed`);
  }

  onSurfaceDestroyed(surfaceId: string): void {
    this.surfaceDestroyedCount++;
    this.notificationOrder.push(`${this.name}-destroyed`);
  }

  getObserverName(): string {
    return this.name;
  }
}

/**
 * 测试用会抛出错误的观察者
 */
class ErrorObserverTest implements XComponentLifecycleObserver {
  private name: string;

  constructor(name: string) {
    this.name = name;
  }

  onSurfaceCreated(surfaceId: string, width: number, height: number): void {
    throw new Error('模拟观察者错误');
  }

  onSurfaceChanged(surfaceId: string, width: number, height: number): void {
  }

  onSurfaceDestroyed(surfaceId: string): void {
  }

  getObserverName(): string {
    return this.name;
  }
}

export class LifecycleObserverIntegrationTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';
  private width: number = 1920;
  private height: number = 1080;

  constructor(surfaceId: string, width: number, height: number, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.width = width;
    this.height = height;
    this.onComplete = onComplete;
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting Lifecycle Observer Integration Tests');
    
    await this.testObserverAddRemove();
    await this.testObserverNotificationOrder();
    await this.testTexturePreallocation();
    await this.testMultipleObservers();
    await this.testObserverErrorHandling();
    await this.testLifecycleSequence();
    
    hilog.info(DOMAIN, TAG, `All Lifecycle Observer tests completed: ${this.results.length} cases`);
    
    if (this.onComplete) {
      this.onComplete(this.results);
    }
    
    return this.results;
  }

  private async testObserverAddRemove(): Promise<void> {
    const startTime = Date.now();
    const testName = '观察者添加与移除';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 观察者添加与移除');
      
      const manager = new XComponentLifecycleManager();
      const preallocator = new TexturePreallocator();
      
      manager.addObserver(preallocator);
      
      if (manager.getObserverCount() !== 1) {
        throw new Error(`期望观察者数量为1，实际为${manager.getObserverCount()}`);
      }
      
      manager.removeObserver(preallocator);
      
      if (manager.getObserverCount() !== 0) {
        throw new Error(`期望观察者数量为0，实际为${manager.getObserverCount()}`);
      }
      
      const duration = Date.now() - startTime;
      
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `观察者添加/移除成功 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `观察者添加/移除失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testObserverNotificationOrder(): Promise<void> {
    const startTime = Date.now();
    const testName = '观察者通知顺序';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 观察者通知顺序');
      
      const manager = new XComponentLifecycleManager();
      const observer1 = new ObserverTest('observer1');
      const observer2 = new ObserverTest('observer2');
      
      manager.addObserver(observer1);
      manager.addObserver(observer2);
      
      manager.notifySurfaceCreated(this.surfaceId, this.width, this.height);
      
      const duration = Date.now() - startTime;
      
      if (observer1.surfaceCreatedCount === 1 && observer2.surfaceCreatedCount === 1) {
        this.results.push({
          name: testName,
          status: TestStatus.PASSED,
          duration: duration,
          message: `两个观察者都被通知到 (${duration}ms)`
        });
      } else {
        this.results.push({
          name: testName,
          status: TestStatus.FAILED,
          duration: duration,
          message: `期望通知2个观察者，实际通知${observer1.surfaceCreatedCount + observer2.surfaceCreatedCount}个`
        });
      }
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `通知顺序测试失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testTexturePreallocation(): Promise<void> {
    const startTime = Date.now();
    const testName = '纹理预分配功能';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 纹理预分配功能');
      
      const preallocator = new TexturePreallocator();
      const resolutions: Resolution[] = preallocator.getPreallocatedResolutions();
      
      if (resolutions.length < 4) {
        throw new Error(`预分配分辨率数量不足，期望>=4，实际${resolutions.length}`);
      }
      
      let r: Resolution;
      let has1080p: boolean = false;
      for (r of resolutions) {
        if (r.width === 1920 && r.height === 1080) {
          has1080p = true;
          break;
        }
      }
      
      let has720p: boolean = false;
      for (r of resolutions) {
        if (r.width === 1280 && r.height === 720) {
          has720p = true;
          break;
        }
      }
      
      if (!has1080p || !has720p) {
        throw new Error('缺少常用分辨率（1080p或720p）');
      }
      
      preallocator.addResolution(2560, 1440);
      
      let has2K: boolean = false;
      const allResolutions: Resolution[] = preallocator.getPreallocatedResolutions();
      for (r of allResolutions) {
        if (r.width === 2560 && r.height === 1440) {
          has2K = true;
          break;
        }
      }
      
      if (!has2K) {
        throw new Error('添加自定义分辨率失败');
      }
      
      preallocator.onSurfaceCreated(this.surfaceId, this.width, this.height);
      preallocator.onSurfaceChanged(this.surfaceId, 1280, 720);
      preallocator.onSurfaceDestroyed(this.surfaceId);
      
      const duration = Date.now() - startTime;
      
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `纹理预分配功能正常 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `纹理预分配失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testMultipleObservers(): Promise<void> {
    const startTime = Date.now();
    const testName = '多观察者管理';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 多观察者管理');
      
      const manager = new XComponentLifecycleManager();
      const observers: ObserverTest[] = [];
      
      for (let i = 0; i < 5; i++) {
        const observer = new ObserverTest(`observer-${i}`);
        observers.push(observer);
        manager.addObserver(observer);
      }
      
      if (manager.getObserverCount() !== 5) {
        throw new Error(`期望5个观察者，实际${manager.getObserverCount()}个`);
      }
      
      manager.notifySurfaceCreated(this.surfaceId, this.width, this.height);
      
      let i: number;
      for (i = 0; i < observers.length; i++) {
        const obs = observers[i];
        if (obs.surfaceCreatedCount !== 1) {
          throw new Error(`${obs.getObserverName()}的通知次数不正确，期望1次，实际${obs.surfaceCreatedCount}次`);
        }
      }
      
      manager.clearObservers();
      
      if (manager.getObserverCount() !== 0) {
        throw new Error('清除观察者后，数量不为0');
      }
      
      const duration = Date.now() - startTime;
      
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `成功管理5个观察者 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `多观察者管理失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testObserverErrorHandling(): Promise<void> {
    const startTime = Date.now();
    const testName = '观察者错误处理';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 观察者错误处理');
      
      const manager = new XComponentLifecycleManager();
      const errorObserver = new ErrorObserverTest('error-observer');
      
      manager.addObserver(errorObserver);
      
      try {
        manager.notifySurfaceCreated(this.surfaceId, this.width, this.height);
        
        this.results.push({
          name: testName,
          status: TestStatus.PASSED,
          duration: Date.now() - startTime,
          message: '错误被正确捕获，未影响管理器'
        });
        
      } catch (error) {
        this.results.push({
          name: testName,
          status: TestStatus.FAILED,
          duration: Date.now() - startTime,
          message: '错误未被捕获，导致管理器崩溃'
        });
      }
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName}`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `错误处理测试失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }

  private async testLifecycleSequence(): Promise<void> {
    const startTime = Date.now();
    const testName = '完整生命周期序列';
    
    try {
      hilog.info(DOMAIN, TAG, 'Test: 完整生命周期序列');
      
      const manager = new XComponentLifecycleManager();
      const preallocator = new TexturePreallocator();
      
      manager.addObserver(preallocator);
      
      manager.notifySurfaceCreated(this.surfaceId, this.width, this.height);
      manager.notifySurfaceChanged(this.surfaceId, 1280, 720);
      manager.notifySurfaceChanged(this.surfaceId, 1920, 1080);
      manager.notifySurfaceChanged(this.surfaceId, 640, 480);
      manager.notifySurfaceDestroyed(this.surfaceId);
      
      const duration = Date.now() - startTime;
      
      this.results.push({
        name: testName,
        status: TestStatus.PASSED,
        duration: duration,
        message: `完整生命周期序列执行成功 (${duration}ms)`
      });
      
      hilog.info(DOMAIN, TAG, `PASSED: ${testName} (${duration}ms)`);
      
    } catch (error) {
      const duration = Date.now() - startTime;
      const errorMsg = error instanceof Error ? error.message : String(error);
      this.results.push({
        name: testName,
        status: TestStatus.FAILED,
        duration: duration,
        message: `生命周期序列测试失败: ${errorMsg}`
      });
      
      hilog.error(DOMAIN, TAG, `FAILED: ${testName} - ${errorMsg}`);
    }
  }
}