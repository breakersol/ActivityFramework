/*
 * Copyright [2024] [Shuang Zhu / Sol]
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

TA_ConnectionTest::TA_ConnectionTest()
{

}

TA_ConnectionTest::~TA_ConnectionTest()
{

}

void TA_ConnectionTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ConnectionTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ConnectionTest, connectSyncTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    CoreAsync::ITA_Connection::disconnect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM);
}

TEST_F(TA_ConnectionTest, connectAsyncTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Queued>(m_pTest, &MetaTest::printTest, m_pTest, &MetaTest::print));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::printTest, m_pTest, &MetaTest::print));
    CoreAsync::ITA_Connection::disconnect(m_pTest, &MetaTest::printTest, m_pTest, &MetaTest::print);
}

TEST_F(TA_ConnectionTest, disconnectTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::active(m_pTest,&MetaTest::startTest,2,3));
}

TEST_F(TA_ConnectionTest, activeTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest,&MetaTest::startTest,5,5));
}

TEST_F(TA_ConnectionTest, asyncActiveTest)
{
    MetaTest *pTempTest = new MetaTest();
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Queued>(pTempTest, &MetaTest::startTest, pTempTest, &MetaTest::productMM));
    //CoreAsync::TA_ThreadHolder::get().setThreadDetached(CoreAsync::TA_ThreadHolder::get().topPriorityThread());
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(pTempTest,&MetaTest::startTest, 6, 6));
}
