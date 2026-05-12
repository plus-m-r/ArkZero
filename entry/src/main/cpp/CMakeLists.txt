# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(XComponent)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DOHOS_PLATFORM)

include_directories(
    ${NATIVERENDER_ROOT_PATH}
    ${NATIVERENDER_ROOT_PATH}/include
)

add_library(nativerender SHARED
    # 分层架构文件（单一职责拆分）
    renderer/backend/EGLContextManager.cpp
    renderer/backend/TextureManager.cpp
    renderer/backend/PixelFormatConverter.cpp
    renderer/backend/YUVShaderManager.cpp
    renderer/backend/TexturePool.cpp
    renderer/backend/GLESBackend.cpp
    renderer/core/Renderer.cpp
    renderer/core/RenderQueue.cpp
    renderer/core/PerformanceMonitor.cpp
    renderer/manager/RendererManager.cpp
    renderer/manager/SurfaceManager.cpp
    renderer/api/RendererApi.cpp
    napi_init.cpp
)

find_library(
    # Sets the name of the path variable.
    EGL-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    EGL
)

find_library(
    # Sets the name of the path variable.
    GLES-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    GLESv3
)

find_library(
    # Sets the name of the path variable.
    hilog-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    hilog_ndk.z
)

find_library(
    # Sets the name of the path variable.
    libace-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    ace_ndk.z
)

find_library(
    # Sets the name of the path variable.
    libnapi-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    ace_napi.z
)

find_library(
    # Sets the name of the path variable.
    libuv-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    uv
)

find_library(
    # Sets the name of the path variable.
    libnative-window-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    native_window
)

target_link_libraries(nativerender PUBLIC
    ${EGL-lib} ${GLES-lib} ${hilog-lib} ${libace-lib} ${libnapi-lib} ${libuv-lib} ${libnative-window-lib})