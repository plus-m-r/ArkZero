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

#ifndef RENDERER_NAPI_H
#define RENDERER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建渲染器（测试专用）
 * 
 * ArkTS调用: createTestRenderer(width: number, height: number, format: number, enableAsync?: boolean): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 渲染器句柄
 */
napi_value CreateTestRenderer(napi_env env, napi_callback_info info);

/**
 * 销毁渲染器（测试专用）
 * 
 * ArkTS调用: destroyTestRenderer(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 */
napi_value DestroyTestRenderer(napi_env env, napi_callback_info info);

/**
 * 初始化渲染器（离屏模式）
 * 
 * ArkTS调用: rendererInitialize(handle: number, width: number, height: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value RendererInitialize(napi_env env, napi_callback_info info);

/**
 * 渲染帧
 * 
 * ArkTS调用: rendererRenderFrame(handle: number, buffer: ArrayBuffer, width: number, height: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value RendererRenderFrame(napi_env env, napi_callback_info info);

/**
 * 调整尺寸
 * 
 * ArkTS调用: rendererResize(handle: number, width: number, height: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value RendererResize(napi_env env, napi_callback_info info);

/**
 * 检查是否已初始化
 * 
 * ArkTS调用: isRendererInitialized(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否已初始化
 */
napi_value IsRendererInitialized(napi_env env, napi_callback_info info);

/**
 * 获取后端名称
 * 
 * ArkTS调用: getRendererBackendName(handle: number): string
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return string - 后端名称
 */
napi_value GetRendererBackendName(napi_env env, napi_callback_info info);

/**
 * 获取性能统计
 * 
 * ArkTS调用: getRendererPerformanceStats(handle: number): string
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return string - 性能统计字符串
 */
napi_value GetRendererPerformanceStats(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // RENDERER_NAPI_H
