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

#include "TA_ThreadPoolTest.h"
#include "Components/TA_Activity.h"

#include <atomic>
#include <chrono>
#include <semaphore>
#include <thread>

TA_ThreadPoolTest::TA_ThreadPoolTest() {
    activities.fill(nullptr);
}


TA_ThreadPoolTest::~TA_ThreadPoolTest() {}

void TA_ThreadPoolTest::SetUp() {
    for (int i = 0; i < activities.size(); ++i) {
        auto activity = CoreAsync::TA_ActivityCreator::create(
            [](int a) {
                //        std::printf("%d\n", a);
                //        std::printf("%d has completed an activity!\n", std::this_thread::get_id());
                return a;
            },
            std::move(i));
        activities[i] = new CoreAsync::TA_ActivityProxy(activity);
    }
}

void TA_ThreadPoolTest::TearDown() {
    for (auto &activity : activities) {
        if (activity) {
            delete activity;
            activity = nullptr;
        }
    }
}

TEST_F(TA_ThreadPoolTest, postActivityTest) {
    auto ft = CoreAsync::TA_ThreadHolder::get().postActivity(activities[0]);
    EXPECT_EQ(0, ft().get<int>());
}

TEST_F(TA_ThreadPoolTest, notifyResultTest) {
    std::vector<CoreAsync::TA_ActivityResultFetcher> testVec;
    std::vector<int> validVec(1024);
    for (int i = 0; i < activities.size(); ++i) {
        testVec.emplace_back(CoreAsync::TA_ThreadHolder::get().postActivity(activities[i]));
        validVec[i] = i;
    }
    EXPECT_EQ(testVec.size(), validVec.size());
    for (int i = 0; i < testVec.size(); ++i) {
        EXPECT_EQ(testVec[i]().get<int>(), validVec[i]);
    }
}

TEST_F(TA_ThreadPoolTest, threadSizeTest) {
    EXPECT_EQ(std::thread::hardware_concurrency(), CoreAsync::TA_ThreadHolder::get().size());
}

TEST_F(TA_ThreadPoolTest, nonStealableActivityRemainsOnItsAffinityThread) {
    CoreAsync::TA_ThreadPool pool{2};
    std::binary_semaphore ownerStarted{0};
    std::binary_semaphore releaseOwner{0};
    std::atomic_bool protectedActivityRan{false};
    std::atomic_bool ranOnWrongThread{false};

    auto blocker = CoreAsync::TA_ActivityCreator::create([&]() {
        ownerStarted.release();
        releaseOwner.acquire();
    });
    blocker->moveToThread(1);
    blocker->setStolenEnabled(false);
    auto blockerResult = pool.postActivity(blocker, true);
    if (!ownerStarted.try_acquire_for(std::chrono::seconds{1})) {
        releaseOwner.release();
        blockerResult();
        FAIL() << "Affinity worker did not start the blocking activity";
    }

    const std::thread::id affinityThread = pool.threadId(1);
    auto protectedActivity = CoreAsync::TA_ActivityCreator::create([&]() {
        ranOnWrongThread.store(std::this_thread::get_id() != affinityThread, std::memory_order_relaxed);
        protectedActivityRan.store(true, std::memory_order_release);
    });
    protectedActivity->moveToThread(1);
    protectedActivity->setStolenEnabled(false);
    auto protectedResult = pool.postActivity(protectedActivity, true);

    auto thiefTrigger = CoreAsync::TA_ActivityCreator::create([]() { return true; });
    thiefTrigger->moveToThread(0);
    auto triggerResult = pool.postActivity(thiefTrigger, true);
    EXPECT_TRUE(triggerResult().get<bool>());

    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    EXPECT_FALSE(protectedActivityRan.load(std::memory_order_acquire));

    releaseOwner.release();
    blockerResult();
    protectedResult();

    EXPECT_TRUE(protectedActivityRan.load(std::memory_order_acquire));
    EXPECT_FALSE(ranOnWrongThread.load(std::memory_order_relaxed));
}
