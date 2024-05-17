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
