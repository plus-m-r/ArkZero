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

#include <hilog/log.h>
#include <ace/xcomponent/native_interface_xcomponent.h>

#include "common/common.h"
#include "renderer/api/RendererApi.h"
#include "renderer/manager/SurfaceManager.h"
#include "napi_bridge/renderer_manager_napi.h"
#include "napi_bridge/surface_manager_napi.h"
#include "napi_bridge/egl_context_manager_napi.h"
#include "napi_bridge/gles_backend_napi.h"
#include "napi_bridge/texture_manager_napi.h"
#include "napi_bridge/texture_pool_napi.h"
#include "napi_bridge/pixel_format_converter_napi.h"
#include "napi_bridge/yuv_shader_manager_napi.h"
#include "napi_bridge/performance_monitor_napi.h"

namespace NativeXComponentSample {

static OH_NativeXComponent* g_nativeXComponent = nullptr;
static std::string g_xcomponentId = "";

static void OnSurfaceCreatedCB(OH_NativeXComponent* component, void* window) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "XComponent", "OnSurfaceCreated: window=%p", window);
    if (window) {
        SurfaceManager::GetInstance().StoreNativeWindow(g_xcomponentId, window);
    }
}

static void OnSurfaceChangedCB(OH_NativeXComponent* component, void* window) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "XComponent", "OnSurfaceChanged: window=%p", window);
    if (window) {
        SurfaceManager::GetInstance().StoreNativeWindow(g_xcomponentId, window);
    }
}

static void OnSurfaceDestroyedCB(OH_NativeXComponent* component, void* window) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "XComponent", "OnSurfaceDestroyed");
    SurfaceManager::GetInstance().RemoveNativeWindow(g_xcomponentId);
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "Init", "Init begins");
    if ((env == nullptr) || (exports == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "env or exports is null");
        return nullptr;
    }

    napi_property_descriptor desc[] = {
        // Renderer API (核心渲染接口)
        { "create", nullptr, CreateRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "renderFrame", nullptr, RenderFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "renderFrameRegions", nullptr, RenderFrameRegions, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "updateDirtyRegions", nullptr, UpdateDirtyRegions, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "renderTileRegions", nullptr, RenderTileRegions, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "presentFrame", nullptr, PresentFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "resize", nullptr, ResizeRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroy", nullptr, DestroyRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setVSync", nullptr, SetVSync, nullptr, nullptr, nullptr, napi_default, nullptr },
        // RendererManager API
        { "managerCreateSurfaceRenderer", nullptr, ManagerCreateSurfaceRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "managerDestroyRenderer", nullptr, ManagerDestroyRenderer, nullptr, nullptr, nullptr, napi_default, nullptr },
        // SurfaceManager API
        { "surfaceManagerCreateNativeWindow", nullptr, SurfaceManagerCreateNativeWindow, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "surfaceManagerDestroyNativeWindow", nullptr, SurfaceManagerDestroyNativeWindow, nullptr, nullptr, nullptr, napi_default, nullptr },
        // EGLContextManager API
        { "createEGLContext", nullptr, CreateEGLContext, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyEGLContext", nullptr, DestroyEGLContext, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "eglMakeCurrent", nullptr, EGLMakeCurrent, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isEGLInitialized", nullptr, IsEGLInitialized, nullptr, nullptr, nullptr, napi_default, nullptr },
        // GLESBackend API
        { "createGLESBackend", nullptr, CreateGLESBackend, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyGLESBackend", nullptr, DestroyGLESBackend, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "glesBackendInitialize", nullptr, GLESBackendInitialize, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "glesBackendRenderFrame", nullptr, GLESBackendRenderFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isGLESBackendInitialized", nullptr, IsGLESBackendInitialized, nullptr, nullptr, nullptr, napi_default, nullptr },
        // TextureManager API
        { "createTextureManager", nullptr, CreateTextureManager, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyTextureManager", nullptr, DestroyTextureManager, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "textureCreate", nullptr, TextureCreate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "textureUpdate", nullptr, TextureUpdate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "textureDestroy", nullptr, TextureDestroy, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getTextureId", nullptr, GetTextureId, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isTextureCreated", nullptr, IsTextureCreated, nullptr, nullptr, nullptr, napi_default, nullptr },
        // TexturePool API
        { "createTexturePool", nullptr, CreateTexturePool, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyTexturePool", nullptr, DestroyTexturePool, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "texturePoolAcquire", nullptr, TexturePoolAcquire, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "texturePoolRelease", nullptr, TexturePoolRelease, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "texturePoolPreallocate", nullptr, TexturePoolPreallocate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "texturePoolClear", nullptr, TexturePoolClear, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "texturePoolSize", nullptr, TexturePoolSize, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "texturePoolGetStats", nullptr, TexturePoolGetStats, nullptr, nullptr, nullptr, napi_default, nullptr },
        // PixelFormatConverter API
        { "getGLInternalFormat", nullptr, GetGLInternalFormat, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getGLFormat", nullptr, GetGLFormat, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getBytesPerPixel", nullptr, GetBytesPerPixel, nullptr, nullptr, nullptr, napi_default, nullptr },
        // YUVShaderManager API
        { "createYUVShaderManager", nullptr, CreateYUVShaderManager, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyYUVShaderManager", nullptr, DestroyYUVShaderManager, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "yuvShaderInitialize", nullptr, YUVShaderInitialize, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "yuvShaderRenderNV21", nullptr, YUVShaderRenderNV21, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "yuvShaderRenderNV12", nullptr, YUVShaderRenderNV12, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isYUVShaderInitialized", nullptr, IsYUVShaderInitialized, nullptr, nullptr, nullptr, napi_default, nullptr },
        // PerformanceMonitor API
        { "createPerformanceMonitor", nullptr, CreatePerformanceMonitor, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroyPerformanceMonitor", nullptr, DestroyPerformanceMonitor, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "monitorBeginFrame", nullptr, MonitorBeginFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "monitorEndFrame", nullptr, MonitorEndFrame, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "monitorReset", nullptr, MonitorReset, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getMonitorStats", nullptr, GetMonitorStats, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    
    if (napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "napi_define_properties failed");
        return nullptr;
    }

    // Extract OH_NativeXComponent from exports (when libraryname is set on XComponent)
    napi_value exportInstance = nullptr;
    napi_status status = napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &exportInstance);
    if (status == napi_ok && exportInstance != nullptr) {
        status = napi_unwrap(env, exportInstance, reinterpret_cast<void**>(&g_nativeXComponent));
        if (status == napi_ok && g_nativeXComponent != nullptr) {
            char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
            uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
            OH_NativeXComponent_GetXComponentId(g_nativeXComponent, idStr, &idSize);
            g_xcomponentId = std::string(idStr);
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                "Init", "XComponent id: %{public}s", g_xcomponentId.c_str());

            OH_NativeXComponent_Callback callback;
            callback.OnSurfaceCreated = OnSurfaceCreatedCB;
            callback.OnSurfaceChanged = OnSurfaceChangedCB;
            callback.OnSurfaceDestroyed = OnSurfaceDestroyedCB;
            callback.DispatchTouchEvent = nullptr;
            OH_NativeXComponent_RegisterCallback(g_nativeXComponent, &callback);
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                "Init", "XComponent callbacks registered");
        }
    } else {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN,
            "Init", "No OH_NATIVE_XCOMPONENT_OBJ found - using surfaceId fallback");
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
