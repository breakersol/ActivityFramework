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

#include <thread>

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
    CoreAsync::TA_ActivityQueue<int, 10240> queue;
    EXPECT_EQ(10240, queue.capacity());
}

TEST_F(TA_ActivityQueueTest, getFront) {
    CoreAsync::TA_ThreadPool::QueueType queue;
    auto activity = CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, m_pTest, 6, 3);
#if defined(__ANDROID__)
    auto handle = new CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle{
        std::make_shared<CoreAsync::TA_ActivityProxy>(activity)};
    queue.push(handle);
    (*CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle::extractActivity(queue.front()))();
    CoreAsync::TA_ActivityResultFetcher fetcher{
        CoreAsync::TA_ThreadPool::PlatformSelector::ActivityHandle::extractActivity(queue.front())};
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
#else
    queue.push(std::make_shared<CoreAsync::TA_ActivityProxy>(activity));
#endif
    auto pActivity = queue.rear();
    EXPECT_EQ(pActivity, nullptr);
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
    CoreAsync::TA_ActivityQueue<int, 10240> queue;
    EXPECT_EQ(queue.isEmpty(), true);
}

TEST_F(TA_ActivityQueueTest, fullTest) {
    CoreAsync::TA_ActivityQueue<int, 10240> queue;
    for (int i = 0; i < queue.capacity(); ++i) {
        queue.push(i);
    }
    EXPECT_EQ(queue.isFull(), true);
}
