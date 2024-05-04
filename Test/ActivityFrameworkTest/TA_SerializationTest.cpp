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
    CoreAsync::TA_Serialization output("./test.afw");
    auto res = !std::is_fundamental<decltype(testObj)>::value &&
           !std::is_enum<decltype(testObj)>::value &&
           !std::is_union<decltype(testObj)>::value &&
           !std::is_array<decltype(testObj)>::value &&
           !std::is_pointer<decltype(testObj)>::value &&
           !std::is_null_pointer<decltype(testObj)>::value &&
           !CoreAsync::StdContainerType<decltype(testObj)> &&
           std::is_class<decltype(testObj)>::value;
    M2Test t;
    std::cout << res << std::endl;
    output << t;
}
