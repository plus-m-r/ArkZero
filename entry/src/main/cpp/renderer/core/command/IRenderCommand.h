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

#ifndef I_RENDER_COMMAND_H
#define I_RENDER_COMMAND_H

#include <cstdint>
#include <cstddef>

namespace NativeXComponentSample {

/**
 * 渲染命令接口
 * 
 * 🎯 设计模式：Command Pattern
 * 将所有渲染操作封装为命令对象，支持：
 * - 异步执行
 * - 命令队列
 * - 撤销/重做（可选）
 * - 批量处理
 */
class IRenderCommand {
public:
    virtual ~IRenderCommand() = default;
    
    /**
     * 执行命令
     * @return true 成功，false 失败
     */
    virtual bool Execute() = 0;
    
    /**
     * 获取命令名称（用于调试和日志）
     */
    virtual const char* GetName() const = 0;
    
    /**
     * 获取命令优先级（数字越小优先级越高）
     */
    virtual int GetPriority() const { return 0; }
};

} // namespace NativeXComponentSample

#endif // I_RENDER_COMMAND_H
