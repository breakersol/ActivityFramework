#include "TA_ActivityQueueTest.h"
#include "ITA_ActivityCreator.h"
#include "Components/TA_ActivityQueue.h"

#include <thread>

TA_ActivityQueueTest::TA_ActivityQueueTest()
{

}

TA_ActivityQueueTest::~TA_ActivityQueueTest()
{

}

void TA_ActivityQueueTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ActivityQueueTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}


TEST_F(TA_ActivityQueueTest, sizeTest)
{
//    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 6,3);
    CoreAsync::ActivityQueue queue;
    EXPECT_EQ(10240,queue.size());
}


TEST_F(TA_ActivityQueueTest, getFront)
{
    CoreAsync::ActivityQueue queue;
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 6,3);
    queue.push(activity);
    auto var =  (*queue.front())();
    int res = var.get<int>();
    EXPECT_EQ(3,res);
}

TEST_F(TA_ActivityQueueTest, getRear)
{
    CoreAsync::ActivityQueue queue;
    auto activity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, 6,3);
    queue.push(activity);
    CoreAsync::TA_BasicActivity *pActivity =  queue.rear();
    EXPECT_EQ(pActivity, nullptr);
}

TEST_F(TA_ActivityQueueTest, multiThreadTest)
{
    CoreAsync::ActivityQueue queue;
    std::function<bool()> func_1 = [&]() {
        for(int i = 0;i < 150;++i)
        {
            queue.push(CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::sub, m_pTest, i,3));
        }
        return true;
    };

    std::function<bool()> func_2 = [&]() {
        for(int i = 0;i < 150;++i)
        {
            std::string str {"321"};
            queue.push(CoreAsync::ITA_ActivityCreator::create(&MetaTest::getStr,str));
        }
        return true;
    };

    std::thread t1 {func_1};
    std::thread t2 {func_2};
    std::thread t3 {func_1};

    t1.join();
    t2.join();
    t3.join();
}

TEST_F(TA_ActivityQueueTest, emptyTest)
{
    CoreAsync::ActivityQueue queue;
    EXPECT_EQ(queue.isEmpty(), true);
}

TEST_F(TA_ActivityQueueTest, fullTest)
{
    CoreAsync::ActivityQueue queue;
    for(int i = 0;i < queue.size();++i)
    {
        std::string str {"321"};
        queue.push(CoreAsync::ITA_ActivityCreator::create(&MetaTest::getStr,str));
    }
    EXPECT_EQ(queue.isFull(), true);
}
