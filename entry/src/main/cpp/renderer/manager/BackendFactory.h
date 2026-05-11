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

#ifndef BACKEND_FACTORY_H
#define BACKEND_FACTORY_H

#include <memory>
#include "IRenderBackend.h"

namespace NativeXComponentSample {

/**
 * 渲染后端工厂
 * 
 * 🎯 职责：
 * - 根据检测结果创建合适的后端
 * - 封装后端选择逻辑
 * - 返回统一接口
 */
class BackendFactory {
public:
    /**
     * 创建最佳后端
     * 
     * 优先级：OpenGL ES > CPU 软渲染
     * （Vulkan 暂不支持）
     * 
     * @return 后端实例，失败返回 nullptr
     */
    static std::unique_ptr<IRenderBackend> CreateBestBackend();

private:
    static std::unique_ptr<IRenderBackend> TryCreateGLES();
    static std::unique_ptr<IRenderBackend> CreateSoftware();
};

} // namespace NativeXComponentSample

#endif // BACKEND_FACTORY_H
