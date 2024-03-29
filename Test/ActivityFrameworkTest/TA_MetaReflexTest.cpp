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
    EXPECT_EQ("sum_1" == name,true);
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
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("sum_1"),m_pTest,1.5);
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
    auto res = CoreAsync::Reflex::TA_TypeInfo<M2Test>::invoke(META_STRING("mx"),m2);
    EXPECT_EQ(res == 5, true);
}

TEST_F(TA_MetaReflexTest, findEnum)
{
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("META_RED"));
    EXPECT_EQ(res == MetaTest::META_RED, true);
}

TEST_F(TA_MetaReflexTest, findMethodsTest)
{
    EXPECT_EQ(CoreAsync::Reflex::TA_TypeInfo<M2Test>::operations.size() == 1, true);
    auto op = CoreAsync::Reflex::TA_TypeInfo<M2Test>::findMethods<0>();
    M2Test m2;
    (m2.*std::get<1>(op))(10);
    auto res = (m2.*std::get<0>(op))();
    EXPECT_EQ(res == 10, true);
}
