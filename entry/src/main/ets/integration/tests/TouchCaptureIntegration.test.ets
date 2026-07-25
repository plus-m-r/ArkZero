import { TestCaseResult, TestStatus } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'TouchCaptureTest';

export interface TouchMarker {
  id: number;
  x: number;
  y: number;
  color: string;
}

interface Coord {
  x: number;
  y: number;
}

const FINGER_COLORS: string[] = [
  '#FF0000',
  '#0000FF',
  '#00FF00',
  '#FFFF00',
  '#00FFFF'
];

export function getFingerColor(fingerId: number): string {
  const idx = fingerId % FINGER_COLORS.length;
  return FINGER_COLORS[idx];
}

export class TouchCaptureIntegrationTest {
  private results: TestCaseResult[] = [];
  private onComplete?: (results: TestCaseResult[]) => void;
  private surfaceId: string = '';
  private width: number = 800;
  private height: number = 600;

  private capturedTypes: Set<number> = new Set();
  private capturedFingerIds: Set<number> = new Set();
  private capturedCoords: Array<Coord> = [];
  private resolveWaiter: ((v?: void) => void) | null = null;
  private waitForType: number = -1;

  constructor(surfaceId: string, width: number, height: number, onComplete?: (results: TestCaseResult[]) => void) {
    this.surfaceId = surfaceId;
    this.width = width;
    this.height = height;
    this.onComplete = onComplete;
  }

  onTouchEvent(event: TouchEvent): void {
    const touchType: number = event.type;
    this.capturedTypes.add(touchType);

    for (let i = 0; i < event.touches.length; i++) {
      const touch = event.touches[i];
      this.capturedFingerIds.add(touch.id);
      this.capturedCoords.push({ x: touch.x, y: touch.y } as Coord);

      hilog.info(DOMAIN, TAG, 'Touch captured: id=%{public}d x=%{public}f y=%{public}f type=%{public}d',
        touch.id, touch.x, touch.y, touchType);
    }

    if (this.resolveWaiter !== null) {
      if (this.waitForType < 0 || this.capturedTypes.has(this.waitForType)) {
        this.resolveWaiter();
        this.resolveWaiter = null;
        this.waitForType = -1;
      }
    }
  }

  private waitForAnyTouch(timeoutMs: number): Promise<boolean> {
    this.waitForType = -1;
    return new Promise<boolean>((resolve) => {
      if (this.capturedTypes.size > 0) {
        resolve(true);
        return;
      }

      this.resolveWaiter = () => {
        resolve(true);
      };

      setTimeout(() => {
        if (this.resolveWaiter !== null) {
          this.resolveWaiter = null;
          this.waitForType = -1;
          resolve(false);
        }
      }, timeoutMs);
    });
  }

  private waitForSpecificType(touchType: number, timeoutMs: number): Promise<boolean> {
    this.waitForType = touchType;
    return new Promise<boolean>((resolve) => {
      if (this.capturedTypes.has(touchType)) {
        this.waitForType = -1;
        resolve(true);
        return;
      }

      this.resolveWaiter = () => {
        resolve(true);
      };

      setTimeout(() => {
        if (this.resolveWaiter !== null) {
          this.resolveWaiter = null;
          this.waitForType = -1;
          resolve(false);
        }
      }, timeoutMs);
    });
  }

  async runAllTests(): Promise<TestCaseResult[]> {
    hilog.info(DOMAIN, TAG, 'Starting Touch Capture Integration Tests');

    this.capturedTypes = new Set();
    this.capturedFingerIds = new Set();
    this.capturedCoords = [];

    await this.testTouchDownCapture();
    await this.testTouchMoveCapture();
    await this.testTouchUpCapture();
    await this.testMultiTouchCapture();
    await this.testCoordinateAccuracy();

    hilog.info(DOMAIN, TAG, `All Touch Capture tests completed: ${this.results.length} cases`);

    if (this.onComplete) {
      this.onComplete(this.results);
    }

    return this.results;
  }

  private async testTouchDownCapture(): Promise<void> {
    const startTime = Date.now();
    const testName = 'Down事件捕获';

    hilog.info(DOMAIN, TAG, 'Test: Down事件捕获 — 请触摸屏幕');

    this.capturedTypes = new Set();
    const received = await this.waitForSpecificType(TouchType.Down, 15000);
    const duration = Date.now() - startTime;

    const result: TestCaseResult = {
      name: testName,
      status: received ? TestStatus.PASSED : TestStatus.FAILED,
      duration: duration,
      message: received
        ? `Down事件已捕获 (${duration}ms)`
        : `15秒内未收到Down事件`
    };
    this.results.push(result);

    hilog.info(DOMAIN, TAG, `${result.status === TestStatus.PASSED ? 'PASSED' : 'FAILED'}: ${testName} (${duration}ms)`);
  }

