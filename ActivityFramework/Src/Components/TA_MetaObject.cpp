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
#include "Components/TA_ThreadPool.h"

namespace CoreAsync
{
    TA_MetaObject::TA_MetaObject() : m_pRegister(new TA_ConnectionsRegister()), m_pRecorder(new TA_ConnectionsRecorder(this)), m_sourceThread(std::this_thread::get_id()),m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread())
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

    TA_MetaObject::TA_MetaObject(const TA_MetaObject &object) : m_pRegister(object.m_pRegister), m_pRecorder(object.m_pRecorder), m_sourceThread(std::this_thread::get_id()),m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread())
    {

    }

    TA_MetaObject::TA_MetaObject(TA_MetaObject &&object) : m_pRegister(std::move(object.m_pRegister)), m_pRecorder(std::move(object.m_pRecorder)), m_sourceThread(std::this_thread::get_id()),m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread())
    {

    }

    TA_MetaObject & TA_MetaObject::operator = (const TA_MetaObject &object)
    {
        if(this != &object)
        {
            m_pRegister = object.m_pRegister;
            m_pRecorder = object.m_pRecorder;
            m_affinityThreadIdx.store(object.affinityThread(), std::memory_order_release);
        }
        return *this;
    }

    TA_MetaObject & TA_MetaObject::operator = (TA_MetaObject &&object)
    {
        if(this != &object)
        {
            m_pRegister = std::move(object.m_pRegister);
            m_pRecorder = std::move(object.m_pRecorder);
            m_affinityThreadIdx.store(std::move(object.affinityThread()), std::memory_order_release);
        }
        return *this;
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

    bool TA_MetaObject::moveToThread(std::size_t idx)
    {
        if(idx >= TA_ThreadHolder::get().size())
        {
            return false;
        }
        m_affinityThreadIdx.store(idx, std::memory_order_release);
        return true;
    }
}

