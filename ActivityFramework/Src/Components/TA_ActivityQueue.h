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

#ifndef TA_ACTIVITYQUEUE_H
#define TA_ACTIVITYQUEUE_H

#include "TA_CircularQueue.h"
#include "TA_TypeFilter.h"

#include <array>
#include <atomic>
#include <optional>
#include <type_traits>
#include <utility>

namespace CoreAsync {
template <typename T, std::size_t N>
    requires (ActivityType<T> || ActivityPtrType<T>)
class TA_ActivityQueue final : private TA_CircularQueue<T, N> {
    using Base = TA_CircularQueue<T, N>;

  public:
    TA_ActivityQueue() noexcept = default;

    TA_ActivityQueue(const TA_ActivityQueue &) = delete;
    TA_ActivityQueue(TA_ActivityQueue &&) = delete;

    TA_ActivityQueue &operator=(const TA_ActivityQueue &) = delete;
    TA_ActivityQueue &operator=(TA_ActivityQueue &&) = delete;

    using Base::capacity;
    using Base::front;
    using Base::isEmpty;
    using Base::isFull;
    using Base::pop;
    using Base::rear;
    using Base::size;
    using Base::top;

    bool push(T &&value) {
        return pushActivity(std::move(value));
    }

    bool push(const T &value) {
        return pushActivity(value);
    }

    std::optional<T> tryPop() {
        return Base::popIf([this](std::size_t cellIndex) {
            return m_stealEligible[cellIndex].load(std::memory_order_relaxed);
        });
    }

  private:
    static bool isStealEligible(const T &value) {
        if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
            return value && value->stolenEnabled();
        } else {
            return value.stolenEnabled();
        }
    }

    bool pushActivity(T value) {
        const bool stealEligible = isStealEligible(value);
        return Base::pushValueWithMetadata(
            std::move(value), [this, stealEligible](std::size_t cellIndex) noexcept {
                m_stealEligible[cellIndex].store(stealEligible, std::memory_order_relaxed);
            });
    }

    std::array<std::atomic_bool, N> m_stealEligible{};
};
} // namespace CoreAsync

#endif // TA_ACTIVITYQUEUE_H
