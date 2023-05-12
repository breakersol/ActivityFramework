#ifndef TA_PIPELINECREATOR_H
#define TA_PIPELINECREATOR_H

#include <list>
#include <variant>

#include "Components/TA_PipelineHolder.h"

namespace CoreAsync {
    class TA_PipelineCreator
    { 
    public:
        using AutoChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> >;
        using ManualChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> >;
        using ParallelHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ParallelPipeline> >;
        using ManualStepsChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> >;
        using ManualKeyActivityChainHolder = TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> >;

        static TA_PipelineCreator & GetInstance();

        ~TA_PipelineCreator();

        TA_PipelineCreator(const TA_PipelineCreator &creator) = delete;
        TA_PipelineCreator(const TA_PipelineCreator &&creator) = delete;

        TA_PipelineCreator & operator = (const TA_PipelineCreator &creator) = delete;

        AutoChainHolder * createAutoChainPipeline();
        ManualChainHolder * createManaualChainPipeline();
        ParallelHolder * createParallelPipeline();
        ManualStepsChainHolder * createManualStepsChainPipeline();
        ManualKeyActivityChainHolder * createManualKeyActivityChainPipeline();

    private:
        TA_PipelineCreator();  

    private:
        using HolderVar = std::variant<AutoChainHolder *, ManualChainHolder *, ParallelHolder *, ManualStepsChainHolder *, ManualKeyActivityChainHolder *>;

        std::list<HolderVar> m_holderList;

    };
}

#endif // TA_PIPELINECREATOR_H
