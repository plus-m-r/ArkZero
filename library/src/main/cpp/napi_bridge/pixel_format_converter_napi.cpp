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

#include "pixel_format_converter_napi.h"
#include "../renderer/backend/PixelFormatConverter.h"
#include <hilog/log.h>

namespace NativeXComponentSample {

napi_value GetGLInternalFormat(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PFCNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int32_t formatValue = 0;
    napi_get_value_int32(env, args[0], &formatValue);
    
    PixelFormat format = static_cast<PixelFormat>(formatValue);
    GLint glFormat = PixelFormatConverter::GetGLInternalFormat(format);
    
    napi_value result;
    napi_create_int32(env, glFormat, &result);
    
    return result;
}

napi_value GetGLFormat(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PFCNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int32_t formatValue = 0;
    napi_get_value_int32(env, args[0], &formatValue);
    
    PixelFormat format = static_cast<PixelFormat>(formatValue);
    GLenum glFormat = PixelFormatConverter::GetGLFormat(format);
    
    napi_value result;
    napi_create_uint32(env, glFormat, &result);
    
    return result;
}

napi_value GetBytesPerPixel(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PFCNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int32_t formatValue = 0;
    napi_get_value_int32(env, args[0], &formatValue);
    
    PixelFormat format = static_cast<PixelFormat>(formatValue);
    int bytesPerPixel = PixelFormatConverter::GetBytesPerPixel(format);
    
    napi_value result;
    napi_create_int32(env, bytesPerPixel, &result);
    
    return result;
}

} // namespace NativeXComponentSample
