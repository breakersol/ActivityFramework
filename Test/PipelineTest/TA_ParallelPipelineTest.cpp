#include "TA_ParallelPipelineTest.h"
#include "ITA_ActivityCreator.h"
#include "ITA_PipelineCreator.h"

TA_ParallelPipelineTest::TA_ParallelPipelineTest()
{

}

TA_ParallelPipelineTest::~TA_ParallelPipelineTest()
{

}

void TA_ParallelPipelineTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ParallelPipelineTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ParallelPipelineTest, executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createParallelPipeline();
    line->add(activity_1,activity_2,activity_3);
    line->execute();
    int res_0, res_1, res_2;
    if(line->waitingComplete())
    {
        line->result(0,res_0);
        line->result(1,res_1);
        line->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(9,res_2);
}
