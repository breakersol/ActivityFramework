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

#ifndef TA_CONCURRENTPIPELINE_H
#define TA_CONCURRENTPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
    class ACTIVITY_FRAMEWORK_EXPORT TA_ConcurrentPipeline : public TA_BasicPipeline
    {
    public:
        TA_ConcurrentPipeline();
        virtual ~TA_ConcurrentPipeline() {}

        TA_ConcurrentPipeline(const TA_ConcurrentPipeline &other) : TA_BasicPipeline(other),m_resultFetchers(other.m_resultFetchers)
        {

        }

        TA_ConcurrentPipeline(TA_ConcurrentPipeline &&other) : TA_BasicPipeline(std::move(other)),
            m_resultFetchers(std::move(other.m_resultFetchers))
        {
            other.m_resultFetchers.clear();
        }

        TA_ConcurrentPipeline & operator = (const TA_ConcurrentPipeline &other)
        {
            if(this != &other)
            {
                TA_BasicPipeline::operator = (other);
                m_resultFetchers = other.m_resultFetchers;
            }
            return *this;
        }

        TA_ConcurrentPipeline & operator = (TA_ConcurrentPipeline &&other)
        {
            if(this != &other)
            {
                TA_BasicPipeline::operator = (std::move(other));
                m_resultFetchers = std::move(other.m_resultFetchers);
                other.m_resultFetchers.clear();
            }
            return *this;
        }

        void clear() override final;
        void reset() override final;

    protected:
        void run() override final;
        TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Eager> runningGenerator();

    private:
        std::vector<TA_ActivityResultFetcher> m_resultFetchers {};

    };

    namespace Reflex
    {
        template <>
        struct ACTIVITY_FRAMEWORK_EXPORT TA_TypeInfo<TA_ConcurrentPipeline> : TA_MetaTypeInfo<TA_ConcurrentPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {};
        };
    }
}

#endif // TA_CONCURRENTPIPELINE_H
