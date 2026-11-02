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

#ifndef TA_LOCALTHREADHANDLE_H
#define TA_LOCALTHREADHANDLE_H

#include <cstddef>
#include <limits>
#include <thread>

namespace CoreAsync {
// A copyable worker identity. A default handle means no explicit affinity.
class TA_LocalThreadHandle {
  public:
    TA_LocalThreadHandle() = default;
    TA_LocalThreadHandle(std::thread::id threadId, std::size_t index)
        : m_threadId(threadId), m_index(index) {}

    std::thread::id threadId() const { return m_threadId; }
    std::size_t index() const { return m_index; }
    bool isValid() const {
        return m_threadId != std::thread::id{} && m_index != std::numeric_limits<std::size_t>::max();
    }
    bool operator==(const TA_LocalThreadHandle &) const = default;

  private:
    std::thread::id m_threadId{};
    std::size_t m_index{std::numeric_limits<std::size_t>::max()};
};
} // namespace CoreAsync

#endif // TA_LOCALTHREADHANDLE_H
