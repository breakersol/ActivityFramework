#include "TA_BasePipelineTest.h"
#include "ITA_ActivityCreator.h"
#include "ITA_PipelineCreator.h"

TA_BasePipelineTest::TA_BasePipelineTest()
{

}

TA_BasePipelineTest::~TA_BasePipelineTest()
{

}

void TA_BasePipelineTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_BasePipelineTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_BasePipelineTest, activitySizeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);
    std::size_t size = line->activitySize();

    EXPECT_EQ(3,size);
}

TEST_F(TA_BasePipelineTest, removeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);

    line->remove(1);

    std::size_t size {line->activitySize()};
    EXPECT_EQ(2,size);

    line->execute();
    int res_0, res_1;
    if(line->waitingComplete())
    {
        line->result(0,res_0);
        line->result(1,res_1);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(9,res_1);
}

TEST_F(TA_BasePipelineTest, resetTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);
    line->execute();
    int res_0, res_1, res_2;
    if(line->waitingComplete())
    {
        line->result(0,res_0);
        line->result(1,res_1);
        line->result(2,res_2);

        EXPECT_EQ(-1,res_0);
        EXPECT_EQ(0,res_1);
        EXPECT_EQ(9,res_2);
    }
    line->reset();
    line->result(0,res_0);
    line->result(1,res_1);
    line->result(2,res_2);

    EXPECT_EQ(0,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(0,res_2);
}

TEST_F(TA_BasePipelineTest, clearTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);

    line->clear();

    EXPECT_EQ(0,line->activitySize());
}

TEST_F(TA_BasePipelineTest, switchBranchTest)
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

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1);
    line->switchActivityBranch(0,{2,2});

    line->execute();
    std::string res {};
    if(line->waitingComplete())
    {
        line->result(0,res);
    }
    EXPECT_EQ("123321",res);
}

TEST_F(TA_BasePipelineTest, stateTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    EXPECT_EQ(CoreAsync::TA_PipelineCreator::AutoChainHolder::PipelineState::Waiting, line->state());
    line->add(activity_1,activity_2,activity_3);
    line->execute();
    EXPECT_EQ(CoreAsync::TA_PipelineCreator::AutoChainHolder::PipelineState::Busy, line->state());
    if(line->waitingComplete())
    {
        EXPECT_EQ(CoreAsync::TA_PipelineCreator::AutoChainHolder::PipelineState::Ready, line->state());
    }
}

TEST_F(TA_BasePipelineTest, startIndexTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);
    line->setStartIndex(1);
    line->execute();
    int res_0, res_1, res_2;
    if(line->waitingComplete())
    {
        line->result(0,res_0);
        line->result(1,res_1);
        line->result(2,res_2);
    }
    EXPECT_EQ(0,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(9,res_2);
}
