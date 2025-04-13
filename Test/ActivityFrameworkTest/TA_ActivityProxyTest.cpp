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

#include "TA_ActivityProxyTest.h"
#include "Components/TA_ActivityProxy.h"
#include "Components/TA_MetaActivity.h"
#include "Components/TA_ThreadPool.h"
#include "ITA_ActivityCreator.h"

TA_ActivityProxyTest::TA_ActivityProxyTest()
{

}

TA_ActivityProxyTest::~TA_ActivityProxyTest()
{

}

void TA_ActivityProxyTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ActivityProxyTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ActivityProxyTest, MetaActivityTest)
{
    decltype(auto) pActivity = new CoreAsync::TA_MetaActivity<decltype(META_STRING("sub")), MetaTest *&, int, int>(META_STRING("sub"), m_pTest, 9, 8);
    CoreAsync::TA_ActivityProxy *pProxy = new CoreAsync::TA_ActivityProxy(pActivity);
    auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(pProxy);
    EXPECT_EQ(fetcher().get<int>(), 1);
}

TEST_F(TA_ActivityProxyTest, SingleActivityTest)
{
    decltype(auto) pActivity = new CoreAsync::TA_SingleActivity<decltype(&MetaTest::sub), MetaTest *, int, int ,int>(&MetaTest::sub, m_pTest, 9, 8);
    CoreAsync::TA_ActivityProxy *pProxy = new CoreAsync::TA_ActivityProxy(pActivity);
    auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(pProxy);
    EXPECT_EQ(fetcher().get<int>(), 1);
}

TEST_F(TA_ActivityProxyTest, LambdaActivityTest)
{
    std::function<int()> func = [&]()->int {return m_pTest->sub(5, 5); };
    auto activity = CoreAsync::ITA_ActivityCreator::create(func);
    CoreAsync::TA_ActivityProxy *pProxy = new CoreAsync::TA_ActivityProxy(activity);
    auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(pProxy);
    EXPECT_EQ(fetcher().get<int>(), 0);
}
