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

#ifndef TA_PIPELINETEST_H
#define TA_PIPELINETEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

namespace CoreAsync {
class TA_AutoChainPipeline;
class TA_ManualChainPipeline;
class TA_ManualKeyActivityChainPipeline;
class TA_ManualStepsChainPipeline;
class TA_ConcurrentPipeline;
} // namespace CoreAsync

class TA_PipelineTest : public ::testing ::Test {
  public:
    TA_PipelineTest();
    ~TA_PipelineTest();

    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<MetaTest> m_pTest{nullptr};
    std::shared_ptr<CoreAsync::TA_AutoChainPipeline> m_pAutoChainPipeline{nullptr};
    std::shared_ptr<CoreAsync::TA_ManualChainPipeline> m_pManualChainPipeline{nullptr};
    std::shared_ptr<CoreAsync::TA_ManualKeyActivityChainPipeline> m_pManualKeyActivityChainPipeline{nullptr};
    std::shared_ptr<CoreAsync::TA_ManualStepsChainPipeline> m_pManualStepsChainPipeline{nullptr};
    std::shared_ptr<CoreAsync::TA_ConcurrentPipeline> m_pConcurrentPipeline{nullptr};
};

#endif // TA_PIPELINETEST_H
