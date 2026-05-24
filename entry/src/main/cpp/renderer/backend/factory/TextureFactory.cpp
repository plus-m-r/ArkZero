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

#include "TextureFactory.h"
#include <hilog/log.h>
#include "../../../common/common.h"

namespace NativeXComponentSample {

std::unique_ptr<TextureManager> TextureFactory::Create(
    int32_t width,
    int32_t height,
    GLint internalFormat,
    GLenum format) {
    
    auto texture = std::make_unique<TextureManager>();
    
    if (!texture->Create(width, height, internalFormat, format)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
            "TextureFactory", "Failed to create texture %dx%d", width, height);
        return nullptr;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "TextureFactory", "✅ Created texture %dx%d (id=%d)",
        width, height, texture->GetTextureId());
    
    return texture;
}

std::vector<std::unique_ptr<TextureManager>> TextureFactory::CreateBatch(
    const std::vector<std::pair<int32_t, int32_t>>& resolutions,
    GLint internalFormat,
    GLenum format) {
    
    std::vector<std::unique_ptr<TextureManager>> textures;
    textures.reserve(resolutions.size());
    
    for (const auto& [width, height] : resolutions) {
        auto texture = Create(width, height, internalFormat, format);
        if (texture) {
            textures.push_back(std::move(texture));
        }
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
        "TextureFactory", "✅ Batch created %zu textures", textures.size());
    
    return textures;
}

} // namespace NativeXComponentSample
