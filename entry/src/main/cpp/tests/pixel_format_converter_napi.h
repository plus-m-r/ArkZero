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

#ifndef PIXEL_FORMAT_CONVERTER_NAPI_H
#define PIXEL_FORMAT_CONVERTER_NAPI_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

/**
 * 获取 OpenGL 内部格式
 * 
 * ArkTS调用: getGLInternalFormat(format: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - GL 内部格式常量
 */
napi_value GetGLInternalFormat(napi_env env, napi_callback_info info);

/**
 * 获取 OpenGL 像素格式
 * 
 * ArkTS调用: getGLFormat(format: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - GL 像素格式常量
 */
napi_value GetGLFormat(napi_env env, napi_callback_info info);

/**
 * 获取每像素字节数
 * 
 * ArkTS调用: getBytesPerPixel(format: number): number
 * 
 * @param env NAPI环境
 * @param info NAPI回调信息
 * @return number - 每像素字节数
 */
napi_value GetBytesPerPixel(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // PIXEL_FORMAT_CONVERTER_NAPI_H
