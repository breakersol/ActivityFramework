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

#include "Components/TA_MetaObject.h"
#include "Components/TA_ConnectionUtils.h"

namespace CoreAsync
{
    TA_MetaObject::TA_MetaObject() : m_pRegister(new TA_ConnectionsRegister()), m_pRecorder(new TA_ConnectionsRecorder(this))
    {

    }

    TA_MetaObject::~TA_MetaObject()
    {
        if (m_pRegister)
        {
            delete m_pRegister;
            m_pRegister = nullptr;
        }
        if(m_pRecorder)
        {
            delete m_pRecorder;
            m_pRecorder = nullptr;
        }
    }

    bool TA_MetaObject::registConnection(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
        return m_pRegister->registConnection(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
    }

    bool TA_MetaObject::removeConnection(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
        return m_pRegister->removeConnection(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
    }

    bool TA_MetaObject::recordSender(TA_MetaObject *pSender)
    {
        return m_pRecorder->record(pSender);
    }

    bool TA_MetaObject::removeSender(TA_MetaObject *pSender)
    {
        return m_pRecorder->remove(pSender);
    }
}

