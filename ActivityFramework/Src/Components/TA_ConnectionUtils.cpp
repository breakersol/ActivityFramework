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

#include "Components/TA_ConnectionUtils.h"
#include "Components/TA_ThreadPool.h"

namespace CoreAsync
{
    TA_ConnectionsRegister::TA_ConnectionsRegister()
    {

    }

    TA_ConnectionsRegister::~TA_ConnectionsRegister()
    {
        clear();
    }

    bool TA_ConnectionsRegister::registConnection(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if(!m_connections.contains(std::forward<std::string_view>(object)))
        {
            m_connections.emplace(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
            return true;
        }
        else
        {
            auto && [start, end] = m_connections.equal_range(std::forward<std::string_view>(object));
            while(start != end)
            {
                if(start->second.operator == (std::forward<TA_ConnectionUnit>(unit)))
                    return false;
                start++;
            }
            m_connections.emplace(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
            return true;
        }
    }

    bool TA_ConnectionsRegister::removeConnection(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto && [start, end] = m_connections.equal_range(std::forward<std::string_view>(object));
        if(start == m_connections.end() && end == m_connections.end())
        {
            return false;
        }
        while(start != end)
        {
            if(start->second.operator == (std::forward<TA_ConnectionUnit>(unit)))
            {
                m_connections.erase(std::forward<std::decay_t<decltype(start)>>(start));
                return true;
            }
            start++;
        }
        return false;
    }

    void TA_ConnectionsRegister::removeConnection(void *pReceiver)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        std::erase_if(m_connections, [&pReceiver](const auto &item)
        {
            auto const &[k, v] = item;
            return v.m_pReceiver == pReceiver;
        });
    }

    void TA_ConnectionsRegister::clear()
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto &[k, v] : m_connections)
        {
            reinterpret_cast<TA_MetaObject *>(v.m_pReceiver)->m_pRecorder->remove(v.m_pSender);
        }
        m_connections.clear();
    }

    std::size_t TA_ConnectionsRegister::size() const
    {
        std::shared_lock<std::shared_mutex> lock(*const_cast<std::shared_mutex *>(&m_mutex));
        return m_connections.size();
    }

    TA_ConnectionResponder & TA_ConnectionResponder::GetIns()
    {
        static TA_ConnectionResponder responder;
        return responder;
    }

    TA_ConnectionResponder::TA_ConnectionResponder()
    {

    }

    TA_ConnectionResponder::~TA_ConnectionResponder()
    {
        TA_CommonTools::debugInfo(META_STRING("Destroy Responder!\n"));
    }

    bool TA_ConnectionResponder::response(TA_ActivityProxy *&pActivity)
    {
        if(!pActivity)
            return false;
        TA_ThreadHolder::get().postActivity(pActivity);
        return true;
    }

    TA_ConnectionsRecorder::TA_ConnectionsRecorder(void *pReceiver) : m_pReceiver(pReceiver)
    {

    }

    TA_ConnectionsRecorder::~TA_ConnectionsRecorder()
    {
        clear();
    }

    bool TA_ConnectionsRecorder::record(void *pObject)
    {
        if (!pObject)
            return false;
        m_mutex.lock();
        m_recordSet.emplace(pObject);
        m_mutex.unlock();
        return true;
    }

    bool TA_ConnectionsRecorder::remove(void *pObject)
    {
        if (!pObject)
            return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_recordSet.count(pObject) != 0)
        {
            m_recordSet.erase(pObject);
            return true;
        } 
        return false;
    }

    void TA_ConnectionsRecorder::clear()
    {
        m_mutex.lock();
        for (auto &item : m_recordSet)
        {
            reinterpret_cast<TA_MetaObject *>(item)->m_pRegister->removeConnection(m_pReceiver);
        }
        m_recordSet.clear();
        m_mutex.unlock();
    }
}

