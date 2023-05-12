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
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,n);
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(-1,res);
}

TEST_F(TA_ActivityTest, createMemberFunctionActivityWithNormalObject)
{
    constexpr int m {3}, n {4};
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, *m_pTest, m,n);
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(-1,res);
}

TEST_F(TA_ActivityTest, createMemberFunctionActivityWithSharedPtr)
{
    constexpr int m {3}, n {4};
    std::shared_ptr<MetaTest> pTest {m_pTest};
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, pTest, m,n);
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(-1,res);
    m_pTest = nullptr;
}

TEST_F(TA_ActivityTest, createActivityWithLeftVal)
{
    constexpr int m {3}, n {4};
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,n);
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(-1,res);
}

TEST_F(TA_ActivityTest, createActivityWithRightVal)
{
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 3, 4);
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(-1,res);
}

TEST_F(TA_ActivityTest, createActivityWithMixedVal)
{
    int m = 5;
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m, 4);
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(1,res);
}

TEST_F(TA_ActivityTest, modifyArguments_1)
{
    int m {3}, n {4};
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,n);
    m = 5;
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(1,res);
}

TEST_F(TA_ActivityTest, modifyArguments_2)
{
    int m {3};
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, m,4);
    m = 5;
    CoreAsync::TA_Variant var = acivity->execute();
    int res = var.get<int>();
    EXPECT_EQ(-1,res);
}

TEST_F(TA_ActivityTest, createNonMemberFunctionActivity)
{
    std::string str {"321"};
    auto activity = CoreAsync::ITA_ActivityCreator::create(&MetaTest::getStr,str);
    auto var = activity->execute();
    auto res = var.get<std::string>();
    EXPECT_EQ("123321",res);
}

TEST_F(TA_ActivityTest, createLambdaActivity)
{
    std::function<int(int,int)> func = [&](int m,int n)->int {return m_pTest->sub(m,n);};
    auto activity = CoreAsync::ITA_ActivityCreator::create(func,5,7);
    auto var = activity->execute();
    auto res = var.get<int>();
    EXPECT_EQ(-2,res);
}

TEST_F(TA_ActivityTest, createLambdaWithoutArgActivity)
{

    std::function<int()> func = [&]()->int {return m_pTest->sub(5,5);};
    auto activity = CoreAsync::ITA_ActivityCreator::create(func);
    auto var = activity->execute();
    auto res = var.get<int>();
    EXPECT_EQ(0,res);
}

TEST_F(TA_ActivityTest, linkAnotherActivity)
{

    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(2,1);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    activity_1->link(activity_2);
    auto var = activity_1->execute();
    auto res = var.get<int>();
    EXPECT_EQ(1,res);
}

TEST_F(TA_ActivityTest, parallelAnotherActivity)
{

    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(8,1);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    activity_1->parallel(activity_2);
    auto var = activity_1->execute();
    auto res = var.get<int>();
    EXPECT_EQ(7,res);
}

TEST_F(TA_ActivityTest, switchActivityBranch)
{

    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(1,2);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    std::function<int()> func_3 = [&]()->int {return m_pTest->sub(99,1);};
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create(func_3);
    std::function<int()> func_4 = [&]()->int {return m_pTest->sub(58,33);};
    auto activity_4 = CoreAsync::ITA_ActivityCreator::create(func_4);
    std::function<std::string()> func_5 = [&]()->std::string {return MetaTest::getStr("321");};
    auto activity_5 = CoreAsync::ITA_ActivityCreator::create(func_5);

    activity_3->branch(activity_4,activity_5);
    activity_1->branch(activity_2,activity_3);

    activity_1->selectBranch({2,2});
    auto var = activity_1->execute();
    auto res = var.get<std::string>();
    EXPECT_EQ("123321",res);
}
