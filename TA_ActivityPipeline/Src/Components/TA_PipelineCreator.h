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

#ifndef TA_PIPELINECREATOR_H
#define TA_PIPELINECREATOR_H

#include <list>

#include "Components/TA_PipelineHolder.h"

namespace CoreAsync {
    class ASYNC_PIPELINE_EXPORT TA_PipelineCreator
    { 
    public:
        using AutoChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> >;
        using ManualChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> >;
        using ParallelHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ParallelPipeline> >;
        using ManualStepsChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> >;
        using ManualKeyActivityChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> >;

        using AutoChainHolderPtr = std::unique_ptr<AutoChainHolder>;
        using ManualChainHolderPtr = std::unique_ptr<ManualChainHolder>;
        using ParallelHolderPtr = std::unique_ptr<ParallelHolder>;
        using ManualStepsChainHolderPtr = std::unique_ptr<ManualStepsChainHolder>;
        using ManualKeyActivityChainHolderPtr = std::unique_ptr<ManualKeyActivityChainHolder>;

        static TA_PipelineCreator & GetInstance();

        ~TA_PipelineCreator();

        TA_PipelineCreator(const TA_PipelineCreator &creator) = delete;
        TA_PipelineCreator(const TA_PipelineCreator &&creator) = delete;

        TA_PipelineCreator & operator = (const TA_PipelineCreator &creator) = delete;

        AutoChainHolderPtr createAutoChainPipeline();
        ManualChainHolderPtr createManaualChainPipeline();
        ParallelHolderPtr createParallelPipeline();
        ManualStepsChainHolderPtr createManualStepsChainPipeline();
        ManualKeyActivityChainHolderPtr createManualKeyActivityChainPipeline();

    private:
        TA_PipelineCreator();  

    };
}

#endif // TA_PIPELINECREATOR_H
