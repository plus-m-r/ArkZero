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

#ifndef POOL_TEXTURE_STRATEGY_H
#define POOL_TEXTURE_STRATEGY_H

#include "ITextureStrategy.h"
#include "../TexturePool.h"
#include <memory>

namespace NativeXComponentSample {

/**
 * 池化纹理策略
 * 
 * ✅ 优点：高性能，适合频繁 resize
 * ❌ 缺点：内存占用较高
 */
class PoolTextureStrategy : public ITextureStrategy {
public:
    explicit PoolTextureStrategy(size_t maxPoolSize = 10);
    ~PoolTextureStrategy() override;
    
    TextureManager* Acquire(int32_t width, int32_t height,
                           GLint internalFormat, GLenum format) override;
    
    void Release(TextureManager* texture) override;
    
    bool Preallocate(int32_t width, int32_t height,
                    GLint internalFormat, GLenum format) override;
    
    void Clear() override;
    
    const char* GetName() const override { return "PoolStrategy"; }
    
private:
    std::unique_ptr<TexturePool> m_pool;
};

} // namespace NativeXComponentSample

#endif // POOL_TEXTURE_STRATEGY_H
