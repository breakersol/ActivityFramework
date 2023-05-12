#ifndef TA_ACTIVITYTEST_H
#define TA_ACTIVITYTEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ActivityTest : public :: testing :: Test
{
public:
    TA_ActivityTest();
    ~TA_ActivityTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;

};

#endif // TA_ACTIVITYTEST_H
