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

#ifndef TA_THREADPOOL_H
#define TA_THREADPOOL_H

#include "TA_ActivityQueue.h"
#include "TA_ActivityProxy.h"
#include "TA_CommonTools.h"
#include "TA_MetaStringView.h"
#include "TA_ActivityFramework_global.h"

#include <thread>
#include <deque>
#include <semaphore>
#include <memory>

namespace CoreAsync {

class ACTIVITY_FRAMEWORK_EXPORT TA_ThreadPool {
  public:
    struct AndroidPlatformTag {};
    struct DefaultPlatformTag {};

#if defined(__ANDROID__)
    struct PlatformSelector {
        using Tag = AndroidPlatformTag;
        using ThreadModel = std::thread;
        using ActivityQueue = TA_ActivityQueue<std::shared_ptr<TA_ActivityProxy>, 10240>;

        struct ThreadState {
            ThreadState() = default;

            std::counting_semaphore<ActivityQueue::capacity()> resource{0};
            std::atomic_bool isBusy{false};
            std::atomic_bool stopRequested{false};
        };
    };
#else
    struct PlatformSelector {
        using Tag = DefaultPlatformTag;
        using ThreadModel = std::jthread;
        using ActivityQueue = TA_ActivityQueue<std::shared_ptr<TA_ActivityProxy>, 10240>;

        struct ThreadState {
            ThreadState() = default;

            std::counting_semaphore<ActivityQueue::capacity()> resource{0};
            std::atomic_bool isBusy{false};
        };
    };
#endif
    using QueueType = typename PlatformSelector::ActivityQueue;
    using LocalThread = typename PlatformSelector::ThreadModel;

    using LocalThreadHandle = TA_LocalThreadHandle;

    explicit TA_ThreadPool(std::size_t size = std::thread::hardware_concurrency())
        : m_states(size), m_activityQueues(size), m_stealIdxs(size) {
        init();
    }

    ~TA_ThreadPool() { shutDown(); }

    TA_ThreadPool(const TA_ThreadPool &pool) = delete;
    TA_ThreadPool(TA_ThreadPool &&pool) = delete;

    TA_ThreadPool &operator=(const TA_ThreadPool &pool) = delete;
    TA_ThreadPool &operator=(TA_ThreadPool &&pool) = delete;

    void shutDown();

    LocalThreadHandle topPriorityThread(std::thread::id depencyThread) const {
        return selectTopPriorityThread(&depencyThread);
    }

    LocalThreadHandle topPriorityThread() const {
        return selectTopPriorityThread(nullptr);
    }

    LocalThreadHandle addThread();

    template <ActivityType Activity>
    [[nodiscard]] auto postActivity(Activity *pActivity, bool autoDelete = false) -> TA_ActivityResultFetcher {
        if (!pActivity)
            throw std::invalid_argument("Activity is null");
        std::shared_ptr<TA_ActivityProxy> pProxy{std::make_shared<TA_ActivityProxy>(pActivity, autoDelete)};
        return enqueueProxy(pProxy);
    }

    [[nodiscard]] auto postActivity(TA_ActivityProxy *&pActivity) -> TA_ActivityResultFetcher {
        if (!pActivity)
            throw std::invalid_argument("Activity proxy is null");
        auto weakRef = pActivity->weakRef();
        std::shared_ptr<TA_ActivityProxy> pProxy =
            weakRef.expired() ? std::shared_ptr<TA_ActivityProxy>(pActivity) : weakRef.lock();
        pActivity = nullptr;
        return enqueueProxy(pProxy);
    }

    [[nodiscard]] auto postActivity(const std::shared_ptr<TA_ActivityProxy> &pActivity) -> TA_ActivityResultFetcher {
        if (!pActivity)
            throw std::invalid_argument("Activity proxy is null");
        return enqueueProxy(pActivity);
    }

    std::size_t size() const { return m_threads.size(); }

    std::thread::id threadId(std::size_t idx) const {
        if (idx >= m_threads.size()) {
            throw std::invalid_argument("Idx is out of the range of thread size");
        }
        return m_threads[idx].get_id();
    }

    LocalThreadHandle threadHandle(std::size_t idx) const {
        return {threadId(idx), idx};
    }

    bool contains(LocalThreadHandle handle) const {
        return handle.isValid() && handle.index() < m_threads.size()
            && m_threads[handle.index()].get_id() == handle.threadId();
    }

    void setThreadDetached(LocalThreadHandle handle) {
        if (!contains(handle)) {
            throw std::invalid_argument("Thread handle does not belong to this thread pool");
        }
        m_threads[handle.index()].detach();
    }

