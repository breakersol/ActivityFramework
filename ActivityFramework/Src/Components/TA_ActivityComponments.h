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

#ifndef TA_ACTIVITYCOMPONMENTS_H
#define TA_ACTIVITYCOMPONMENTS_H

#include <atomic>
#include <thread>

#include "TA_ThreadPool.h"

namespace CoreAsync
{
    class TA_ActivityAffinityThread
    {
    public:
        explicit TA_ActivityAffinityThread(std::size_t affinityThread = TA_ThreadHolder::get().topPriorityThread()) : m_sourceThread(std::this_thread::get_id()), m_affinityThread(affinityThread)
        {

        }

        ~TA_ActivityAffinityThread() {}

        TA_ActivityAffinityThread(const TA_ActivityAffinityThread &another) : m_sourceThread(another.m_sourceThread), m_affinityThread(another.affinityThread())
        {

        }

        TA_ActivityAffinityThread(TA_ActivityAffinityThread &&another) : m_sourceThread(std::move(another.m_sourceThread)), m_affinityThread(std::move(another.affinityThread()))
        {

        }

        TA_ActivityAffinityThread & operator = (const TA_ActivityAffinityThread &another)
        {
            m_affinityThread.store(another.affinityThread(), std::memory_order_release);
            return *this;
        }

        TA_ActivityAffinityThread & operator = (TA_ActivityAffinityThread &&another)
        {
            m_affinityThread.store(std::move(another.affinityThread()), std::memory_order_release);
            return *this;
        }

        const std::thread::id sourceThread() const
        {
            return m_sourceThread;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.load(std::memory_order_acquire);
        }

        bool moveToThread(std::size_t thread)
        {
            if(thread >= TA_ThreadHolder::get().size())
                return false;
            m_affinityThread.store(thread, std::memory_order_release);
            return true;
        }

    private:
        const std::thread::id m_sourceThread;
        std::atomic_size_t m_affinityThread;
    };

    class TA_ActivityId
    {
    public:
        TA_ActivityId() : m_id(m_count.load(std::memory_order_acquire))
        {
            m_count.fetch_add(1);
        }

        std::int64_t id() const
        {
            return m_id;
        }

    private:
        inline static std::atomic_int64_t m_count {0};
        const std::int64_t m_id;

    };

}

#endif // TA_ACTIVITYCOMPONMENTS_H
