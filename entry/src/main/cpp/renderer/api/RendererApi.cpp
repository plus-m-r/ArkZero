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

#include "RendererApi.h"
#include "../manager/RendererManager.h"
#include "../manager/SurfaceManager.h"
#include <hilog/log.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../../common/common.h"

namespace NativeXComponentSample {



/**
 * create(surfaceId: string, width: number, height: number, format: number): Promise<number>
 */
napi_value CreateRenderer(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "CreateRenderer: env or info is null");
        return nullptr;
    }

    size_t argCnt = 4;
    napi_value args[4] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "CreateRenderer: napi_get_cb_info failed");
        return nullptr;
    }

    if (argCnt != 4) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments. Expected: surfaceId, width, height, format");
        return nullptr;
    }

    // 参数验证和提取
    napi_valuetype valuetype;
    
    // 获取surfaceId（字符串）
    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_string) {
        napi_throw_type_error(env, NULL, "First argument must be a string (surfaceId)");
        return nullptr;
    }
    
    char surfaceId[256];
    size_t result;
    if (napi_get_value_string_utf8(env, args[0], surfaceId, sizeof(surfaceId), &result) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get surfaceId");
        return nullptr;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RendererApi", "CreateRenderer: surfaceId=%{public}s", surfaceId);
    
    // 获取width
    if (napi_typeof(env, args[1], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Second argument must be a number (width)");
        return nullptr;
    }
    double width;
    if (napi_get_value_double(env, args[1], &width) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get width value");
        return nullptr;
    }

    // 获取height
    if (napi_typeof(env, args[2], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Third argument must be a number (height)");
        return nullptr;
    }
    double height;
    if (napi_get_value_double(env, args[2], &height) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get height value");
        return nullptr;
    }

    // 获取format
    if (napi_typeof(env, args[3], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Fourth argument must be a number (format)");
        return nullptr;
    }
    double formatValue;
    if (napi_get_value_double(env, args[3], &formatValue) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get format value");
        return nullptr;
    }
    PixelFormat format = static_cast<PixelFormat>(static_cast<int>(formatValue));

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RendererApi", "Creating with XComponent Surface: width=%{public}f, height=%{public}f, format=%{public}d", 
        width, height, static_cast<int>(format));

    // ⭐ 优先使用 XComponent callback 存储的 NativeWindow
    void* nativeWindow = SurfaceManager::GetInstance().GetStoredNativeWindow(std::string(surfaceId));
    if (nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "RendererApi", "✅ Using stored NativeWindow from XComponent callback: %{public}s", surfaceId);
    } else {
        // Fallback: 通过 surfaceId 创建 NativeWindow
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN,
            "RendererApi", "No stored NativeWindow, falling back to CreateNativeWindow");
        nativeWindow = SurfaceManager::GetInstance().CreateNativeWindow(std::string(surfaceId));
    }
    if (!nativeWindow) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "❌ Failed to create NativeWindow from surfaceId: %{public}s", surfaceId);
        napi_throw_error(env, NULL, "Failed to create NativeWindow from surfaceId");
        return nullptr;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RendererApi", "✅ Created NativeWindow from surfaceId: %{public}s", surfaceId);
    
    // 调用管理器创建渲染器
    int32_t handle = RendererManager::GetInstance().CreateRenderer(
        nativeWindow,
        static_cast<int32_t>(width), 
        static_cast<int32_t>(height),
        format
    );

    if (handle < 0) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, LOG_PRINT_DOMAIN,
            "RendererApi", "[ARKZERO-FATAL] CreateRenderer: CreateRenderer returned handle=%{public}d", handle);
        napi_throw_error(env, NULL, "Failed to create Renderer with surface");
        return nullptr;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "RendererApi", "[ARKZERO] CreateRenderer: SUCCESS, handle=%{public}d", handle);

    // 创建Promise并resolve
    napi_value promise;
    napi_deferred deferred;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "CreateRenderer: napi_create_promise failed");
        return nullptr;
    }

    napi_value resolveValue;
    if (napi_create_int32(env, handle, &resolveValue) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "CreateRenderer: napi_create_int32 failed");
        return nullptr;
    }

    if (napi_resolve_deferred(env, deferred, resolveValue) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "CreateRenderer: napi_resolve_deferred failed");
        return nullptr;
    }

    return promise;
}

