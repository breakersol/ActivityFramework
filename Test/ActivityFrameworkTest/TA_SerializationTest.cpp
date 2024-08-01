#include "TA_SerializationTest.h"
#include "Components/TA_Serialization.h"

TA_SerializationTest::TA_SerializationTest()
{

}

TA_SerializationTest::~TA_SerializationTest()
{

}

void TA_SerializationTest::SetUp()
{

}

void TA_SerializationTest::TearDown()
{

}

TEST_F(TA_SerializationTest, CustomTypeTest)
{
    float *ptr = new float(5.3);
    M3Test t, p1, p2;
    {
        CoreAsync::TA_Serializer output("./test.afw", 1, 1024);
        t.setVec({2,3,4,5});
        t.setRawPtr(ptr);
        t.setArray({2,3,4,5,6});
        t.setList({9,9,9});
        t.setForwardList({9,9,9});
        t.setDeque({8,7,6,5,4});
        t.setStack({t.getDeque().begin(), t.getDeque().end()});
        t.mx = 999;
        t.m_vec = {1,1,1,1};
        t.setQueue({t.getDeque().begin(), t.getDeque().end()});
        t.setPrioritQueue({t.getDeque().begin(), t.getDeque().end()});
        output << t << t;
    }
    {
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input("./test.afw", 1, 1024);
        input >> p1 >> p2;
    }
    EXPECT_EQ(t.getVec(), p2.getVec());
    EXPECT_EQ(*t.getRawPtr(), *p2.getRawPtr());
    EXPECT_EQ(t.getArray(), p2.getArray());
    EXPECT_EQ(t.getList(), p2.getList());
    EXPECT_EQ(t.getForwardList(), p2.getForwardList());
    EXPECT_EQ(t.getDeque(), p2.getDeque());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(5, p2.mx);
    EXPECT_EQ(t.m_vec, p2.m_vec);
    EXPECT_EQ(t.getQueue(), p2.getQueue());

    auto pq1 {t.getPriorityQueue()};
    auto pq2 {p2.getPriorityQueue()};
    EXPECT_TRUE(arePriorityQueueEqual(pq1, pq2));

    delete ptr;
}

TEST_F(TA_SerializationTest, VersionTest)
{
    float *ptr = new float(5.3);
    M3Test t, p1, p2;
    {
        CoreAsync::TA_Serializer output("./test.afw", 2);
        t.setVec({2,3,4,5});
        t.setRawPtr(ptr);
        t.setArray({2,3,4,5,6});
        t.setList({9,9,9});
        t.setForwardList({9,9,9});
        t.setDeque({8,7,6,5,4});
        t.setStack({t.getDeque().begin(), t.getDeque().end()});
        t.mx = 999;
        t.m_vec = {1,1,1,1};
        t.setQueue({t.getDeque().begin(), t.getDeque().end()});
        t.setPrioritQueue({t.getDeque().begin(), t.getDeque().end()});
        output << t << t;
    }
    {
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input("./test.afw", 2);
        input >> p1 >> p2;
    }
    EXPECT_EQ(t.getVec(), p2.getVec());
    EXPECT_EQ(*t.getRawPtr(), *p2.getRawPtr());
    EXPECT_EQ(t.getArray(), p2.getArray());
    EXPECT_EQ(t.getList(), p2.getList());
    EXPECT_EQ(t.getForwardList(), p2.getForwardList());
    EXPECT_EQ(t.getDeque(), p2.getDeque());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(t.mx, p2.mx);
    EXPECT_EQ(t.m_vec, p2.m_vec);
    EXPECT_EQ(t.getQueue(), p2.getQueue());

    auto pq1 {t.getPriorityQueue()};
    auto pq2 {p2.getPriorityQueue()};
    EXPECT_TRUE(arePriorityQueueEqual(pq1, pq2));

    delete ptr;
}


TEST_F(TA_SerializationTest, LargeScaleTest)
{
    CoreAsync::TA_Serializer output("./test.afw", 2, 10);
    M3Test t;
    for(std::size_t i = 0;i < 1000;++i)
    {
        output << t;
    }
    output.flush();
    output.close();
    std::vector<M3Test> vec(1000);
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input("./test.afw", 2, 10);
    for(std::size_t i = 0;i < 1000;++i)
    {
        input >> vec[i];
    }

    EXPECT_EQ(t.getVec(), vec[999].getVec());
    EXPECT_EQ(*t.getRawPtr(), *vec[999].getRawPtr());
    EXPECT_EQ(t.getArray(), vec[999].getArray());
    EXPECT_EQ(t.getList(), vec[999].getList());
    EXPECT_EQ(t.getForwardList(), vec[999].getForwardList());
    EXPECT_EQ(t.getDeque(), vec[999].getDeque());
    EXPECT_EQ(t.getStack(), vec[999].getStack());
    EXPECT_EQ(t.getStack(), vec[999].getStack());
    EXPECT_EQ(t.mx, vec[999].mx);
    EXPECT_EQ(t.m_vec, vec[999].m_vec);
    EXPECT_EQ(t.getQueue(), vec[999].getQueue());

    auto pq1 {t.getPriorityQueue()};
    auto pq2 {vec[999].getPriorityQueue()};
    EXPECT_TRUE(arePriorityQueueEqual(pq1, pq2));
}
