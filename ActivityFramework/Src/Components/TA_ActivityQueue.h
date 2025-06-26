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

#include <atomic>
#include <array>

namespace CoreAsync {
template <typename T, std::size_t N> class TA_ActivityQueue {
  public:
    constexpr TA_ActivityQueue() {}

    //        void print()
    //        {
    //            for(int i = m_frontIndex.load(std::memory_order_acquire);i <
    //            m_rearIndex.load(std::memory_order_acquire);++i)
    //            {
    //                if(m_data[i].load(std::memory_order_acquire) != T {})
    //                    std::cout << "Index: " << i << " " << "Res: " << m_data[i].load(std::memory_order_acquire) <<
    //                    std::endl;
    //            }
    //        }

    TA_ActivityQueue(const TA_ActivityQueue &queue) = delete;
    TA_ActivityQueue(TA_ActivityQueue &&queue) = delete;

    TA_ActivityQueue &operator=(const TA_ActivityQueue &queue) = delete;
    TA_ActivityQueue &operator=(TA_ActivityQueue &&queue) = delete;

    static constexpr std::size_t capacity() { return N; }

    bool isFull() const {
        return m_frontIndex.load(std::memory_order_acquire) == (m_rearIndex.load(std::memory_order_acquire) + 1) % N;
    }

    bool isEmpty() const {
        return m_frontIndex.load(std::memory_order_acquire) == m_rearIndex.load(std::memory_order_acquire);
    }

    std::size_t size() const {
        std::size_t r{m_rearIndex.load(std::memory_order_acquire)};
        std::size_t f{m_frontIndex.load(std::memory_order_acquire)};

        return r >= f ? r - f : (capacity() - (f - r));
    }

    bool push(T &&t) {
        std::size_t rearIndexOld, rearIndexNew;
        do {
            rearIndexOld = m_rearIndex.load(std::memory_order_acquire);
            rearIndexNew = (rearIndexOld + 1) % N;
            if (rearIndexNew == m_frontIndex.load(std::memory_order_acquire))
                return false;
        } while (!m_rearIndex.compare_exchange_weak(rearIndexOld, rearIndexNew, std::memory_order_acq_rel));

        m_data[rearIndexOld].exchange(t);
        return true;
    }

    bool push(const T &t) {
        std::size_t rearIndexOld, rearIndexNew;
        do {
            rearIndexOld = m_rearIndex.load(std::memory_order_acquire);
            rearIndexNew = (rearIndexOld + 1) % N;
            if (rearIndexNew == m_frontIndex.load(std::memory_order_acquire))
                return false;
        } while (!m_rearIndex.compare_exchange_weak(rearIndexOld, rearIndexNew, std::memory_order_acq_rel));

        m_data[rearIndexOld].exchange(t);
        return true;
    }

    bool pop(T &t) {
        std::size_t frontIndexOld, frontIndexNew;
        do {
            frontIndexOld = m_frontIndex.load(std::memory_order_acquire);
            if (frontIndexOld == m_rearIndex.load(std::memory_order_acquire))
                return false;
            frontIndexNew = (frontIndexOld + 1) % N;
        } while (!m_frontIndex.compare_exchange_weak(frontIndexOld, frontIndexNew, std::memory_order_acq_rel));

        t = m_data[frontIndexOld].load(std::memory_order_acquire);
        return true;
    }

    constexpr auto front() const {
        return m_data[m_frontIndex.load(std::memory_order_acquire)].load(std::memory_order_acquire);
    }

    constexpr auto rear() const {
        return m_data[m_rearIndex.load(std::memory_order_acquire)].load(std::memory_order_acquire);
    }

  private:
    std::array<std::atomic<T>, N> m_data{};
    std::atomic<std::size_t> m_frontIndex{0}, m_rearIndex{0};
};
} // namespace CoreAsync

#endif // TA_ACTIVITYQUEUE_H
