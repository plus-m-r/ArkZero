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

#include "DirectTextureStrategy.h"
#include <hilog/log.h>
#include "../../../common/common.h"

namespace NativeXComponentSample {

DirectTextureStrategy::DirectTextureStrategy() {
}

DirectTextureStrategy::~DirectTextureStrategy() {
    Clear();
}

TextureManager* DirectTextureStrategy::Acquire(int32_t width, int32_t height,
                                              GLint internalFormat, GLenum format) {
    // 直接创建新纹理
    auto texture = std::make_unique<TextureManager>();
    
    if (!texture->Create(width, height, internalFormat, format)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "DirectStrategy", "Failed to create texture %dx%d", width, height);
        return nullptr;
    }
    
    // 保存引用以便后续管理
    m_currentTexture = std::move(texture);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "DirectStrategy", "✅ Created texture %dx%d (id=%d)",
        width, height, m_currentTexture->GetTextureId());
    
    return m_currentTexture.get();
}

void DirectTextureStrategy::Release(TextureManager* texture) {
    // 直接策略不需要归还，纹理会在 Clear 时统一销毁
    (void)texture;  // 避免未使用参数警告
}

bool DirectTextureStrategy::Preallocate(int32_t width, int32_t height,
                                       GLint internalFormat, GLenum format) {
    // 预分配就是创建一个纹理并保存
    auto texture = std::make_unique<TextureManager>();
    
    if (!texture->Create(width, height, internalFormat, format)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "DirectStrategy", "Failed to preallocate texture %dx%d", width, height);
        return false;
    }
    
    m_allocatedTextures.push_back(std::move(texture));
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "DirectStrategy", "✅ Preallocated texture %dx%d", width, height);
    
    return true;
}

void DirectTextureStrategy::Clear() {
    m_currentTexture.reset();
    m_allocatedTextures.clear();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "DirectStrategy", "♻️ Cleared all textures");
}

} // namespace NativeXComponentSample
