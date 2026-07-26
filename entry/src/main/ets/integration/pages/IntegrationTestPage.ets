import { RendererIntegrationTest } from '../tests/RendererIntegration.test';
import { EGLContextLifecycleTest } from '../tests/EGLContextLifecycle.test';
import { PixelFormatIntegrationTest } from '../tests/PixelFormatIntegration.test';
import { StressIntegrationTest } from '../tests/StressIntegration.test';
import { CallbackIntegrationTest } from '../tests/CallbackIntegration.test';
import { LifecycleObserverIntegrationTest } from '../tests/LifecycleObserverIntegration.test';
import { IncrementalBenchmarkTest } from '../tests/IncrementalBenchmark.test';
import { TouchCaptureIntegrationTest, TouchMarker, getFingerColor } from '../tests/TouchCaptureIntegration.test';
import { TileRenderIntegrationTest } from '../tests/TileRenderIntegration.test';
import { RenderSessionIntegrationTest } from '../tests/RenderSessionIntegration.test';
import { TestResultPanel, TestCaseResult } from '../components/TestResultPanel';
import { router } from '@kit.ArkUI';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'IntegrationTestPage';

enum TestModule {
  RENDERER = '渲染器基础测试',
  EGL_CONTEXT = 'EGL上下文生命周期',
  PIXEL_FORMAT = '像素格式兼容性',
  STRESS = '压力测试',
  CALLBACK = '回调机制测试',
  LIFECYCLE_OBSERVER = '生命周期观察者',
  INCREMENTAL_BENCH = '增量渲染基准测试',
  TOUCH_CAPTURE = '触控捕获测试',
  TILE_RENDER = 'Tile渲染测试',
  RENDER_SESSION = 'Session会话层测试'
}

@Entry
@Component
struct IntegrationTestPage {
  @State surfaceId: string = '';
  @State isInitialized: boolean = false;
  @State showTestPanel: boolean = false;
  @State testResults: TestCaseResult[] = [];
  @State totalTests: number = 0;
  @State completedTests: number = 0;
  @State currentTestModule: string = '';
  @State showModuleSelector: boolean = false;
  @State isRunning: boolean = false;
  @State touchMarkers: TouchMarker[] = [];
  @State isTouchTestActive: boolean = false;
  @State touchInstruction: string = '';

  private xComponentController: XComponentController = new XComponentController();
  private touchTest?: TouchCaptureIntegrationTest;

  aboutToAppear() {
    hilog.info(DOMAIN, TAG, 'Integration Test Page loaded');
  }

