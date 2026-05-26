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

import { ArkZeroRenderer, PixelFormat } from '../components/rendering/ArkZeroRenderer';
import hilog from '@ohos.hilog';
import type { DisposeAsyncOptions } from '../components/rendering/ArkZeroRenderer';

const DOMAIN = 0x0001;
const TAG = 'SurfaceDemoPage';

/**
 * XComponent Surface 直出渲染示例
 * 
 * 🎯 **演示内容**：
 * - 使用 XComponent 作为底层渲染表面
 * - Direct Surface Rendering（直接渲染到屏幕）
 * - 超低延迟 <10ms
 * - VSync 同步，消除画面撕裂
 */
@Entry
@Component
struct SurfaceDemoPage {
  @State surfaceId: string = '';
  @State isInitialized: boolean = false;
  @State frameCount: number = 0;
  
  private renderer?: ArkZeroRenderer;
  private pixelBuffer?: ArrayBuffer;
  private pixelView?: Uint32Array;
  private running: boolean = false;
  private renderLoopPromise?: Promise<void>;
  private readonly disposeOptions: DisposeAsyncOptions = { wait: true, timeoutMs: 2000 };
  private xComponentController: XComponentController = new XComponentController();
  
  // 恢复为原始分辨率，验证是否可见
  private readonly renderWidth = 640;
  private readonly renderHeight = 480;
  aboutToAppear() {
    // 初始化渲染器
    this.renderer = new ArkZeroRenderer({
      width: this.renderWidth,
      height: this.renderHeight,
      format: PixelFormat.RGBA
    });
    
    // 设置渲染完成回调
    this.renderer.setOnFrameRendered(() => {
      this.frameCount++;
      hilog.debug(DOMAIN, TAG, '🎨 Frame rendered: %{public}d', this.frameCount);
    });
  }

  aboutToDisappear() {
    // 停止渲染循环，并在飞行渲染完成后安全释放资源
    this.running = false;

    // 等待循环结束后再销毁 renderer，避免 native 仍在使用 buffer 时释放
    (async () => {
      try {
        if (this.renderLoopPromise) {
          await this.renderLoopPromise;
        }
      } catch (e) {
        // 忽略渲染错误，仍要尝试释放资源
      } finally {
        if (this.renderer) {
          await this.renderer.disposeAsync(this.disposeOptions);
        }
      }
    })();
  }

  /**
   * 生成测试图案（简化版本，提高性能）
   */
  private generateTestPattern(bufferView: Uint32Array, width: number, height: number, time: number): void {
    // 先整帧填充为深灰底色，成本低且能稳定看见输出
    bufferView.fill(0xFF202020);

    const barWidth = 48;
    const barHeight = 48;
    const movingX = Math.floor((time / 4) % Math.max(1, width - barWidth));
    const movingY = Math.floor((time / 6) % Math.max(1, height - barHeight));

    // 横向移动的红色条
    for (let y = 0; y < height; y++) {
      const rowOffset = y * width;
      for (let x = movingX; x < movingX + barWidth; x++) {
        if (x >= 0 && x < width) {
          bufferView[rowOffset + x] = 0xFFFF4040;
        }
      }
    }

    // 纵向移动的蓝色条
    for (let y = movingY; y < movingY + barHeight; y++) {
      if (y >= 0 && y < height) {
        const rowOffset = y * width;
        for (let x = 0; x < width; x += 2) {
          bufferView[rowOffset + x] = 0xFF4080FF;
        }
      }
    }
  }

  private async sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve) => {
      setTimeout(resolve, ms);
    });
  }

  /**
   * 开始渲染循环
   */
  private async startRendering(): Promise<void> {
    if (!this.renderer || !this.isInitialized) {
      hilog.error(DOMAIN, TAG, '❌ Renderer not initialized');
      return;
    }

    // 分配一次像素缓冲区并在整个页面生命周期中复用，避免每帧分配造成 GC 压力
    const bufferSize = this.renderWidth * this.renderHeight * 4; // RGBA
    if (!this.pixelBuffer || this.pixelBuffer.byteLength !== bufferSize) {
      this.pixelBuffer = new ArrayBuffer(bufferSize);
    }
    this.pixelView = new Uint32Array(this.pixelBuffer);

    let frameTime = 0;
    this.running = true;

    // 单 buffer 复用：更新内容后立即通知渲染器刷新，渲染提交仍保持序列化，避免 buffer 被 native 侧并发读取
    this.renderLoopPromise = (async () => {
      while (this.running) {
        try {
          // 生成测试图案
          this.generateTestPattern(this.pixelView!, this.renderWidth, this.renderHeight, frameTime);

          // 通知渲染器刷新当前 buffer，使用端不再重新分配 buffer
          await this.renderer!.renderFrame(
            this.pixelBuffer!,
            this.renderWidth,
            this.renderHeight
          );

          frameTime += 8; // 降低时间增量
        } catch (error) {
          const errorMsg = error instanceof Error ? error.message : String(error);
          hilog.error(DOMAIN, TAG, '❌ Render failed: %{public}s', errorMsg);
        }

        // 实验期间放慢一点，便于观察稳定性
        await this.sleep(100);
      }
    })();

    hilog.info(DOMAIN, TAG, '✅ Rendering started with single reusable ArrayBuffer, resolution: %{public}dx%{public}d', this.renderWidth, this.renderHeight);
  }

  build() {
    Column() {
      // ⭐ XComponent 组件（SURFACE 模式）
      XComponent({
        id: 'surface_renderer',
        type: XComponentType.SURFACE,
        controller: this.xComponentController
      })
        .onLoad(() => {
          // 获取 surface ID
          this.surfaceId = this.xComponentController.getXComponentSurfaceId();
          hilog.info(DOMAIN, TAG, '✅ XComponent loaded, surfaceId: %{public}s', this.surfaceId);
          
          // 初始化渲染器
          this.renderer!.initialize(this.surfaceId)
            .then(() => {
              this.isInitialized = true;
              hilog.info(DOMAIN, TAG, '✅ Renderer initialized');
              
              // 开始渲染
              this.startRendering();
            })
            .catch((error: Error) => {
              const errorMsg = error instanceof Error ? error.message : String(error);
              hilog.error(DOMAIN, TAG, '❌ Initialization failed: %{public}s', errorMsg);
            });
        })
        .onDestroy(() => {
          hilog.info(DOMAIN, TAG, '♻️ XComponent destroyed');
        })
        .width('100%')
        .height('100%');

      // 状态显示
      Row() {
        Text(`Status: ${this.isInitialized ? '✅ Running' : '⏳ Initializing'}`)
          .fontSize(14)
          .fontColor(Color.White)
          .margin({ right: 20 });
        
        Text(`Frames: ${this.frameCount}`)
          .fontSize(14)
          .fontColor(Color.White);
      }
      .padding(10)
      .backgroundColor('rgba(0, 0, 0, 0.5)')
      .position({ x: 10, y: 10 });
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.Black);
  }
}
