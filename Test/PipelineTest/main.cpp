#include "gtest/gtest.h"
#include "MetaTest.h"
#include "Components/TA_ThreadPool.h"
#include "ITA_ActivityCreator.h"
#include "ITA_PipelineCreator.h"



static bool testFunc()
{
    MetaTest *m_pTest = new MetaTest();

    auto activity_1 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 1,2);
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 2,2);
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 10,1);

    auto line = CoreAsync::ITA_PipelineCreator::createParallelPipeline();
    line->add(activity_1/*,activity_2,activity_3*/);
    line->execute();
    int res_0, res_1, res_2;
    if(line->waitingComplete())
    {
        line->result(0,res_0);
//        line->result(1,res_1);
//        line->result(2,res_2);
    }
    EXPECT_EQ(-1,res_0);
//    EXPECT_EQ(0,res_1);
//    EXPECT_EQ(9,res_2);

    delete m_pTest;
    m_pTest = nullptr;

    return true;
}

int main(int argc, char *argv[])
{
//    ::testing::InitGoogleTest(&argc,argv);

//    return RUN_ALL_TESTS();

    auto line = CoreAsync::ITA_PipelineCreator::createParallelPipeline();
    auto a1 = CoreAsync::ITA_ActivityCreator::create<bool>([]()->bool {return testFunc();});
    line->add(a1);
    line->execute();
    if(line->waitingComplete())
    {
        bool res {false};
        line->result(0, res);
        std::cout << res << std::endl;
    }

    return 0;
}
