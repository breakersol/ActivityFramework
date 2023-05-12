#ifndef TA_AUTOCHAINPIPELINETEST_H
#define TA_AUTOCHAINPIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_AutoChainPipelineTest : public :: testing :: Test
{
public:
    TA_AutoChainPipelineTest();
    ~TA_AutoChainPipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_AUTOCHAINPIPELINETEST_H
