﻿/*
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

#ifndef TA_MANUALSTEPSCHAINPIPELINE_H
#define TA_MANUALSTEPSCHAINPIPELINE_H

#include "TA_ManualChainPipeline.h"

namespace CoreAsync {
    class TA_ManualStepsChainPipeline : public TA_ManualChainPipeline
    {
    public:
        ACTIVITY_FRAMEWORK_EXPORT TA_ManualStepsChainPipeline();
        virtual ~TA_ManualStepsChainPipeline(){}

        TA_ManualStepsChainPipeline(const TA_ManualStepsChainPipeline &activity) = delete;
        TA_ManualStepsChainPipeline(TA_ManualStepsChainPipeline &&activity) = delete;
        TA_ManualStepsChainPipeline & operator = (const TA_ManualStepsChainPipeline &) = delete;

        void ACTIVITY_FRAMEWORK_EXPORT setSteps(unsigned int steps);

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
        struct ACTIVITY_FRAMEWORK_EXPORT TA_TypeInfo<TA_ManualStepsChainPipeline> : TA_MetaTypeInfo<TA_ManualStepsChainPipeline,TA_BasicPipeline>
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
