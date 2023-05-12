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

    TA_Variant::TA_Variant(TA_Variant &&var)
    {
        m_ptr = var.m_ptr;
        m_typeId = var.m_typeId;
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
}

