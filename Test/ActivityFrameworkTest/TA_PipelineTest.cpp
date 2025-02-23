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

#include "TA_PipelineTest.h"
#include "ITA_ActivityCreator.h"
#include "ITA_PipelineCreator.h"

TA_PipelineTest::TA_PipelineTest()
{

}

TA_PipelineTest::~TA_PipelineTest()
{

}

void TA_PipelineTest::SetUp()
{
    m_pTest = new MetaTest();
    m_pAutoChainPipeline = new CoreAsync::TA_AutoChainPipeline();
    m_pManualChainPipeline = new CoreAsync::TA_ManualChainPipeline();
    m_pManualKeyActivityChainPipeline = new CoreAsync::TA_ManualKeyActivityChainPipeline();
    m_pManualStepsChainPipeline = new CoreAsync::TA_ManualStepsChainPipeline();
    m_pConcurrentPipeline = new CoreAsync::TA_ConcurrentPipeline();
}

void TA_PipelineTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
    if(m_pAutoChainPipeline)
        delete m_pAutoChainPipeline;
    m_pAutoChainPipeline = nullptr;
    if(m_pManualChainPipeline)
        delete m_pManualChainPipeline;
    m_pManualChainPipeline = nullptr;
    if(m_pManualKeyActivityChainPipeline)
        delete m_pManualKeyActivityChainPipeline;
    m_pManualKeyActivityChainPipeline = nullptr;
    if(m_pManualStepsChainPipeline)
        delete m_pManualStepsChainPipeline;
    m_pManualStepsChainPipeline = nullptr;
    if(m_pConcurrentPipeline)
        delete m_pConcurrentPipeline;
    m_pConcurrentPipeline = nullptr;
}

TEST_F(TA_PipelineTest, activitySizeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    std::size_t size = m_pAutoChainPipeline->activitySize();

    EXPECT_EQ(3,size);
}

