#ifndef TA_MANUALKEYACTIVITYCHAINPIPELINETEST_H
#define TA_MANUALKEYACTIVITYCHAINPIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ManualKeyActivityChainPipelineTest : public :: testing :: Test
{
public:
    TA_ManualKeyActivityChainPipelineTest();
    ~TA_ManualKeyActivityChainPipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_MANUALKEYACTIVITYCHAINPIPELINETEST_H
