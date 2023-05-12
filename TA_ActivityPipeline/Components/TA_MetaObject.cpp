#include "Components/TA_MetaObject.h"
#include "Components/TA_ConnectionUtils.h"

namespace CoreAsync
{
    TA_MetaObject::TA_MetaObject() : m_pRegister(new TA_ConnectionsRegister())
    {

    }

    TA_MetaObject::~TA_MetaObject()
    {
        if(m_pRegister)
        {
            clear();
            delete m_pRegister;
            m_pRegister = nullptr;
        }
    }

    bool TA_MetaObject::insert(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
        return m_pRegister->insert(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
    }

    bool TA_MetaObject::remove(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
        return m_pRegister->remove(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
    }

    void TA_MetaObject::clear()
    {
        m_pRegister->clear();
    }
}

