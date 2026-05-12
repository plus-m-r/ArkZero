import { UIAbility, Want, AbilityConstant } from '@kit.AbilityKit';
import { window } from '@kit.ArkUI';

/**
 * 测试 Ability - 用于运行 Instrument Test
 */
export default class TestAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    console.info('TestAbility onCreate');
  }

  onDestroy(): void {
    console.info('TestAbility onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    console.info('TestAbility onWindowStageCreate');

    windowStage.loadContent('testability/pages/Index', (err, data) => {
      if (err.code) {
        console.error('Failed to load content. Cause:', JSON.stringify(err) ?? '');
        return;
      }
      console.info('Succeeded in loading content. Data:', JSON.stringify(data) ?? '');
    });
  }

  onWindowStageDestroy(): void {
    console.info('TestAbility onWindowStageDestroy');
  }

  onForeground(): void {
    console.info('TestAbility onForeground');
  }

  onBackground(): void {
    console.info('TestAbility onBackground');
  }
}
