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

#include "TA_UniqueId.h"

#include <atomic>
#include <limits>
#include <stdexcept>

namespace CoreAsync {
namespace {
std::atomic<std::uint64_t> nextId{1};

std::uint64_t allocateId() {
    auto current = nextId.load(std::memory_order_relaxed);
    for (;;) {
        if (current == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("TA_UniqueId space exhausted");
        if (nextId.compare_exchange_weak(current, current + 1, std::memory_order_relaxed))
            return current;
    }
}
} // namespace

TA_UniqueId::TA_UniqueId() : m_id(allocateId()) {}

} // namespace CoreAsync
