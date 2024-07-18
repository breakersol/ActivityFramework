#include "TA_SerializationTest.h"
#include "Components/TA_Serialization.h"
#include "Components/TA_TypeFilter.h"

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
        CoreAsync::TA_Serializer output("./test.afw");
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
        CoreAsync::TA_Serializer<CoreAsync::OperationType::Deserialization> input("./test.afw");
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
