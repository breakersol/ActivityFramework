/*
 * Copyright [2026] [Shuang Zhu / Sol]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TA_UNIQUEID_H
#define TA_UNIQUEID_H

#include <cstdint>

#include "TA_ActivityFramework_global.h"

namespace CoreAsync {

// Identifies one live instance within this process. IDs are not persistent.
class TA_UniqueId {
  public:
    ACTIVITY_FRAMEWORK_EXPORT TA_UniqueId();

    TA_UniqueId(const TA_UniqueId &) = delete;
    TA_UniqueId(TA_UniqueId &&) = delete;
    TA_UniqueId &operator=(const TA_UniqueId &) = delete;
    TA_UniqueId &operator=(TA_UniqueId &&) = delete;

    std::uint64_t id() const noexcept { return m_id; }

  private:
    const std::uint64_t m_id;
};

} // namespace CoreAsync

#endif // TA_UNIQUEID_H
