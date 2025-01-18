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

#ifndef TA_METAOBJECT_H
#define TA_METAOBJECT_H

#include "TA_ActivityFramework_global.h"
#include "Components/TA_ConnectionUtils.h"
#include "Components/TA_ThreadPool.h"

#include <string_view>
#include <thread>
#include <memory>
#include <unordered_map>

namespace CoreAsync
{
    namespace TA_ConnectionUtils
    {
        class TA_ConnectionObject;
    }

    class ACTIVITY_FRAMEWORK_EXPORT TA_MetaObject
    {
    public:
        TA_MetaObject() : m_sourceThread(std::this_thread::get_id()),m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread())
        {

        }

        ~TA_MetaObject()
        {
            destroyConnections();
        }

        TA_MetaObject(const TA_MetaObject &object) : m_sourceThread(std::this_thread::get_id()),m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()), m_outputConnections(object.m_outputConnections), m_inputConnections(object.m_inputConnections)
        {

        }

        TA_MetaObject(TA_MetaObject &&object) : m_sourceThread(std::this_thread::get_id()),m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()), m_outputConnections(std::move(object.m_outputConnections)), m_inputConnections(std::move(object.m_inputConnections))
        {

        }

        TA_MetaObject & operator = (const TA_MetaObject &object)
        {
            if(this != &object)
            {
                m_outputConnections = object.m_outputConnections;
                m_inputConnections = object.m_inputConnections;
                m_affinityThreadIdx.store(object.affinityThread(), std::memory_order_release);
            }
            return *this;
        }

        TA_MetaObject & operator = (TA_MetaObject &&object)
        {
            if(this != &object)
            {
                m_outputConnections = std::move(object.m_outputConnections);
                m_inputConnections = std::move(object.m_inputConnections);
                m_affinityThreadIdx.store(std::move(object.affinityThread()), std::memory_order_release);
            }
            return *this;
        }

        const std::thread::id & sourceThread() const {return m_sourceThread;}
        std::size_t affinityThread() const {return m_affinityThreadIdx.load(std::memory_order_acquire);}
        bool moveToThread(std::size_t idx)
        {
            if(idx >= TA_ThreadHolder::get().size())
            {
                return false;
            }
            m_affinityThreadIdx.store(idx, std::memory_order_release);
            return true;
        }

        template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
        static bool registerConnection(Sender *pSender, Signal signal, Receiver *pReceiver, Slot slot, TA_ConnectionType type)
        {
            if(!pSender || !pReceiver)
            {
                return false;
            }
            if constexpr(!Reflex::TA_TypeInfo<std::decay_t<Sender> >::containsValue(signal) || !Reflex::TA_TypeInfo<std::decay_t<Receiver> >::containsValue(slot))
            {
                return false;
            }
            auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signal);
            while(startSendIter != endSendIter)
            {
                if(startSendIter->second->receiver() == pReceiver && startSendIter->second->slot() == slot)
                {
                    return false;
                }
                startSendIter++;
            }
            auto &&conn = new TA_ConnectionUtils::TA_ConnectionObject(pSender, signal, pReceiver, slot, type);
            pSender->m_outputConnections.emplace(signal, conn);
            pReceiver->m_inputConnections.emplace(slot, conn);
            return true;
        }

        template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
        static bool unregisterConnection(Sender *pSender, Signal signal, Receiver *pReceiver, Slot slot)
        {
            if(!pSender || !pReceiver)
            {
                return false;
            }
            auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signal);
            if(startSendIter == endSendIter)
                return false;
            TA_ConnectionUtils::TA_ConnectionObject *pConnecton {nullptr};
            while(startSendIter != endSendIter)
            {
                if(startSendIter->second->receiver() == pReceiver && startSendIter->second->slot() == slot)
                {
                    pConnecton = startSendIter->second;
                    pSender->m_outputConnections.erase(startSendIter);
                    break;
                }
                startSendIter++;
            }
            auto &&[startRecIter, endRecIter] = pReceiver->m_inputConnections.equal_range(slot);
            if(startRecIter == endRecIter)
                return false;
            while(startRecIter != endRecIter)
            {
                if(startRecIter->second == pConnecton)
                {
                    pReceiver->m_inputConnections.erase(startRecIter);
                    break;
                }
                startRecIter++;
            }
            if(pConnecton)
            {
                delete pConnecton;
                pConnecton = nullptr;
            }
            return true;
        }

    private:
        void destroyConnections()
        {
            for(auto &&[signal, obj] : m_outputConnections)
            {
                auto &&[start, end] = obj->receiver()->m_inputConnections.equal_range(obj->slot());
                while(start != end)
                {
                    if(start->second == obj)
                    {
                        obj->receiver()->m_inputConnections.erase(start);
                        break;
                    }
                    ++start;
                }
                delete obj;
                obj = nullptr;
            }
            m_outputConnections.clear();

            for(auto &&[slot, obj] : m_inputConnections)
            {
                auto &&[start, end] = obj->sender()->m_outputConnections.equal_range(obj->signal());
                while(start != end)
                {
                    if(start->second == obj)
                    {
                        obj->sender()->m_outputConnections.erase(start);
                        break;
                    }
                    ++start;
                }
                delete obj;
                obj = nullptr;
            }
            m_inputConnections.clear();
        }

    private:
        const std::thread::id m_sourceThread;
        std::atomic_size_t m_affinityThreadIdx;

        std::unordered_multimap<TA_ConnectionUtils::TA_ConnectionObject::FuncAddr, TA_ConnectionUtils::TA_ConnectionObject *> m_outputConnections {};
        std::unordered_multimap<TA_ConnectionUtils::TA_ConnectionObject::FuncAddr, TA_ConnectionUtils::TA_ConnectionObject *> m_inputConnections {};

    };
}

#endif // TA_METAOBJECT_H
