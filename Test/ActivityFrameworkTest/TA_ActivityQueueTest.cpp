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

#include "TA_ActivityQueueTest.h"
#include "Components/TA_Activity.h"
#include "Components/TA_ActivityQueue.h"
#include "Components/TA_CircularQueue.h"

#include <array>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

namespace {
template <typename Queue>
concept HasQueueObservers = requires(const Queue &queue) {
    queue.top();
    queue.front();
    queue.rear();
};

template <typename Queue>
concept HasTryPop = requires(Queue &queue) {
    queue.tryPop();
};

static_assert(HasQueueObservers<CoreAsync::TA_CircularQueue<int, 4>>);
static_assert(!HasQueueObservers<CoreAsync::TA_CircularQueue<int *, 4>>);
static_assert(!HasTryPop<CoreAsync::TA_CircularQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4>>);
static_assert(HasTryPop<CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4>>);
} // namespace

TA_ActivityQueueTest::TA_ActivityQueueTest() {}

TA_ActivityQueueTest::~TA_ActivityQueueTest() {}

void TA_ActivityQueueTest::SetUp() { m_pTest = new MetaTest(); }

void TA_ActivityQueueTest::TearDown() {
    if (m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ActivityQueueTest, capacityTest) {
    //    auto activity = CoreAsync::TA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 6,3);
    CoreAsync::TA_CircularQueue<int, 10240> queue;
    EXPECT_EQ(10240, queue.capacity());
}

TEST_F(TA_ActivityQueueTest, getFront) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    auto activity = CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, 6, 3);
#if defined(__ANDROID__)
    auto handle = new CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle{
        std::make_shared<CoreAsync::TA_ActivityProxy>(activity)};
    queue.push(handle);
    const auto poppedHandle = queue.pop();
    ASSERT_TRUE(poppedHandle.has_value());
    auto proxy = CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle::extractActivity(*poppedHandle);
    (*proxy)();
    CoreAsync::TA_ActivityResultFetcher fetcher{proxy};
#else
    queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(activity));
    (*queue.front())();
    CoreAsync::TA_ActivityResultFetcher fetcher{queue.front()};
#endif
    int res = fetcher().get<int>();
    EXPECT_EQ(3, res);
}

TEST_F(TA_ActivityQueueTest, getRear) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    auto activity = CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, 6, 3);
#if defined(__ANDROID__)
    auto handle = new CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle{
        std::make_shared<CoreAsync::TA_ActivityProxy>(activity)};
    queue.push(handle);
    const auto poppedHandle = queue.pop();
    ASSERT_TRUE(poppedHandle.has_value());
    CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle::extractActivity(*poppedHandle);
#else
    queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(activity));
    auto pActivity = queue.rear();
    EXPECT_EQ(pActivity, nullptr);
#endif
}

TEST_F(TA_ActivityQueueTest, multiThreadTest) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    std::function<bool()> func_1 = [&]() {
        for (int i = 0; i < 150; ++i) {
#if defined(__ANDROID__)
            auto handle = new CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle{
                std::make_shared<CoreAsync::TA_ActivityProxy>(
                    CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, i, 3))};
            queue.push(handle);
#else
            queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(
                CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, i, 3)));
#endif
        }
        return true;
    };

    std::function<bool()> func_2 = [&]() {
        for (int i = 0; i < 150; ++i) {
            std::string str{"321"};
#if defined(__ANDROID__)
            auto handle = new CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle{
                std::make_shared<CoreAsync::TA_ActivityProxy>(
                    CoreAsync::TA_ActivityCreator::create(&MetaTest::getStr, str))};
            queue.push(handle);
#else
            queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(
                CoreAsync::TA_ActivityCreator::create(&MetaTest::getStr, str)));
#endif
        }
        return true;
    };

    std::thread t1{func_1};
    std::thread t2{func_2};
    std::thread t3{func_1};

    t1.join();
    t2.join();
    t3.join();
}

TEST_F(TA_ActivityQueueTest, emptyTest) {
    CoreAsync::TA_CircularQueue<int, 10240> queue;
    EXPECT_EQ(queue.isEmpty(), true);
}

TEST_F(TA_ActivityQueueTest, fullTest) {
    CoreAsync::TA_CircularQueue<int, 10240> queue;
    for (int i = 0; i < queue.capacity(); ++i) {
        queue.push(i);
    }
    EXPECT_EQ(queue.isFull(), true);
}

TEST_F(TA_ActivityQueueTest, popPublishesAndReclaimsSlotsInOrder) {
    CoreAsync::TA_CircularQueue<int, 4> queue;

    EXPECT_FALSE(queue.pop().has_value());
    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_TRUE(queue.push(3));
    EXPECT_TRUE(queue.push(4));
    EXPECT_FALSE(queue.push(5));
    EXPECT_TRUE(queue.isFull());

    for (int expected = 1; expected <= 4; ++expected) {
        const auto value = queue.pop();
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(*value, expected);
    }

    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.pop().has_value());
}

