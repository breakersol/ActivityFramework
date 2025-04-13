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

#ifndef TA_PIPELINECREATOR_H
#define TA_PIPELINECREATOR_H

#include <list>
#include <variant>

#include "Components/TA_PipelineHolder.h"

namespace CoreAsync {
    class ACTIVITY_FRAMEWORK_EXPORT TA_PipelineCreator
    { 
    public:
        using AutoChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> >;
        using ManualChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> >;
        using ConcurrentHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ConcurrentPipeline> >;
        using ManualStepsChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> >;
        using ManualKeyActivityChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> >;

        static TA_PipelineCreator & GetInstance();

        ~TA_PipelineCreator();

        TA_PipelineCreator(const TA_PipelineCreator &creator) = delete;
        TA_PipelineCreator(const TA_PipelineCreator &&creator) = delete;

        TA_PipelineCreator & operator = (const TA_PipelineCreator &creator) = delete;

        AutoChainHolder * createAutoChainPipeline();
        ManualChainHolder * createManualChainPipeline();
        ConcurrentHolder * createConcurrentPipeline();
        ManualStepsChainHolder * createManualStepsChainPipeline();
        ManualKeyActivityChainHolder * createManualKeyActivityChainPipeline();

    private:
        TA_PipelineCreator();  

    private:
        using HolderVar = std::variant<AutoChainHolder *, ManualChainHolder *, ConcurrentHolder *, ManualStepsChainHolder *, ManualKeyActivityChainHolder *>;

        std::list<HolderVar> m_holderList;

    };
}

#endif // TA_PIPELINECREATOR_H
