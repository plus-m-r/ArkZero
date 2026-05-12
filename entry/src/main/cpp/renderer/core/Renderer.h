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
#include <memory>
#include "../backend/IRenderBackend.h"

namespace NativeXComponentSample {

/**
 * Renderer - 外观类（Facade）
 * 
 * 🎯 职责：
 * - 提供统一的渲染接口
 * - 委托给具体的后端实现
 * - 不包含任何渲染逻辑
 * 
 * 📊 架构：
 * ArkTS → NAPI → Renderer → IRenderBackend → GLES/Vulkan/Software
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
    
    /**
     * 析构函数
     */
    ~Renderer();

    /**
     * 初始化渲染器（使用 XComponent Surface）
     * @param nativeWindow NativeWindow 指针
     * @return true 成功，false 失败
     */
    bool InitializeWithSurface(void* nativeWindow);

    /**
     * 渲染帧
     * @param pixelData 像素数据指针
     * @param dataSize 数据大小
     * @param width 宽度
     * @param height 高度
     * @return true 成功，false 失败
     */
    bool RenderFrame(const void* pixelData, size_t dataSize, 
                    int32_t width, int32_t height);

    /**
     * 调整尺寸
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
     * @return true 已初始化
     */
    bool IsInitialized() const;

    /**
     * 获取当前后端名称
     * @return 后端名称字符串
     */
    const char* GetBackendName() const;

private:
    // 禁止拷贝
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

private:
    std::unique_ptr<IRenderBackend> m_backend; // ⭐ 多态后端
    int32_t m_width;
    int32_t m_height;
    PixelFormat m_format;
};

} // namespace NativeXComponentSample

#endif // RENDERER_H
