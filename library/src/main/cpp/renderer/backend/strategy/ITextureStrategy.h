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

#ifndef I_TEXTURE_STRATEGY_H
#define I_TEXTURE_STRATEGY_H

#include "../TextureManager.h"
#include <cstdint>

namespace NativeXComponentSample {

/**
 * 纹理管理策略接口
 * 
 * 🎯 设计模式：Strategy Pattern
 * 允许在运行时切换不同的纹理管理策略
 */
class ITextureStrategy {
public:
    virtual ~ITextureStrategy() = default;
    
    /**
     * 获取或创建纹理
     */
    virtual TextureManager* Acquire(int32_t width, int32_t height,
                                   GLint internalFormat, GLenum format) = 0;
    
    /**
     * 归还纹理
     */
    virtual void Release(TextureManager* texture) = 0;
    
    /**
     * 预分配纹理
     */
    virtual bool Preallocate(int32_t width, int32_t height,
                            GLint internalFormat, GLenum format) = 0;
    
    /**
     * 清空所有纹理
     */
    virtual void Clear() = 0;
    
    /**
     * 获取策略名称（用于日志）
     */
    virtual const char* GetName() const = 0;
};

} // namespace NativeXComponentSample

#endif // I_TEXTURE_STRATEGY_H
