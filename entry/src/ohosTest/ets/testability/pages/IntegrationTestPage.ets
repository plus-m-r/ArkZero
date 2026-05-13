import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import { Hypium } from '@ohos/hypium';
import testsuite from '../../test/List.test';
import integrationTests from '../../test/integration/List.integration.test';
import nativerender from 'libnativerender.so';

/**
 * 集成测试执行页面 - 使用真实 XComponent Surface
 * 
 * ⭐ 关键：通过 XComponent 获取真实的 NativeWindow Surface
 */
@Entry
@Component
struct IntegrationTestPage {
  @State surfaceId: string = '';
  @State testStatus: string = '等待 XComponent Surface...';
  @State testResult: string = '';
  
  private xcomponentId: string = 'integration_test_xcomponent';
  private isSurfaceReady: boolean = false;
  private xComponentController: XComponentController = new XComponentController();
  
  aboutToAppear() {
    console.info('[INTEGRATION_TEST] Page appeared, waiting for XComponent surface...');
  }
  
  /**
   * XComponent Surface 创建回调
   * ⭐ 在这里获取真实的 Surface ID
   */
  onXComponentSurfaceCreated(): void {
    const surfaceId: string = this.xComponentController.getXComponentSurfaceId();
    console.info(`[INTEGRATION_TEST] XComponent Surface created: ${surfaceId}`);
    this.surfaceId = surfaceId;
    this.isSurfaceReady = true;
    this.testStatus = 'XComponent Surface 已就绪，开始执行集成测试...';
    
    // 延迟执行测试，确保 Surface 完全初始化
    setTimeout(() => {
      this.runIntegrationTests();
    }, 1000);
  }
  
  /**
   * 执行集成测试
   */
  async runIntegrationTests(): Promise<void> {
    try {
      console.info('[INTEGRATION_TEST] Starting integration tests...');
      this.testStatus = '正在执行集成测试...';
      
      // 获取测试代理和参数
      let abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
      let abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
      
      // 配置测试参数
      abilityDelegatorArguments.bundleName = "com.samples.ndkxcomponent";
      abilityDelegatorArguments.parameters = {
        "-b": "com.samples.ndkxcomponent",
        "-m": "entry",
        "-s class": "",
        "-s timeout": "60000",
        "-s coverage": "false"
      };
      
      // 执行集成测试
      await Hypium.hypiumTest(abilityDelegator, abilityDelegatorArguments, integrationTests);
      
      this.testStatus = '集成测试执行完成';
      this.testResult = '请查看 HiLog 获取详细测试结果';
      
      console.info('[INTEGRATION_TEST] Integration tests completed');
    } catch (error) {
      console.error(`[INTEGRATION_TEST] Test failed: ${JSON.stringify(error)}`);
      this.testStatus = '测试执行失败';
      this.testResult = `错误: ${JSON.stringify(error)}`;
    }
  }
  
  build() {
    Column() {
      Text('集成测试 - 真实 XComponent Surface')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20 })
      
      Text(this.testStatus)
        .fontSize(16)
        .fontColor('#0D9FFB')
        .margin({ top: 20 })
        .textAlign(TextAlign.Center)
      
      if (this.testResult.length > 0) {
        Text(this.testResult)
          .fontSize(14)
          .fontColor('#333333')
          .margin({ top: 10 })
          .textAlign(TextAlign.Center)
      }
      
      // ⭐ 隐藏的 XComponent，用于获取真实 Surface
      XComponent({
        id: this.xcomponentId,
        type: XComponentType.SURFACE,
        controller: this.xComponentController
      })
        .width('1px')  // 最小尺寸，不可见
        .height('1px')
        .onLoad(() => {
          this.onXComponentSurfaceCreated();
        })
        .visibility(Visibility.Hidden)  // 完全隐藏
      
      Text('查看 HiLog 标签: [INTEGRATION_TEST] 和 [Hypium]')
        .fontSize(12)
        .fontColor('#999999')
        .margin({ top: 20 })
    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .backgroundColor('#F5F5F5')
  }
}
