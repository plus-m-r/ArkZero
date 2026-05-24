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

#ifndef RENDER_FRAME_COMMAND_H
#define RENDER_FRAME_COMMAND_H

#include "IRenderCommand.h"
#include "../../backend/GLESBackend.h"
#include <cstdint>
#include <cstddef>

namespace NativeXComponentSample {

/**
 * 渲染帧命令
 * 
 * 🎯 职责：封装单帧渲染操作
 * 包含像素数据、尺寸等信息，可在任意时刻执行
 */
class RenderFrameCommand : public IRenderCommand {
public:
    /**
     * 构造函数
     * @param backend 渲染后端
     * @param pixelData 像素数据指针（命令拥有所有权）
     * @param dataSize 数据大小
     * @param width 宽度
     * @param height 高度
     */
    RenderFrameCommand(
        GLESBackend* backend,
        const void* pixelData,
        size_t dataSize,
        int32_t width,
        int32_t height
    );
    
    ~RenderFrameCommand() override;
    
    bool Execute() override;
    const char* GetName() const override { return "RenderFrameCommand"; }
    int GetPriority() const override { return 1; }  // 高优先级
    
private:
    GLESBackend* m_backend;
    const void* m_pixelData;
    size_t m_dataSize;
    int32_t m_width;
    int32_t m_height;
};

} // namespace NativeXComponentSample

#endif // RENDER_FRAME_COMMAND_H
