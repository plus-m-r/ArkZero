#ifndef NATIVE_XCOMPONENT_RENDERER_API_H
#define NATIVE_XCOMPONENT_RENDERER_API_H

#include <napi/native_api.h>

namespace NativeXComponentSample {

napi_value CreateRenderer(napi_env env, napi_callback_info info);

napi_value RenderFrame(napi_env env, napi_callback_info info);

napi_value RenderFrameRegions(napi_env env, napi_callback_info info);

napi_value UpdateDirtyRegions(napi_env env, napi_callback_info info);

napi_value PresentFrame(napi_env env, napi_callback_info info);

napi_value ResizeRenderer(napi_env env, napi_callback_info info);

napi_value DestroyRenderer(napi_env env, napi_callback_info info);

} // namespace NativeXComponentSample

#endif // NATIVE_XCOMPONENT_RENDERER_API_H
