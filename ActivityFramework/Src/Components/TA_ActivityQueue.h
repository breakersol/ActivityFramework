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

#ifndef TA_ACTIVITYQUEUE_H
#define TA_ACTIVITYQUEUE_H

#include "TA_ActivityState.h"
#include "TA_CircularQueue.h"
#include "TA_TypeFilter.h"

#include <array>
#include <atomic>
#include <cassert>
#include <exception>
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
    using Base::rear;
    using Base::size;

    bool push(T &&value) {
        return prepareAndPush(std::move(value));
    }

    bool push(const T &value) {
        return prepareAndPush(value);
    }

    bool push(T &&value, TA_ActivitySubmission &&submission) {
        return pushActivity(std::move(value), std::move(submission));
    }

    bool push(const T &value, TA_ActivitySubmission &&submission) {
        return pushActivity(value, std::move(submission));
    }

    [[nodiscard]] std::optional<T> pop() {
        return markDequeued(Base::pop());
    }

    [[nodiscard]] std::optional<T> tryPop() {
        return markDequeued(
            Base::popIf([this](std::size_t cellIndex) {
                return m_stealEligible[cellIndex].load(std::memory_order_relaxed);
            }));
    }

  private:
    static bool isStealEligible(const T &value) {
        if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
            return value && value->stolenEnabled();
        } else {
            return value.stolenEnabled();
        }
    }

    static std::optional<TA_ActivitySubmission> prepareSubmission(T &value) {
        if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
            return value ? value->prepareSubmission() : std::nullopt;
        } else {
            return value.prepareSubmission();
        }
    }

    static std::int64_t activityId(const T &value) {
        if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
            return value->id();
        } else {
            return value.id();
        }
    }

    static bool tryMarkDequeued(T &value) {
        if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
            return value && value->tryMarkDequeued();
        } else {
            return value.tryMarkDequeued();
        }
    }

    static std::optional<T> markDequeued(std::optional<T> value) {
        if (value.has_value()) {
            const bool transitioned = tryMarkDequeued(*value);
            assert(transitioned && "Dequeued activity was not in the Queued state");
            static_cast<void>(transitioned);
        }
        return value;
    }

    bool prepareAndPush(T value) {
        auto submission = prepareSubmission(value);
        if (!submission.has_value()) {
            return false;
        }
        return pushActivity(std::move(value), std::move(*submission));
    }

    bool pushActivity(T value, TA_ActivitySubmission submission) {
        struct SubmissionLifetime {
            T owner;
            TA_ActivitySubmission submission;
        } lifetime{value, std::move(submission)};

        if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
            if (!lifetime.owner) {
                return false;
            }
        }
        if (!Detail::TA_ActivitySubmissionAccess::matches(lifetime.submission,
                                                          activityId(lifetime.owner))) {
            return false;
        }

        const bool stealEligible = isStealEligible(value);
        const bool pushed = Base::pushValueWithMetadata(
            std::move(value), [this, stealEligible, &lifetime](std::size_t cellIndex) noexcept {
                m_stealEligible[cellIndex].store(stealEligible, std::memory_order_relaxed);
                const bool committed =
                    Detail::TA_ActivitySubmissionAccess::commitQueued(lifetime.submission);
                assert(committed && "Published activity could not enter the Queued state");
                if (!committed) {
                    std::terminate();
                }
            });
        return pushed;
    }

    std::array<std::atomic_bool, N> m_stealEligible{};
};
} // namespace CoreAsync

#endif // TA_ACTIVITYQUEUE_H
