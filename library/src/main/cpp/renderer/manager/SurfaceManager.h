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

#ifndef SURFACE_MANAGER_H
#define SURFACE_MANAGER_H

#include <string>
#include <mutex>
#include <unordered_map>

namespace NativeXComponentSample {

class SurfaceManager {
public:
    static SurfaceManager& GetInstance();
    
    void* CreateNativeWindow(const std::string& surfaceId);
    void DestroyNativeWindow(void* nativeWindow);

    void StoreNativeWindow(const std::string& id, void* window);
    void* GetStoredNativeWindow(const std::string& id);
    void RemoveNativeWindow(const std::string& id);

private:
    SurfaceManager() = default;
    ~SurfaceManager() = default;
    
    SurfaceManager(const SurfaceManager&) = delete;
    SurfaceManager& operator=(const SurfaceManager&) = delete;

    std::unordered_map<std::string, void*> m_storedWindows;
    std::mutex m_mutex;
};

} // namespace NativeXComponentSample

#endif // SURFACE_MANAGER_H
