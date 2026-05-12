import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import { Hypium } from '@ohos/hypium';
import testsuite from '../../test/List.test';

/**
 * 测试执行页面 - Instrument Test 入口
 */
@Entry
@Component
struct Index {
  aboutToAppear() {
    console.info('[INSTRUMENT_TEST] START');
    
    // 获取测试代理和参数
    let abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
    let abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
    
    // 配置测试参数
    abilityDelegatorArguments.bundleName = "com.samples.ndkxcomponent";
    abilityDelegatorArguments.parameters = {
      "-b": "com.samples.ndkxcomponent",
      "-m": "entry",
      "-s class": "",
      "-s timeout": "30000",
      "-s coverage": "false"
    };
    
    console.info('[INSTRUMENT_TEST] Running test suite...');
    
    // 执行 Hypium 测试
    Hypium.hypiumTest(abilityDelegator, abilityDelegatorArguments, testsuite);
    
    console.info('[INSTRUMENT_TEST] END');
  }

  build() {
    Column() {
      Text('Running Instrument Tests...')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20 })
      
      Text('PerformanceMonitor NAPI Tests')
        .fontSize(18)
        .fontColor('#0D9FFB')
        .margin({ top: 10 })
      
      Text('Please check HiLog for detailed results')
        .fontSize(14)
        .fontColor('#666666')
        .margin({ top: 20 })
      
      Text('Look for tags: [Hypium] and [LOCAL_TEST]')
        .fontSize(12)
        .fontColor('#999999')
        .margin({ top: 5 })
    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .backgroundColor('#F5F5F5')
  }
}
