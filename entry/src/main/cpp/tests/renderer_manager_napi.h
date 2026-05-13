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

#ifndef RENDERER_MANAGER_NAPI_H
#define RENDERER_MANAGER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 创建离屏渲染器（通过 Manager）
 * 
 * ArkTS调用: managerCreateOffscreenRenderer(width: number, height: number, format: number, enableAsync?: boolean): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 渲染器句柄
 */
napi_value ManagerCreateOffscreenRenderer(napi_env env, napi_callback_info info);

/**
 * 销毁渲染器（通过 Manager）
 * 
 * ArkTS调用: managerDestroyRenderer(handle: number): boolean
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return boolean - 是否成功
 */
napi_value ManagerDestroyRenderer(napi_env env, napi_callback_info info);

/**
 * 获取渲染器数量
 * 
 * ArkTS调用: managerGetRendererCount(): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 当前管理的渲染器数量
 */
napi_value ManagerGetRendererCount(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // RENDERER_MANAGER_NAPI_H
