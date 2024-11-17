#ifndef TA_VARIANTTEST_H
#define TA_VARIANTTEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_VariantTest : public :: testing :: Test
{
public:
    TA_VariantTest();
    ~TA_VariantTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest {nullptr};
};

#endif // TA_VARIANTTEST_H
