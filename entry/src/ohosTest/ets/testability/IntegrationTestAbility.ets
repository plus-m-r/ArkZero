import { UIAbility, Want, AbilityConstant } from '@kit.AbilityKit';
import { window } from '@kit.ArkUI';

/**
 * 集成测试 Ability - 使用真实 XComponent Surface
 */
export default class IntegrationTestAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    console.info('[INTEGRATION_TEST] IntegrationTestAbility onCreate');
  }

  onDestroy(): void {
    console.info('[INTEGRATION_TEST] IntegrationTestAbility onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    console.info('[INTEGRATION_TEST] IntegrationTestAbility onWindowStageCreate');

    // ⭐ 加载集成测试页面（包含 XComponent）
    windowStage.loadContent('testability/pages/IntegrationTestPage', (err, data) => {
      if (err.code) {
        console.error('[INTEGRATION_TEST] Failed to load content. Cause:', JSON.stringify(err) ?? '');
        return;
      }
      console.info('[INTEGRATION_TEST] Succeeded in loading content. Data:', JSON.stringify(data) ?? '');
    });
  }

  onWindowStageDestroy(): void {
    console.info('[INTEGRATION_TEST] IntegrationTestAbility onWindowStageDestroy');
  }

  onForeground(): void {
    console.info('[INTEGRATION_TEST] IntegrationTestAbility onForeground');
  }

  onBackground(): void {
    console.info('[INTEGRATION_TEST] IntegrationTestAbility onBackground');
  }
}
