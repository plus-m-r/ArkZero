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

#include "performance_monitor_napi.h"
#include "../renderer/core/PerformanceMonitor.h"
#include <hilog/log.h>
#include <string>

namespace NativeXComponentSample {

// 使用 handle 作为指针存储 PerformanceMonitor 实例
static PerformanceMonitor* GetMonitorFromHandle(int64_t handle) {
    return reinterpret_cast<PerformanceMonitor*>(handle);
}

napi_value CreatePerformanceMonitor(napi_env env, napi_callback_info info) {
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "PerfMonitorNAPI", "CreatePerformanceMonitor called");
    
    try {
        // 创建 PerformanceMonitor 实例
        auto* monitor = new PerformanceMonitor();
        
        // 将指针转换为 int64_t 作为 handle 返回
        int64_t handle = reinterpret_cast<int64_t>(monitor);
        
        napi_value result;
        napi_create_int64(env, handle, &result);
        
        OH_LOG_Print(LOG_APP, LOG_INFO, 0, "PerfMonitorNAPI", 
                     "PerformanceMonitor created, handle=%{public}ld", handle);
        
        return result;
    } catch (const std::exception& e) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", 
                     "Failed to create PerformanceMonitor: %{public}s", e.what());
        return nullptr;
    }
}

napi_value DestroyPerformanceMonitor(napi_env env, napi_callback_info info) {
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "PerfMonitorNAPI", "DestroyPerformanceMonitor called");
    
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* monitor = GetMonitorFromHandle(handle);
    if (monitor) {
        delete monitor;
        OH_LOG_Print(LOG_APP, LOG_INFO, 0, "PerfMonitorNAPI", 
                     "PerformanceMonitor destroyed, handle=%{public}ld", handle);
    }
    
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value MonitorBeginFrame(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* monitor = GetMonitorFromHandle(handle);
    if (monitor) {
        monitor->BeginFrame();
    }
    
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value MonitorEndFrame(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 2) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    bool dropped = false;
    napi_get_value_bool(env, args[1], &dropped);
    
    auto* monitor = GetMonitorFromHandle(handle);
    if (monitor) {
        monitor->EndFrame(dropped);
    }
    
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value MonitorReset(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* monitor = GetMonitorFromHandle(handle);
    if (monitor) {
        monitor->Reset();
        OH_LOG_Print(LOG_APP, LOG_INFO, 0, "PerfMonitorNAPI", 
                     "PerformanceMonitor reset, handle=%{public}ld", handle);
    }
    
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value GetMonitorStats(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", "Invalid arguments");
        return nullptr;
    }
    
    int64_t handle = 0;
    napi_get_value_int64(env, args[0], &handle);
    
    auto* monitor = GetMonitorFromHandle(handle);
    if (!monitor) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, 0, "PerfMonitorNAPI", "Invalid monitor handle");
        return nullptr;
    }
    
    // 获取统计数据
    auto stats = monitor->GetStats();
    
    // 创建 JavaScript 对象
    napi_value result;
    napi_create_object(env, &result);
    
    // 设置属性
    napi_value totalFrames;
    napi_create_int64(env, static_cast<int64_t>(stats.totalFrames), &totalFrames);
    napi_set_named_property(env, result, "totalFrames", totalFrames);
    
    napi_value droppedFrames;
    napi_create_int64(env, static_cast<int64_t>(stats.droppedFrames), &droppedFrames);
    napi_set_named_property(env, result, "droppedFrames", droppedFrames);
    
    napi_value fps;
    napi_create_double(env, static_cast<double>(stats.fps), &fps);
    napi_set_named_property(env, result, "fps", fps);
    
    napi_value dropRate;
    napi_create_double(env, static_cast<double>(stats.dropRate), &dropRate);
    napi_set_named_property(env, result, "dropRate", dropRate);
    
    napi_value frameTimeMs;
    napi_create_double(env, static_cast<double>(stats.frameTimeMs), &frameTimeMs);
    napi_set_named_property(env, result, "frameTimeMs", frameTimeMs);
    
    return result;
}

} // namespace NativeXComponentSample
