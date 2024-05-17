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

#ifndef TA_VARIANT_H
#define TA_VARIANT_H

#include <typeinfo>
#include <memory>

#include "../TA_ActivityFramework_global.h"

namespace CoreAsync {
    class TA_Variant
    {
    public:
        ACTIVITY_FRAMEWORK_EXPORT TA_Variant();
        ACTIVITY_FRAMEWORK_EXPORT ~TA_Variant();

        ACTIVITY_FRAMEWORK_EXPORT TA_Variant(const TA_Variant &var);
        ACTIVITY_FRAMEWORK_EXPORT TA_Variant(TA_Variant &&var);
        ACTIVITY_FRAMEWORK_EXPORT TA_Variant & operator = (const TA_Variant &);
        ACTIVITY_FRAMEWORK_EXPORT TA_Variant & operator = (TA_Variant &&);

        template<typename VAR>
        void set(VAR v)
        {
            if constexpr(std::is_same_v<VAR, TA_Variant>)
            {
                (*this) = v;
            }
            else
            {
                m_typeId = typeid (VAR).hash_code();
                if constexpr(!std::is_pointer_v<VAR>)
                {
                    m_ptr = std::make_shared<VAR>(v);
                }
                else
                {
                    m_ptr.reset(v);
                }
            }
        }

        template <typename VAR>
        VAR get() const requires (!std::is_pointer<VAR>::value)
        {
            if(m_typeId == typeid (VAR).hash_code())
            {
                return *static_cast<VAR *>(m_ptr.get());
            }
            return VAR();
        }

        template <typename VAR>
        VAR get() const requires std::is_pointer<VAR>::value
        {
            if(m_typeId == typeid (VAR).hash_code())
            {
                return static_cast<VAR>(m_ptr.get());
            }
            return nullptr;
        }

        bool isValid() const
        {
            return m_typeId != typeid (std::nullptr_t).hash_code();
        }

        template <typename VAR>
        bool isSameType() const
        {
            return typeid (VAR).hash_code() == m_typeId;
        }

        std::size_t typeId() const
        {
            return m_typeId;
        }

    private:
        std::shared_ptr<void> m_ptr;
        std::size_t m_typeId;

    };
}

#endif // TA_VARIANT_H
