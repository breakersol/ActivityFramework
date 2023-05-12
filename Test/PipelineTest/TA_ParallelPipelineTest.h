#ifndef TA_PARALLELPIPELINETEST_H
#define TA_PARALLELPIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ParallelPipelineTest : public :: testing :: Test
{
public:
    TA_ParallelPipelineTest();
    ~TA_ParallelPipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_PARALLELPIPELINETEST_H
