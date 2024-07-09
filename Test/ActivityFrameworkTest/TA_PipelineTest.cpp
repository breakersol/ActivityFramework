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
}

void TA_PipelineTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_PipelineTest, activitySizeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);
    std::size_t size = line->activitySize();

    EXPECT_EQ(3,size);
}

TEST_F(TA_PipelineTest, removeTest)
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

TEST_F(TA_PipelineTest, resetTest)
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

TEST_F(TA_PipelineTest, clearTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    line->add(activity_1,activity_2,activity_3);

    line->clear();

    EXPECT_EQ(0,line->activitySize());
}

TEST_F(TA_PipelineTest, switchBranchTest)
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

TEST_F(TA_PipelineTest, stateTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createAutoChainPipeline();
    EXPECT_EQ(CoreAsync::TA_BasicPipeline::State::Waiting, line->state());
    line->add(activity_1,activity_2,activity_3);
    line->execute();
    if(line->waitingComplete())
    {
        EXPECT_EQ(CoreAsync::TA_BasicPipeline::State::Ready, line->state());
    }
}

TEST_F(TA_PipelineTest, startIndexTest)
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


TEST_F(TA_PipelineTest, autoChainPipeline_executeTest)
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

    auto line = CoreAsync::ITA_PipelineCreator::createManualChainPipeline();
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
    EXPECT_EQ(0,res_2);

    line->execute();
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

TEST_F(TA_PipelineTest, manualKeyActivityChainPipeline_executeTest)
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

TEST_F(TA_PipelineTest, manualStepsChainPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 5,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createManualStepsChainPipeline();
    line->add(activity_1,activity_2,activity_3);
    line->setSteps(2);
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

TEST_F(TA_PipelineTest, parallelPipeline_executeTest)
{
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createConcurrentPipeline();
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

    //line->reset();

    std::function<bool()> testFunc = [&]()->bool {
        auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
        auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
        auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

        auto line = CoreAsync::ITA_PipelineCreator::createConcurrentPipeline();
        line->add(activity_1,activity_2,activity_3);
        line->execute();
        int res_0, res_1, res_2;
        if(line->waitingComplete())
        {
            line->result(0,res_0);
            line->result(1,res_1);
            line->result(2,res_2);
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
