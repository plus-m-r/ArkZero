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

#ifndef TEXTURE_POOL_NAPI_H
#define TEXTURE_POOL_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建纹理池
 * 
 * ArkTS调用: createTexturePool(maxSize?: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 纹理池句柄
 */
napi_value CreateTexturePool(napi_env env, napi_callback_info info);

/**
 * 销毁纹理池
 * 
 * ArkTS调用: destroyTexturePool(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value DestroyTexturePool(napi_env env, napi_callback_info info);

/**
 * 获取或创建纹理
 * 
 * ArkTS调用: texturePoolAcquire(handle: number, width: number, height: number, internalFormat: number, format: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value TexturePoolAcquire(napi_env env, napi_callback_info info);

/**
 * 归还纹理到池中
 * 
 * ArkTS调用: texturePoolRelease(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value TexturePoolRelease(napi_env env, napi_callback_info info);

/**
 * 预分配常用分辨率纹理
 * 
 * ArkTS调用: texturePoolPreallocate(handle: number, resolutions: Array<{width: number, height: number}>, internalFormat: number, format: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value TexturePoolPreallocate(napi_env env, napi_callback_info info);

/**
 * 清空池
 * 
 * ArkTS调用: texturePoolClear(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value TexturePoolClear(napi_env env, napi_callback_info info);

/**
 * 获取池当前大小
 * 
 * ArkTS调用: texturePoolSize(handle: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 池大小
 */
napi_value TexturePoolSize(napi_env env, napi_callback_info info);

/**
 * 获取命中率统计
 * 
 * ArkTS调用: texturePoolGetStats(handle: number): object
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return object - {hitCount, missCount, hitRate}
 */
napi_value TexturePoolGetStats(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // TEXTURE_POOL_NAPI_H
