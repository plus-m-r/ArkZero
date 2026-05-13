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

#ifndef SURFACE_MANAGER_NAPI_H
#define SURFACE_MANAGER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 通过 surfaceId 创建 NativeWindow（模拟）
 * 
 * ArkTS调用: surfaceManagerCreateNativeWindow(surfaceId: string): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - NativeWindow 指针（模拟值）
 */
napi_value SurfaceManagerCreateNativeWindow(napi_env env, napi_callback_info info);

/**
 * 销毁 NativeWindow（模拟）
 * 
 * ArkTS调用: surfaceManagerDestroyNativeWindow(windowPtr: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 */
napi_value SurfaceManagerDestroyNativeWindow(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // SURFACE_MANAGER_NAPI_H
