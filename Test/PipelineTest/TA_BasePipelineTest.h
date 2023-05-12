#ifndef TA_BASEPIPELINETEST_H
#define TA_BASEPIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_BasePipelineTest : public :: testing :: Test
{
public:
    TA_BasePipelineTest();
    ~TA_BasePipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_BASEPIPELINETEST_H
