/*
 * Copyright [2024] [Shuang Zhu / Sol]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

    std::array<CoreAsync::TA_ActivityProxy *, 1024> activities;

};

#endif // TA_THREADPOOLTEST_H
