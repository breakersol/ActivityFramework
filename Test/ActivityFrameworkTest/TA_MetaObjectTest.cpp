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

#include "TA_MetaObjectTest.h"
#include "MetaTest.h"
#include "Components/TA_MetaObject.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <cstdint>
#include <thread>
#include <utility>
#include <vector>

TA_MetaObjectTest::TA_MetaObjectTest() {}

TA_MetaObjectTest::~TA_MetaObjectTest() {}

void TA_MetaObjectTest::SetUp() {}

void TA_MetaObjectTest::TearDown() {}

TEST_F(TA_MetaObjectTest, InvokeMethodTest) {
    auto fetcher = CoreAsync::TA_MetaObject::invokeMethod(META_STRING("contains<int>"), m_pMetaTest, 3);
    EXPECT_TRUE(fetcher().get<bool>());
    auto fetcher_2 = CoreAsync::TA_MetaObject::invokeMethod([](int a) { std::cout << a << std::endl; return a;}, 1);
    EXPECT_EQ(fetcher_2().get<int>(), 1);
    auto fetcher_3 = CoreAsync::TA_MetaObject::invokeMethod(&MetaTest::productMM, m_pMetaTest, 2, 5);
    fetcher_3();
    auto fetcher_4 = CoreAsync::TA_MetaObject::invokeMethod(&MetaTest::printStr, m_str);
    fetcher_4();
}

TEST_F(TA_MetaObjectTest, UniqueIdsBelongToInstances) {
    CoreAsync::TA_MetaObject first;
    CoreAsync::TA_MetaObject second(first);
    CoreAsync::TA_MetaObject third(std::move(first));
    EXPECT_NE(first.id(), 0u);
    EXPECT_NE(first.id(), CoreAsync::TA_UniqueId{}.id());
    EXPECT_NE(first.id(), second.id());
    EXPECT_NE(first.id(), third.id());
    EXPECT_NE(second.id(), third.id());

    const auto secondId = second.id();
    second = third;
    EXPECT_EQ(second.id(), secondId);
    second = std::move(third);
    EXPECT_EQ(second.id(), secondId);
}

TEST_F(TA_MetaObjectTest, UniqueIdsAreDistinctAcrossThreads) {
    constexpr std::size_t count = 256;
    std::vector<std::uint64_t> ids(count);
    std::vector<std::thread> threads;
    for (std::size_t worker = 0; worker < 4; ++worker) {
        threads.emplace_back([&, worker] {
            for (std::size_t i = worker; i < count; i += 4)
                ids[i] = CoreAsync::TA_UniqueId{}.id();
        });
    }
    for (auto &thread : threads)
        thread.join();
    std::sort(ids.begin(), ids.end());
    EXPECT_NE(ids.front(), 0u);
    EXPECT_EQ(std::adjacent_find(ids.begin(), ids.end()), ids.end());
}
