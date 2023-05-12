#ifndef TA_MANUALCHAINPIPELINETEST_H
#define TA_MANUALCHAINPIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ManualChainPipelineTest : public :: testing :: Test
{
public:
    TA_ManualChainPipelineTest();
    ~TA_ManualChainPipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_MANUALCHAINPIPELINETEST_H
