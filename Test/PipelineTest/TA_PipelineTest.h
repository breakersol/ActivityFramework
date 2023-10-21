#ifndef TA_PIPELINETEST_H
#define TA_PIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_PipelineTest : public :: testing :: Test
{
public:
    TA_PipelineTest();
    ~TA_PipelineTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_PIPELINETEST_H