TEST_F(TA_ActivityQueueTest, nonStealableActivityRemainsAvailableToOwner) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    auto activity = CoreAsync::TA_ActivityCreator::create([]() {});
    activity->setStolenEnabled(false);

#if defined(__ANDROID__)
    auto handle = new CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle{
        std::make_shared<CoreAsync::TA_ActivityProxy>(activity)};
    ASSERT_TRUE(queue.push(handle));
    EXPECT_FALSE(queue.tryPop().has_value());
    const auto ownerHandle = queue.pop();
    ASSERT_TRUE(ownerHandle.has_value());
    CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle::extractActivity(*ownerHandle);
#else
    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity);
    ASSERT_FALSE(proxy->stolenEnabled());
    ASSERT_TRUE(queue.push(proxy));
    EXPECT_FALSE(queue.tryPop().has_value());
    const auto ownerActivity = queue.pop();
    ASSERT_TRUE(ownerActivity.has_value());
    EXPECT_EQ(*ownerActivity, proxy);
#endif
}

TEST_F(TA_ActivityQueueTest, concurrentOwnerAndThiefClaimActivityOnce) {
    constexpr int iterationCount = 10000;
    CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4> queue;
    auto activity = CoreAsync::TA_ActivityCreator::create([]() {});
    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity);
    std::optional<std::shared_ptr<CoreAsync::TA_ActivityProxy>> ownerResult;
    std::optional<std::shared_ptr<CoreAsync::TA_ActivityProxy>> thiefResult;
    std::atomic_int iterationStarted{0};
    std::atomic_int ownerFinished{0};
    std::atomic_int thiefFinished{0};

    std::thread owner([&]() {
        for (int iteration = 1; iteration <= iterationCount; ++iteration) {
            while (iterationStarted.load(std::memory_order_acquire) < iteration) {
                std::this_thread::yield();
            }
            ownerResult = queue.pop();
            ownerFinished.store(iteration, std::memory_order_release);
        }
    });
    std::thread thief([&]() {
        for (int iteration = 1; iteration <= iterationCount; ++iteration) {
            while (iterationStarted.load(std::memory_order_acquire) < iteration) {
                std::this_thread::yield();
            }
            thiefResult = queue.tryPop();
            thiefFinished.store(iteration, std::memory_order_release);
        }
    });

    for (int iteration = 1; iteration <= iterationCount; ++iteration) {
        EXPECT_TRUE(queue.push(proxy));
        iterationStarted.store(iteration, std::memory_order_release);
        while (ownerFinished.load(std::memory_order_acquire) < iteration ||
               thiefFinished.load(std::memory_order_acquire) < iteration) {
            std::this_thread::yield();
        }

        EXPECT_NE(ownerResult.has_value(), thiefResult.has_value());
        if (ownerResult) {
            EXPECT_EQ(*ownerResult, proxy);
        }
        if (thiefResult) {
            EXPECT_EQ(*thiefResult, proxy);
        }
    }

    owner.join();
    thief.join();
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(TA_ActivityQueueTest, concurrentPushAndPopPublishesEachValueOnce) {
    constexpr int producerCount = 4;
    constexpr int consumerCount = 4;
    constexpr int valuesPerProducer = 5000;
    constexpr int valueCount = producerCount * valuesPerProducer;

    CoreAsync::TA_CircularQueue<int, 64> queue;
    std::array<std::atomic_uint, valueCount> seen{};
    std::atomic_int consumed{0};
    std::atomic_bool timedOut{false};
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{10};

    std::vector<std::thread> consumers;
    consumers.reserve(consumerCount);
    for (int idx = 0; idx < consumerCount; ++idx) {
        consumers.emplace_back([&]() {
            while (consumed.load(std::memory_order_relaxed) < valueCount) {
                if (std::chrono::steady_clock::now() >= deadline) {
                    timedOut.store(true, std::memory_order_relaxed);
                    return;
                }

                const auto value = queue.pop();
                if (!value.has_value()) {
                    std::this_thread::yield();
                    continue;
                }

                if (*value >= 0 && *value < valueCount) {
                    seen[static_cast<std::size_t>(*value)].fetch_add(1, std::memory_order_relaxed);
                }
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> producers;
    producers.reserve(producerCount);
    for (int producer = 0; producer < producerCount; ++producer) {
        producers.emplace_back([&, producer]() {
            const int firstValue = producer * valuesPerProducer;
            for (int offset = 0; offset < valuesPerProducer; ++offset) {
                const int value = firstValue + offset;
                while (!queue.push(value)) {
                    if (timedOut.load(std::memory_order_relaxed)) {
                        return;
                    }
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto &producer : producers) {
        producer.join();
    }
    for (auto &consumer : consumers) {
        consumer.join();
    }

    EXPECT_FALSE(timedOut.load(std::memory_order_relaxed));
    EXPECT_EQ(consumed.load(std::memory_order_relaxed), valueCount);
    for (const auto &count : seen) {
        EXPECT_EQ(count.load(std::memory_order_relaxed), 1U);
    }
    EXPECT_TRUE(queue.isEmpty());
}