/**
 * renderFrame(handle: number, pixelData: ArrayBuffer, width: number, height: number): Promise<void>
 */
napi_value RenderFrame(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "RenderFrame: env or info is null");
        return nullptr;
    }

    size_t argCnt = 4;
    napi_value args[4] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "RenderFrame: napi_get_cb_info failed");
        return nullptr;
    }

    if (argCnt != 4) {
        napi_throw_type_error(env, NULL, 
            "Wrong number of arguments. Expected: handle, pixelData, width, height");
        return nullptr;
    }

    // 获取handle
    napi_valuetype valuetype;
    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "First argument must be a number (handle)");
        return nullptr;
    }
    int32_t handle;
    if (napi_get_value_int32(env, args[0], &handle) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get handle value");
        return nullptr;
    }

    // 获取pixelData（ArrayBuffer）- ⭐ 零拷贝关键
    if (napi_typeof(env, args[1], &valuetype) != napi_ok || valuetype != napi_object) {
        napi_throw_type_error(env, NULL, "Second argument must be an ArrayBuffer");
        return nullptr;
    }

    bool isArrayBuffer;
    if (napi_is_arraybuffer(env, args[1], &isArrayBuffer) != napi_ok || !isArrayBuffer) {
        napi_throw_type_error(env, NULL, "Second argument must be an ArrayBuffer");
        return nullptr;
    }

    void* data = nullptr;
    size_t byteLength = 0;
    if (napi_get_arraybuffer_info(env, args[1], &data, &byteLength) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "RenderFrame: napi_get_arraybuffer_info failed");
        return nullptr;
    }

    // 获取width
    if (napi_typeof(env, args[2], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Third argument must be a number (width)");
        return nullptr;
    }
    double width;
    if (napi_get_value_double(env, args[2], &width) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get width value");
        return nullptr;
    }

    // 获取height
    if (napi_typeof(env, args[3], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Fourth argument must be a number (height)");
        return nullptr;
    }
    double height;
    if (napi_get_value_double(env, args[3], &height) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get height value");
        return nullptr;
    }

    Renderer* renderer = RendererManager::GetInstance().GetRenderer(handle);
    if (renderer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] RenderFrame: handle=%{public}d NOT found", handle);
        napi_throw_error(env, NULL, "Invalid renderer handle for renderFrame");
        return nullptr;
    }

    bool renderSuccess = renderer->RenderFrame(
        data,
        byteLength,
        static_cast<int32_t>(width),
        static_cast<int32_t>(height));

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        napi_throw_error(env, NULL, "Failed to create promise");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (renderSuccess) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_value message = nullptr;
        napi_value error = nullptr;
        napi_create_string_utf8(env, "RenderFrame failed", NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &error);
        napi_reject_deferred(env, deferred, error);
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[ARKZERO-FATAL] RenderFrame: FAILED (synchronous)");
    }

    return promise;
}

