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

#ifndef TA_BASICACTIVITY_H
#define TA_BASICACTIVITY_H

#include <deque>
#include <atomic>

#include "TA_Variant.h"
#include "TA_ActivityFramework_global.h"

namespace CoreAsync {
    class TA_BasicActivity
    {
    public:
        TA_BasicActivity() : m_id(m_count.load(std::memory_order_acquire))
        {
            m_count.fetch_add(1);
        }

        virtual ~TA_BasicActivity() = default;

        virtual TA_Variant operator()() = 0;

        virtual bool hasBranch() const = 0;

        virtual bool selectBranch(std::deque<unsigned int> branches) = 0;

        virtual TA_Variant caller() const = 0;

        std::size_t id() const
        {
            return m_id;
        }

    private:
        static std::atomic_size_t ACTIVITY_FRAMEWORK_EXPORT m_count;
        const std::size_t m_id;

    };
}

#endif // TA_BASICACTIVITY_H
