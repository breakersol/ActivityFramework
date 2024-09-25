/*
 * Copyright [2024] [Shuang Zhu / Sol]
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

#include <thread>
#include <vector>
#include <semaphore>

namespace CoreAsync {
    class TA_ThreadPool
    {
        using ActivityQueue = TA_ActivityQueue<std::shared_ptr<TA_ActivityProxy>, 10240>;

        struct ThreadState
        {
            std::counting_semaphore<ActivityQueue::capacity()> resource {0};
        };

    public:
        explicit TA_ThreadPool(std::size_t size = std::thread::hardware_concurrency()) : m_states(size), m_activityQueues(size), m_stealIdxs(size)
        {
            init();
        }

        ~TA_ThreadPool()
        {
            shutDown();
        }

        TA_ThreadPool(const TA_ThreadPool &pool) = delete;
        TA_ThreadPool(TA_ThreadPool &&pool) = delete;

        TA_ThreadPool & operator = (const TA_ThreadPool &pool) = delete;
        TA_ThreadPool & operator = (TA_ThreadPool &&pool) = delete;

        void shutDown()
        {
            for(std::size_t idx = 0; idx < m_threads.size();++idx)
            {
                m_threads[idx].request_stop();
                m_states[idx].resource.release();
            }
            m_threads.clear();
        }

        std::size_t topPriorityThread() const
        {
            std::size_t lowIdx {std::numeric_limits<std::size_t>::max()}, lowSize {std::numeric_limits<std::size_t>::max()};
            for(std::size_t idx = 0;idx < m_activityQueues.size();++idx)
            {
                if(m_activityQueues[idx].size() < lowSize)
                {
                    lowIdx = idx;
                    lowSize = m_activityQueues[idx].size();
                }
            }
            return lowIdx;
        }

        template <ActivityType Activity>
        auto postActivity(Activity *pActivity, bool autoDelete = false)->ActivityResultFetcher
        {
            if(!pActivity)
                throw std::invalid_argument("Activity is null");
            std::shared_ptr<TA_ActivityProxy> pProxy {std::make_shared<TA_ActivityProxy>(pActivity, autoDelete)};
            auto activityId {pActivity->id()};
            auto affinityId {pActivity->affinityThread()};
            std::size_t idx = affinityId < m_threads.size() ? affinityId : activityId % m_threads.size();
            if(!m_activityQueues[idx].push(pProxy))
                throw std::runtime_error("Failed to push activity to queue");
            m_states[idx].resource.release();
            return {pProxy};
        }

        auto postActivity(TA_ActivityProxy *&pActivity)->ActivityResultFetcher
        {
            if(!pActivity)
                throw std::invalid_argument("Activity proxy is null");
            std::shared_ptr<TA_ActivityProxy> pProxy {pActivity};
            auto activityId {pActivity->id()};
            auto affinityId {pActivity->affinityThread()};
            pActivity = nullptr;
            std::size_t idx = affinityId < m_threads.size() ? affinityId : activityId % m_threads.size();
            if(!m_activityQueues[idx].push(pProxy))
                throw std::runtime_error("Failed to push activity to queue");
            m_states[idx].resource.release();
            return {pProxy};
        }

        std::size_t size() const
        {
            return m_threads.size();
        }

    private:
        void init()
        {  
            for(std::size_t idx = 0; idx < m_states.size();++idx)
            {
                m_stealIdxs[idx] = (idx + 1) % m_stealIdxs.size();
                m_threads.emplace_back(
                    [&, idx](const std::stop_token &st) {
                        std::shared_ptr<TA_ActivityProxy> pActivity {nullptr};
                        while (!st.stop_requested()) {
                            m_states[idx].resource.acquire();
                            while (!m_activityQueues[idx].isEmpty())
                            {
                                if(m_activityQueues[idx].pop(pActivity) && pActivity)
                                {
                                    (*pActivity)();
                                }
                            }
                            if(trySteal(pActivity, idx) && pActivity)
                            {
                                (*pActivity)();
                            }
                        }
                        TA_CommonTools::debugInfo(META_STRING("Shut down successuflly!\n"));
                    }
                );
            }
        }

        bool trySteal(std::shared_ptr<TA_ActivityProxy> &stolenActivity, std::size_t excludedIdx)
        {
            std::size_t startIdx {m_stealIdxs[excludedIdx]};
            std::size_t idx {(startIdx + 1) % m_threads.size()};
            while(idx != startIdx)
            {
                if(idx != excludedIdx && !m_activityQueues[idx].isEmpty())
                {
                    if(m_activityQueues[idx].pop(stolenActivity))
                    {
                        return true;
                    }
                }
                idx = (idx + 1) % m_threads.size();
            }
            return false;
        }

    private:
        std::vector<ThreadState> m_states;;
        std::vector<std::jthread> m_threads;
        std::vector<ActivityQueue> m_activityQueues;
        std::vector<std::size_t> m_stealIdxs;
    };

    struct TA_ThreadHolder
    {
        static TA_ThreadPool & get()
        {
            static TA_ThreadPool pool;
            return pool;
        }
    };
}

#endif // TA_THREADPOOL_H
