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

ConnectionHolder conn {};

TA_ConnectionTest::TA_ConnectionTest() {}

TA_ConnectionTest::~TA_ConnectionTest() {}

void TA_ConnectionTest::SetUp() { m_pTest = std::make_shared<MetaTest>(); }

void TA_ConnectionTest::TearDown() {}

TEST_F(TA_ConnectionTest, connectSyncTest) {
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM));
    CoreAsync::ITA_Connection::disconnect(m_pTest.get(), &MetaTest::startTest, m_pTest.get(), &MetaTest::productMM);
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
    conn = CoreAsync::ITA_Connection::connect(
        m_pTest.get(), &MetaTest::startTest, [c](int a, int b) { std::printf("The numbers are: %d, %d, %d\n.", a, b, c); });
    EXPECT_TRUE(conn.valid());
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest.get(), &MetaTest::startTest, 8, 8));
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(conn));
}
