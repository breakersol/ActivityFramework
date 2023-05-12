#ifndef TA_CONNECTIONTEST_H
#define TA_CONNECTIONTEST_H

#include "gtest/gtest.h"

class MetaTest;

class TA_ConnectionTest : public :: testing :: Test
{
public:
    TA_ConnectionTest();
    ~TA_ConnectionTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_CONNECTIONTEST_H
