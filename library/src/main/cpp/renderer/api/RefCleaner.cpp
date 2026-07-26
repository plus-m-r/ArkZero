#include "RefCleaner.h"
#include <hilog/log.h>

namespace NativeXComponentSample {

RefCleaner& RefCleaner::Instance() {
    static RefCleaner instance;
    return instance;
}

RefCleaner::~RefCleaner() {
    Shutdown();
}

void RefCleaner::Initialize(napi_env env) {
    if (m_initialized.load()) {
        return;
    }

    napi_value resourceName;
    napi_create_string_utf8(env, "RefCleaner", NAPI_AUTO_LENGTH, &resourceName);

    napi_status status = napi_create_threadsafe_function(
        env,
        nullptr,
        nullptr,
        resourceName,
        0,
        1,
        nullptr,
        nullptr,
        nullptr,
        CallJs,
        &m_tsfn);

    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_FATAL, 0x0001,
            "ArkZeroRenderer", "[REF-CLEANER] napi_create_threadsafe_function FAILED: %{public}d", static_cast<int>(status));
        return;
    }

    m_initialized.store(true);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[REF-CLEANER] Initialized");
}

void RefCleaner::Shutdown() {
    if (!m_initialized.load()) {
        return;
    }

    if (m_tsfn != nullptr) {
        napi_release_threadsafe_function(m_tsfn, napi_tsfn_abort);
        m_tsfn = nullptr;
    }

    m_initialized.store(false);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0x0001,
        "ArkZeroRenderer", "[REF-CLEANER] Shutdown");
}

void RefCleaner::ScheduleDelete(napi_env env, napi_ref ref) {
    if (!m_initialized.load() || m_tsfn == nullptr || ref == nullptr) {
        if (ref != nullptr) {
            napi_delete_reference(env, ref);
        }
        return;
    }

    napi_status status = napi_acquire_threadsafe_function(m_tsfn);
    if (status != napi_ok) {
        napi_delete_reference(env, ref);
        return;
    }

    status = napi_call_threadsafe_function(m_tsfn, ref, napi_tsfn_nonblocking);

    if (status != napi_ok) {
        napi_delete_reference(env, ref);
    }

    napi_release_threadsafe_function(m_tsfn, napi_tsfn_release);
}

void RefCleaner::CallJs(napi_env env, napi_value jsCb, void* context, void* data) {
    if (env == nullptr || data == nullptr) {
        return;
    }

    napi_ref ref = reinterpret_cast<napi_ref>(data);
    napi_delete_reference(env, ref);
}

} // namespace NativeXComponentSample
