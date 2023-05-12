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

    TA_MainPipelineHolder<TA_PipelineHolder<TA_ParallelPipeline> > * TA_PipelineCreator::createParallelPipeline()
    {
        auto pHolder = new TA_PipelineHolder<TA_ParallelPipeline>();
        m_holderList.push_back(pHolder);
        return pHolder;
    }

    TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> > * TA_PipelineCreator::createManaualChainPipeline()
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
