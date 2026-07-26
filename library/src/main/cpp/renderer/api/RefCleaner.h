#ifndef REF_CLEANER_H
#define REF_CLEANER_H

#include <napi/native_api.h>
#include <mutex>
#include <vector>
#include <atomic>

namespace NativeXComponentSample {

class RefCleaner {
public:
    static RefCleaner& Instance();

    void Initialize(napi_env env);
    void Shutdown();

    void ScheduleDelete(napi_env env, napi_ref ref);

private:
    RefCleaner() = default;
    ~RefCleaner();

    RefCleaner(const RefCleaner&) = delete;
    RefCleaner& operator=(const RefCleaner&) = delete;

    static void CallJs(napi_env env, napi_value jsCb, void* context, void* data);

    napi_threadsafe_function m_tsfn = nullptr;
    std::atomic<bool> m_initialized{false};
};

} // namespace NativeXComponentSample

#endif // REF_CLEANER_H
