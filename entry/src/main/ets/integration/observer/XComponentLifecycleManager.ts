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
 * XComponent 生命周期管理器
 * 
 * 🎯 职责：
 * - 管理观察者列表
 * - 分发生命周期事件
 * - 支持动态添加/移除观察者
 */
export class XComponentLifecycleManager {
  private observers: XComponentLifecycleObserver[] = [];
  
  /**
   * 添加观察者
   * @param observer 观察者对象
   */
  addObserver(observer: XComponentLifecycleObserver): void {
    if (!this.observers.includes(observer)) {
      this.observers.push(observer);
      console.info(`[XComponentLifecycleManager] ✅ Observer added: ${observer.getObserverName()}`);
    } else {
      console.warn(`[XComponentLifecycleManager] ⚠️ Observer already exists: ${observer.getObserverName()}`);
    }
  }
  
  /**
   * 移除观察者
   * @param observer 观察者对象
   */
  removeObserver(observer: XComponentLifecycleObserver): void {
    const index = this.observers.indexOf(observer);
    if (index !== -1) {
      this.observers.splice(index, 1);
      console.info(`[XComponentLifecycleManager] ❌ Observer removed: ${observer.getObserverName()}`);
    }
  }
  
  /**
   * 通知所有观察者 Surface 已创建
   * @param surfaceId Surface ID
   * @param width 宽度
   * @param height 高度
   */
  notifySurfaceCreated(surfaceId: string, width: number, height: number): void {
    console.info(`[XComponentLifecycleManager] 📢 Notifying surface created: ${surfaceId} (${width}x${height})`);
    
    this.observers.forEach(observer => {
      try {
        observer.onSurfaceCreated(surfaceId, width, height);
      } catch (error) {
        console.error(`[XComponentLifecycleManager] ❌ Error in observer ${observer.getObserverName()}: ${error}`);
      }
    });
  }
  
  /**
   * 通知所有观察者 Surface 已改变
   * @param surfaceId Surface ID
   * @param width 新宽度
   * @param height 新高度
   */
  notifySurfaceChanged(surfaceId: string, width: number, height: number): void {
    console.info(`[XComponentLifecycleManager] 📢 Notifying surface changed: ${surfaceId} (${width}x${height})`);
    
    this.observers.forEach(observer => {
      try {
        observer.onSurfaceChanged(surfaceId, width, height);
      } catch (error) {
        console.error(`[XComponentLifecycleManager] ❌ Error in observer ${observer.getObserverName()}: ${error}`);
      }
    });
  }
  
  /**
   * 通知所有观察者 Surface 已销毁
   * @param surfaceId Surface ID
   */
  notifySurfaceDestroyed(surfaceId: string): void {
    console.info(`[XComponentLifecycleManager] 📢 Notifying surface destroyed: ${surfaceId}`);
    
    this.observers.forEach(observer => {
      try {
        observer.onSurfaceDestroyed(surfaceId);
      } catch (error) {
        console.error(`[XComponentLifecycleManager] ❌ Error in observer ${observer.getObserverName()}: ${error}`);
      }
    });
  }
  
  /**
   * 获取观察者数量
   */
  getObserverCount(): number {
    return this.observers.length;
  }
  
  /**
   * 清空所有观察者
   */
  clearObservers(): void {
    this.observers = [];
    console.info('[XComponentLifecycleManager] 🗑️ All observers cleared');
  }
}
