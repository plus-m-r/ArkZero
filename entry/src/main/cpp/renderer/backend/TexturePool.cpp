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

#include "TexturePool.h"
#include <hilog/log.h>
#include "../../common/common.h"
#include <algorithm>

namespace NativeXComponentSample {

TexturePool::TexturePool(size_t maxSize)
    : m_maxSize(maxSize), m_stats{0, 0}  // ⭐ 显式初始化统计
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TexturePool", "Created with maxSize=%zu", maxSize);
}

TexturePool::~TexturePool() {
    Clear();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TexturePool", "Destroyed");
}

TextureManager* TexturePool::Acquire(int32_t width, int32_t height, 
                                     GLint internalFormat, GLenum format) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // ⭐ 1. 尝试从池中查找可用的纹理
    TextureManager* texture = FindAvailable(width, height);
    if (texture) {
        m_stats.hitCount++;
        
        OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
            "TexturePool", "✅ Hit: Reusing texture %dx%d (hitRate=%.1f%%)", 
            width, height, m_stats.GetHitRate() * 100);
        
        return texture;
    }
    
    // ⭐ 2. 未命中，创建新纹理
    m_stats.missCount++;
    
    OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
        "TexturePool", "❌ Miss: Creating new texture %dx%d (hitRate=%.1f%%)", 
        width, height, m_stats.GetHitRate() * 100);
    
    return CreateAndAdd(width, height, internalFormat, format);
}

void TexturePool::Release(TextureManager* texture) {
    if (!texture) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 标记为未使用
    for (auto& entry : m_pool) {
        if (entry.texture.get() == texture) {
            entry.inUse = false;
            
            OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
                "TexturePool", "Released texture %dx%d", entry.width, entry.height);
            
            return;
        }
    }
    
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
        "TexturePool", "Texture not found in pool");
}

void TexturePool::Preallocate(const std::vector<std::pair<int32_t, int32_t>>& resolutions,
                              GLint internalFormat, GLenum format) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TexturePool", "Preallocating %zu textures...", resolutions.size());
    
    for (const auto& res : resolutions) {
        int32_t width = res.first;
        int32_t height = res.second;
        
        // 检查是否已存在
        bool exists = false;
        for (const auto& entry : m_pool) {
            if (entry.width == width && entry.height == height) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            auto texture = std::make_unique<TextureManager>();
            if (texture->Create(width, height, internalFormat, format)) {
                m_pool.emplace_back(std::move(texture), width, height);
                
                OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
                    "TexturePool", "✅ Preallocated: %dx%d", width, height);
            } else {
                OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
                    "TexturePool", "Failed to preallocate: %dx%d", width, height);
            }
        }
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TexturePool", "Preallocation complete, pool size=%zu", m_pool.size());
}

void TexturePool::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TexturePool", "Clearing pool (%zu entries)...", m_pool.size());
    
    m_pool.clear();
    m_stats = Stats();
}

size_t TexturePool::Size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pool.size();
}

TextureManager* TexturePool::FindAvailable(int32_t width, int32_t height) {
    // 查找相同尺寸且未使用的纹理
    for (auto& entry : m_pool) {
        if (entry.width == width && entry.height == height && !entry.inUse) {
            entry.inUse = true;
            return entry.texture.get();
        }
    }
    
    return nullptr;
}

TextureManager* TexturePool::CreateAndAdd(int32_t width, int32_t height,
                                          GLint internalFormat, GLenum format) {
    // 如果池已满，移除最旧的未使用纹理
    if (m_pool.size() >= m_maxSize) {
        // 查找未使用的纹理并移除
        auto it = std::find_if(m_pool.begin(), m_pool.end(),
            [](const PoolEntry& entry) { return !entry.inUse; });
        
        if (it != m_pool.end()) {
            OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_PRINT_DOMAIN, 
                "TexturePool", "Pool full, removing old texture %dx%d", 
                it->width, it->height);
            
            m_pool.erase(it);
        } else {
            OH_LOG_Print(LOG_APP, LOG_WARN, LOG_PRINT_DOMAIN, 
                "TexturePool", "Pool full and all textures in use");
        }
    }
    
    // 创建新纹理
    auto texture = std::make_unique<TextureManager>();
    if (!texture->Create(width, height, internalFormat, format)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, 
            "TexturePool", "Failed to create texture %dx%d", width, height);
        return nullptr;
    }
    
    // 加入池
    m_pool.emplace_back(std::move(texture), width, height);
    m_pool.back().inUse = true;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, 
        "TexturePool", "✅ Created and added: %dx%d (pool size=%zu)", 
        width, height, m_pool.size());
    
    return m_pool.back().texture.get();
}

} // namespace NativeXComponentSample
