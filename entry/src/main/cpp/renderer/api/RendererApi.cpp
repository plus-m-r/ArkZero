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

#include "../../common/common.h"

namespace NativeXComponentSample {

struct RenderFrameWorkContext {
    napi_env env { nullptr };
    napi_deferred deferred { nullptr };
    napi_async_work work { nullptr };
    napi_ref bufferRef { nullptr };
    int32_t handle { 0 };
    void* data { nullptr };
    size_t byteLength { 0 };
    int32_t width { 0 };
    int32_t height { 0 };
    std::string errorMessage;
};

static void ExecuteRenderFrameWork(napi_env env, void* data)
{
    auto* context = static_cast<RenderFrameWorkContext*>(data);
    if (context == nullptr) {
        return;
    }

    Renderer* renderer = RendererManager::GetInstance().GetRenderer(context->handle);
    if (renderer == nullptr) {
        context->errorMessage = "Invalid renderer handle";
        return;
    }

    bool renderSuccess = renderer->RenderFrame(
        context->data,
        context->byteLength,
        context->width,
        context->height);

    if (!renderSuccess) {
        context->errorMessage = "RenderFrame failed";
        return;
    }
}

static void CompleteRenderFrameWork(napi_env env, napi_status status, void* data)
{
    auto* context = static_cast<RenderFrameWorkContext*>(data);
    if (context == nullptr) {
        return;
    }

    napi_value result = nullptr;
    if ((status == napi_ok) && context->errorMessage.empty()) {
        napi_get_undefined(env, &result);
        napi_resolve_deferred(env, context->deferred, result);
    } else {
        std::string errorMessage = context->errorMessage.empty() ? "RenderFrame failed" : context->errorMessage;
        napi_value message = nullptr;
        napi_value error = nullptr;
        napi_create_string_utf8(env, errorMessage.c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &error);
        napi_reject_deferred(env, context->deferred, error);
    }

    if (context->bufferRef != nullptr) {
        napi_delete_reference(env, context->bufferRef);
        context->bufferRef = nullptr;
    }

    if (context->work != nullptr) {
        napi_delete_async_work(env, context->work);
        context->work = nullptr;
    }

    delete context;
}

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

    // ⭐ 通过 SurfaceManager 从 surfaceId 创建 NativeWindow
    void* nativeWindow = SurfaceManager::GetInstance().CreateNativeWindow(std::string(surfaceId));
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
        napi_throw_error(env, NULL, "Failed to create Renderer with surface");
        return nullptr;
    }

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

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "RendererApi",
        "RenderFrame: handle=%{public}d, width=%{public}f, height=%{public}f, bufferSize=%{public}zu",
        handle, width, height, byteLength);

    auto* context = new RenderFrameWorkContext();
    context->env = env;
    context->handle = handle;
    context->data = data;
    context->byteLength = byteLength;
    context->width = static_cast<int32_t>(width);
    context->height = static_cast<int32_t>(height);

    if (napi_create_reference(env, args[1], 1, &context->bufferRef) != napi_ok) {
        delete context;
        napi_throw_error(env, NULL, "Failed to retain ArrayBuffer reference");
        return nullptr;
    }

    napi_value resourceName;
    if (napi_create_string_utf8(env, "ArkZeroRendererRenderFrame", NAPI_AUTO_LENGTH, &resourceName) != napi_ok) {
        napi_delete_reference(env, context->bufferRef);
        delete context;
        napi_throw_error(env, NULL, "Failed to create async resource name");
        return nullptr;
    }

    napi_value promise = nullptr;
    if (napi_create_promise(env, &context->deferred, &promise) != napi_ok) {
        napi_delete_reference(env, context->bufferRef);
        delete context;
        napi_throw_error(env, NULL, "Failed to create promise");
        return nullptr;
    }

    if (napi_create_async_work(
        env,
        nullptr,
        resourceName,
        ExecuteRenderFrameWork,
        CompleteRenderFrameWork,
        context,
        &context->work) != napi_ok) {
        napi_delete_reference(env, context->bufferRef);
        delete context;
        napi_throw_error(env, NULL, "Failed to create async work");
        return nullptr;
    }

    if (napi_queue_async_work(env, context->work) != napi_ok) {
        napi_delete_async_work(env, context->work);
        napi_delete_reference(env, context->bufferRef);
        delete context;
        napi_throw_error(env, NULL, "Failed to queue async work");
        return nullptr;
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
