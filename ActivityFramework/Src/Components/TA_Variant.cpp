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

#include "TA_Variant.h"

namespace CoreAsync {
    TA_Variant::TA_Variant() : m_ptr(nullptr),m_typeId(typeid (std::nullptr_t).hash_code())
    {

    }

    TA_Variant::~TA_Variant()
    {

    }

    TA_Variant::TA_Variant(const TA_Variant &var)
    {
        m_ptr = var.m_ptr;
        m_typeId = var.m_typeId;
    }

    TA_Variant::TA_Variant(TA_Variant &&var) : m_ptr(std::move(var.m_ptr)), m_typeId(std::move(var.m_typeId))
    {

    }

    TA_Variant & TA_Variant::operator = (const TA_Variant &var)
    {
        if(this != &var)
        {
            m_ptr = var.m_ptr;
            m_typeId = var.m_typeId;
        }
        return *this;
    }

    TA_Variant & TA_Variant::operator = (TA_Variant &&var)
    {
        if(this != &var)
        {
            m_ptr = var.m_ptr;
            m_typeId = var.m_typeId;
        }
        return *this;
    }
}

