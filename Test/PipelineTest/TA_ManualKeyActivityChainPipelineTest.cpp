#include "TA_ManualKeyActivityChainPipelineTest.h"
#include "ITA_ActivityCreator.h"
#include "ITA_PipelineCreator.h"

TA_ManualKeyActivityChainPipelineTest::TA_ManualKeyActivityChainPipelineTest()
{

}

TA_ManualKeyActivityChainPipelineTest::~TA_ManualKeyActivityChainPipelineTest()
{

}

void TA_ManualKeyActivityChainPipelineTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ManualKeyActivityChainPipelineTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ManualKeyActivityChainPipelineTest, executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 5,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createManualKeyActivityChainPipeline();
    line->add(activity_1,activity_2,activity_3);
    line->setKeyActivityIndex(1);
    line->execute();
    if(line->waitingComplete())
        line->execute();
    if(line->waitingComplete())
        line->execute();
    int res_0, res_1, res_2;
    if(line->waitingComplete())
    {
        line->result(0,res_0);
        line->result(1,res_1);
        line->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(3,res_1);
    EXPECT_EQ(0,res_2);
}