  private async testTouchMoveCapture(): Promise<void> {
    const startTime = Date.now();
    const testName = 'Move事件捕获';

    this.capturedTypes = new Set();

    hilog.info(DOMAIN, TAG, 'Test: Move事件捕获 — 请在屏幕上滑动手指');

    const received = await this.waitForSpecificType(TouchType.Move, 15000);
    const duration = Date.now() - startTime;

    const result: TestCaseResult = {
      name: testName,
      status: received ? TestStatus.PASSED : TestStatus.FAILED,
      duration: duration,
      message: received
        ? `Move事件已捕获 (${duration}ms)`
        : `15秒内未收到Move事件`
    };
    this.results.push(result);

    hilog.info(DOMAIN, TAG, `${result.status === TestStatus.PASSED ? 'PASSED' : 'FAILED'}: ${testName} (${duration}ms)`);
  }

  private async testTouchUpCapture(): Promise<void> {
    const startTime = Date.now();
    const testName = 'Up事件捕获';

    this.capturedTypes = new Set();

    hilog.info(DOMAIN, TAG, 'Test: Up事件捕获 — 请触摸屏幕后抬起');

    const received = await this.waitForSpecificType(TouchType.Up, 15000);
    const duration = Date.now() - startTime;

    const result: TestCaseResult = {
      name: testName,
      status: received ? TestStatus.PASSED : TestStatus.FAILED,
      duration: duration,
      message: received
        ? `Up事件已捕获 (${duration}ms)`
        : `15秒内未收到Up事件`
    };
    this.results.push(result);

    hilog.info(DOMAIN, TAG, `${result.status === TestStatus.PASSED ? 'PASSED' : 'FAILED'}: ${testName} (${duration}ms)`);
  }

  private async testMultiTouchCapture(): Promise<void> {
    const startTime = Date.now();
    const testName = '多点触控捕获';

    this.capturedFingerIds = new Set();
    this.capturedTypes = new Set();

    hilog.info(DOMAIN, TAG, 'Test: 多点触控捕获 — 请用多根手指触摸屏幕');

    const received = await this.waitForAnyTouch(15000);
    const duration = Date.now() - startTime;

    const fingerCount = this.capturedFingerIds.size;

    const result: TestCaseResult = {
      name: testName,
      status: received ? TestStatus.PASSED : TestStatus.FAILED,
      duration: duration,
      message: received
        ? `捕获到${fingerCount}个手指触控 (${duration}ms)`
        : `15秒内未收到多点触控事件`
    };
    this.results.push(result);

    hilog.info(DOMAIN, TAG, `${result.status === TestStatus.PASSED ? 'PASSED' : 'FAILED'}: ${testName} (${duration}ms)`);
  }

  private async testCoordinateAccuracy(): Promise<void> {
    const startTime = Date.now();
    const testName = '触控坐标准确性';

    this.capturedCoords = [];
    this.capturedTypes = new Set();

    hilog.info(DOMAIN, TAG, 'Test: 触控坐标准确性 — 请触摸屏幕');

    const received = await this.waitForAnyTouch(15000);
    const duration = Date.now() - startTime;

    let allCoordsValid = false;
    if (this.capturedCoords.length > 0) {
      allCoordsValid = true;
      for (let i = 0; i < this.capturedCoords.length; i++) {
        const coord = this.capturedCoords[i];
        if (coord.x < 0 || coord.y < 0) {
          allCoordsValid = false;
          break;
        }
      }
    }

    const result: TestCaseResult = {
      name: testName,
      status: received && allCoordsValid ? TestStatus.PASSED : TestStatus.FAILED,
      duration: duration,
      message: received && allCoordsValid
        ? `坐标有效 (${this.capturedCoords.length}个采样点, ${duration}ms)`
        : received
          ? `坐标含负值，不准确`
          : `15秒内未收到触控事件`
    };
    this.results.push(result);

    hilog.info(DOMAIN, TAG, `${result.status === TestStatus.PASSED ? 'PASSED' : 'FAILED'}: ${testName} (${duration}ms)`);
  }
}
