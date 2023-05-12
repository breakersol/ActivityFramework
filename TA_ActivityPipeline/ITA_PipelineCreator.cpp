#include "ITA_PipelineCreator.h"

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
