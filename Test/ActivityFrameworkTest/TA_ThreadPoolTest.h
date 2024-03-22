#ifndef TA_THREADPOOLTEST_H
#define TA_THREADPOOLTEST_H

#include "gtest/gtest.h"
#include "Components/TA_ThreadPool.h"

namespace CoreAsync {
    class TA_BasicActivity;
}

class TA_ThreadPoolTest : public :: testing :: Test
{
public:
    TA_ThreadPoolTest();
    ~TA_ThreadPoolTest();

    void SetUp() override;
    void TearDown() override;

    std::array<CoreAsync::TA_BasicActivity *, 1024> activities;

};

#endif // TA_THREADPOOLTEST_H
