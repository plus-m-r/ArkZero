/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { RendererIntegrationTest } from '../tests/RendererIntegration.test';
import { TestResultPanel, TestCaseResult } from '../components/TestResultPanel';
import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'IntegrationTestPage';

/**
 * 集成测试主页面
 * 
 * 🎯 功能：
 * - 在真实环境中测试 ArkZeroRenderer
 * - 使用 XComponent Surface 进行渲染验证
 * - 实时显示测试结果
 */
@Entry
@Component
struct IntegrationTestPage {
  @State surfaceId: string = '';
  @State isInitialized: boolean = false;
  @State showTestPanel: boolean = false;
  @State testResults: TestCaseResult[] = [];
  @State totalTests: number = 5;
  @State completedTests: number = 0;
  
  private xComponentController: XComponentController = new XComponentController();

  aboutToAppear() {
    hilog.info(DOMAIN, TAG, '🧪 Integration Test Page loaded');
  }

  /**
   * 开始运行集成测试
   */
  private async startIntegrationTests(): Promise<void> {
    if (!this.isInitialized || !this.surfaceId) {
      hilog.error(DOMAIN, TAG, '❌ Surface not ready');
      return;
    }
    
    hilog.info(DOMAIN, TAG, '🚀 Starting integration tests...');
    this.showTestPanel = true;
    this.testResults = [];
    this.completedTests = 0;
    
    try {
      const test = new RendererIntegrationTest(this.surfaceId, (results: TestCaseResult[]) => {
        this.testResults = results;
        this.completedTests = results.length;
        
        hilog.info(DOMAIN, TAG, `✅ All tests completed: ${results.length} cases`);
      });
      
      await test.runAllTests();
      
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : String(error);
      hilog.error(DOMAIN, TAG, `❌ Test execution failed: ${errorMsg}`);
    }
  }

  build() {
    Stack() {
      // 背景：XComponent 渲染区域
      XComponent({
        id: 'integration_test_surface',
        type: XComponentType.SURFACE,
        controller: this.xComponentController
      })
        .onLoad(() => {
          this.surfaceId = this.xComponentController.getXComponentSurfaceId();
          this.isInitialized = true;
          hilog.info(DOMAIN, TAG, `✅ XComponent loaded, surfaceId: ${this.surfaceId}`);
        })
        .onDestroy(() => {
          hilog.info(DOMAIN, TAG, '♻️ XComponent destroyed');
        })
        .width('100%')
        .height('100%')

      // 控制面板
      Column() {
        Text('🧪 ArkZeroRenderer 集成测试')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.White)
          .margin({ top: 20 })
        
        Text('在真实 XComponent Surface 上测试渲染功能')
          .fontSize(14)
          .fontColor('#CCCCCC')
          .margin({ top: 10 })
        
        Button('开始集成测试')
          .fontSize(16)
          .width(200)
          .margin({ top: 30 })
          .enabled(this.isInitialized)
          .onClick(() => {
            this.startIntegrationTests();
          })
        
        if (!this.isInitialized) {
          Text('⏳ 等待 Surface 初始化...')
            .fontSize(12)
            .fontColor('#FF9800')
            .margin({ top: 10 })
        } else {
          Text('✅ Surface 已就绪')
            .fontSize(12)
            .fontColor('#4CAF50')
            .margin({ top: 10 })
        }
      }
      .width('100%')
      .height('100%')
      .justifyContent(FlexAlign.Center)
      .backgroundColor(this.showTestPanel ? 'transparent' : 'rgba(0, 0, 0, 0.8)')
      .visibility(this.showTestPanel ? Visibility.None : Visibility.Visible)

      // 测试结果面板（覆盖层）
      if (this.showTestPanel) {
        TestResultPanel({
          testResults: this.testResults,
          totalTests: this.totalTests,
          completedTests: this.completedTests
        })
          .width('60%')
          .height('80%')
          .position({ x: '20%', y: '10%' })
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.Black)
  }
}
