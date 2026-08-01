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

#ifndef TA_CIRCULARQUEUE_H
#define TA_CIRCULARQUEUE_H

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace CoreAsync {
namespace Detail {
template <typename T> class TA_AtomicQueueValue {
  public:
    static_assert(std::is_trivially_copyable_v<T>,
                  "Queue values must be trivially copyable or std::shared_ptr");

    void store(T value, std::memory_order order) noexcept {
        m_value.store(std::move(value), order);
    }

    T load(std::memory_order order) const noexcept {
        return m_value.load(order);
    }

    T exchange(T value, std::memory_order order) noexcept {
        return m_value.exchange(std::move(value), order);
    }

  private:
    std::atomic<T> m_value{};
};

// Android libc++ does not provide the C++20 std::atomic<std::shared_ptr<T>>
// specialization. The shared_ptr atomic access functions are available there
// and provide the same synchronization without requiring a trivially copyable
// shared_ptr representation.
template <typename T> class TA_AtomicQueueValue<std::shared_ptr<T>> {
  public:
    void store(std::shared_ptr<T> value, std::memory_order order) noexcept {
        std::atomic_store_explicit(&m_value, std::move(value), order);
    }

    std::shared_ptr<T> load(std::memory_order order) const noexcept {
        return std::atomic_load_explicit(&m_value, order);
    }

    std::shared_ptr<T> exchange(std::shared_ptr<T> value, std::memory_order order) noexcept {
        return std::atomic_exchange_explicit(&m_value, std::move(value), order);
    }

  private:
    std::shared_ptr<T> m_value{};
};
} // namespace Detail

template <typename T, std::size_t N> class TA_CircularQueue {
  public:
    TA_CircularQueue() noexcept {
        static_assert(N > 1, "Queue capacity must be greater than zero");
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
        return popIf([](std::size_t) { return true; });
    }

    std::optional<T> front() const requires (!std::is_pointer_v<T>) {
        const std::size_t position = m_frontIndex.load(std::memory_order_relaxed);
        return snapshot(position);
    }

    std::optional<T> rear() const requires (!std::is_pointer_v<T>) {
        const std::size_t rear = m_rearIndex.load(std::memory_order_relaxed);
        const std::size_t front = m_frontIndex.load(std::memory_order_relaxed);
        if (rear == front) {
            return std::nullopt;
        }

        return snapshot(rear - 1);
    }

  protected:
    template <typename Publisher>
    bool pushValueWithMetadata(T value, Publisher publisher) {
        static_assert(std::is_nothrow_invocable_v<Publisher &, std::size_t>,
                      "Queue metadata publication must not throw after a slot is claimed");
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

        // Republish the claimed generation before changing the value. A front()
        // that acquires this value must then observe this generation or a later
        // one during its final sequence validation.
        cell->sequence.store(position, std::memory_order_relaxed);
        cell->value.store(std::move(value), std::memory_order_release);
        publisher(position % N);
        cell->sequence.store(position + 1, std::memory_order_release);
        return true;
    }

    template <typename Predicate>
    std::optional<T> popIf(Predicate predicate) {
        std::size_t position = m_frontIndex.load(std::memory_order_relaxed);
        Cell *cell = nullptr;

        for (;;) {
            cell = &m_data[position % N];
            const std::size_t sequence = cell->sequence.load(std::memory_order_acquire);
            const std::intptr_t difference = sequenceDifference(sequence, position + 1);
            if (difference == 0) {
                if (!predicate(position % N)) {
                    if (cell->sequence.load(std::memory_order_acquire) != sequence) {
                        position = m_frontIndex.load(std::memory_order_relaxed);
                        continue;
                    }
                    return std::nullopt;
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

        // Mark the cell claimed before clearing its value. If front() observes the
        // cleared value through its acquire load, the release exchange makes
        // this marker visible to its final sequence validation. Producers still
        // cannot reuse the cell until the free sequence is published below.
        cell->sequence.store(position, std::memory_order_relaxed);
        T value = cell->value.exchange(T{}, std::memory_order_release);
        cell->sequence.store(position + N, std::memory_order_release);
        return std::optional<T>{std::move(value)};
    }

  private:
    struct Cell {
        std::atomic<std::size_t> sequence{0};
        Detail::TA_AtomicQueueValue<T> value{};
    };

    std::optional<T> snapshot(std::size_t position) const requires (!std::is_pointer_v<T>) {
        const Cell &cell = m_data[position % N];
        const std::size_t sequence = cell.sequence.load(std::memory_order_acquire);
        if (sequenceDifference(sequence, position + 1) != 0) {
            return std::nullopt;
        }

        T value = cell.value.load(std::memory_order_acquire);
        if (cell.sequence.load(std::memory_order_acquire) != sequence) {
            return std::nullopt;
        }
        return std::optional<T>{std::move(value)};
    }

    static std::intptr_t sequenceDifference(std::size_t lhs, std::size_t rhs) {
        return static_cast<std::intptr_t>(lhs) - static_cast<std::intptr_t>(rhs);
    }

    bool pushValue(T value) {
        return pushValueWithMetadata(std::move(value), [](std::size_t) noexcept {});
    }

    std::array<Cell, N> m_data{};
    std::atomic<std::size_t> m_frontIndex{0}, m_rearIndex{0};
};

} // namespace CoreAsync

#endif // TA_CIRCULARQUEUE_H
