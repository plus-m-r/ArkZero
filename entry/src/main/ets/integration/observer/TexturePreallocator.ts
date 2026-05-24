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

import { XComponentLifecycleObserver } from './XComponentLifecycleObserver';

/**
 * 纹理预分配器观察者
 * 
 * 🎯 职责：
 * - 监听 Surface 创建事件
 * - 自动预分配常用分辨率的纹理
 * - 优化首次渲染性能
 */
export class TexturePreallocator implements XComponentLifecycleObserver {
  private preallocatedResolutions: Array<{width: number, height: number}> = [];
  
  constructor() {
    // 初始化常用分辨率列表
    this.preallocatedResolutions = [
      { width: 1920, height: 1080 },  // 1080p
      { width: 1280, height: 720 },   // 720p
      { width: 3840, height: 2160 },  // 4K
      { width: 800, height: 600 },    // SVGA
    ];
  }
  
  /**
   * Surface 创建时调用
   * @param surfaceId Surface ID
   * @param width 宽度
   * @param height 高度
   */
  onSurfaceCreated(surfaceId: string, width: number, height: number): void {
    console.info(`[TexturePreallocator] 📦 Surface created: ${surfaceId} (${width}x${height})`);
    console.info(`[TexturePreallocator] 📦 Will preallocate ${this.preallocatedResolutions.length} resolutions`);
    
    // ⭐ 在这里可以调用 Native 层的预分配命令
    // 未来可以通过 NAPI 桥接调用 GLESBackend 的策略接口
    this.preallocateTextures(width, height);
  }
  
  /**
   * Surface 改变时调用（resize）
   * @param surfaceId Surface ID
   * @param width 新宽度
   * @param height 新高度
   */
  onSurfaceChanged(surfaceId: string, width: number, height: number): void {
    console.info(`[TexturePreallocator] 📐 Surface resized: ${surfaceId} -> ${width}x${height}`);
    
    // ⭐ Resize 时也可以触发预分配，确保新尺寸可用
    this.preallocateTextures(width, height);
  }
  
  /**
   * Surface 销毁时调用
   * @param surfaceId Surface ID
   */
  onSurfaceDestroyed(surfaceId: string): void {
    console.info(`[TexturePreallocator] 🗑️ Surface destroyed: ${surfaceId}`);
    // Surface 销毁时不需要特殊处理
  }
  
  /**
   * 获取观察者名称
   */
  getObserverName(): string {
    return 'TexturePreallocator';
  }
  
  /**
   * 预分配纹理
   * @param currentWidth 当前宽度
   * @param currentHeight 当前高度
   */
  private preallocateTextures(currentWidth: number, currentHeight: number): void {
    console.info(`[TexturePreallocator] 🔧 Preallocating textures...`);
    
    // 检查当前尺寸是否在预分配列表中
    const currentInList = this.preallocatedResolutions.some(
      res => res.width === currentWidth && res.height === currentHeight
    );
    
    if (!currentInList) {
      console.info(`[TexturePreallocator]   - Adding current resolution: ${currentWidth}x${currentHeight}`);
    }
    
    // 输出预分配计划
    this.preallocatedResolutions.forEach((res, index) => {
      console.info(`[TexturePreallocator]   [${index + 1}] ${res.width}x${res.height}`);
    });
    
    console.info(`[TexturePreallocator] ✅ Preallocation plan ready`);
    
    // ⭐ TODO: 实际预分配需要通过 NAPI 调用 Native 层
    // 例如: nativerender.preallocateTextures(this.preallocatedResolutions);
  }
  
  /**
   * 添加自定义分辨率到预分配列表
   * @param width 宽度
   * @param height 高度
   */
  addResolution(width: number, height: number): void {
    const exists = this.preallocatedResolutions.some(
      res => res.width === width && res.height === height
    );
    
    if (!exists) {
      this.preallocatedResolutions.push({ width, height });
      console.info(`[TexturePreallocator] ➕ Added resolution: ${width}x${height}`);
    }
  }
  
  /**
   * 获取预分配分辨率列表
   */
  getPreallocatedResolutions(): Array<{width: number, height: number}> {
    return [...this.preallocatedResolutions];
  }
}
