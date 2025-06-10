/*
 * Copyright [2025] [Shuang Zhu / Sol]
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

#ifndef TA_ACTIVITYPROXYTEST_H
#define TA_ACTIVITYPROXYTEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_ActivityProxyTest : public :: testing :: Test
{
public:
    TA_ActivityProxyTest();
    ~TA_ActivityProxyTest();

    void SetUp() override;
    void TearDown() override;

    MetaTest *m_pTest;
};

#endif // TA_ACTIVITYPROXYTEST_H
