/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hilog/log.h>

#include "common/common.h"
#include "renderer/api/RendererApi.h"
#include "tests/performance_monitor_napi.h"
#include "tests/pixel_format_converter_napi.h"
#include "tests/render_queue_napi.h"
#include "tests/texture_manager_napi.h"
#include "tests/egl_context_manager_napi.h"

namespace NativeXComponentSample {
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "Init", "Init begins");
    if ((env == nullptr) || (exports == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "env or exports is null");
        return nullptr;
    }

    napi_property_descriptor desc[] = { 
        { "create", nullptr, CreateRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "renderFrame", nullptr, RenderFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "resize", nullptr, ResizeRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getPerformanceStats", nullptr, GetPerformanceStats, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroy", nullptr, DestroyRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        // PerformanceMonitor 测试接口
        { "createPerformanceMonitor", nullptr, CreatePerformanceMonitor, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyPerformanceMonitor", nullptr, DestroyPerformanceMonitor, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "monitorBeginFrame", nullptr, MonitorBeginFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "monitorEndFrame", nullptr, MonitorEndFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "monitorReset", nullptr, MonitorReset, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getMonitorStats", nullptr, GetMonitorStats, nullptr, nullptr, nullptr, napi_default, nullptr },
        // PixelFormatConverter 测试接口
        { "getGLInternalFormat", nullptr, GetGLInternalFormat, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getGLFormat", nullptr, GetGLFormat, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getBytesPerPixel", nullptr, GetBytesPerPixel, nullptr, nullptr, nullptr, napi_default, nullptr },
        // RenderQueue 测试接口
        { "createRenderQueue", nullptr, CreateRenderQueue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyRenderQueue", nullptr, DestroyRenderQueue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "queueSubmit", nullptr, QueueSubmit, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "queueDequeue", nullptr, QueueDequeue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "queueStop", nullptr, QueueStop, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getQueueInfo", nullptr, GetQueueInfo, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "queuePeek", nullptr, QueuePeek, nullptr, nullptr, nullptr, napi_default, nullptr },
        // TextureManager 测试接口
        { "createTextureManager", nullptr, CreateTextureManager, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyTextureManager", nullptr, DestroyTextureManager, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "textureCreate", nullptr, TextureCreate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "textureUpdate", nullptr, TextureUpdate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "textureDestroy", nullptr, TextureDestroy, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getTextureId", nullptr, GetTextureId, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isTextureCreated", nullptr, IsTextureCreated, nullptr, nullptr, nullptr, napi_default, nullptr },
        // EGLContextManager 测试接口
        { "createEGLContext", nullptr, CreateEGLContext, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyEGLContext", nullptr, DestroyEGLContext, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "eglMakeCurrent", nullptr, EGLMakeCurrent, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isEGLInitialized", nullptr, IsEGLInitialized, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    if (napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "napi_define_properties failed");
        return nullptr;
    }

    return exports;
}
EXTERN_C_END

static napi_module nativerenderModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "nativerender",
    .nm_priv = ((void*)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void RegisterModule(void) {
    napi_module_register(&nativerenderModule);
}
} // namespace NativeXComponentSample
