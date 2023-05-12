#ifndef ITA_PIPELINECREATOR_H
#define ITA_PIPELINECREATOR_H

#include "TA_ActivityPipeline_global.h"
#include "Components/TA_PipelineCreator.h"

namespace CoreAsync {
    class ASYNC_PIPELINE_EXPORT ITA_PipelineCreator
    {
    public:
        static TA_PipelineCreator::AutoChainHolder * createAutoChainPipeline() ;
        static TA_PipelineCreator::ManualChainHolder * createManaualChainPipeline();
        static TA_PipelineCreator::ParallelHolder * createParallelPipeline();
        static TA_PipelineCreator::ManualStepsChainHolder * createManualStepsChainPipeline();
        static TA_PipelineCreator::ManualKeyActivityChainHolder * createManualKeyActivityChainPipeline();

    };
}

#endif // ITA_PIPELINECREATOR_H
