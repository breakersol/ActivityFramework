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

#include "TA_ConnectionTest.h"
#include "ITA_Connection.h"
#include "MetaTest.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <type_traits>
#include <vector>

TA_ConnectionTest::TA_ConnectionTest() {}

TA_ConnectionTest::~TA_ConnectionTest() {}

void TA_ConnectionTest::SetUp() { m_pTest = std::make_shared<MetaTest>(); }

void TA_ConnectionTest::TearDown() {}

TEST_F(TA_ConnectionTest, connectSyncTest) {
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    CoreAsync::ITA_Connection::disconnect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM);
}

TEST_F(TA_ConnectionTest, compileTimeConnectionTest) {
    EXPECT_TRUE((CoreAsync::ITA_Connection::connect<&MetaTest::startTest, &MetaTest::productMM>(
        m_pTest.get(), m_pTest.get())));
    EXPECT_FALSE((CoreAsync::ITA_Connection::connect<&MetaTest::startTest, &MetaTest::productMM>(
        m_pTest.get(), m_pTest.get())));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active<&MetaTest::startTest>(m_pTest.get(), 4, 7));
    EXPECT_TRUE((CoreAsync::ITA_Connection::disconnect<&MetaTest::startTest, &MetaTest::productMM>(
        m_pTest.get(), m_pTest.get())));
}

TEST_F(TA_ConnectionTest, compileTimeInheritedStorageTest) {
    EXPECT_TRUE((CoreAsync::ITA_Connection::connect<&TestA::print, &MetaTest::printTest>(
        m_pTest.get(), m_pTest.get())));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active<&TestA::print>(m_pTest.get()));
    EXPECT_TRUE((CoreAsync::ITA_Connection::disconnect<&TestA::print, &MetaTest::printTest>(
        m_pTest.get(), m_pTest.get())));
}

TEST_F(TA_ConnectionTest, compileTimeLambdaConnectionTest) {
    std::promise<int> resultPromise;
    auto result = resultPromise.get_future();
    auto holder = CoreAsync::ITA_Connection::connect<&MetaTest::startTest>(
        m_pTest.get(), [&resultPromise](int lhs, int rhs) { resultPromise.set_value(lhs + rhs); });
    ASSERT_TRUE(holder.valid());
    EXPECT_TRUE(CoreAsync::ITA_Connection::active<&MetaTest::startTest>(m_pTest.get(), 8, 9));
    ASSERT_EQ(result.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(result.get(), 17);
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(holder));
}

TEST_F(TA_ConnectionTest, sameTypeLambdaConnectionsRemainIndependent) {
    std::vector<int> results;
    std::mutex resultsMutex;
    std::condition_variable resultsChanged;
    auto makeHandler = [&](int tag) {
        return [&, tag](int lhs, int rhs) {
            {
                std::lock_guard lock(resultsMutex);
                results.push_back(tag + lhs + rhs);
            }
            resultsChanged.notify_one();
        };
    };

    auto firstHandler = makeHandler(10);
    auto secondHandler = makeHandler(20);
    static_assert(std::is_same_v<decltype(firstHandler), decltype(secondHandler)>);

    auto first = CoreAsync::ITA_Connection::connect<&MetaTest::startTest>(m_pTest.get(), std::move(firstHandler));
    auto second = CoreAsync::ITA_Connection::connect<&MetaTest::startTest>(m_pTest.get(), std::move(secondHandler));
    ASSERT_TRUE(first.valid());
    ASSERT_TRUE(second.valid());

    EXPECT_TRUE(CoreAsync::ITA_Connection::active<&MetaTest::startTest>(m_pTest.get(), 1, 2));
    {
        std::unique_lock lock(resultsMutex);
        ASSERT_TRUE(resultsChanged.wait_for(lock, std::chrono::seconds(1), [&] { return results.size() == 2; }));
        EXPECT_EQ(results[0], 13);
        EXPECT_EQ(results[1], 23);
    }

    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(first));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active<&MetaTest::startTest>(m_pTest.get(), 2, 3));
    {
        std::unique_lock lock(resultsMutex);
        ASSERT_TRUE(resultsChanged.wait_for(lock, std::chrono::seconds(1), [&] { return results.size() == 3; }));
        EXPECT_EQ(results[2], 25);
    }
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(second));
}

TEST_F(TA_ConnectionTest, connectAsyncTest) {
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Queued>(
        m_pTest.get(), &MetaTest::printTest, m_pTest.get(), &MetaTest::print));
    EXPECT_FALSE(
        CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::printTest, m_pTest.get(), &MetaTest::print));
    CoreAsync::ITA_Connection::disconnect(m_pTest.get(), &MetaTest::printTest, m_pTest.get(), &MetaTest::print);
}

TEST_F(TA_ConnectionTest, disconnectTest) {
    EXPECT_TRUE(
        CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(),
                                                      &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest.get(), &MetaTest::startTest, 2, 3));
}

TEST_F(TA_ConnectionTest, activeTest) {
    EXPECT_TRUE(
        CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest.get(), &MetaTest::startTest, 5, 5));
}

TEST_F(TA_ConnectionTest, asyncActiveTest) {
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Queued>(
        m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    // CoreAsync::TA_ThreadHolder::get().setThreadDetached(CoreAsync::TA_ThreadHolder::get().topPriorityThread());
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest.get(), &MetaTest::startTest, 6, 6));
}

TEST_F(TA_ConnectionTest, lambdaExpTest) {
    int c = 9;
    ConnectionHolder conn = CoreAsync::ITA_Connection::connect(
        m_pTest.get(), &MetaTest::startTest, [c](int a, int b) { std::printf("The numbers are: %d, %d, %d\n.", a, b, c); });
    EXPECT_TRUE(conn.valid());
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest.get(), &MetaTest::startTest, 8, 8));
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(conn));
}

TEST_F(TA_ConnectionTest, rejectUnregisteredSignalAndSlot) {
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(
        m_pTest.get(), &MetaTest::unregisteredTest, m_pTest.get(), &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(
        m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::unregisteredTest));

    ConnectionHolder conn = CoreAsync::ITA_Connection::connect(
        m_pTest.get(), &MetaTest::unregisteredTest, [](int, int) {});
    EXPECT_FALSE(conn.valid());
    EXPECT_FALSE(CoreAsync::ITA_Connection::active(m_pTest.get(), &MetaTest::unregisteredTest, 1, 2));
    EXPECT_FALSE(CoreAsync::ITA_Connection::disconnect(
        m_pTest.get(), &MetaTest::unregisteredTest, m_pTest.get(), &MetaTest::productMM));
}
