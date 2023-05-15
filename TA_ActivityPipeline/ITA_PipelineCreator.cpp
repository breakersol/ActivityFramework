/*
 * Copyright [2023] [Shuang Zhu / Sol]
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

#include "ITA_PipelineCreator.h"
#include "Components/TA_PipelineCreator.h"

namespace CoreAsync {
    TA_PipelineCreator::ParallelHolder * ITA_PipelineCreator::createParallelPipeline()
    {
        return TA_PipelineCreator::GetInstance().createParallelPipeline();
    }

    TA_PipelineCreator::AutoChainHolder * ITA_PipelineCreator::createAutoChainPipeline()
    {
        return TA_PipelineCreator::GetInstance().createAutoChainPipeline();
    }

    TA_PipelineCreator::ManualChainHolder * ITA_PipelineCreator::createManaualChainPipeline()
    {
        return TA_PipelineCreator::GetInstance().createManaualChainPipeline();
    }

    TA_PipelineCreator::ManualStepsChainHolder * ITA_PipelineCreator::createManualStepsChainPipeline()
    {
        return TA_PipelineCreator::GetInstance().createManualStepsChainPipeline();
    }

    TA_PipelineCreator::ManualKeyActivityChainHolder * ITA_PipelineCreator::createManualKeyActivityChainPipeline()
    {
        return TA_PipelineCreator::GetInstance().createManualKeyActivityChainPipeline();
    }
}
