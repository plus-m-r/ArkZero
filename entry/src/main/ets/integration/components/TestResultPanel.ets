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

import hilog from '@ohos.hilog';

const DOMAIN = 0x0001;
const TAG = 'TestResultPanel';

/**
 * 测试结果状态
 */
export enum TestStatus {
  PENDING = 'pending',
  RUNNING = 'running',
  PASSED = 'passed',
  FAILED = 'failed'
}

/**
 * 单个测试用例结果
 */
export interface TestCaseResult {
  name: string;
  status: TestStatus;
  duration: number; // 毫秒
  message?: string;
}

/**
 * 测试结果面板组件
 * 
 * 🎯 功能：
 * - 实时显示测试进度
 * - 展示每个测试用例的结果
 * - 统计通过率
 */
@Component
export struct TestResultPanel {
  @Prop testResults: TestCaseResult[] = [];
  @Prop totalTests: number = 0;
  @Prop completedTests: number = 0;

  private getPassCount(): number {
    return this.testResults.filter(r => r.status === TestStatus.PASSED).length;
  }

  private getFailCount(): number {
    return this.testResults.filter(r => r.status === TestStatus.FAILED).length;
  }

  private getPassRate(): string {
    if (this.completedTests === 0) return '0%';
    return `${Math.floor((this.getPassCount() / this.completedTests) * 100)}%`;
  }

  private getStatusColor(status: TestStatus): string {
    switch (status) {
      case TestStatus.PASSED:
        return '#4CAF50';
      case TestStatus.FAILED:
        return '#F44336';
      case TestStatus.RUNNING:
        return '#FF9800';
      default:
        return '#9E9E9E';
    }
  }

  private getStatusIcon(status: TestStatus): string {
    switch (status) {
      case TestStatus.PASSED:
        return '✅';
      case TestStatus.FAILED:
        return '❌';
      case TestStatus.RUNNING:
        return '⏳';
      default:
        return '⏸️';
    }
  }

  build() {
    Column() {
      // 标题栏
      Row() {
        Text('🧪 集成测试结果')
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.White)
        
        Blank()
        
        Text(`${this.getPassRate()} (${this.getPassCount()}/${this.completedTests})`)
          .fontSize(14)
          .fontColor(Color.White)
      }
      .width('100%')
      .padding({ left: 15, right: 15, top: 10, bottom: 10 })
      .backgroundColor('rgba(0, 0, 0, 0.7)')

      // 进度条
      if (this.totalTests > 0) {
        Progress({ value: this.completedTests, total: this.totalTests, type: ProgressType.Linear })
          .width('100%')
          .height(4)
          .color('#4CAF50')
      }

      // 测试结果列表
      Scroll() {
        Column() {
          ForEach(this.testResults, (result: TestCaseResult) => {
            Row() {
              Text(this.getStatusIcon(result.status))
                .fontSize(16)
                .margin({ right: 8 })
              
              Column() {
                Text(result.name)
                  .fontSize(14)
                  .fontColor(Color.White)
                  .textAlign(TextAlign.Start)
                
                if (result.message) {
                  Text(result.message)
                    .fontSize(12)
                    .fontColor('#CCCCCC')
                    .textAlign(TextAlign.Start)
                }
              }
              .alignItems(HorizontalAlign.Start)
              .layoutWeight(1)
              
              Text(`${result.duration}ms`)
                .fontSize(12)
                .fontColor('#AAAAAA')
            }
            .width('100%')
            .padding({ left: 15, right: 15, top: 8, bottom: 8 })
            .backgroundColor(this.getStatusColor(result.status) + '20')
            .border({
              width: { left: 3 },
              color: this.getStatusColor(result.status)
            })
          }, (result: TestCaseResult) => result.name)
        }
      }
      .layoutWeight(1)
      .width('100%')

      // 底部统计
      Row() {
        Text(`总计: ${this.totalTests}`)
          .fontSize(12)
          .fontColor('#AAAAAA')
          .margin({ right: 15 })
        
        Text(`通过: ${this.getPassCount()}`)
          .fontSize(12)
          .fontColor('#4CAF50')
          .margin({ right: 15 })
        
        Text(`失败: ${this.getFailCount()}`)
          .fontSize(12)
          .fontColor('#F44336')
      }
      .width('100%')
      .padding(10)
      .backgroundColor('rgba(0, 0, 0, 0.7)')
    }
    .width('100%')
    .height('100%')
    .backgroundColor('rgba(0, 0, 0, 0.85)')
  }
}
