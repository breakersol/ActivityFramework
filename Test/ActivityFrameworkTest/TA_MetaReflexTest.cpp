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

#include "TA_MetaReflexTest.h"
#include "MetaTest.h"
#include "Components/TA_MetaReflex.h"

#include <iostream>

TA_MetaReflexTest::TA_MetaReflexTest()
{

}

TA_MetaReflexTest::~TA_MetaReflexTest()
{

}

void TA_MetaReflexTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_MetaReflexTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_MetaReflexTest, findMemberFunctionName)
{
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(static_cast<float(MetaTest::*)(float)const>(&MetaTest::Sum));
    EXPECT_EQ("Sum<float>" == name,true);
}

TEST_F(TA_MetaReflexTest, findNonMemberFunctionName)
{
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(&MetaTest::getStr);
    EXPECT_EQ("getStr" == name,true);
}

TEST_F(TA_MetaReflexTest, findMemberVariableName)
{
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<M2Test>::findName(&M2Test::mx);
    EXPECT_EQ("mx" == name,true);
}

TEST_F(TA_MetaReflexTest, findNonMemberVariableName)
{
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(&MetaTest::str);
    EXPECT_EQ("str" == name,true);
}

TEST_F(TA_MetaReflexTest, findEnumName)
{
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(MetaTest::META_RED);
    EXPECT_EQ("META_RED" == name,true);
}

TEST_F(TA_MetaReflexTest, findMemberFunction)
{
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("Sum<float>"),m_pTest,1.5);
    EXPECT_EQ(res == 4.5, true);
}

TEST_F(TA_MetaReflexTest, findNonMemberFunction)
{
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("getStr"), "123");
    EXPECT_EQ(res == "123123", true);
}

TEST_F(TA_MetaReflexTest, findNonMemberVariable)
{
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("str"));
    EXPECT_EQ(res == "123", true);
}

TEST_F(TA_MetaReflexTest, findMemberVariable)
{
    M2Test m2;
    auto res = CoreAsync::Reflex::TA_TypeInfo<M2Test>::invoke(META_STRING("px"),m2);
    EXPECT_EQ(*res == 10, true);
}

TEST_F(TA_MetaReflexTest, findEnum)
{
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("META_RED"));
    EXPECT_EQ(res == MetaTest::META_RED, true);
}

TEST_F(TA_MetaReflexTest, updatePrivateMember)
{
    M2Test m2;;
    CoreAsync::Reflex::TA_TypeInfo<M2Test>::update(m2, 111, META_STRING("px"));
    auto res = CoreAsync::Reflex::TA_TypeInfo<M2Test>::invoke(META_STRING("px"),m2);
    EXPECT_EQ(*res == 111, true);
}
