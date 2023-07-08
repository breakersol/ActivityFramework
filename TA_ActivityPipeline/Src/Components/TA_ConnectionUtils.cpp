/*
 * Copyright [2023] [Shuang Zhu / Sol]
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

#include <thread>

namespace CoreAsync
{
    TA_ConnectionsRegister::TA_ConnectionsRegister()
    {

    }

    TA_ConnectionsRegister::~TA_ConnectionsRegister()
    {

    }

    bool TA_ConnectionsRegister::insert(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
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
            if(end != m_connections.end() && end->second.operator == (std::forward<TA_ConnectionUnit>(unit)))
            {
                return false;
            }
            m_connections.emplace(std::forward<std::string_view>(object), std::forward<TA_ConnectionUnit>(unit));
            return true;
        }
    }

    bool TA_ConnectionsRegister::remove(std::string_view &&object, TA_ConnectionUnit &&unit)
    {
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
        if(end != m_connections.end() && end->second.operator == (std::forward<TA_ConnectionUnit>(unit)))
        {
            m_connections.erase(std::forward<std::decay_t<decltype(end)>>(end));
            return true;
        }
        return false;
    }

    void TA_ConnectionsRegister::clear()
    {
        m_connections.clear();
    }

    std::size_t TA_ConnectionsRegister::size() const
    {
        return m_connections.size();
    }

    std::atomic<bool> TA_ConnectionResponder::m_enableConsume {true};

    TA_ConnectionResponder & TA_ConnectionResponder::GetIns()
    {
        static TA_ConnectionResponder responder;
        return responder;
    }

    TA_ConnectionResponder::TA_ConnectionResponder()
    {
        static std::jthread consumeThread {[this]() {consume();}};
    }

    TA_ConnectionResponder::~TA_ConnectionResponder()
    {
       m_enableConsume.store(false,std::memory_order_release);
       TA_CommonTools::debugInfo(META_STRING("Destroy Responder!\n"));
    }

    bool TA_ConnectionResponder::response(TA_BasicActivity *&pActivity, TA_ConnectionType type)
    {
        if(!pActivity)
            return false;
        switch (type) {
        case TA_ConnectionType::Direct:
        {
            auto ft = TA_ThreadHolder::get().postActivity(pActivity, true);
            return true;
        }
        case TA_ConnectionType::Queued:
        {
            return m_queue.push(pActivity);
        }
        default:
            return false;
        }
    }

    void TA_ConnectionResponder::consume()
    {
        while(m_enableConsume.load(std::memory_order_acquire))
        {
            TA_BasicActivity *pActivity {nullptr};
            if(m_queue.front() && m_queue.pop(pActivity))
            {
                (*pActivity)();
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if(pActivity)
            {
                delete pActivity;
                pActivity = nullptr;
            }
        }
    }
}
