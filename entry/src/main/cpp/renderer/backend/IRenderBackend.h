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

#ifndef I_RENDER_BACKEND_H
#define I_RENDER_BACKEND_H

#include <cstdint>
#include <cstddef>
#include "../../common/common.h"

namespace NativeXComponentSample {

/**
 * 渲染后端接口
 * 
 * 🎯 设计原则：
 * - 所有渲染后端必须实现此接口
 * - 上层代码只依赖接口，不依赖具体实现
 * - 符合开闭原则（OCP）和依赖倒置原则（DIP）
 */
class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    /**
     * 初始化后端
     * @param nativeWindow NativeWindow 指针（来自 XComponent Surface）
     * @param width 宽度
     * @param height 高度
     * @param format 像素格式
     * @return true 成功，false 失败
     */
    virtual bool Initialize(void* nativeWindow, int32_t width, int32_t height, PixelFormat format) = 0;

    /**
     * 渲染帧
     * @param pixelData 像素数据指针
     * @param dataSize 数据大小
     * @param width 宽度
     * @param height 高度
     * @return true 成功，false 失败
     */
    virtual bool RenderFrame(const void* pixelData, size_t dataSize, 
                            int32_t width, int32_t height) = 0;

    /**
     * 调整尺寸
     * @param width 新宽度
     * @param height 新高度
     * @return true 成功，false 失败
     */
    virtual bool Resize(int32_t width, int32_t height) = 0;

    /**
     * 销毁后端，释放资源
     */
    virtual void Destroy() = 0;

    /**
     * 获取后端类型名称
     * @return 后端类型字符串
     */
    virtual const char* GetBackendName() const = 0;

    /**
     * 检查是否已初始化
     * @return true 已初始化
     */
    virtual bool IsInitialized() const = 0;
};

} // namespace NativeXComponentSample

#endif // I_RENDER_BACKEND_H
