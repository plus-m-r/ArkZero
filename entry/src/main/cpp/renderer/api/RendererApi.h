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

#ifndef NATIVE_XCOMPONENT_RENDERER_API_H
#define NATIVE_XCOMPONENT_RENDERER_API_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建渲染器（使用 XComponent Surface）
 * 
 * ⭐ 推荐方式：Direct Surface Rendering
 * 
 * ArkTS调用: createWithSurface(surfaceId: string, width: number, height: number, format: PixelFormat): Promise<number>
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return Promise<number> - 渲染器句柄
 */
napi_value CreateRendererWithSurface(napi_env env, napi_callback_info info);

/**
 * 渲染帧
 * 
 * ArkTS调用: renderFrame(handle: number, pixelData: ArrayBuffer, width: number, height: number): Promise<void>
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return Promise<void>
 */
napi_value RenderFrame(napi_env env, napi_callback_info info);

/**
 * 调整渲染尺寸
 * 
 * ArkTS调用: resize(handle: number, width: number, height: number): Promise<void>
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return Promise<void>
 */
napi_value ResizeRenderer(napi_env env, napi_callback_info info);

/**
 * 销毁渲染器
 * 
 * ArkTS调用: destroy(handle: number): Promise<void>
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return Promise<void>
 */
napi_value DestroyRenderer(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // NATIVE_XCOMPONENT_RENDERER_API_H
