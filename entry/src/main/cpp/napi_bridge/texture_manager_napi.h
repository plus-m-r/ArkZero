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

#ifndef TEXTURE_MANAGER_NAPI_H
#define TEXTURE_MANAGER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建纹理管理器
 * 
 * ArkTS调用: createTextureManager(): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 纹理管理器句柄
 */
napi_value CreateTextureManager(napi_env env, napi_callback_info info);

/**
 * 销毁纹理管理器
 * 
 * ArkTS调用: destroyTextureManager(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value DestroyTextureManager(napi_env env, napi_callback_info info);

/**
 * 创建纹理
 * 
 * ArkTS调用: textureCreate(handle: number, width: number, height: number, internalFormat: number, format: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value TextureCreate(napi_env env, napi_callback_info info);

/**
 * 更新纹理数据
 * 
 * ArkTS调用: textureUpdate(handle: number, buffer: ArrayBuffer, width: number, height: number, format: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value TextureUpdate(napi_env env, napi_callback_info info);

/**
 * 销毁纹理
 * 
 * ArkTS调用: textureDestroy(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value TextureDestroy(napi_env env, napi_callback_info info);

/**
 * 获取纹理ID
 * 
 * ArkTS调用: getTextureId(handle: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 纹理ID（0表示未创建）
 */
napi_value GetTextureId(napi_env env, napi_callback_info info);

/**
 * 检查纹理是否已创建
 * 
 * ArkTS调用: isTextureCreated(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否已创建
 */
napi_value IsTextureCreated(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // TEXTURE_MANAGER_NAPI_H
