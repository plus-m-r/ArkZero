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

#ifndef DIRECT_TEXTURE_STRATEGY_H
#define DIRECT_TEXTURE_STRATEGY_H

#include "ITextureStrategy.h"
#include <vector>
#include <memory>

namespace NativeXComponentSample {

/**
 * 直接创建纹理策略
 * 
 * ✅ 优点：内存占用低
 * ❌ 缺点：每次 resize 都要重新创建
 */
class DirectTextureStrategy : public ITextureStrategy {
public:
    DirectTextureStrategy();
    ~DirectTextureStrategy() override;
    
    TextureManager* Acquire(int32_t width, int32_t height,
                           GLint internalFormat, GLenum format) override;
    
    void Release(TextureManager* texture) override;
    
    bool Preallocate(int32_t width, int32_t height,
                    GLint internalFormat, GLenum format) override;
    
    void Clear() override;
    
    const char* GetName() const override { return "DirectStrategy"; }
    
private:
    std::unique_ptr<TextureManager> m_currentTexture;
    std::vector<std::unique_ptr<TextureManager>> m_allocatedTextures;
};

} // namespace NativeXComponentSample

#endif // DIRECT_TEXTURE_STRATEGY_H
