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

#include "TA_ActivityTest.h"
#include "ITA_ActivityCreator.h"

TA_ActivityTest::TA_ActivityTest()
{

}

TA_ActivityTest::~TA_ActivityTest()
{

}

void TA_ActivityTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ActivityTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ActivityTest, createMemberFunctionActivityWithPointer)
{
    constexpr int m {3}, n {4};
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,n);
    decltype(auto) var = (*activity)();
    EXPECT_EQ(-1,var);
}

TEST_F(TA_ActivityTest, createMemberFunctionActivityWithNormalObject)
{
    constexpr int m {3}, n {4};
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, *m_pTest, m,n);
    decltype(auto) var = (*activity)();
    EXPECT_EQ(-1,var);
}

TEST_F(TA_ActivityTest, createMemberFunctionActivityWithSharedPtr)
{
    constexpr int m {3}, n {4};
    std::shared_ptr<MetaTest> pTest {m_pTest};
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, pTest, m,n);
    decltype(auto) var = (*activity)();
    EXPECT_EQ(-1,var);
    m_pTest = nullptr;
}

TEST_F(TA_ActivityTest, createActivityWithLeftVal)
{
    constexpr int m {3}, n {4};
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,n);
    decltype(auto) var = (*activity)();
    EXPECT_EQ(-1,var);
}

TEST_F(TA_ActivityTest, createActivityWithRightVal)
{
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 3, 4);
    decltype(auto) var = (*activity)();
    EXPECT_EQ(-1,var);
}

TEST_F(TA_ActivityTest, createActivityWithMixedVal)
{
    int m = 5;
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m, 4);
    decltype(auto) var = (*activity)();
    EXPECT_EQ(1,var);
}

TEST_F(TA_ActivityTest, modifyArguments_1)
{
    int m {3}, n {4};
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,n);
    m = 5;
    decltype(auto) var = (*activity)();
    EXPECT_EQ(1,var);
}

TEST_F(TA_ActivityTest, modifyArguments_2)
{
    int m {3};
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,4);
    m = 5;
    decltype(auto) var = (*activity)();
    EXPECT_EQ(-1,var);
}

TEST_F(TA_ActivityTest, createNonMemberFunctionActivity)
{
    std::string str {"321"};
    auto activity = CoreAsync::ITA_ActivityCreator::create(&MetaTest::getStr,str);
    auto var = (*activity)();
    EXPECT_EQ("123321",var);
}

TEST_F(TA_ActivityTest, createLambdaActivity)
{
    std::function<int(int,int)> func = [&](int m,int n)->int {return m_pTest->sub(m,n);};
    auto activity = CoreAsync::ITA_ActivityCreator::create(func,5,7);
    auto var = (*activity)();
    EXPECT_EQ(-2,var);
}

TEST_F(TA_ActivityTest, createLambdaWithoutArgActivity)
{

    std::function<int()> func = [&]()->int {return m_pTest->sub(5,5);};
    auto activity = CoreAsync::ITA_ActivityCreator::create(func);
    auto var = (*activity)();
    EXPECT_EQ(0,var);
}

TEST_F(TA_ActivityTest, createMetaActivity)
{
    auto activity = CoreAsync::ITA_ActivityCreator::create(META_STRING("sub"), m_pTest, 3, 9);
    auto var = (*activity)();
    delete activity;
    EXPECT_EQ(var,-6);

    std::shared_ptr<MetaTest> pSharedTest = std::make_shared<MetaTest>();
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create(META_STRING("sub"), pSharedTest, 3, 9);
    auto var_3 = (*activity_3)();
    delete activity_3;
    EXPECT_EQ(var_3,-6);

    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(META_STRING("getStr"), MetaTest {}, "321");
    auto var_2 = (*activity_2)();
    delete activity_2;
    EXPECT_EQ(var_2, "123321");

    std::unique_ptr<MetaTest> pUniqueTest = std::make_unique<MetaTest>();
    auto activity_4 = CoreAsync::ITA_ActivityCreator::create(META_STRING("sub"), pUniqueTest, 3, 9);
    auto var_4 = (*activity_4)();
    delete activity_4;
    EXPECT_EQ(var_4,-6);

}
