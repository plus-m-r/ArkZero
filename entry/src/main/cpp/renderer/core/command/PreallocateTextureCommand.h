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

#ifndef PREALLOCATE_TEXTURE_COMMAND_H
#define PREALLOCATE_TEXTURE_COMMAND_H

#include "IRenderCommand.h"
#include "../../backend/GLESBackend.h"
#include <cstdint>
#include <vector>
#include <utility>

namespace NativeXComponentSample {

/**
 * 预分配纹理命令
 * 
 * 🎯 职责：批量预分配常用分辨率的纹理
 * 用于在空闲时预先准备纹理，避免渲染时的延迟
 */
class PreallocateTextureCommand : public IRenderCommand {
public:
    /**
     * 构造函数
     * @param backend 渲染后端
     * @param resolutions 需要预分配的分辨率列表
     */
    PreallocateTextureCommand(
        GLESBackend* backend,
        const std::vector<std::pair<int32_t, int32_t>>& resolutions
    );
    
    ~PreallocateTextureCommand() override = default;
    
    bool Execute() override;
    const char* GetName() const override { return "PreallocateTextureCommand"; }
    int GetPriority() const override { return 10; }  // 低优先级，可在空闲时执行
    
private:
    GLESBackend* m_backend;
    std::vector<std::pair<int32_t, int32_t>> m_resolutions;
};

} // namespace NativeXComponentSample

#endif // PREALLOCATE_TEXTURE_COMMAND_H
