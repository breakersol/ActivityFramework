#ifndef TA_AUTOCHAINPIPELINE_H
#define TA_AUTOCHAINPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
    class TA_AutoChainPipeline : public TA_BasicPipeline
    { 
    public:
        explicit ASYNC_PIPELINE_EXPORT TA_AutoChainPipeline();
        virtual ~TA_AutoChainPipeline(){}

        TA_AutoChainPipeline(const TA_AutoChainPipeline &activity) = delete;
        TA_AutoChainPipeline(TA_AutoChainPipeline &&activity) = delete;
        TA_AutoChainPipeline & operator = (const TA_AutoChainPipeline &) = delete;

    protected:
        virtual void run() override final;

    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_AutoChainPipeline> : TA_MetaTypeInfo<TA_AutoChainPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {};
        };
    }
}

#endif // TA_AUTOCHAINPIPELINE_H
