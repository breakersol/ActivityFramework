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

#include "TA_VariantTest.h"
#include "Components/TA_Variant.h"

TA_VariantTest::TA_VariantTest() {}

TA_VariantTest::~TA_VariantTest() {}

void TA_VariantTest::SetUp() { m_pTest = new MetaTest(); }

void TA_VariantTest::TearDown() {
    if (m_pTest) {
        delete m_pTest;
        m_pTest = nullptr;
    }
}

TEST_F(TA_VariantTest, constructTest) {
    CoreAsync::TA_Variant var{m_pTest};
    auto res = var.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, setTest) {
    CoreAsync::TA_Variant var{};
    var.set(m_pTest);
    auto res = var.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, validTest) {
    CoreAsync::TA_Variant var{};
    EXPECT_EQ(var.isValid(), false);
    var.set(m_pTest);
    EXPECT_EQ(var.isValid(), true);
}

TEST_F(TA_VariantTest, assignmentOperatorTest) {
    CoreAsync::TA_Variant var_1{m_pTest};
    CoreAsync::TA_Variant var2 = var_1;
    auto res = var2.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, copyTest) {
    CoreAsync::TA_Variant var_1{m_pTest};
    CoreAsync::TA_Variant var2{var_1};
    auto res = var2.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}