  private:
    TA_ActivityResultFetcher enqueueProxy(const std::shared_ptr<TA_ActivityProxy> &activity) {
        auto submission = activity->prepareSubmission();
        if (!submission.has_value()) {
            throw std::runtime_error("Activity is not ready for submission");
        }

        const auto affinity = activity->affinityThread();
        if (affinity != LocalThreadHandle{} && !contains(affinity)) {
            throw std::invalid_argument("Affinity handle does not belong to this thread pool");
        }
        const auto dependencyThreadId = activity->dependencyThreadId();
        const auto threadHandle = affinity.isValid() ? affinity : topPriorityThread(dependencyThreadId);
        const std::size_t idx = threadHandle.index();
        if (!m_activityQueues[idx].push(activity, std::move(*submission))) {
            throw std::runtime_error("Failed to push activity to queue");
        }

        m_states[idx].resource.release();
        return {activity};
    }

    struct ThreadSnapshot {
        std::size_t index;
        std::size_t queueSize;
        bool isBusy;
    };

    static bool betterThreadSnapshot(const ThreadSnapshot &lhs, const ThreadSnapshot &rhs) noexcept {
        if (lhs.queueSize != rhs.queueSize) {
            return lhs.queueSize < rhs.queueSize;
        }
        if (lhs.isBusy != rhs.isBusy) {
            return !lhs.isBusy;
        }
        return lhs.index < rhs.index;
    }

    LocalThreadHandle selectTopPriorityThread(const std::thread::id *dependencyThread) const {
        ThreadSnapshot bestThread = {};
        bool hasBestThread = false;
        std::size_t threadCount = m_threads.size();
        for (std::size_t idx = 0; idx < threadCount; ++idx) {
            if (dependencyThread && m_threads[idx].get_id() == *dependencyThread) {
                continue;
            }

            const ThreadSnapshot candidate = {
                idx,
                m_activityQueues[idx].size(),
                m_states[idx].isBusy.load(std::memory_order_acquire),
            };
            if (!hasBestThread || betterThreadSnapshot(candidate, bestThread)) {
                bestThread = candidate;
                hasBestThread = true;
            }
        }

        if (!hasBestThread) {
            throw std::runtime_error("No available thread found in the thread pool.");
        }
        return LocalThreadHandle{m_threads[bestThread.index].get_id(), bestThread.index};
    }

    [[deprecated("Use the version with const std::thread::id * instead")]] LocalThreadHandle selectTopPriorityThread(std::optional<std::thread::id> dependencyThread) const {
        auto indices = std::views::iota(std::size_t{0}, m_activityQueues.size());

        auto validIndices = indices | std::views::filter([&](std::size_t idx) {
            return !(dependencyThread.has_value() && m_threads[idx].get_id() == dependencyThread.value());
        });

        auto bestIter = std::ranges::min_element(validIndices, [&](std::size_t lhs, std::size_t rhs) {
            ThreadSnapshot snapL = { lhs, m_activityQueues[lhs].size(), m_states[lhs].isBusy.load(std::memory_order_acquire) };
            ThreadSnapshot snapR = { rhs, m_activityQueues[rhs].size(), m_states[rhs].isBusy.load(std::memory_order_acquire) };
            
            return betterThreadSnapshot(snapL, snapR);
        });

        if (bestIter == validIndices.end()) {
            throw std::runtime_error("No available thread found in the thread pool.");
        }

        return LocalThreadHandle{m_threads[*bestIter].get_id(), *bestIter};
    }

    void init();
    bool trySteal(std::shared_ptr<TA_ActivityProxy> &stolenActivity, std::size_t excludedIdx);

    void waitStationary() {
        bool expected = true;
        m_isStationary.wait(false, std::memory_order_acquire);
    }

    void notifyStationary() {
        m_isStationary.store(true, std::memory_order_release);
        m_isStationary.notify_all();
    }

  private:
    std::deque<PlatformSelector::ThreadState> m_states;
    std::deque<LocalThread> m_threads;
    std::deque<QueueType> m_activityQueues;
    std::deque<std::size_t> m_stealIdxs;
    std::atomic_bool m_isStationary{false};
    std::atomic_bool m_requestPause{false};
};

struct ACTIVITY_FRAMEWORK_EXPORT TA_ThreadHolder {
    static void create(std::size_t size = std::thread::hardware_concurrency());
    static TA_ThreadPool &get();

    static TA_ThreadPool::LocalThreadHandle addThread();

  private:
    static TA_ThreadPool *m_pThreadPool;
};
} // namespace CoreAsync

#endif // TA_THREADPOOL_H
