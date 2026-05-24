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

/**
 * XComponent 生命周期观察者接口
 * 
 * 🎯 设计模式：Observer Pattern
 * 监听 XComponent 的生命周期事件，自动执行相应操作
 */
export interface XComponentLifecycleObserver {
  /**
   * Surface 创建时调用
   * @param surfaceId Surface ID
   * @param width 宽度
   * @param height 高度
   */
  onSurfaceCreated(surfaceId: string, width: number, height: number): void;
  
  /**
   * Surface 改变时调用（resize）
   * @param surfaceId Surface ID
   * @param width 新宽度
   * @param height 新高度
   */
  onSurfaceChanged(surfaceId: string, width: number, height: number): void;
  
  /**
   * Surface 销毁时调用
   * @param surfaceId Surface ID
   */
  onSurfaceDestroyed(surfaceId: string): void;
  
  /**
   * 获取观察者名称（用于调试）
   */
  getObserverName(): string;
}
