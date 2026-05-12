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
import { hilog } from '@ohos.hilog';

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
  private animationTimer?: number;

  aboutToAppear() {
    // 初始化渲染器
    this.renderer = new ArkZeroRenderer({
      width: 1920,
      height: 1080,
      format: PixelFormat.RGBA
    });
    
    // 设置渲染完成回调
    this.renderer.setOnFrameRendered(() => {
      this.frameCount++;
      hilog.debug(DOMAIN, TAG, '🎨 Frame rendered: %{public}d', this.frameCount);
    });
  }

  aboutToDisappear() {
    // 停止动画
    if (this.animationTimer) {
      clearInterval(this.animationTimer);
    }
    
    // 清理资源
    if (this.renderer) {
      this.renderer.dispose();
    }
  }

  /**
   * 生成测试图案（渐变色彩）
   */
  private generateTestPattern(buffer: ArrayBuffer, width: number, height: number, time: number): void {
    const data = new Uint8Array(buffer);
    const timeFactor = time % 360; // 0-360度
    
    for (let y = 0; y < height; y++) {
      for (let x = 0; x < width; x++) {
        const index = (y * width + x) * 4;
        
        // 创建动态渐变效果
        const r = Math.floor((Math.sin(x / 100 + timeFactor / 180 * Math.PI) + 1) * 127.5);
        const g = Math.floor((Math.sin(y / 100 + timeFactor / 180 * Math.PI) + 1) * 127.5);
        const b = Math.floor((Math.sin((x + y) / 200 + timeFactor / 180 * Math.PI) + 1) * 127.5);
        const a = 255;
        
        data[index] = r;
        data[index + 1] = g;
        data[index + 2] = b;
        data[index + 3] = a;
      }
    }
  }

  /**
   * 开始渲染循环
   */
  private async startRendering(): Promise<void> {
    if (!this.renderer || !this.isInitialized) {
      hilog.error(DOMAIN, TAG, '❌ Renderer not initialized');
      return;
    }
    
    // 分配像素缓冲区
    const bufferSize = 1920 * 1080 * 4; // RGBA
    this.pixelBuffer = new ArrayBuffer(bufferSize);
    
    let frameTime = 0;
    
    // 启动渲染循环（60fps）
    this.animationTimer = setInterval(async () => {
      try {
        // 生成测试图案
        this.generateTestPattern(this.pixelBuffer!, 1920, 1080, frameTime);
        
        // 渲染帧（零拷贝）
        await this.renderer!.renderFrame(
          this.pixelBuffer!,
          1920,
          1080
        );
        
        frameTime += 16; // 约60fps
        
      } catch (error) {
        const errorMsg = error instanceof Error ? error.message : String(error);
        hilog.error(DOMAIN, TAG, '❌ Render failed: %{public}s', errorMsg);
      }
    }, 16); // 16ms ≈ 60fps
    
    hilog.info(DOMAIN, TAG, '✅ Rendering started at 60fps');
  }

  build() {
    Column() {
      // ⭐ XComponent 组件（SURFACE 模式）
      XComponent({
        id: 'surface_renderer',
        type: XComponentType.SURFACE,
        controller: new XComponentController()
      })
        .onLoad((context) => {
          // 获取 surface ID
          this.surfaceId = context.getXComponentSurfaceId();
          hilog.info(DOMAIN, TAG, '✅ XComponent loaded, surfaceId: %{public}s', this.surfaceId);
          
          // 初始化渲染器（使用 XComponent Surface）
          this.renderer!.initializeWithSurface(this.surfaceId)
            .then(() => {
              this.isInitialized = true;
              hilog.info(DOMAIN, TAG, '✅ Renderer initialized with XComponent Surface');
              
              // 开始渲染
              this.startRendering();
            })
            .catch((error) => {
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
