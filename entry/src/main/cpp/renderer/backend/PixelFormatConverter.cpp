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

#include "PixelFormatConverter.h"

namespace NativeXComponentSample {

GLint PixelFormatConverter::GetGLInternalFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA:
        case PixelFormat::BGRA:
            return GL_RGBA;
        case PixelFormat::RGB:
            return GL_RGB;
        default:
            return GL_RGBA;
    }
}

GLenum PixelFormatConverter::GetGLFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA:
            return GL_RGBA;
        case PixelFormat::RGB:
            return GL_RGB;
        case PixelFormat::BGRA:
#ifdef GL_BGRA_EXT
            return GL_BGRA_EXT;
#else
            return GL_RGBA;  // Fallback
#endif
        default:
            return GL_RGBA;
    }
}

int PixelFormatConverter::GetBytesPerPixel(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA:
        case PixelFormat::BGRA:
            return 4;
        case PixelFormat::RGB:
            return 3;
        default:
            return 4;
    }
}

} // namespace NativeXComponentSample
