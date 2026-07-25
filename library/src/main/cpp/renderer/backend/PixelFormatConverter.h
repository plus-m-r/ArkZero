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

#ifndef PIXEL_FORMAT_CONVERTER_H
#define PIXEL_FORMAT_CONVERTER_H

#include <GLES3/gl3.h>
#include "../../common/common.h"

namespace NativeXComponentSample {

/**
 * 像素格式转换器
 * 
 * 职责：
 * - PixelFormat → GL 内部格式映射
 * - PixelFormat → GL 像素格式映射
 * - 计算每像素字节数
 * 
 * 设计原则：单一职责 - 仅负责格式转换，无状态
 */
class PixelFormatConverter {
public:
    /**
     * 获取 OpenGL 内部格式
     * @param format 像素格式
     * @return GL 内部格式（GL_RGBA, GL_RGB 等）
     */
    static GLint GetGLInternalFormat(PixelFormat format);

    /**
     * 获取 OpenGL 像素格式
     * @param format 像素格式
     * @return GL 像素格式（GL_RGBA, GL_RGB, GL_BGRA_EXT 等）
     */
    static GLenum GetGLFormat(PixelFormat format);

    /**
     * 获取每像素字节数
     * @param format 像素格式
     * @return 字节数（RGBA=4, RGB=3, NV21/NV12=1.5）
     */
    static int GetBytesPerPixel(PixelFormat format);
};

} // namespace NativeXComponentSample

#endif // PIXEL_FORMAT_CONVERTER_H
