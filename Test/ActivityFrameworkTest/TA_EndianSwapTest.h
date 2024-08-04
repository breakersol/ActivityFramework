#ifndef TA_ENDIANSWAPTEST_H
#define TA_ENDIANSWAPTEST_H

#include "gtest/gtest.h"

class TA_EndianSwapTest : public :: testing :: Test
{
public:
    TA_EndianSwapTest();
    ~TA_EndianSwapTest();

    void SetUp() override;
    void TearDown() override;
};

#endif // TA_ENDIANSWAPTEST_H