TEST_F(TA_PipelineTest, removeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    m_pAutoChainPipeline->remove(1);

    std::size_t size {m_pAutoChainPipeline->activitySize()};
    EXPECT_EQ(2,size);

    m_pAutoChainPipeline->execute();
    int res_0, res_1;
    if(m_pAutoChainPipeline->waitingComplete())
    {
        m_pAutoChainPipeline->result(0,res_0);
        m_pAutoChainPipeline->result(1,res_1);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(9,res_1);
}

TEST_F(TA_PipelineTest, resetTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    m_pAutoChainPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pAutoChainPipeline->waitingComplete())
    {
        m_pAutoChainPipeline->result(0,res_0);
        m_pAutoChainPipeline->result(1,res_1);
        m_pAutoChainPipeline->result(2,res_2);

        EXPECT_EQ(-1,res_0);
        EXPECT_EQ(0,res_1);
        EXPECT_EQ(9,res_2);
    }
    m_pAutoChainPipeline->reset();
    m_pAutoChainPipeline->result(0,res_0);
    m_pAutoChainPipeline->result(1,res_1);
    m_pAutoChainPipeline->result(2,res_2);

    EXPECT_EQ(0,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(0,res_2);
}

TEST_F(TA_PipelineTest, clearTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    m_pAutoChainPipeline->clear();

    EXPECT_EQ(0,m_pAutoChainPipeline->activitySize());
}

TEST_F(TA_PipelineTest, stateTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    EXPECT_EQ(CoreAsync::TA_BasicPipeline::State::Waiting, m_pAutoChainPipeline->state());
    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    m_pAutoChainPipeline->execute();
    if(m_pAutoChainPipeline->waitingComplete())
    {
        EXPECT_EQ(CoreAsync::TA_BasicPipeline::State::Ready, m_pAutoChainPipeline->state());
    }
}

TEST_F(TA_PipelineTest, startIndexTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    m_pAutoChainPipeline->setStartIndex(1);
    m_pAutoChainPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pAutoChainPipeline->waitingComplete())
    {
        m_pAutoChainPipeline->result(0,res_0);
        m_pAutoChainPipeline->result(1,res_1);
        m_pAutoChainPipeline->result(2,res_2);
    }
    EXPECT_EQ(0,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(9,res_2);
}


TEST_F(TA_PipelineTest, autoChainPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pAutoChainPipeline->add(activity_1,activity_2,activity_3);
    m_pAutoChainPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pAutoChainPipeline->waitingComplete())
    {
        m_pAutoChainPipeline->result(0,res_0);
        m_pAutoChainPipeline->result(1,res_1);
        m_pAutoChainPipeline->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(9,res_2);
}

TEST_F(TA_PipelineTest, manualChainPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 5,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pManualChainPipeline->add(activity_1,activity_2,activity_3);
    m_pManualChainPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pManualChainPipeline->waitingComplete())
    {
        m_pManualChainPipeline->result(0,res_0);
        m_pManualChainPipeline->result(1,res_1);
        m_pManualChainPipeline->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(0,res_2);

    m_pManualChainPipeline->execute();
    if(m_pManualChainPipeline->waitingComplete())
    {
        m_pManualChainPipeline->result(0,res_0);
        m_pManualChainPipeline->result(1,res_1);
        m_pManualChainPipeline->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(3,res_1);
    EXPECT_EQ(0,res_2);
}

TEST_F(TA_PipelineTest, manualKeyActivityChainPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 5,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pManualKeyActivityChainPipeline->add(activity_1,activity_2,activity_3);
    m_pManualKeyActivityChainPipeline->setKeyActivity(1);
    m_pManualKeyActivityChainPipeline->execute();
    if(m_pManualKeyActivityChainPipeline->waitingComplete())
        m_pManualKeyActivityChainPipeline->execute();
    if(m_pManualKeyActivityChainPipeline->waitingComplete())
        m_pManualKeyActivityChainPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pManualKeyActivityChainPipeline->waitingComplete())
    {
        m_pManualKeyActivityChainPipeline->result(0,res_0);
        m_pManualKeyActivityChainPipeline->result(1,res_1);
        m_pManualKeyActivityChainPipeline->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(3,res_1);
    EXPECT_EQ(0,res_2);
}

TEST_F(TA_PipelineTest, manualStepsChainPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 5,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pManualStepsChainPipeline->add(activity_1,activity_2,activity_3);
    m_pManualStepsChainPipeline->setSteps(2);
    m_pManualStepsChainPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pManualStepsChainPipeline->waitingComplete())
    {
        m_pManualStepsChainPipeline->result(0,res_0);
        m_pManualStepsChainPipeline->result(1,res_1);
        m_pManualStepsChainPipeline->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(3,res_1);
    EXPECT_EQ(0,res_2);
}

TEST_F(TA_PipelineTest, parallelPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    m_pConcurrentPipeline->add(activity_1,activity_2,activity_3);
    m_pConcurrentPipeline->execute();
    int res_0, res_1, res_2;
    if(m_pConcurrentPipeline->waitingComplete())
    {
        m_pConcurrentPipeline->result(0,res_0);
        m_pConcurrentPipeline->result(1,res_1);
        m_pConcurrentPipeline->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
    EXPECT_EQ(0,res_1);
    EXPECT_EQ(9,res_2);

    std::function<bool()> testFunc = [&]()->bool {
        m_pConcurrentPipeline->reset();

        auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
        auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
        auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

        m_pConcurrentPipeline->add(activity_1,activity_2,activity_3);
        m_pConcurrentPipeline->execute();
        int res_0, res_1, res_2;
        if(m_pConcurrentPipeline->waitingComplete())
        {
            m_pConcurrentPipeline->result(0,res_0);
            m_pConcurrentPipeline->result(1,res_1);
            m_pConcurrentPipeline->result(2,res_2);
        }
        if(res_0 != -1 || res_1 != 0 || res_2 != 9)
            return false;
        return true;
    };

    EXPECT_EQ(testFunc(), true);
    EXPECT_EQ(testFunc(), true);

    //line->clear();

    //line = CoreAsync::ITA_PipelineCreator::createConcurrentPipeline();
    //auto a1 = CoreAsync::ITA_ActivityCreator::create<bool>(testFunc);
    //line->add(a1);
    //line->execute();
    //if(line->waitingComplete())
    //{
    //    bool res {false};
    //    line->result(0, res);
    //    EXPECT_EQ(true,res);
    //}
}
