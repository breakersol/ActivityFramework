#ifndef TA_ACTIVITYQUEUETEST_H
#define TA_ACTIVITYQUEUETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ActivityQueueTest : public :: testing :: Test
{
public:
    TA_ActivityQueueTest();
    ~TA_ActivityQueueTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_ACTIVITYQUEUETEST_H
