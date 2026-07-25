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

#include "PreallocateTextureCommand.h"
#include <hilog/log.h>
#include "../../../common/common.h"

namespace NativeXComponentSample {

PreallocateTextureCommand::PreallocateTextureCommand(
    GLESBackend* backend,
    const std::vector<std::pair<int32_t, int32_t>>& resolutions)
    : m_backend(backend)
    , m_resolutions(resolutions) {
}

bool PreallocateTextureCommand::Execute() {
    if (!m_backend) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "PreallocateTextureCommand", "Backend is null");
        return false;
    }
    
    if (m_resolutions.empty()) {
        OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN,
            "PreallocateTextureCommand", "No resolutions to preallocate");
        return true;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "PreallocateTextureCommand", 
        "📦 Preallocating %zu textures...", m_resolutions.size());
    
    // ⭐ 注意：当前 GLESBackend 没有直接暴露策略接口
    // 这里只是记录日志，实际预分配在 Initialize 时完成
    // 未来可以通过扩展 GLESBackend API 来支持运行时预分配
    
    for (const auto& [width, height] : m_resolutions) {
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN,
            "PreallocateTextureCommand", 
            "  - Requested resolution: %dx%d", width, height);
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "PreallocateTextureCommand", 
        "✅ Preallocation request logged (%zu resolutions)", m_resolutions.size());
    
    return true;
}

} // namespace NativeXComponentSample
