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

#ifndef YUV_SHADER_MANAGER_NAPI_H
#define YUV_SHADER_MANAGER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建 YUV Shader 管理器
 * 
 * ArkTS调用: createYUVShaderManager(): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - YUV Shader 管理器句柄
 */
napi_value CreateYUVShaderManager(napi_env env, napi_callback_info info);

/**
 * 销毁 YUV Shader 管理器
 * 
 * ArkTS调用: destroyYUVShaderManager(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value DestroyYUVShaderManager(napi_env env, napi_callback_info info);

/**
 * 初始化 Shader 程序
 * 
 * ArkTS调用: yuvShaderInitialize(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value YUVShaderInitialize(napi_env env, napi_callback_info info);

/**
 * 渲染 NV21 数据
 * 
 * ArkTS调用: yuvShaderRenderNV21(handle: number, yBuffer: ArrayBuffer, uvBuffer: ArrayBuffer, width: number, height: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value YUVShaderRenderNV21(napi_env env, napi_callback_info info);

/**
 * 渲染 NV12 数据
 * 
 * ArkTS调用: yuvShaderRenderNV12(handle: number, yBuffer: ArrayBuffer, uvBuffer: ArrayBuffer, width: number, height: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value YUVShaderRenderNV12(napi_env env, napi_callback_info info);

/**
 * 检查是否已初始化
 * 
 * ArkTS调用: isYUVShaderInitialized(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否已初始化
 */
napi_value IsYUVShaderInitialized(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // YUV_SHADER_MANAGER_NAPI_H
