#ifndef TA_SERIALIZATIONTEST_H
#define TA_SERIALIZATIONTEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_SerializationTest : public ::testing::Test
{
public:
    TA_SerializationTest();
    ~TA_SerializationTest();

    void SetUp() override;
    void TearDown() override;

    M3Test testObj;

};

#endif // TA_SERIALIZATIONTEST_H