napi_value RenderFrameRegions(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        return nullptr;
    }

    size_t argCnt = 6;
    napi_value args[6] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        return nullptr;
    }

    if (argCnt != 6) {
        napi_throw_type_error(env, NULL,
            "Wrong number of arguments. Expected: handle, pixelData, frameWidth, frameHeight, regions, swap");
        return nullptr;
    }

    napi_valuetype valuetype;

    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "First argument must be a number (handle)");
        return nullptr;
    }
    int32_t handle;
    if (napi_get_value_int32(env, args[0], &handle) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get handle value");
        return nullptr;
    }

    bool isArrayBuffer;
    if (napi_is_arraybuffer(env, args[1], &isArrayBuffer) != napi_ok || !isArrayBuffer) {
        napi_throw_type_error(env, NULL, "Second argument must be an ArrayBuffer");
        return nullptr;
    }
    void* data = nullptr;
    size_t byteLength = 0;
    if (napi_get_arraybuffer_info(env, args[1], &data, &byteLength) != napi_ok) {
        return nullptr;
    }

    if (napi_typeof(env, args[2], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Third argument must be a number (frameWidth)");
        return nullptr;
    }
    double frameWidth;
    if (napi_get_value_double(env, args[2], &frameWidth) != napi_ok) {
        return nullptr;
    }

    if (napi_typeof(env, args[3], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Fourth argument must be a number (frameHeight)");
        return nullptr;
    }
    double frameHeight;
    if (napi_get_value_double(env, args[3], &frameHeight) != napi_ok) {
        return nullptr;
    }

    bool isArray = false;
    if (napi_is_array(env, args[4], &isArray) != napi_ok || !isArray) {
        napi_throw_type_error(env, NULL, "Fifth argument must be an array of DirtyRect");
        return nullptr;
    }
    uint32_t regionCount = 0;
    if (napi_get_array_length(env, args[4], &regionCount) != napi_ok) {
        return nullptr;
    }

    std::vector<DirtyRect> regions(regionCount);
    for (uint32_t i = 0; i < regionCount; i++) {
        napi_value element = nullptr;
        if (napi_get_element(env, args[4], i, &element) != napi_ok) {
            return nullptr;
        }

        napi_value xVal = nullptr;
        napi_value yVal = nullptr;
        napi_value wVal = nullptr;
        napi_value hVal = nullptr;

        if (napi_get_named_property(env, element, "x", &xVal) != napi_ok ||
            napi_get_named_property(env, element, "y", &yVal) != napi_ok ||
            napi_get_named_property(env, element, "w", &wVal) != napi_ok ||
            napi_get_named_property(env, element, "h", &hVal) != napi_ok) {
            return nullptr;
        }

        double xd, yd, wd, hd;
        if (napi_get_value_double(env, xVal, &xd) != napi_ok ||
            napi_get_value_double(env, yVal, &yd) != napi_ok ||
            napi_get_value_double(env, wVal, &wd) != napi_ok ||
            napi_get_value_double(env, hVal, &hd) != napi_ok) {
            return nullptr;
        }

        regions[i].x = static_cast<int32_t>(xd);
        regions[i].y = static_cast<int32_t>(yd);
        regions[i].w = static_cast<int32_t>(wd);
        regions[i].h = static_cast<int32_t>(hd);
    }

    bool swap = false;
    napi_get_value_bool(env, args[5], &swap);

    Renderer* renderer = RendererManager::GetInstance().GetRenderer(handle);
    if (renderer == nullptr) {
        napi_throw_error(env, NULL, "Invalid renderer handle");
        return nullptr;
    }

    bool renderSuccess = renderer->RenderFrameRegions(
        data, byteLength,
        static_cast<int32_t>(frameWidth),
        static_cast<int32_t>(frameHeight),
        regions.data(),
        static_cast<int32_t>(regionCount),
        swap);

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        napi_throw_error(env, NULL, "Failed to create promise");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (renderSuccess) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_value message = nullptr;
        napi_value error = nullptr;
        napi_create_string_utf8(env, "RenderFrameRegions failed", NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &error);
        napi_reject_deferred(env, deferred, error);
    }

    return promise;
}

