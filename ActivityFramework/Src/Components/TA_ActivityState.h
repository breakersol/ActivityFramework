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

#ifndef TA_ACTIVITYSTATE_H
#define TA_ACTIVITYSTATE_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

namespace CoreAsync {
enum class TA_ActivityState : std::uint8_t {
    Configuring,
    Queued,
    Running,
    Completed,
};

namespace Detail {
class TA_ActivityLifecycle;
struct TA_ActivitySubmissionAccess;
} // namespace Detail

class TA_ActivitySubmission {
  public:
    TA_ActivitySubmission(const TA_ActivitySubmission &) = delete;
    TA_ActivitySubmission &operator=(const TA_ActivitySubmission &) = delete;

    TA_ActivitySubmission(TA_ActivitySubmission &&other) noexcept;
    TA_ActivitySubmission &operator=(TA_ActivitySubmission &&other) noexcept;

    ~TA_ActivitySubmission();

  private:
    friend class Detail::TA_ActivityLifecycle;
    friend struct Detail::TA_ActivitySubmissionAccess;

    TA_ActivitySubmission(Detail::TA_ActivityLifecycle &lifecycle, std::int64_t activityId) noexcept;

    bool matches(std::int64_t activityId) const noexcept;
    bool commitQueued() noexcept;
    void reset() noexcept;

    Detail::TA_ActivityLifecycle *m_lifecycle = nullptr;
    std::int64_t m_activityId = 0;
};

namespace Detail {
class TA_ActivityLifecycle {
  public:
    TA_ActivityLifecycle() noexcept = default;

    TA_ActivityLifecycle(const TA_ActivityLifecycle &) = delete;
    TA_ActivityLifecycle(TA_ActivityLifecycle &&) = delete;
    TA_ActivityLifecycle &operator=(const TA_ActivityLifecycle &) = delete;
    TA_ActivityLifecycle &operator=(TA_ActivityLifecycle &&) = delete;

    TA_ActivityState state() const noexcept {
        switch (m_state.load(std::memory_order_acquire)) {
        case InternalState::Configuring:
        case InternalState::Updating:
        case InternalState::Submitting:
            return TA_ActivityState::Configuring;
        case InternalState::Queued:
            return TA_ActivityState::Queued;
        case InternalState::Ready:
        case InternalState::DirectRunning:
        case InternalState::Running:
            return TA_ActivityState::Running;
        case InternalState::Completed:
            return TA_ActivityState::Completed;
        }

        return TA_ActivityState::Completed;
    }

    template <typename Modifier>
    bool updateConfiguration(Modifier &&modifier) {
        InternalState expected = InternalState::Configuring;
        if (!m_state.compare_exchange_strong(expected, InternalState::Updating,
                                             std::memory_order_acquire, std::memory_order_relaxed)) {
            return false;
        }

        try {
            std::invoke(std::forward<Modifier>(modifier));
        } catch (...) {
            m_state.store(InternalState::Configuring, std::memory_order_release);
            throw;
        }

        m_state.store(InternalState::Configuring, std::memory_order_release);
        return true;
    }

    std::optional<TA_ActivitySubmission> prepareSubmission(std::int64_t activityId) noexcept {
        InternalState expected = InternalState::Configuring;
        if (!m_state.compare_exchange_strong(expected, InternalState::Submitting,
                                             std::memory_order_acq_rel, std::memory_order_relaxed)) {
            return std::nullopt;
        }

        TA_ActivitySubmission submission{*this, activityId};
        return submission;
    }

    bool tryMarkDequeued() noexcept {
        InternalState expected = InternalState::Queued;
        return m_state.compare_exchange_strong(expected, InternalState::Ready,
                                               std::memory_order_acq_rel, std::memory_order_relaxed);
    }

    bool tryStartExecution() noexcept {
        InternalState expected = InternalState::Configuring;
        if (m_state.compare_exchange_strong(expected, InternalState::DirectRunning,
                                            std::memory_order_acq_rel, std::memory_order_relaxed)) {
            return true;
        }

        expected = InternalState::Ready;
        return m_state.compare_exchange_strong(expected, InternalState::Running,
                                               std::memory_order_acq_rel, std::memory_order_relaxed);
    }

