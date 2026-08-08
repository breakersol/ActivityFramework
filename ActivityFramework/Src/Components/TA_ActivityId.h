/*
 * Copyright [2025] [Shuang Zhu / Sol]
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

#ifndef TA_ACTIVITYID_H
#define TA_ACTIVITYID_H

#include <atomic>
#include <cstdint>

namespace CoreAsync {
class TA_ActivityId {
  public:
    TA_ActivityId() : m_id(m_count.fetch_add(1, std::memory_order_relaxed)) {}

    std::int64_t id() const { return m_id; }

  private:
    inline static std::atomic_int64_t m_count{0};
    const std::int64_t m_id;
};
} // namespace CoreAsync

#endif // TA_ACTIVITYID_H
