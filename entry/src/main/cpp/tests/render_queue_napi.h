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

#ifndef RENDER_QUEUE_NAPI_H
#define RENDER_QUEUE_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建渲染队列
 * 
 * ArkTS调用: createRenderQueue(maxSize?: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 队列句柄（指针转换）
 */
napi_value CreateRenderQueue(napi_env env, napi_callback_info info);

/**
 * 销毁渲染队列
 * 
 * ArkTS调用: destroyRenderQueue(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value DestroyRenderQueue(napi_env env, napi_callback_info info);

/**
 * 提交渲染命令
 * 
 * ArkTS调用: queueSubmit(handle: number, buffer: ArrayBuffer, dataSize: number, width: number, height: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功提交
 */
napi_value QueueSubmit(napi_env env, napi_callback_info info);

/**
 * 消费渲染命令
 * 
 * ArkTS调用: queueDequeue(handle: number): object | null
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return object - {dataSize, width, height} 或 null
 */
napi_value QueueDequeue(napi_env env, napi_callback_info info);

/**
 * 停止队列
 * 
 * ArkTS调用: queueStop(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value QueueStop(napi_env env, napi_callback_info info);

/**
 * 获取队列信息
 * 
 * ArkTS调用: getQueueInfo(handle: number): object
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return object - {size, maxSize, isRunning}
 */
napi_value GetQueueInfo(napi_env env, napi_callback_info info);

/**
 * 窥视队列头部命令（不消费）
 * 
 * ArkTS调用: queuePeek(handle: number): object | null
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return object - {dataSize, width, height} 或 null
 */
napi_value QueuePeek(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // RENDER_QUEUE_NAPI_H