    void markCompleted() noexcept {
        InternalState expected = InternalState::Running;
        if (m_state.compare_exchange_strong(expected, InternalState::Completed,
                                            std::memory_order_release, std::memory_order_relaxed)) {
            return;
        }

        expected = InternalState::DirectRunning;
        m_state.compare_exchange_strong(expected, InternalState::Configuring,
                                        std::memory_order_release, std::memory_order_relaxed);
    }

  private:
    friend class ::CoreAsync::TA_ActivitySubmission;

    bool commitSubmission() noexcept {
        InternalState expected = InternalState::Submitting;
        return m_state.compare_exchange_strong(expected, InternalState::Queued,
                                               std::memory_order_release, std::memory_order_relaxed);
    }

    bool rollbackSubmission() noexcept {
        InternalState expected = InternalState::Submitting;
        return m_state.compare_exchange_strong(expected, InternalState::Configuring,
                                               std::memory_order_release, std::memory_order_relaxed);
    }

    enum class InternalState : std::uint8_t {
        Configuring,
        Updating,
        Submitting,
        Queued,
        Ready,
        DirectRunning,
        Running,
        Completed,
    };

    std::atomic<InternalState> m_state = {InternalState::Configuring};
};

class TA_ActivityCompletionGuard {
  public:
    explicit TA_ActivityCompletionGuard(TA_ActivityLifecycle &lifecycle) noexcept
        : m_lifecycle(lifecycle) {}

    TA_ActivityCompletionGuard(const TA_ActivityCompletionGuard &) = delete;
    TA_ActivityCompletionGuard(TA_ActivityCompletionGuard &&) = delete;
    TA_ActivityCompletionGuard &operator=(const TA_ActivityCompletionGuard &) = delete;
    TA_ActivityCompletionGuard &operator=(TA_ActivityCompletionGuard &&) = delete;

    ~TA_ActivityCompletionGuard() { m_lifecycle.markCompleted(); }

  private:
    TA_ActivityLifecycle &m_lifecycle;
};

struct TA_ActivitySubmissionAccess {
    static bool matches(const TA_ActivitySubmission &submission, std::int64_t activityId) noexcept {
        return submission.matches(activityId);
    }

    static bool commitQueued(TA_ActivitySubmission &submission) noexcept {
        return submission.commitQueued();
    }
};
} // namespace Detail

inline TA_ActivitySubmission::TA_ActivitySubmission(Detail::TA_ActivityLifecycle &lifecycle,
                                                    std::int64_t activityId) noexcept
    : m_lifecycle(&lifecycle), m_activityId(activityId) {}

inline TA_ActivitySubmission::TA_ActivitySubmission(TA_ActivitySubmission &&other) noexcept
    : m_lifecycle(std::exchange(other.m_lifecycle, nullptr)),
      m_activityId(std::exchange(other.m_activityId, 0)) {}

inline TA_ActivitySubmission &TA_ActivitySubmission::operator=(TA_ActivitySubmission &&other) noexcept {
    if (this != &other) {
        reset();
        m_lifecycle = std::exchange(other.m_lifecycle, nullptr);
        m_activityId = std::exchange(other.m_activityId, 0);
    }
    return *this;
}

inline TA_ActivitySubmission::~TA_ActivitySubmission() { reset(); }

inline bool TA_ActivitySubmission::matches(std::int64_t activityId) const noexcept {
    return m_lifecycle && m_activityId == activityId;
}

inline bool TA_ActivitySubmission::commitQueued() noexcept {
    if (!m_lifecycle || !m_lifecycle->commitSubmission()) {
        return false;
    }
    m_lifecycle = nullptr;
    return true;
}

inline void TA_ActivitySubmission::reset() noexcept {
    if (m_lifecycle) {
        m_lifecycle->rollbackSubmission();
        m_lifecycle = nullptr;
    }
}
} // namespace CoreAsync

#endif // TA_ACTIVITYSTATE_H