napi_value UpdateDirtyRegions(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        return nullptr;
    }

    size_t argCnt = 5;
    napi_value args[5] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        return nullptr;
    }

    if (argCnt != 5) {
        napi_throw_type_error(env, NULL,
            "Wrong number of arguments. Expected: handle, pixelData, frameWidth, frameHeight, regions");
        return nullptr;
    }

    napi_valuetype valuetype;

    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "First argument must be a number (handle)");
        return nullptr;
    }
    int32_t handle;
    if (napi_get_value_int32(env, args[0], &handle) != napi_ok) {
        return nullptr;
    }

    bool isArrayBuffer;
    if (napi_is_arraybuffer(env, args[1], &isArrayBuffer) != napi_ok || !isArrayBuffer) {
        napi_throw_type_error(env, NULL, "Second argument must be an ArrayBuffer");
        return nullptr;
    }
    void* data = nullptr;
    size_t byteLength = 0;
    if (napi_get_arraybuffer_info(env, args[1], &data, &byteLength) != napi_ok) {
        return nullptr;
    }

    if (napi_typeof(env, args[2], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Third argument must be a number (frameWidth)");
        return nullptr;
    }
    double frameWidth;
    if (napi_get_value_double(env, args[2], &frameWidth) != napi_ok) {
        return nullptr;
    }

    if (napi_typeof(env, args[3], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Fourth argument must be a number (frameHeight)");
        return nullptr;
    }
    double frameHeight;
    if (napi_get_value_double(env, args[3], &frameHeight) != napi_ok) {
        return nullptr;
    }

    bool isArray = false;
    if (napi_is_array(env, args[4], &isArray) != napi_ok || !isArray) {
        napi_throw_type_error(env, NULL, "Fifth argument must be an array of DirtyRect");
        return nullptr;
    }
    uint32_t regionCount = 0;
    if (napi_get_array_length(env, args[4], &regionCount) != napi_ok) {
        return nullptr;
    }

    std::vector<DirtyRect> regions(regionCount);
    for (uint32_t i = 0; i < regionCount; i++) {
        napi_value element = nullptr;
        if (napi_get_element(env, args[4], i, &element) != napi_ok) {
            return nullptr;
        }

        napi_value xVal = nullptr;
        napi_value yVal = nullptr;
        napi_value wVal = nullptr;
        napi_value hVal = nullptr;

        if (napi_get_named_property(env, element, "x", &xVal) != napi_ok ||
            napi_get_named_property(env, element, "y", &yVal) != napi_ok ||
            napi_get_named_property(env, element, "w", &wVal) != napi_ok ||
            napi_get_named_property(env, element, "h", &hVal) != napi_ok) {
            return nullptr;
        }

        double xd, yd, wd, hd;
        if (napi_get_value_double(env, xVal, &xd) != napi_ok ||
            napi_get_value_double(env, yVal, &yd) != napi_ok ||
            napi_get_value_double(env, wVal, &wd) != napi_ok ||
            napi_get_value_double(env, hVal, &hd) != napi_ok) {
            return nullptr;
        }

        regions[i].x = static_cast<int32_t>(xd);
        regions[i].y = static_cast<int32_t>(yd);
        regions[i].w = static_cast<int32_t>(wd);
        regions[i].h = static_cast<int32_t>(hd);
    }

    Renderer* renderer = RendererManager::GetInstance().GetRenderer(handle);
    if (renderer == nullptr) {
        napi_throw_error(env, NULL, "Invalid renderer handle");
        return nullptr;
    }

    bool renderSuccess = renderer->UpdateDirtyRegions(
        data, byteLength,
        static_cast<int32_t>(frameWidth),
        static_cast<int32_t>(frameHeight),
        regions.data(),
        static_cast<int32_t>(regionCount));

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        napi_throw_error(env, NULL, "Failed to create promise");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (renderSuccess) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_value message = nullptr;
        napi_value error = nullptr;
        napi_create_string_utf8(env, "UpdateDirtyRegions failed", NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &error);
        napi_reject_deferred(env, deferred, error);
    }

    return promise;
}

napi_value PresentFrame(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        return nullptr;
    }

    size_t argCnt = 1;
    napi_value args[1] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        return nullptr;
    }

    if (argCnt != 1) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments. Expected: handle");
        return nullptr;
    }

    napi_valuetype valuetype;
    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Argument must be a number (handle)");
        return nullptr;
    }
    int32_t handle;
    if (napi_get_value_int32(env, args[0], &handle) != napi_ok) {
        return nullptr;
    }

    Renderer* renderer = RendererManager::GetInstance().GetRenderer(handle);
    if (renderer == nullptr) {
        napi_throw_error(env, NULL, "Invalid renderer handle");
        return nullptr;
    }

    bool success = renderer->PresentFrame();

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        napi_throw_error(env, NULL, "Failed to create promise");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (success) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_value message = nullptr;
        napi_value error = nullptr;
        napi_create_string_utf8(env, "PresentFrame failed", NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &error);
        napi_reject_deferred(env, deferred, error);
    }

    return promise;
}

