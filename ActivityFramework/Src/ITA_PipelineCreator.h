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

#ifndef ITA_PIPELINECREATOR_H
#define ITA_PIPELINECREATOR_H

#include "Components/TA_PipelineCreator.h"

namespace CoreAsync {
class ITA_PipelineCreator {
  public:
    static TA_PipelineCreator::ConcurrentHolder *createConcurrentPipeline() {
        return TA_PipelineCreator::GetInstance().createConcurrentPipeline();
    }

    static TA_PipelineCreator::AutoChainHolder *createAutoChainPipeline() {
        return TA_PipelineCreator::GetInstance().createAutoChainPipeline();
    }

    static TA_PipelineCreator::ManualChainHolder *createManualChainPipeline() {
        return TA_PipelineCreator::GetInstance().createManualChainPipeline();
    }

    static TA_PipelineCreator::ManualStepsChainHolder *createManualStepsChainPipeline() {
        return TA_PipelineCreator::GetInstance().createManualStepsChainPipeline();
    }

    static TA_PipelineCreator::ManualKeyActivityChainHolder *createManualKeyActivityChainPipeline() {
        return TA_PipelineCreator::GetInstance().createManualKeyActivityChainPipeline();
    }
};
} // namespace CoreAsync

#endif // ITA_PIPELINECREATOR_H
