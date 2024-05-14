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

#ifndef TA_MANUALKEYACTIVITYCHAINPIPELINE_H
#define TA_MANUALKEYACTIVITYCHAINPIPELINE_H

#include "TA_ManualChainPipeline.h"

namespace CoreAsync {
    class TA_ManualKeyActivityChainPipeline : public TA_ManualChainPipeline
    {
    public:
        ACTIVITY_FRAMEWORK_EXPORT TA_ManualKeyActivityChainPipeline();
        virtual ~TA_ManualKeyActivityChainPipeline(){}

        TA_ManualKeyActivityChainPipeline(const TA_ManualKeyActivityChainPipeline &activity) = delete;
        TA_ManualKeyActivityChainPipeline(TA_ManualKeyActivityChainPipeline &&activity) = delete;
        TA_ManualKeyActivityChainPipeline & operator = (const TA_ManualKeyActivityChainPipeline &) = delete;

        void ACTIVITY_FRAMEWORK_EXPORT setKeyActivity(int index);
        void ACTIVITY_FRAMEWORK_EXPORT skipKeyActivity();
        void clear() override final;
        void reset() override final;

    protected:
        virtual void run() override final;

    private:
        std::atomic<int> m_keyIndex {-1};

    };

    namespace Reflex
    {
        template <>
        struct ACTIVITY_FRAMEWORK_EXPORT TA_TypeInfo<TA_ManualKeyActivityChainPipeline> : TA_MetaTypeInfo<TA_ManualKeyActivityChainPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::setKeyActivity, META_STRING("setKeyActivity")},
                TA_MetaField {&Raw::skipKeyActivity, META_STRING("skipKeyActivity")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
            };
        };
    }
}


#endif // TA_ManualKeyActivityChainPipeline_H
