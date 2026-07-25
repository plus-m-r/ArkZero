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
    # 最小功能源文件
    renderer/backend/EGLContextManager.cpp
    renderer/backend/TextureManager.cpp
    renderer/backend/TextureShader.cpp
    renderer/backend/PixelFormatConverter.cpp
    renderer/backend/YUVShaderManager.cpp
    renderer/backend/TexturePool.cpp
    renderer/backend/GLESBackend.cpp
    # 策略模式文件
    renderer/backend/strategy/PoolTextureStrategy.cpp
    renderer/backend/strategy/DirectTextureStrategy.cpp
    # 工厂模式文件
    renderer/backend/factory/TextureFactory.cpp
    # 渲染命令文件
    renderer/core/command/RenderFrameCommand.cpp
    renderer/core/command/PreallocateTextureCommand.cpp
    renderer/core/PerformanceMonitor.cpp
    renderer/core/RenderThread.cpp
    renderer/core/Renderer.cpp
    renderer/manager/RendererManager.cpp
    renderer/manager/SurfaceManager.cpp
    renderer/api/RendererApi.cpp
    # NAPI 桥接（核心接口）
    napi_bridge/renderer_manager_napi.cpp
    napi_bridge/surface_manager_napi.cpp
    # NAPI 桥接（扩展接口）
    napi_bridge/egl_context_manager_napi.cpp
    napi_bridge/gles_backend_napi.cpp
    napi_bridge/texture_manager_napi.cpp
    napi_bridge/texture_pool_napi.cpp
    napi_bridge/pixel_format_converter_napi.cpp
    napi_bridge/yuv_shader_manager_napi.cpp
    napi_bridge/performance_monitor_napi.cpp
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

# 🧪 注意：C++ 单元测试通过 Hypium + NAPI 桥接在 Instrument Test 中进行
# 不需要在此处定义 add_executable，所有测试都在 entry/src/ohosTest/ets/test/native/ 目录下