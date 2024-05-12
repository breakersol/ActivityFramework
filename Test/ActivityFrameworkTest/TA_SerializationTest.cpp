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
    M3Test t, p;
    {
        CoreAsync::TA_Serialization output("./test.afw");
        t.setVec({2,3,4,5});
        t.setRawPtr(ptr);
        t.setArray({2,3,4,5,6});
        t.setList({9,9,9});
        output << t;
    }
    {
        CoreAsync::TA_Serialization<CoreAsync::OperationType::Input> input("./test.afw");
        input >> p;
    }
    EXPECT_EQ(t.getVec(), p.getVec());
    EXPECT_EQ(*t.getRawPtr(), *p.getRawPtr());
    EXPECT_EQ(t.getArray(), p.getArray());
    EXPECT_EQ(t.getList(), p.getList());
    delete ptr;
}
