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

#ifndef PERFORMANCE_MONITOR_NAPI_H
#define PERFORMANCE_MONITOR_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建 PerformanceMonitor 实例
 * 
 * ArkTS调用: createPerformanceMonitor(): Promise<number>
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return Promise<number> - PerformanceMonitor 句柄
 */
napi_value CreatePerformanceMonitor(napi_env env, napi_callback_info info);

/**
 * 销毁 PerformanceMonitor 实例
 * 
 * ArkTS调用: destroyPerformanceMonitor(handle: number): Promise<void>
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return Promise<void>
 */
napi_value DestroyPerformanceMonitor(napi_env env, napi_callback_info info);

/**
 * 开始记录一帧
 * 
 * ArkTS调用: monitorBeginFrame(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return void
 */
napi_value MonitorBeginFrame(napi_env env, napi_callback_info info);

/**
 * 结束记录一帧
 * 
 * ArkTS调用: monitorEndFrame(handle: number, dropped: boolean): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return void
 */
napi_value MonitorEndFrame(napi_env env, napi_callback_info info);

/**
 * 重置性能统计
 * 
 * ArkTS调用: monitorReset(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return void
 */
napi_value MonitorReset(napi_env env, napi_callback_info info);

/**
 * 获取性能统计数据
 * 
 * ArkTS调用: getMonitorStats(handle: number): string
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return string - JSON格式的性能数据
 */
napi_value GetMonitorStats(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // PERFORMANCE_MONITOR_NAPI_H
