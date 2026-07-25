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

#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <cstddef>
#include <memory>
#include <mutex>

#include "../backend/IRenderBackend.h"

namespace NativeXComponentSample {

/**
 * 渲染器（异步架构 + 零拷贝 + 最小功能集）
 * 
 * 🎯 设计目标：
 * - API 简洁，只保留核心功能
 * - 内部 100% 异步，主线程不阻塞
 * - 零拷贝设计（直接使用用户 ArrayBuffer）
 * - 线程安全
 */
class Renderer {
public:
    /**
     * 构造函数
     * @param width 初始宽度
     * @param height 初始高度
     * @param format 像素格式
     */
    Renderer(int32_t width, int32_t height, PixelFormat format);
    
    ~Renderer();

    /**
     * 初始化渲染器（关联 NativeWindow）
     * @param nativeWindow 原生窗口指针
     * @return true 成功，false 失败
     */
    bool Initialize(void* nativeWindow);

    /**
     * 渲染一帧（异步！主线程不阻塞！）
     * 
     * ⭐ 零拷贝设计！直接使用用户传入的指针
     * ⭐ 用户保证指针在返回前不修改/不释放
     * ⭐ 调用方通过 NAPI 异步工作线程等待真实完成
     * 
     * @param pixelData 像素数据指针
     * @param dataSize 数据大小
     * @param width 宽度
     * @param height 高度
     * @return true 成功提交，false 失败
     */
    bool RenderFrame(const void* pixelData, size_t dataSize, int32_t width, int32_t height);

    bool RenderFrameRegions(const void* pixelData, size_t dataSize,
                            int32_t frameWidth, int32_t frameHeight,
                            const DirtyRect* regions, int32_t regionCount,
                            bool swapBuffers);

    bool UpdateDirtyRegions(const void* pixelData, size_t dataSize,
                            int32_t frameWidth, int32_t frameHeight,
                            const DirtyRect* regions, int32_t regionCount);

    bool PresentFrame();

    /**
     * 调整大小
     * @param width 新宽度
     * @param height 新高度
     * @return true 成功，false 失败
     */
    bool Resize(int32_t width, int32_t height);

    /**
     * 销毁渲染器
     */
    void Destroy();

    /**
     * 检查是否已初始化
     */
    bool IsInitialized() const;

private:
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // ========================================
    // 成员变量
    // ========================================
    
    // 渲染后端
    std::unique_ptr<IRenderBackend> m_backend;
    
    // 渲染互斥锁（保证单槽 ownership transfer）
    std::mutex m_renderMutex;
    
    // 渲染尺寸
    int32_t m_width;
    int32_t m_height;
    PixelFormat m_format;
};

} // namespace NativeXComponentSample

#endif // RENDERER_H
