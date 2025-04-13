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

#ifndef TA_VARIANT_H
#define TA_VARIANT_H

#include <typeinfo>
#include <memory>
#include <cstring>

#include "TA_CommonTools.h"
#include "TA_MetaStringView.h"

namespace CoreAsync {
    template <std::size_t SSO_SIZE = 64>
    class TA_Variant
    {
    public:
        TA_Variant()
        {

        }

        template <typename T, typename ...Args>
        TA_Variant(T &&value, Args &&...args)
        {
            using RawType = std::remove_cvref_t<T>;
            m_typeId = typeid (RawType).hash_code();
            if constexpr(sizeof(RawType) <= ms_smallObjSize && std::alignment_of_v<RawType> <= ms_alignment)
            {
                new (m_storage.m_data) RawType(std::forward<RawType>(value), std::forward<Args>(args)...);
                m_destroySSOExp = [](void *data) {std::destroy_at(reinterpret_cast<RawType *>(data));};
                m_isSmallObject = true;
            }
            else
            {
                m_storage.m_ptr = std::make_shared<RawType>(std::forward<RawType>(value), std::forward<Args>(args)...);
                m_destroySSOExp = [](void *ptr) {std::destroy_at(reinterpret_cast<RawType *>(ptr));};
                m_isSmallObject = false;
            }
        }

        ~TA_Variant()
        {
            destroy();
        }

        TA_Variant(const TA_Variant &var) : m_typeId(var.m_typeId), m_isSmallObject(var.m_isSmallObject)
        {
            destroy();
            if(m_isSmallObject)
            {
                std::memcpy(m_storage.m_data, var.m_storage.m_data, ms_smallObjSize);
            }
            else
            {
                m_storage.m_ptr = var.m_storage.m_ptr;
            }
            m_destroySSOExp = var.m_destroySSOExp;
        }

        TA_Variant(TA_Variant &&var) : m_typeId(std::move(var.m_typeId)), m_isSmallObject(std::move(var.m_isSmallObject))
        {
            destroy();
            if(m_isSmallObject)
            { 
                std::memcpy(m_storage.m_data, var.m_storage.m_data, ms_smallObjSize);
            }
            else
            {
                m_storage.m_ptr = std::exchange(var.m_storage.m_ptr, nullptr);
            }
            m_destroySSOExp = std::exchange(var.m_destroySSOExp, nullptr);
        }

        TA_Variant & operator = (const TA_Variant &var)
        {
            if(this != &var)
            {
                m_typeId = var.m_typeId;
                m_isSmallObject = var.m_isSmallObject;
                destroy();
                if(m_isSmallObject)
                {       
                    std::memcpy(m_storage.m_data, var.m_storage.m_data, ms_smallObjSize);
                }
                else
                {
                    m_storage.m_ptr = var.m_storage.m_ptr;
                }
                m_destroySSOExp = var.m_destroySSOExp;
            }
            return *this;
        }

        TA_Variant & operator = (TA_Variant &&var)
        {
            if(this != &var)
            {
                m_typeId = std::exchange(var.m_typeId, 0);
                m_isSmallObject = std::exchange(var.m_isSmallObject, false);
                destroy();
                if(m_isSmallObject)
                {                       
                    std::memcpy(m_storage.m_data, var.m_storage.m_data, ms_smallObjSize);
                }
                else
                {
                    m_storage.m_ptr = std::exchange(var.m_storage.m_ptr, nullptr);
                }
                m_destroySSOExp = std::exchange(var.m_destroySSOExp, nullptr);
            }
            return *this;
        }

        template<typename T>
        void set(T &&obj)
        {
            using RawType = std::remove_cvref_t<T>;
            destroy();
            if constexpr(std::is_same_v<RawType, TA_Variant>)
            {
                (*this) = obj;
            }
            else
            {
                m_typeId = typeid (RawType).hash_code();
                if constexpr(sizeof(RawType) <= ms_smallObjSize && std::alignment_of_v<RawType> <= ms_alignment)
                {
                    new (m_storage.m_data) RawType(std::forward<RawType>(obj));
                    m_destroySSOExp = [](void *data) {std::destroy_at(reinterpret_cast<RawType *>(data));};
                    m_isSmallObject = true;
                }
                else
                {
                    m_storage.m_ptr = std::make_shared<RawType>(std::forward<RawType>(obj));
                    m_destroySSOExp = [](void *ptr) {std::destroy_at(reinterpret_cast<RawType *>(ptr));};
                    m_isSmallObject = false;
                }
            }
        }

        template <typename VAR>
        VAR get() const/* requires (!std::is_pointer<VAR>::value)*/
        {
            if(m_typeId == typeid (VAR).hash_code())
            {
                if(m_isSmallObject)
                {
                    return *reinterpret_cast<const VAR *>(m_storage.m_data);
                }
                else
                {
                    return *reinterpret_cast<VAR *>(m_storage.m_ptr.get());
                }
            }
            TA_CommonTools::debugInfo(META_STRING("Input type is not matched."));
            return VAR {};
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
        void destroy()
        {
            if(m_typeId != 0 && m_destroySSOExp)
            {
                if(m_isSmallObject)
                    m_destroySSOExp(m_storage.m_data);
                else if(m_storage.m_ptr.use_count() == 1)
                    m_destroySSOExp(m_storage.m_ptr.get());
            }
            m_destroySSOExp = nullptr;
        }

    private:
        std::size_t m_typeId {typeid (std::nullptr_t).hash_code()};

        static constexpr std::size_t ms_smallObjSize {SSO_SIZE};
        static constexpr std::size_t ms_alignment {std::alignment_of_v<std::max_align_t>};

        union Storage
        {
            std::byte m_data[ms_smallObjSize];
            std::shared_ptr<void> m_ptr {nullptr};

            ~Storage()
            {

            }
        } m_storage;

        bool m_isSmallObject {false};
        void (*m_destroySSOExp)(void *) {nullptr};
    };

    using TA_DefaultVariant = TA_Variant<>;
}

#endif // TA_VARIANT_H
