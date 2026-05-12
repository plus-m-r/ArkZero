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

#ifndef TEXTURE_POOL_H
#define TEXTURE_POOL_H

#include "TextureManager.h"
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>

namespace NativeXComponentSample {

/**
 * 纹理池
 * 
 * 🎯 职责：
 * - 预分配常用分辨率纹理
 * - Resize 时直接复用，避免重新创建
 * - 管理纹理生命周期
 * 
 * 设计原则：单一职责 - 仅负责纹理缓存和复用
 */
class TexturePool {
public:
    /**
     * 构造函数
     * @param maxSize 最大池大小（默认 10 个纹理）
     */
    explicit TexturePool(size_t maxSize = 10);
    
    ~TexturePool();

    /**
     * 获取或创建纹理
     * 
     * ⭐ 优先从池中复用相同尺寸的纹理
     * ⭐ 如果池中不存在，创建新纹理
     * 
     * @param width 宽度
     * @param height 高度
     * @param internalFormat GL 内部格式
     * @param format GL 像素格式
     * @return TextureManager 指针，失败返回 nullptr
     */
    TextureManager* Acquire(int32_t width, int32_t height, 
                           GLint internalFormat, GLenum format);

    /**
     * 归还纹理到池中
     * 
     * @param texture 纹理管理器指针
     */
    void Release(TextureManager* texture);

    /**
     * 预分配常用分辨率纹理
     * 
     * ⭐ 提前创建常用尺寸，避免运行时开销
     * 
     * @param resolutions 分辨率列表 [{width, height}, ...]
     * @param internalFormat GL 内部格式
     * @param format GL 像素格式
     */
    void Preallocate(const std::vector<std::pair<int32_t, int32_t>>& resolutions,
                    GLint internalFormat, GLenum format);

    /**
     * 清空池
     */
    void Clear();

    /**
     * 获取池当前大小
     */
    size_t Size() const;

    /**
     * 获取命中率统计
     */
    struct Stats {
        size_t hitCount;   // 命中次数（复用）
        size_t missCount;  // 未命中次数（新建）
        
        float GetHitRate() const {
            if (hitCount + missCount == 0) return 0.0f;
            return static_cast<float>(hitCount) / (hitCount + missCount);
        }
    };
    
    Stats GetStats() const { return m_stats; }

private:
    /**
     * 纹理池条目
     */
    struct PoolEntry {
        std::unique_ptr<TextureManager> texture;
        int32_t width;
        int32_t height;
        bool inUse;
        
        PoolEntry() : width(0), height(0), inUse(false) {}
        
        PoolEntry(std::unique_ptr<TextureManager> tex, int32_t w, int32_t h)
            : texture(std::move(tex)), width(w), height(h), inUse(false) {}
    };

    /**
     * 查找可用的纹理
     */
    TextureManager* FindAvailable(int32_t width, int32_t height);

    /**
     * 创建新纹理并加入池
     */
    TextureManager* CreateAndAdd(int32_t width, int32_t height,
                                GLint internalFormat, GLenum format);

    std::vector<PoolEntry> m_pool;
    size_t m_maxSize;
    mutable std::mutex m_mutex;
    
    Stats m_stats;
};

} // namespace NativeXComponentSample

#endif // TEXTURE_POOL_H
