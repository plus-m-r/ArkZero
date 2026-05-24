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

#ifndef EGL_CONTEXT_MANAGER_NAPI_H
#define EGL_CONTEXT_MANAGER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建 EGL 上下文管理器（离屏模式）
 * 
 * ArkTS调用: createEGLContext(width: number, height: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - EGL 上下文管理器句柄
 */
napi_value CreateEGLContext(napi_env env, napi_callback_info info);

/**
 * 销毁 EGL 上下文管理器
 * 
 * ArkTS调用: destroyEGLContext(handle: number): void
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return undefined
 */
napi_value DestroyEGLContext(napi_env env, napi_callback_info info);

/**
 * 使 EGL 上下文当前化
 * 
 * ArkTS调用: eglMakeCurrent(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value EGLMakeCurrent(napi_env env, napi_callback_info info);

/**
 * 检查 EGL 是否已初始化
 * 
 * ArkTS调用: isEGLInitialized(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否已初始化
 */
napi_value IsEGLInitialized(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // EGL_CONTEXT_MANAGER_NAPI_H