/**
 * resize(handle: number, width: number, height: number): Promise<void>
 */
napi_value ResizeRenderer(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "ResizeRenderer: env or info is null");
        return nullptr;
    }

    size_t argCnt = 3;
    napi_value args[3] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "ResizeRenderer: napi_get_cb_info failed");
        return nullptr;
    }

    if (argCnt != 3) {
        napi_throw_type_error(env, NULL, 
            "Wrong number of arguments. Expected: handle, width, height");
        return nullptr;
    }

    // 获取handle
    napi_valuetype valuetype;
    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "First argument must be a number (handle)");
        return nullptr;
    }
    int32_t handle;
    if (napi_get_value_int32(env, args[0], &handle) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get handle value");
        return nullptr;
    }

    // 获取width
    if (napi_typeof(env, args[1], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Second argument must be a number (width)");
        return nullptr;
    }
    double width;
    if (napi_get_value_double(env, args[1], &width) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get width value");
        return nullptr;
    }

    // 获取height
    if (napi_typeof(env, args[2], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Third argument must be a number (height)");
        return nullptr;
    }
    double height;
    if (napi_get_value_double(env, args[2], &height) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get height value");
        return nullptr;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RendererApi",
        "ResizeRenderer: handle=%{public}d, width=%{public}f, height=%{public}f", 
        handle, width, height);

    // 获取renderer实例
    Renderer* renderer = RendererManager::GetInstance().GetRenderer(handle);
    if (renderer == nullptr) {
        napi_throw_error(env, NULL, "Invalid renderer handle");
        return nullptr;
    }

    // 调用resize方法
    bool success = renderer->Resize(static_cast<int32_t>(width), 
                                    static_cast<int32_t>(height));

    if (!success) {
        napi_throw_error(env, NULL, "Resize failed");
        return nullptr;
    }

    // 创建Promise并resolve
    napi_value promise;
    napi_deferred deferred;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "ResizeRenderer: napi_create_promise failed");
        return nullptr;
    }

    napi_value resolveValue;
    if (napi_get_undefined(env, &resolveValue) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "ResizeRenderer: napi_get_undefined failed");
        return nullptr;
    }

    if (napi_resolve_deferred(env, deferred, resolveValue) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "ResizeRenderer: napi_resolve_deferred failed");
        return nullptr;
    }

    return promise;
}

/**
 * destroy(handle: number): Promise<void>
 */
napi_value DestroyRenderer(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "DestroyRenderer: env or info is null");
        return nullptr;
    }

    size_t argCnt = 1;
    napi_value args[1] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "DestroyRenderer: napi_get_cb_info failed");
        return nullptr;
    }

    if (argCnt != 1) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments. Expected: handle");
        return nullptr;
    }

    // 获取handle
    napi_valuetype valuetype;
    if (napi_typeof(env, args[0], &valuetype) != napi_ok || valuetype != napi_number) {
        napi_throw_type_error(env, NULL, "Argument must be a number (handle)");
        return nullptr;
    }
    int32_t handle;
    if (napi_get_value_int32(env, args[0], &handle) != napi_ok) {
        napi_throw_type_error(env, NULL, "Failed to get handle value");
        return nullptr;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RendererApi", "DestroyRenderer: handle=%{public}d", handle);

    // 调用管理器销毁renderer
    bool success = RendererManager::GetInstance().DestroyRenderer(handle);

    if (!success) {
        napi_throw_error(env, NULL, "Invalid renderer handle");
        return nullptr;
    }

    // 创建Promise并resolve
    napi_value promise;
    napi_deferred deferred;
    if (napi_create_promise(env, &deferred, &promise) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "DestroyRenderer: napi_create_promise failed");
        return nullptr;
    }

    napi_value resolveValue;
    if (napi_get_undefined(env, &resolveValue) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "DestroyRenderer: napi_get_undefined failed");
        return nullptr;
    }

    if (napi_resolve_deferred(env, deferred, resolveValue) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "RendererApi", "DestroyRenderer: napi_resolve_deferred failed");
        return nullptr;
    }

    return promise;
}

} // namespace NativeXComponentSample
