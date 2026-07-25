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

#include "PoolTextureStrategy.h"
#include <hilog/log.h>
#include "../../../common/common.h"

namespace NativeXComponentSample {

PoolTextureStrategy::PoolTextureStrategy(size_t maxPoolSize)
    : m_pool(std::make_unique<TexturePool>(maxPoolSize)) {
}

PoolTextureStrategy::~PoolTextureStrategy() {
    Clear();
}

TextureManager* PoolTextureStrategy::Acquire(int32_t width, int32_t height,
                                            GLint internalFormat, GLenum format) {
    return m_pool->Acquire(width, height, internalFormat, format);
}

void PoolTextureStrategy::Release(TextureManager* texture) {
    m_pool->Release(texture);
}

bool PoolTextureStrategy::Preallocate(int32_t width, int32_t height,
                                     GLint internalFormat, GLenum format) {
    auto texture = m_pool->Acquire(width, height, internalFormat, format);
    if (texture) {
        m_pool->Release(texture);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
            "PoolStrategy", "Preallocated texture %dx%d", width, height);
        return true;
    }
    return false;
}

void PoolTextureStrategy::Clear() {
    m_pool->Clear();
}

} // namespace NativeXComponentSample
