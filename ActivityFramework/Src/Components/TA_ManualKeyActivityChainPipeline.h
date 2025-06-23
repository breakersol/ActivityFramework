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

#ifndef TA_MANUALKEYACTIVITYCHAINPIPELINE_H
#define TA_MANUALKEYACTIVITYCHAINPIPELINE_H

#include "TA_ManualChainPipeline.h"

namespace CoreAsync {
    class TA_ManualKeyActivityChainPipeline : public TA_ManualChainPipeline
    {
    public:
        ACTIVITY_FRAMEWORK_EXPORT TA_ManualKeyActivityChainPipeline();
        virtual ~TA_ManualKeyActivityChainPipeline(){}

        TA_ManualKeyActivityChainPipeline(const TA_ManualKeyActivityChainPipeline &other) :
            TA_ManualChainPipeline(other),
            m_keyIndex(other.m_keyIndex.load(std::memory_order_acquire))
        {

        }

        TA_ManualKeyActivityChainPipeline(TA_ManualKeyActivityChainPipeline &&other) :
            TA_ManualChainPipeline(std::move(other)),
            m_keyIndex(std::move(other.m_keyIndex.load(std::memory_order_acquire)))
        {

        }

        TA_ManualKeyActivityChainPipeline & operator = (const TA_ManualKeyActivityChainPipeline &other)
        {
            if(this != &other)
            {
                TA_ManualChainPipeline::operator = (other);
                m_keyIndex.store(other.m_keyIndex.load(std::memory_order_acquire), std::memory_order_release);
            }
            return *this;
        }

        TA_ManualKeyActivityChainPipeline & operator = (TA_ManualKeyActivityChainPipeline &&other)
        {
            if(this != &other)
            {
                TA_ManualChainPipeline::operator = (std::move(other));
                m_keyIndex.store(std::move(other.m_keyIndex.load(std::memory_order_acquire)), std::memory_order_release);
            }
            return *this;
        }

        void ACTIVITY_FRAMEWORK_EXPORT setKeyActivity(int index);
        void ACTIVITY_FRAMEWORK_EXPORT skipKeyActivity();
        void clear() override final;
        void reset() override final;

    protected:
        virtual void run() override final;
        virtual TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Lazy> runningGenerator() override final;

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
