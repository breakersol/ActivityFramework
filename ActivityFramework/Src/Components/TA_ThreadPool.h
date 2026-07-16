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
#include <vector>
#include <semaphore>
#include <memory>
#include <optional>

namespace CoreAsync {

class ACTIVITY_FRAMEWORK_EXPORT TA_ThreadPool {
  public:
    struct AndroidPlatformTag {};
    struct DefaultPlatformTag {};

#if defined(__ANDROID__)
    struct PlatformSelector {
        using Tag = AndroidPlatformTag;
        using ThreadModel = std::thread;
        static constexpr bool activityHandleRequired = true;

        struct ActivityHandle {
            ActivityHandle() = default;
            explicit ActivityHandle(const std::shared_ptr<TA_ActivityProxy> &proxyIn) : proxy(proxyIn) {}
            std::shared_ptr<TA_ActivityProxy> proxy{nullptr};
            bool stolenEnabled() const { return proxy && proxy->stolenEnabled(); }

            static std::shared_ptr<TA_ActivityProxy> extractActivity(ActivityHandle *handle) {
                if (!handle) {
                    return nullptr;
                }
                std::shared_ptr<TA_ActivityProxy> proxy{std::move(handle->proxy)};
                delete handle;
                return proxy;
            }
        };

        using ActivityQueue = TA_ActivityQueue<ActivityHandle *, 10240>;

        struct ThreadState {
            ThreadState() = default;

            std::counting_semaphore<ActivityQueue::capacity()> resource{0};
            std::atomic_bool isBusy{false};
            std::atomic_bool stopRequested{false};
        };
    };
    using HandleType = typename PlatformSelector::ActivityHandle;
#else
    struct PlatformSelector {
        using Tag = DefaultPlatformTag;
        using ThreadModel = std::jthread;
        static constexpr bool activityHandleRequired = false;
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

    std::size_t topPriorityThread(std::thread::id depencyThread) const {
        return selectTopPriorityThread(&depencyThread);
    }

    std::size_t topPriorityThread() const {
        return selectTopPriorityThread(nullptr);
    }

    template <ActivityType Activity>
    [[nodiscard]] auto postActivity(Activity *pActivity, bool autoDelete = false) -> TA_ActivityResultFetcher {
        if (!pActivity)
            throw std::invalid_argument("Activity is null");
        std::shared_ptr<TA_ActivityProxy> pProxy{std::make_shared<TA_ActivityProxy>(pActivity, autoDelete)};
        auto affinityId{pActivity->affinityThread()};
        std::size_t idx =
            affinityId < m_threads.size() ? affinityId : topPriorityThread(pActivity->dependencyThreadId());
        //std::cout << "Post activity to thread: " << idx << "\n";
#if defined(__ANDROID__)
            auto handle = std::unique_ptr<HandleType>(new HandleType{pProxy});
            if (!m_activityQueues[idx].push(handle.get()))
                throw std::runtime_error("Failed to push activity to queue");
            handle.release();
#else
            if (!m_activityQueues[idx].push(pProxy))
                throw std::runtime_error("Failed to push activity to queue");
#endif
        m_states[idx].resource.release();
        return {pProxy};
    }

    [[nodiscard]] auto postActivity(TA_ActivityProxy *&pActivity) -> TA_ActivityResultFetcher {
        if (!pActivity)
            throw std::invalid_argument("Activity proxy is null");
        auto weakRef = pActivity->weakRef();
        std::shared_ptr<TA_ActivityProxy> pProxy =
            weakRef.expired() ? std::shared_ptr<TA_ActivityProxy>(pActivity) : weakRef.lock();
        auto affinityId{pActivity->affinityThread()};
        auto dependencyThreadId{pActivity->dependencyThreadId()};
        pActivity = nullptr;
        std::size_t idx = affinityId < m_threads.size() ? affinityId : topPriorityThread(dependencyThreadId);
        //std::cout << "Post activity to thread: " << idx << "\n";
#if defined(__ANDROID__)
            auto handle = std::unique_ptr<HandleType>(new HandleType{pProxy});
            if (!m_activityQueues[idx].push(handle.get()))
                throw std::runtime_error("Failed to push activity to queue");
            handle.release();
#else
            if (!m_activityQueues[idx].push(pProxy))
                throw std::runtime_error("Failed to push activity to queue");
#endif
        m_states[idx].resource.release();
        return {pProxy};
    }

    [[nodiscard]] auto postActivity(const std::shared_ptr<TA_ActivityProxy> &pActivity) -> TA_ActivityResultFetcher {
        if (!pActivity)
            throw std::invalid_argument("Activity proxy is null");
        auto affinityId{pActivity->affinityThread()};
        auto dependencyThreadId{pActivity->dependencyThreadId()};
        std::size_t idx = affinityId < m_threads.size() ? affinityId : topPriorityThread(dependencyThreadId);
        //std::cout << "Post activity to thread: " << idx << "\n";
#if defined(__ANDROID__)
            auto handle = std::unique_ptr<HandleType>(new HandleType{pActivity});
            if (!m_activityQueues[idx].push(handle.get()))
                throw std::runtime_error("Failed to push activity to queue");
            handle.release();
#else
            if (!m_activityQueues[idx].push(pActivity))
                throw std::runtime_error("Failed to push activity to queue");
#endif
        m_states[idx].resource.release();
        return {pActivity};
    }

    std::size_t size() const { return m_threads.size(); }

    std::thread::id threadId(std::size_t idx) const {
        if (idx >= m_threads.size()) {
            throw std::invalid_argument("Idx is out of the range of thread size");
        }
        return m_threads[idx].get_id();
    }

    void setThreadDetached(std::size_t idx) {
        if (idx >= m_threads.size()) {
            throw std::invalid_argument("Idx is out of the range of thread size");
        }
        m_threads[idx].detach();
    }

  private:
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

    std::size_t selectTopPriorityThread(const std::thread::id *dependencyThread) const {
        ThreadSnapshot bestThread = {};
        bool hasBestThread = false;
        for (std::size_t idx = 0; idx < m_activityQueues.size(); ++idx) {
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
        return bestThread.index;
    }

    [[deprecated("Use the version with const std::thread::id * instead")]] std::size_t selectTopPriorityThread(std::optional<std::thread::id> dependencyThread) const {
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

        return *bestIter;
    }

    void init();
    bool trySteal(std::shared_ptr<TA_ActivityProxy> &stolenActivity, std::size_t excludedIdx);

  private:
    std::vector<PlatformSelector::ThreadState> m_states;
    std::vector<LocalThread> m_threads;
    std::vector<QueueType> m_activityQueues;
    std::vector<std::size_t> m_stealIdxs;
};

struct ACTIVITY_FRAMEWORK_EXPORT TA_ThreadHolder {
    static void create(std::size_t size = std::thread::hardware_concurrency());
    static TA_ThreadPool &get();

  private:
    static TA_ThreadPool *m_pThreadPool;
};
} // namespace CoreAsync

#endif // TA_THREADPOOL_H
