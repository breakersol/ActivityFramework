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
#include <functional>
#include <thread>
#include <vector>

namespace {
template <typename Queue>
concept HasQueueObservers = requires(const Queue &queue) {
    queue.front();
    queue.rear();
};

template <typename Queue>
concept HasTop = requires(const Queue &queue) {
    queue.top();
};

template <typename Queue>
concept HasTryPop = requires(Queue &queue) {
    queue.tryPop();
};

static_assert(HasQueueObservers<CoreAsync::TA_CircularQueue<int, 4>>);
static_assert(!HasQueueObservers<CoreAsync::TA_CircularQueue<int *, 4>>);
static_assert(!HasTop<CoreAsync::TA_CircularQueue<int, 4>>);
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
    queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(activity));
    const auto front = queue.front();
    ASSERT_TRUE(front.has_value());
    const auto popped = queue.pop();
    ASSERT_TRUE(popped.has_value());
    EXPECT_EQ(*popped, *front);
    (**popped)();
    CoreAsync::TA_ActivityResultFetcher fetcher{*popped};
    int res = fetcher().get<int>();
    EXPECT_EQ(3, res);
}

TEST_F(TA_ActivityQueueTest, getRear) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    auto activity = CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, 6, 3);
    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity);
    queue.push(proxy);
    const auto rear = queue.rear();
    ASSERT_TRUE(rear.has_value());
    EXPECT_EQ(*rear, proxy);
}

TEST_F(TA_ActivityQueueTest, multiThreadTest) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    std::function<bool()> func_1 = [&]() {
        for (int i = 0; i < 150; ++i) {
            queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(
                CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, i, 3)));
        }
        return true;
    };

    std::function<bool()> func_2 = [&]() {
        for (int i = 0; i < 150; ++i) {
            std::string str{"321"};
            queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(
                CoreAsync::TA_ActivityCreator::create(&MetaTest::getStr, str)));
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

TEST_F(TA_ActivityQueueTest, observerSnapshotsHandleEmptyAndWraparound) {
    CoreAsync::TA_CircularQueue<int, 2> queue;

    EXPECT_FALSE(queue.front().has_value());
    EXPECT_FALSE(queue.rear().has_value());

    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_EQ(queue.front(), 1);
    EXPECT_EQ(queue.rear(), 2);

    EXPECT_EQ(queue.pop(), 1);
    EXPECT_EQ(queue.front(), 2);
    EXPECT_EQ(queue.rear(), 2);
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_FALSE(queue.front().has_value());
    EXPECT_FALSE(queue.rear().has_value());

    EXPECT_TRUE(queue.push(3));
    EXPECT_EQ(queue.front(), 3);
    EXPECT_EQ(queue.rear(), 3);
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

// Exercise both consumer clearing and immediate producer reuse of the observed cell.
TEST_F(TA_ActivityQueueTest, concurrentFrontPopAndReuseNeverReturnsWrongGeneration) {
    constexpr int iterationCount = 20000;
    CoreAsync::TA_CircularQueue<int, 2> queue;
    std::optional<int> frontResult = std::nullopt;
    std::optional<int> rearResult = std::nullopt;
    std::optional<int> popResult = std::nullopt;
    std::atomic_int iterationStarted = {0};
    std::atomic_int observerFinished = {0};
    std::atomic_int popFinished = {0};
    std::atomic_int producerFinished = {0};

    std::thread observer = std::thread([&]() {
        for (int iteration = 1; iteration <= iterationCount; ++iteration) {
            while (iterationStarted.load(std::memory_order_acquire) < iteration) {
                std::this_thread::yield();
            }
            frontResult = queue.front();
            rearResult = queue.rear();
            observerFinished.store(iteration, std::memory_order_release);
        }
    });
    std::thread consumer = std::thread([&]() {
        for (int iteration = 1; iteration <= iterationCount; ++iteration) {
            while (iterationStarted.load(std::memory_order_acquire) < iteration) {
                std::this_thread::yield();
            }
            popResult = queue.pop();
            popFinished.store(iteration, std::memory_order_release);
        }
    });
    std::thread producer = std::thread([&]() {
        for (int iteration = 1; iteration <= iterationCount; ++iteration) {
            while (iterationStarted.load(std::memory_order_acquire) < iteration) {
                std::this_thread::yield();
            }
            const int value = iteration * 3;
            while (!queue.push(value)) {
                std::this_thread::yield();
            }
            producerFinished.store(iteration, std::memory_order_release);
        }
    });

    for (int iteration = 1; iteration <= iterationCount; ++iteration) {
        const int frontValue = iteration * 3 - 2;
        const int secondValue = iteration * 3 - 1;
        EXPECT_TRUE(queue.push(frontValue));
        EXPECT_TRUE(queue.push(secondValue));
        iterationStarted.store(iteration, std::memory_order_release);
        while (observerFinished.load(std::memory_order_acquire) < iteration ||
               popFinished.load(std::memory_order_acquire) < iteration ||
               producerFinished.load(std::memory_order_acquire) < iteration) {
            std::this_thread::yield();
        }

        EXPECT_TRUE(popResult.has_value());
        if (popResult.has_value()) {
            EXPECT_EQ(*popResult, frontValue);
        }
        if (frontResult.has_value()) {
            EXPECT_TRUE(*frontResult == frontValue || *frontResult == secondValue);
        }
        if (rearResult.has_value()) {
            EXPECT_TRUE(*rearResult == secondValue || *rearResult == iteration * 3);
        }

        const auto secondResult = queue.pop();
        EXPECT_TRUE(secondResult.has_value());
        if (secondResult.has_value()) {
            EXPECT_EQ(*secondResult, secondValue);
        }
        const auto producerResult = queue.pop();
        EXPECT_TRUE(producerResult.has_value());
        if (producerResult.has_value()) {
            EXPECT_EQ(*producerResult, iteration * 3);
        }
    }

    observer.join();
    consumer.join();
    producer.join();
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(TA_ActivityQueueTest, nonStealableActivityRemainsAvailableToOwner) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    auto activity = CoreAsync::TA_ActivityCreator::create([]() {});
    activity->setStolenEnabled(false);

    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity);
    ASSERT_FALSE(proxy->stolenEnabled());
    ASSERT_TRUE(queue.push(proxy));
    const auto stolenActivity = queue.tryPop();
    EXPECT_FALSE(stolenActivity.has_value());
    const auto ownerActivity = stolenActivity.has_value() ? stolenActivity : queue.pop();
    ASSERT_TRUE(ownerActivity.has_value());
    EXPECT_EQ(*ownerActivity, proxy);
}

TEST_F(TA_ActivityQueueTest, enqueueSealsConfigurationAndRejectsDuplicatePush) {
    CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4> queue;
    auto activity = CoreAsync::TA_ActivityCreator::create([](int value) { return value; }, 1);
    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity);

    EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Configuring);
    EXPECT_TRUE(activity->setPara(2));
    EXPECT_TRUE(queue.push(proxy));
    EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Queued);

    EXPECT_FALSE(activity->setPara(3));
    EXPECT_FALSE(activity->setStolenEnabled(false));
    EXPECT_TRUE(activity->stolenEnabled());
    EXPECT_FALSE(queue.push(proxy));
    EXPECT_THROW((*proxy)(), std::logic_error);
    EXPECT_FALSE(proxy->isExecuted());

    const auto queuedActivity = queue.pop();
    ASSERT_TRUE(queuedActivity.has_value());
    EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Running);
    (**queuedActivity)();
    EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Completed);
    EXPECT_EQ((*queuedActivity)->result().get<int>(), 2);
    EXPECT_FALSE(queue.push(proxy));
}

