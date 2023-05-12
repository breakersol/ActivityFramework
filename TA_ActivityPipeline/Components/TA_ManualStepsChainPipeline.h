#ifndef TA_MANUALSTEPSCHAINPIPELINE_H
#define TA_MANUALSTEPSCHAINPIPELINE_H

#include "TA_ManualChainPipeline.h"

namespace CoreAsync {
    class TA_ManualStepsChainPipeline : public TA_ManualChainPipeline
    {
    public:
        explicit ASYNC_PIPELINE_EXPORT TA_ManualStepsChainPipeline();
        virtual ~TA_ManualStepsChainPipeline(){}

        TA_ManualStepsChainPipeline(const TA_ManualStepsChainPipeline &activity) = delete;
        TA_ManualStepsChainPipeline(TA_ManualStepsChainPipeline &&activity) = delete;
        TA_ManualStepsChainPipeline & operator = (const TA_ManualStepsChainPipeline &) = delete;

        void ASYNC_PIPELINE_EXPORT setSteps(unsigned int steps);

        void clear() override final;
        void reset() override final;

    protected:
        virtual void run() override final;

    private:
        std::atomic<unsigned int> m_steps {1};

    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_ManualStepsChainPipeline> : TA_MetaTypeInfo<TA_ManualStepsChainPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::setSteps, META_STRING("setSteps")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
            };
        };
    }
}

#endif // TA_MANUALSTEPSCHAINPIPELINE_H
