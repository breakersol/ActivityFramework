#ifndef TA_METAREFLEXTEST_H
#define TA_METAREFLEXTEST_H

#include "gtest/gtest.h"

class MetaTest;

class TA_MetaReflexTest : public :: testing :: Test
{
public:
    TA_MetaReflexTest();
    ~TA_MetaReflexTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;

};

#endif // TA_METAREFLEXTEST_H
