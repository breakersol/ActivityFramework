#ifndef TA_PARALLELPIPELINE_H
#define TA_PARALLELPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
    class TA_ParallelPipeline : public TA_BasicPipeline
    {
    public:
        explicit ASYNC_PIPELINE_EXPORT TA_ParallelPipeline();
        virtual ~TA_ParallelPipeline(){}

        TA_ParallelPipeline(const TA_ParallelPipeline &activity) = delete;
        TA_ParallelPipeline(TA_ParallelPipeline &&activity) = delete;
        TA_ParallelPipeline & operator = (const TA_ParallelPipeline &) = delete;

    protected:
        void run() override final;

    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_ParallelPipeline> : TA_MetaTypeInfo<TA_ParallelPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {};
        };
    }
}

#endif // TA_PARALLELPIPELINE_H
