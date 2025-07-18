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

#ifndef TA_ENDIANSWAPTEST_H
#define TA_ENDIANSWAPTEST_H

#include "gtest/gtest.h"

class TA_EndianSwapTest : public ::testing ::Test {
  public:
    TA_EndianSwapTest();
    ~TA_EndianSwapTest();

    void SetUp() override;
    void TearDown() override;
};

#endif // TA_ENDIANSWAPTEST_H
