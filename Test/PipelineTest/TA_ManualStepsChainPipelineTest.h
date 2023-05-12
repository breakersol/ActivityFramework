#ifndef TA_MANUALSTEPSCHAINPIPELINETEST_H
#define TA_MANUALSTEPSCHAINPIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ManualStepsChainPipelineTest : public :: testing :: Test
{
public:
    TA_ManualStepsChainPipelineTest();
    ~TA_ManualStepsChainPipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_MANUALSTEPSCHAINPIPELINETEST_H
