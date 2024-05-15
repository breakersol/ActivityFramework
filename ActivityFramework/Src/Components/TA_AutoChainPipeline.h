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

#ifndef TA_AUTOCHAINPIPELINE_H
#define TA_AUTOCHAINPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
    class TA_AutoChainPipeline : public TA_BasicPipeline
    { 
    public:
        ACTIVITY_FRAMEWORK_EXPORT TA_AutoChainPipeline();
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
        struct ACTIVITY_FRAMEWORK_EXPORT TA_TypeInfo<TA_AutoChainPipeline> : TA_MetaTypeInfo<TA_AutoChainPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {};
        };
    }
}

#endif // TA_AUTOCHAINPIPELINE_H
