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

#ifndef TA_PARALLELPIPELINE_H
#define TA_PARALLELPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
    class ASYNC_PIPELINE_EXPORT TA_ParallelPipeline : public TA_BasicPipeline
    {
    public:
        TA_ParallelPipeline();
        virtual ~TA_ParallelPipeline() {}

        TA_ParallelPipeline(const TA_ParallelPipeline &activity) = delete;
        TA_ParallelPipeline(TA_ParallelPipeline &&activity) = delete;
        TA_ParallelPipeline & operator = (const TA_ParallelPipeline &) = delete;

        void taskCompleted(std::size_t id, TA_Variant var);

    protected:
        void run() override final;

    private:
        std::vector<std::size_t> m_activityIds;
        std::size_t m_waitingCount;

    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_ParallelPipeline> : TA_MetaTypeInfo<TA_ParallelPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::taskCompleted, META_STRING("taskCompleted")},
            };
        };
    }
}

#endif // TA_PARALLELPIPELINE_H
