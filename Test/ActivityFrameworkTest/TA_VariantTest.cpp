#include "TA_VariantTest.h"
#include "Components/TA_Variant.h"

TA_VariantTest::TA_VariantTest()
{

}

TA_VariantTest::~TA_VariantTest()
{

}

void TA_VariantTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_VariantTest::TearDown()
{
    if(m_pTest)
    {
        delete m_pTest;
        m_pTest = nullptr;
    }
}

TEST_F(TA_VariantTest, constructTest)
{
    CoreAsync::TA_Variant var{m_pTest};
    auto res = var.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, setTest)
{
    CoreAsync::TA_Variant var{};
    var.set(m_pTest);
    auto res = var.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, validTest)
{
    CoreAsync::TA_Variant var{};
    EXPECT_EQ(var.isValid(), false);
    var.set(m_pTest);
    EXPECT_EQ(var.isValid(), true);
}

TEST_F(TA_VariantTest, assignmentOperatorTest)
{
    CoreAsync::TA_Variant var_1{m_pTest};
    CoreAsync::TA_Variant var2 = var_1;
    auto res = var2.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, copyTest)
{
    CoreAsync::TA_Variant var_1{m_pTest};
    CoreAsync::TA_Variant var2 {var_1};
    auto res = var2.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}
