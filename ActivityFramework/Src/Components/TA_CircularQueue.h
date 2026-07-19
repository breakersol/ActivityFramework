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

#ifndef TA_CIRCULARQUEUE_H
#define TA_CIRCULARQUEUE_H

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <utility>

#include "TA_TypeFilter.h"

namespace CoreAsync {
template <typename T, std::size_t N> class TA_CircularQueue {
  public:
    TA_CircularQueue() noexcept {
        static_assert(N > 0, "Queue capacity must be greater than zero");
        for (std::size_t idx = 0; idx < N; ++idx) {
            m_data[idx].sequence.store(idx, std::memory_order_relaxed);
        }
    }

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

    TA_CircularQueue(const TA_CircularQueue &queue) = delete;
    TA_CircularQueue(TA_CircularQueue &&queue) = delete;

    TA_CircularQueue &operator=(const TA_CircularQueue &queue) = delete;
    TA_CircularQueue &operator=(TA_CircularQueue &&queue) = delete;

    static constexpr std::size_t capacity() { return N; }

    bool isFull() const {
        const std::size_t position = m_rearIndex.load(std::memory_order_relaxed);
        const Cell &cell = m_data[position % N];
        const std::size_t sequence = cell.sequence.load(std::memory_order_acquire);
        return sequenceDifference(sequence, position) < 0;
    }

    bool isEmpty() const {
        const std::size_t position = m_frontIndex.load(std::memory_order_relaxed);
        const Cell &cell = m_data[position % N];
        const std::size_t sequence = cell.sequence.load(std::memory_order_acquire);
        return sequenceDifference(sequence, position + 1) < 0;
    }

    std::size_t size() const {
        const std::size_t rear = m_rearIndex.load(std::memory_order_acquire);
        const std::size_t front = m_frontIndex.load(std::memory_order_acquire);
        return rear >= front ? (std::min)(rear - front, capacity()) : 0;
    }

    bool push(T &&t) {
        return pushValue(std::move(t));
    }

    bool push(const T &t) {
        return pushValue(t);
    }

    std::optional<T> pop() {
        std::size_t position = m_frontIndex.load(std::memory_order_relaxed);
        Cell *cell = nullptr;

        for (;;) {
            cell = &m_data[position % N];
            const std::size_t sequence = cell->sequence.load(std::memory_order_acquire);
            const std::intptr_t difference = sequenceDifference(sequence, position + 1);
            if (difference == 0) {
                if (m_frontIndex.compare_exchange_weak(position, position + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (difference < 0) {
                return std::nullopt;
            } else {
                position = m_frontIndex.load(std::memory_order_relaxed);
            }
        }

        T value = cell->value.exchange(T{}, std::memory_order_relaxed);
        cell->sequence.store(position + N, std::memory_order_release);
        return std::optional<T>{std::move(value)};
    }

    std::optional<T> pop() requires (ActivityType<T> || ActivityPtrType<T>) {
        std::size_t position = m_frontIndex.load(std::memory_order_relaxed);
        Cell *cell = nullptr;

        for (;;) {
            cell = &m_data[position % N];
            const std::size_t sequence = cell->sequence.load(std::memory_order_acquire);
            const std::intptr_t difference = sequenceDifference(sequence, position + 1);
            if (difference == 0) {
                if constexpr (std::is_pointer_v<T> || IsSmartPtr_v<T>) {
                    if (!cell->value.load(std::memory_order_relaxed)->stolenEnabled()) {
                        return std::nullopt;
                    }
                } else {
                    if (!cell->value.load(std::memory_order_relaxed).stolenEnabled()) {
                        return std::nullopt;
                    }
                }
                if (m_frontIndex.compare_exchange_weak(position, position + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (difference < 0) {
                return std::nullopt;
            } else {
                position = m_frontIndex.load(std::memory_order_relaxed);
            }
        }

        T value = cell->value.exchange(T{}, std::memory_order_relaxed);
        cell->sequence.store(position + N, std::memory_order_release);
        return std::optional<T>{std::move(value)};
    }

    std::optional<T> top() const {
        const std::size_t position = m_frontIndex.load(std::memory_order_relaxed);
        const Cell &cell = m_data[position % N];
        const std::size_t sequence = cell.sequence.load(std::memory_order_acquire);
        if (sequenceDifference(sequence, position + 1) != 0) {
            return std::nullopt;
        }

        T value = cell.value.load(std::memory_order_relaxed);
        if (cell.sequence.load(std::memory_order_acquire) != sequence) {
            return std::nullopt;
        }
        return std::optional<T>{std::move(value)};
    }

    constexpr auto front() const {
        const std::size_t position = m_frontIndex.load(std::memory_order_acquire);
        return m_data[position % N].value.load(std::memory_order_acquire);
    }

    constexpr auto rear() const {
        const std::size_t position = m_rearIndex.load(std::memory_order_acquire);
        return m_data[position % N].value.load(std::memory_order_acquire);
    }

  private:
    struct Cell {
        std::atomic<std::size_t> sequence{0};
        std::atomic<T> value{};
    };

    static std::intptr_t sequenceDifference(std::size_t lhs, std::size_t rhs) {
        return static_cast<std::intptr_t>(lhs) - static_cast<std::intptr_t>(rhs);
    }

    bool pushValue(T value) {
        std::size_t position = m_rearIndex.load(std::memory_order_relaxed);
        Cell *cell = nullptr;

        for (;;) {
            cell = &m_data[position % N];
            const std::size_t sequence = cell->sequence.load(std::memory_order_acquire);
            const std::intptr_t difference = sequenceDifference(sequence, position);
            if (difference == 0) {
                if (m_rearIndex.compare_exchange_weak(position, position + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (difference < 0) {
                return false;
            } else {
                position = m_rearIndex.load(std::memory_order_relaxed);
            }
        }

        cell->value.store(std::move(value), std::memory_order_relaxed);
        cell->sequence.store(position + 1, std::memory_order_release);
        return true;
    }

    std::array<Cell, N> m_data{};
    std::atomic<std::size_t> m_frontIndex{0}, m_rearIndex{0};
};

} // namespace CoreAsync

#endif // TA_CIRCULARQUEUE_H
