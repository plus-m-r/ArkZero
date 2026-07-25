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

#ifndef TEXTURE_FACTORY_H
#define TEXTURE_FACTORY_H

#include "../TextureManager.h"
#include <memory>
#include <vector>
#include <cstdint>

namespace NativeXComponentSample {

/**
 * 纹理工厂
 * 
 * 🎯 设计模式：Factory Pattern
 * 统一负责纹理对象的创建和销毁
 */
class TextureFactory {
public:
    /**
     * 创建单个纹理
     * @param width 宽度
     * @param height 高度
     * @param internalFormat GL 内部格式
     * @param format GL 像素格式
     * @return TextureManager 智能指针，失败返回 nullptr
     */
    static std::unique_ptr<TextureManager> Create(
        int32_t width,
        int32_t height,
        GLint internalFormat,
        GLenum format
    );
    
    /**
     * 批量创建纹理
     * @param resolutions 分辨率列表 [{width, height}, ...]
     * @param internalFormat GL 内部格式
     * @param format GL 像素格式
     * @return TextureManager 智能指针列表
     */
    static std::vector<std::unique_ptr<TextureManager>> CreateBatch(
        const std::vector<std::pair<int32_t, int32_t>>& resolutions,
        GLint internalFormat,
        GLenum format
    );
    
private:
    TextureFactory() = delete;  // 静态工具类，不允许实例化
};

} // namespace NativeXComponentSample

#endif // TEXTURE_FACTORY_H