  private async runTestModule(module: TestModule): Promise<void> {
    if (!this.isInitialized || !this.surfaceId) {
      hilog.error(DOMAIN, TAG, 'Surface not ready');
      return;
    }

    this.isRunning = true;
    this.showTestPanel = true;
    this.showModuleSelector = false;
    this.testResults = [];
    this.completedTests = 0;
    this.currentTestModule = module;

    hilog.info(DOMAIN, TAG, `Starting test module: ${module}`);

    try {
      switch (module) {
        case TestModule.RENDERER:
          this.totalTests = 5;
          await this.runRendererTest();
          break;
        case TestModule.EGL_CONTEXT:
          this.totalTests = 5;
          await this.runEGLContextTest();
          break;
        case TestModule.PIXEL_FORMAT:
          this.totalTests = 6;
          await this.runPixelFormatTest();
          break;
        case TestModule.STRESS:
          this.totalTests = 5;
          await this.runStressTest();
          break;
        case TestModule.CALLBACK:
          this.totalTests = 5;
          await this.runCallbackTest();
          break;
        case TestModule.LIFECYCLE_OBSERVER:
          this.totalTests = 6;
          await this.runLifecycleObserverTest();
          break;
        case TestModule.INCREMENTAL_BENCH:
          this.totalTests = 8;
          await this.runIncrementalBenchmark();
          break;
        case TestModule.TOUCH_CAPTURE:
          this.totalTests = 5;
          await this.runTouchCaptureTest();
          break;
        case TestModule.TILE_RENDER:
          this.totalTests = 5;
          await this.runTileRenderTest();
          break;
        case TestModule.RENDER_SESSION:
          this.totalTests = 8;
          await this.runRenderSessionTest();
          break;
      }
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, `Test module execution failed: ${errorMsg}`);
    } finally {
      this.isRunning = false;
      this.isTouchTestActive = false;
      this.touchMarkers = [];
    }
  }

  private async runRendererTest(): Promise<void> {
    const test = new RendererIntegrationTest(this.surfaceId, (results: TestCaseResult[]) => {
      this.testResults = results;
      this.completedTests = results.length;
      hilog.info(DOMAIN, TAG, `Renderer tests completed: ${results.length} cases`);
    });
    await test.runAllTests();
  }

  private async runEGLContextTest(): Promise<void> {
    const test = new EGLContextLifecycleTest(
      this.surfaceId,
      1920,
      1080,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        hilog.info(DOMAIN, TAG, `EGL Context tests completed: ${results.length} cases`);
      }
    );
    await test.runAllTests();
  }

  private async runPixelFormatTest(): Promise<void> {
    const test = new PixelFormatIntegrationTest(
      this.surfaceId,
      800,
      600,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        hilog.info(DOMAIN, TAG, `Pixel Format tests completed: ${results.length} cases`);
      }
    );
    await test.runAllTests();
  }

  private async runStressTest(): Promise<void> {
    const test = new StressIntegrationTest(
      this.surfaceId,
      800,
      600,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        hilog.info(DOMAIN, TAG, `Stress tests completed: ${results.length} cases`);
      }
    );
    await test.runAllTests();
  }

  private async runCallbackTest(): Promise<void> {
    const test = new CallbackIntegrationTest(
      this.surfaceId,
      800,
      600,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        hilog.info(DOMAIN, TAG, `Callback tests completed: ${results.length} cases`);
      }
    );
    await test.runAllTests();
  }

  private async runLifecycleObserverTest(): Promise<void> {
    const test = new LifecycleObserverIntegrationTest(
      this.surfaceId,
      1920,
      1080,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        hilog.info(DOMAIN, TAG, `Lifecycle Observer tests completed: ${results.length} cases`);
      }
    );
    await test.runAllTests();
  }

  private async runIncrementalBenchmark(): Promise<void> {
    const test = new IncrementalBenchmarkTest(
      this.surfaceId,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        hilog.info(DOMAIN, TAG, `Incremental Benchmark tests completed: ${results.length} cases`);
      }
    );
    await test.runAllTests();
  }

  private async runTouchCaptureTest(): Promise<void> {
    this.isTouchTestActive = true;
    this.touchMarkers = [];
    this.touchInstruction = '请在屏幕上触摸';

    this.touchTest = new TouchCaptureIntegrationTest(
      this.surfaceId,
      800,
      600,
      (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        this.touchInstruction = '测试完成';
        hilog.info(DOMAIN, TAG, `Touch Capture tests completed: ${results.length} cases`);
      }
    );

    await this.touchTest.runAllTests();
  }

  private async runTileRenderTest(): Promise<void> {
    const test = new TileRenderIntegrationTest(this.surfaceId, (results: TestCaseResult[]) => {
      this.testResults = results;
      this.completedTests = results.length;
      hilog.info(DOMAIN, TAG, `Tile Render tests completed: ${results.length} cases`);
    });
    await test.runAllTests();
  }

  private async runRenderSessionTest(): Promise<void> {
    const test = new RenderSessionIntegrationTest(this.surfaceId, (results: TestCaseResult[]) => {
      this.testResults = results;
      this.completedTests = results.length;
      hilog.info(DOMAIN, TAG, `Render Session tests completed: ${results.length} cases`);
    });
    await test.runAllTests();
  }

  private handleTouchOverlayEvent(event: TouchEvent): void {
    if (this.touchTest) {
      this.touchTest.onTouchEvent(event);
    }

    const newMarkers: TouchMarker[] = [];
    for (let i = 0; i < event.touches.length; i++) {
      const touch = event.touches[i];
      const color = getFingerColor(touch.id);
      const marker: TouchMarker = {
        id: touch.id,
        x: touch.x,
        y: touch.y,
        color: color
      };
      newMarkers.push(marker);

      hilog.info(DOMAIN, TAG, 'TouchOverlay: id=%{public}d x=%{public}f y=%{public}f type=%{public}d',
        touch.id, touch.x, touch.y, event.type);
    }

    if (event.type === TouchType.Up || event.type === TouchType.Cancel) {
      this.touchMarkers = [];
    } else {
      this.touchMarkers = newMarkers;
    }
  }

  @Builder
  TouchMarkerCircles() {
    ForEach(this.touchMarkers, (marker: TouchMarker) => {
      Circle()
        .width(60)
        .height(60)
        .fill(marker.color + '80')
        .stroke(marker.color)
        .strokeWidth(3)
        .position({ x: marker.x - 30, y: marker.y - 30 })
    }, (marker: TouchMarker) => `${marker.id}_${marker.x}_${marker.y}`)
  }

  build() {
    Stack() {
      XComponent({
        id: 'integration_test_surface',
        type: XComponentType.SURFACE,
        controller: this.xComponentController
      })
        .onLoad(() => {
          this.surfaceId = this.xComponentController.getXComponentSurfaceId();
          this.isInitialized = true;
          hilog.info(DOMAIN, TAG, 'XComponent loaded, surfaceId: %{public}s', this.surfaceId);
        })
        .onDestroy(() => {
          hilog.info(DOMAIN, TAG, 'XComponent destroyed');
        })
        .width('100%')
        .height('100%')

      if (this.isTouchTestActive) {
        Column() {
          this.TouchMarkerCircles()
        }
        .width('100%')
        .height('100%')
        .hitTestBehavior(HitTestMode.Default)
        .onTouch((event: TouchEvent) => {
          this.handleTouchOverlayEvent(event);
        })
      }

      if (!this.showTestPanel && !this.showModuleSelector) {
        Column() {
          Text('ArkZeroRenderer 集成测试')
            .fontSize(24)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.White)
            .margin({ top: 20 })

          Text('选择要运行的测试模块')
            .fontSize(14)
            .fontColor('#CCCCCC')
            .margin({ top: 10 })

          Button('返回首页')
            .fontSize(14)
            .width(250)
            .margin({ top: 10 })
            .backgroundColor('#666666')
            .onClick(() => {
              router.back();
            })

          Column() {
            Button(TestModule.RENDERER)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.RENDERER);
              })

            Button(TestModule.EGL_CONTEXT)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.EGL_CONTEXT);
              })

            Button(TestModule.PIXEL_FORMAT)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.PIXEL_FORMAT);
              })

            Button(TestModule.STRESS)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.STRESS);
              })

            Button(TestModule.CALLBACK)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.CALLBACK);
              })

            Button(TestModule.LIFECYCLE_OBSERVER)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.LIFECYCLE_OBSERVER);
              })

            Button(TestModule.INCREMENTAL_BENCH)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.INCREMENTAL_BENCH);
              })

            Button(TestModule.TOUCH_CAPTURE)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .backgroundColor('#9C27B0')
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.TOUCH_CAPTURE);
              })

            Button(TestModule.TILE_RENDER)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .backgroundColor('#009688')
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.TILE_RENDER);
              })

            Button(TestModule.RENDER_SESSION)
              .fontSize(14)
              .width(250)
              .margin({ top: 10 })
              .backgroundColor('#795548')
              .enabled(this.isInitialized)
              .onClick(() => {
                this.runTestModule(TestModule.RENDER_SESSION);
              })
          }
          .margin({ top: 30 })

          if (!this.isInitialized) {
            Text('等待 Surface 初始化...')
              .fontSize(12)
              .fontColor('#FF9800')
              .margin({ top: 20 })
          } else {
            Text('Surface 已就绪')
              .fontSize(12)
              .fontColor('#4CAF50')
              .margin({ top: 20 })
          }
        }
        .width('100%')
        .height('100%')
        .justifyContent(FlexAlign.Center)
        .backgroundColor('rgba(0, 0, 0, 0.8)')
        .hitTestBehavior(HitTestMode.Transparent)
      }

      if (this.showTestPanel) {
        Column() {
          Row() {
            Text(`测试模块: ${this.currentTestModule}`)
              .fontSize(16)
              .fontWeight(FontWeight.Bold)
              .fontColor(Color.White)

            Blank()

            if (this.isTouchTestActive) {
              Text(this.touchInstruction)
                .fontSize(12)
                .fontColor('#FF9800')
                .margin({ right: 10 })
            }

            Button('返回')
              .fontSize(12)
              .width(80)
              .height(30)
              .enabled(!this.isRunning)
              .onClick(() => {
                this.showTestPanel = false;
                this.isTouchTestActive = false;
                this.touchMarkers = [];
                this.touchTest = undefined;
                this.testResults = [];
                this.completedTests = 0;
                this.currentTestModule = '';
              })
          }
          .width('100%')
          .padding({ left: 15, right: 15, top: 10, bottom: 10 })
          .backgroundColor('rgba(0, 0, 0, 0.9)')

          TestResultPanel({
            testResults: this.testResults,
            totalTests: this.totalTests,
            completedTests: this.completedTests
          })
            .layoutWeight(1)
            .width('100%')
        }
        .width('70%')
        .height('85%')
        .position({ x: '15%', y: '7.5%' })
        .hitTestBehavior(HitTestMode.Transparent)
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.Black)
  }
}
