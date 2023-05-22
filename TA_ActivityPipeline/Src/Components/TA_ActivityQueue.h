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

#ifndef TA_ACTIVITYQUEUE_H
#define TA_ACTIVITYQUEUE_H

#include <atomic>
#include <iostream>
#include <array>

namespace CoreAsync
{
    class TA_BasicActivity;

    template <typename T, std::size_t N>
    class TA_ActivityQueue
    {
    public:
        constexpr TA_ActivityQueue() {}
        ~TA_ActivityQueue()
        {

        }

//        void print()
//        {
//            for(int i = m_frontIndex.load(std::memory_order_acquire);i < m_rearIndex.load(std::memory_order_acquire);++i)
//            {
//                if(m_data[i].load(std::memory_order_acquire) != T {})
//                    std::cout << "Index: " << i << " " << "Res: " << m_data[i].load(std::memory_order_acquire) << std::endl;
//            }
//        }

        TA_ActivityQueue(const TA_ActivityQueue &queue) = delete;
        TA_ActivityQueue(TA_ActivityQueue &&queue) = delete;

        TA_ActivityQueue & operator = (const TA_ActivityQueue &queue) = delete;
        TA_ActivityQueue & operator = (TA_ActivityQueue &&queue) = delete;

        constexpr std::size_t size() const
        {
            return N;
        }

        bool isFull() const
        {
            return m_frontIndex.load(std::memory_order_acquire) == (m_rearIndex.load(std::memory_order_acquire) + 1) % N;
        }

        bool isEmpty() const
        {
            return m_frontIndex.load(std::memory_order_acquire) == m_rearIndex.load(std::memory_order_acquire);
        }

        bool push(T &&t)
        {
            if(isFull())
                return false;
            m_data[m_rearIndex.load(std::memory_order_acquire)].exchange(t);
            m_rearIndex.fetch_add(1);
            m_rearIndex.exchange(m_rearIndex.load(std::memory_order_acquire) % N);
            printf("Front: %Id, Rear: %Id.\n", m_frontIndex.load(std::memory_order_acquire), m_rearIndex.load(std::memory_order_acquire));
            return true;
        }

        bool push(const T &t)
        {
            if(isFull())
                return false;
            m_data[m_rearIndex.load(std::memory_order_acquire)].exchange(t);
            m_rearIndex.fetch_add(1);
            m_rearIndex.exchange(m_rearIndex.load(std::memory_order_acquire) % N);
            return true;
        }

        bool pop(T &t)
        {
            if(isEmpty())
                return false;
            t = m_data[m_frontIndex.load(std::memory_order_acquire)].load(std::memory_order_acquire);
            m_frontIndex.fetch_add(1);
            m_frontIndex.exchange(m_frontIndex.load(std::memory_order_acquire) % N);
            return true;
        }

        constexpr auto front() const
        {
            return m_data[m_frontIndex.load(std::memory_order_acquire)].load(std::memory_order_acquire);
        }

        constexpr auto rear() const
        {
            return m_data[m_rearIndex.load(std::memory_order_acquire)].load(std::memory_order_acquire);
        }

    private:
        std::array<std::atomic<T>,N> m_data {nullptr};
        std::atomic<std::size_t> m_frontIndex {0}, m_rearIndex {0};

    };

    using ActivityQueue = TA_ActivityQueue<TA_BasicActivity *, 500>;
}

#endif // TA_ACTIVITYQUEUE_H
