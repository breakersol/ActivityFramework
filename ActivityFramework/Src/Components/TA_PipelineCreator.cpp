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

#include "TA_PipelineCreator.h"

namespace CoreAsync {
    TA_PipelineCreator & TA_PipelineCreator::GetInstance()
    {
        static TA_PipelineCreator creator;
        return creator;
    }

    TA_PipelineCreator::TA_PipelineCreator()
    {

    }

    TA_PipelineCreator::~TA_PipelineCreator()
    {
        auto deleteVistor = [](auto &&value) {
            if(value)
            {
                delete value;
            }
            value = nullptr;
        };

        for(auto &pHolder : m_holderList)
        {
            std::visit(deleteVistor, pHolder);
        }
    }

    TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> > * TA_PipelineCreator::createAutoChainPipeline()
    {
        auto pHolder = new TA_PipelineHolder<TA_AutoChainPipeline>();
        m_holderList.push_back(pHolder);
        return pHolder;
    }

    TA_MainPipelineHolder<TA_PipelineHolder<TA_ConcurrentPipeline> > * TA_PipelineCreator::createConcurrentPipeline()
    {
        auto pHolder = new TA_PipelineHolder<TA_ConcurrentPipeline>();
        m_holderList.push_back(pHolder);
        return pHolder;
    }

    TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> > * TA_PipelineCreator::createManualChainPipeline()
    {
        auto pHolder = new TA_PipelineHolder<TA_ManualChainPipeline>();
        m_holderList.push_back(pHolder);
        return pHolder;
    }

    TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> > * TA_PipelineCreator::createManualStepsChainPipeline()
    {
        auto pHolder = new TA_PipelineHolder<TA_ManualStepsChainPipeline>();
        m_holderList.push_back(pHolder);
        return pHolder;
    }

    TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> > * TA_PipelineCreator::createManualKeyActivityChainPipeline()
    {
        auto pHolder = new TA_PipelineHolder<TA_ManualKeyActivityChainPipeline>();
        m_holderList.push_back(pHolder);
        return pHolder;
    }
}
