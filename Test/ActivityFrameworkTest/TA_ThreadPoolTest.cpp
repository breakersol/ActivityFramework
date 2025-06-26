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

TA_ThreadPoolTest::TA_ThreadPoolTest() {}

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
