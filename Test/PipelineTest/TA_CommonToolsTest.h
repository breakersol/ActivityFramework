#ifndef TA_COMMONTOOLSTEST_H
#define TA_COMMONTOOLSTEST_H

#include "gtest/gtest.h"

class TA_CommonToolsTest : public :: testing :: Test
{
public:
    TA_CommonToolsTest();
    ~TA_CommonToolsTest();

    void SetUp() override;
    void TearDown() override;

};

#endif // TA_COMMONTOOLSTEST_H
