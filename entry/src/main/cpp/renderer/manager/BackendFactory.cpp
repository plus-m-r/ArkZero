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

#include "BackendFactory.h"
#include "../backend/GLESBackend.h"
#include <hilog/log.h>

#include "../../common/common.h"

namespace NativeXComponentSample {

std::unique_ptr<IRenderBackend> BackendFactory::CreateBestBackend() {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "BackendFactory", "🔍 Detecting best render backend...");
    
    // 1. 尝试 OpenGL ES（当前唯一支持的硬件加速后端）
    auto glesBackend = TryCreateGLES();
    if (glesBackend) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "BackendFactory", "✅ Using OpenGL ES backend");
        return glesBackend;
    }
    
    // 2. 兜底：CPU 软渲染（TODO: 实现）
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
        "BackendFactory", "⚠️ Fallback to CPU software rendering (not implemented yet)");
    return CreateSoftware();
}

std::unique_ptr<IRenderBackend> BackendFactory::TryCreateGLES() {
    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
        "BackendFactory", "Trying to create OpenGL ES backend...");
    
    // 检查 EGL 支持
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "BackendFactory", "EGL not available");
        return nullptr;
    }
    
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
            "BackendFactory", "Failed to initialize EGL");
        return nullptr;
    }
    
    // 检查 OpenGL ES 版本
    const char* version = eglQueryString(display, EGL_VERSION);
    if (version && (strstr(version, "OpenGL ES 3.") || strstr(version, "OpenGL ES 2."))) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
            "BackendFactory", "✅ OpenGL ES detected: %s", version);
        
        eglTerminate(display);
        return std::make_unique<GLESBackend>();
    }
    
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
        "BackendFactory", "OpenGL ES not supported: %s", version ? version : "unknown");
    eglTerminate(display);
    return nullptr;
}

std::unique_ptr<IRenderBackend> BackendFactory::CreateSoftware() {
    // TODO: 实现 CPU 软渲染后端
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
        "BackendFactory", "❌ No render backend available");
    return nullptr;
}

} // namespace NativeXComponentSample
