/*
 * Copyright [2023] [Shuang Zhu / Sol]
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

#include "TA_MetaObject.h"
#include "TA_ActivityQueue.h"
#include "TA_LinkedActivity.h"
#include "TA_Connection.h"

#include <thread>
#include <vector>
#include <semaphore>
#include <future>
#include <random>

namespace CoreAsync {
    class TA_ThreadPool : public TA_MetaObject
    {
        using SharedPromise = std::shared_ptr<std::promise<TA_Variant> >;

        struct ThreadState
        {
            std::atomic_bool m_isBusy {false};
            std::binary_semaphore resource {0};
        };

    public:
        explicit TA_ThreadPool(std::size_t size = std::thread::hardware_concurrency()) : m_states(size), m_activityQueues(size)
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

        auto postActivity(TA_BasicActivity *pActivity, bool autoDelete = false)
        {
            if(!pActivity)
                return std::make_pair(std::future<TA_Variant> {}, std::size_t {});
            SharedPromise pr {std::make_shared<std::promise<TA_Variant>>()};
            std::future<TA_Variant> ft {pr->get_future()}; 
            auto wrapperActivity = new TA_LinkedActivity<LambdaType<TA_Variant,SharedPromise>,INVALID_INS,TA_Variant,SharedPromise>([pActivity, autoDelete](SharedPromise pr)->TA_Variant {
                TA_Variant var = (*pActivity)();
                pr->set_value(var);
                if(autoDelete)
                {
                    delete pActivity;
                }
                return var;
            }, std::move(pr));
            auto activityId {wrapperActivity->id()};
            std::size_t idx = activityId % m_threads.size();
            if(!m_activityQueues[idx].push(wrapperActivity))
                return std::make_pair(std::future<TA_Variant> {}, std::size_t {});
            if(!m_states[idx].m_isBusy.load(std::memory_order_acquire))
            {
                m_states[idx].resource.release();
            }
            return std::make_pair(std::move(ft), activityId);
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
                m_threads.emplace_back(
                    [&, idx](const std::stop_token &st) {
                        TA_BasicActivity *pActivity {nullptr};
                        while (!st.stop_requested()) {
                            m_states[idx].resource.acquire();
                            m_states[idx].m_isBusy.store(true, std::memory_order_release);
                            while (!m_activityQueues[idx].isEmpty())
                            {
                                if(m_activityQueues[idx].pop(pActivity) && pActivity)
                                {
                                    TA_Variant var = (*pActivity)();
                                    TA_Connection::active(this, &TA_ThreadPool::taskCompleted, pActivity->id(), var);
                                    delete pActivity;
                                    pActivity = nullptr;
                                }
                            }
                            if(trySteal(pActivity) && pActivity)
                            {
                                TA_Variant var = (*pActivity)();
                                TA_Connection::active(this, &TA_ThreadPool::taskCompleted, pActivity->id(), var);
                                delete pActivity;
                                pActivity = nullptr;
                            }
                            m_states[idx].m_isBusy.store(false, std::memory_order_release);
                        }
                        TA_CommonTools::debugInfo(META_STRING("Shut down successuflly!\n"));
                    }
                );
            }
        }

        bool trySteal(TA_BasicActivity *&stolenActivity)
        {
            std::uniform_int_distribution<std::size_t> distribution(0, m_states.size() - 1);
            for (std::size_t i = 0; i < m_states.size(); ++i)
            {
                std::size_t targetIndex = distribution(m_randomGenerator) % m_states.size();
                if (!m_activityQueues[targetIndex].isEmpty())
                {
                    if(m_activityQueues[targetIndex].pop(stolenActivity))
                        return true;
                }
            }
            return false;
        }

    TA_Signals:
        void taskCompleted(std::size_t id, TA_Variant var) {}

    private:
        std::vector<ThreadState> m_states;;
        std::vector<std::jthread> m_threads;
        std::vector<ActivityQueue> m_activityQueues;
        std::mt19937 m_randomGenerator;
    };

    struct TA_ThreadHolder
    {
        using Handle = std::pair<std::future<TA_Variant>, std::size_t>;

        static TA_ThreadPool & get()
        {
            static TA_ThreadPool pool;
            return pool;
        }
    };

    template <>
    struct Reflex::TA_TypeInfo<TA_ThreadPool> : TA_MetaTypeInfo<TA_ThreadPool>
    {
        static constexpr TA_MetaFieldList fields = {
            TA_MetaField {&Raw::taskCompleted, META_STRING("taskCompleted")},
        };
    };
}

#endif // TA_THREADPOOL_H
