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

#ifndef TA_METAOBJECTTEST_H
#define TA_METAOBJECTTEST_H

#include "gtest/gtest.h"

class MetaTest;

class TA_MetaObjectTest : public ::testing ::Test {
  public:
    TA_MetaObjectTest();
    ~TA_MetaObjectTest() override;

  protected:
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<MetaTest> m_pMetaTest{std::make_shared<MetaTest>()};
    std::string m_str{"This is a test.\n"};
};

#endif // TA_METAOBJECTTEST_H
