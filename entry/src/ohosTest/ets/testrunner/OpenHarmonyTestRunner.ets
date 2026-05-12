import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import { Hypium } from '@ohos/hypium';
import testsuite from '../test/List.test';

/**
 * OpenHarmony 测试运行器
 * 用于 Instrument Test 环境
 */
export default class OpenHarmonyTestRunner {
  constructor() {
  }

  onPrepare() {
    console.info('[TestRunner] onPrepare - Initializing test environment');
  }

  onRun() {
    console.info('[TestRunner] onRun - Starting test execution');
    
    try {
      const abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
      const abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
      
      console.info('[TestRunner] Executing test suite');
      Hypium.hypiumTest(abilityDelegator, abilityDelegatorArguments, testsuite);
      
      console.info('[TestRunner] Test suite execution completed');
    } catch (error) {
      console.error('[TestRunner] Failed to execute test suite:', (error as Error).message);
    }
  }
}