TEST_F(TA_ActivityQueueTest, failedEnqueueRestoresConfiguringState) {
    CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 2> queue;
    auto first = std::make_shared<CoreAsync::TA_ActivityProxy>(CoreAsync::TA_ActivityCreator::create([]() {}));
    auto second = std::make_shared<CoreAsync::TA_ActivityProxy>(CoreAsync::TA_ActivityCreator::create([]() {}));
    auto rejectedActivity = CoreAsync::TA_ActivityCreator::create([]() {});
    auto rejected = std::make_shared<CoreAsync::TA_ActivityProxy>(rejectedActivity);

    ASSERT_TRUE(queue.push(first));
    ASSERT_TRUE(queue.push(second));
    EXPECT_FALSE(queue.push(rejected));
    EXPECT_EQ(rejectedActivity->state(), CoreAsync::TA_ActivityState::Configuring);
    EXPECT_TRUE(rejectedActivity->setStolenEnabled(false));
}

TEST_F(TA_ActivityQueueTest, submissionTokenCannotPublishAnotherActivity) {
    CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4> queue;
    auto firstActivity = CoreAsync::TA_ActivityCreator::create([]() {});
    auto secondActivity = CoreAsync::TA_ActivityCreator::create([]() {});
    auto first = std::make_shared<CoreAsync::TA_ActivityProxy>(firstActivity);
    auto second = std::make_shared<CoreAsync::TA_ActivityProxy>(secondActivity);
    auto submission = first->prepareSubmission();

    ASSERT_TRUE(submission.has_value());
    EXPECT_FALSE(queue.push(second, std::move(*submission)));
    EXPECT_EQ(firstActivity->state(), CoreAsync::TA_ActivityState::Configuring);
    EXPECT_EQ(secondActivity->state(), CoreAsync::TA_ActivityState::Configuring);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(TA_ActivityQueueTest, concurrentDuplicatePushPublishesActivityOnce) {
    CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4> queue;
    auto activity = CoreAsync::TA_ActivityCreator::create([]() {});
    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity);
    std::atomic_int ready = {0};
    std::atomic_bool start = {false};
    bool firstResult = false;
    bool secondResult = false;

    const auto push = [&](bool &result) {
        ready.fetch_add(1, std::memory_order_release);
        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        result = queue.push(proxy);
    };
    std::thread first = std::thread(push, std::ref(firstResult));
    std::thread second = std::thread(push, std::ref(secondResult));

    while (ready.load(std::memory_order_acquire) != 2) {
        std::this_thread::yield();
    }
    start.store(true, std::memory_order_release);
    first.join();
    second.join();

    EXPECT_NE(firstResult, secondResult);
    EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Queued);
    EXPECT_TRUE(queue.pop().has_value());
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(TA_ActivityQueueTest, concurrentOwnerAndThiefClaimActivityOnce) {
    constexpr int iterationCount = 10000;
    CoreAsync::TA_ActivityQueue<std::shared_ptr<CoreAsync::TA_ActivityProxy>, 4> queue;
    std::vector<std::shared_ptr<CoreAsync::TA_ActivityProxy>> proxies;
    proxies.reserve(iterationCount);
    for (int iteration = 0; iteration < iterationCount; ++iteration) {
        proxies.emplace_back(
            std::make_shared<CoreAsync::TA_ActivityProxy>(CoreAsync::TA_ActivityCreator::create([]() {})));
    }
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
        const auto &proxy = proxies[static_cast<std::size_t>(iteration - 1)];
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
