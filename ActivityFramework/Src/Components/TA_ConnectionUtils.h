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

#ifndef TA_CONNECTIONUTILS_H
#define TA_CONNECTIONUTILS_H

#include <any>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <memory>
#include <shared_mutex>

#include "TA_ReceiverObject.h"
#include "TA_MetaObject.h"
#include "TA_ActivityQueue.h"

#define TA_Signals  public  

namespace CoreAsync
{
    template <typename T>
    concept EnableConnectObjectType = requires (T *t)
    {
        {t}->std::convertible_to<TA_MetaObject *>;
    };

    enum class TA_ConnectionType
    {
        Auto,
        Direct,
        Queued
    };

    namespace TA_ConnectionUtils
    {
        template <typename ...Args>
        using Executor = void(*)(void *, Args...);

        class TA_ConnectionObject
        {
        public:
            TA_ConnectionObject() = default;
            template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
            TA_ConnectionObject(Sender *pSender, Signal signal, Receiver *pReceiver, Slot slot, TA_ConnectionType type) : m_pSender(pSender),m_pReceiver(pReceiver), m_type(type)
            {
                using SlotParaTuple = typename FunctionTypeInfo<Slot>::ParaGroup::Tuple;
                using Ret = typename FunctionTypeInfo<Slot>::Ret;
                m_para = SlotParaTuple {};
                m_slot = [&,slot]()->void {
                    decltype(auto) rObj {dynamic_cast<std::decay_t<Receiver> *>(m_pReceiver)};
                    if constexpr(std::is_invocable_v<Slot>)
                        std::apply(slot, std::move(std::tuple_cat(std::make_tuple(rObj), std::any_cast<SlotParaTuple &>(m_para))));
                };
                {
                    auto activity = new TA_SingleActivity<LambdaTypeWithoutPara<Ret>, INVALID_INS,Ret,INVALID_INS>(std::forward<decltype(m_slot)>(m_slot), pReceiver->affinityThread());
                    m_pSlotProxy = new TA_ActivityProxy(activity);
                }

            }

            ~TA_ConnectionObject()
            {
                if(m_pSlotProxy)
                {
                    delete m_pSlotProxy;
                    m_pSlotProxy = nullptr;
                }
            }

            template <typename ...Args>
            void setPara(Args &&...args)
            {
                if constexpr(sizeof...(Args) != 0)
                    m_para.emplace(std::move(std::make_tuple(std::forward<Args>(args)...)));
            }

            void callSlot()
            {
                if(m_slot)
                {
                    TA_MetaObject *pSender = reinterpret_cast<TA_MetaObject *>(m_pSender);
                    TA_MetaObject *pReceiver = reinterpret_cast<TA_MetaObject *>(m_pReceiver);
                    if(pReceiver->sourceThread() == pSender->sourceThread() && (m_type == TA_ConnectionType::Direct || m_type == TA_ConnectionType::Auto))
                    {
                        m_slot();
                    }
                    else
                    {
                        auto fetcher = TA_ThreadHolder::get().postActivity(m_pSlotProxy);
                    }
                }
            }

        private:
            TA_MetaObject *m_pSender {nullptr}, *m_pReceiver {nullptr};
            TA_ConnectionType m_type;
            std::function<void()> m_slot;
            std::any m_para;
            TA_ActivityProxy *m_pSlotProxy {nullptr};
        };
    }

    class TA_ConnectionUnit
    {
        friend class TA_ConnectionsRegister;

        TA_MetaObject *m_pSender {nullptr};
        TA_MetaObject *m_pReceiver {nullptr};
        std::string_view m_senderFunc {};
        std::string_view m_receiverFunc {};
        std::shared_ptr<TA_BaseReceiverObject> m_pReceiverObject {nullptr};
        TA_ConnectionType m_type {TA_ConnectionType::Auto};

    public:
        TA_ConnectionUnit() {}

        template <typename Sender, typename SenderFunc, typename Receiver, typename RecRet, typename RClass, typename ...RPara>
        TA_ConnectionUnit(Sender *&pSender, SenderFunc &&sFunc, Receiver *&pReceiver, RecRet(RClass::*&&rFunc)(RPara...), TA_ConnectionType type = TA_ConnectionType::Auto) : m_pSender(pSender),m_pReceiver(pReceiver),m_senderFunc(Reflex::TA_TypeInfo<std::decay_t<Sender> >::findName(std::forward<SenderFunc>(sFunc))),m_receiverFunc(Reflex::TA_TypeInfo<std::decay_t<Receiver> >::findName(std::forward<RecRet(RClass::*)(RPara...)>(rFunc))),m_pReceiverObject(new TA_ReceiverObject<std::decay_t<Receiver>>()),m_type(type)
        {

        }

        bool operator == (const TA_ConnectionUnit &unit) const
        {
            return m_pSender == unit.m_pSender && m_pReceiver == unit.m_pReceiver && m_senderFunc == unit.m_senderFunc && m_receiverFunc == unit.m_receiverFunc;
        }

        bool operator == (TA_ConnectionUnit &&unit) const
        {
            return m_pSender == unit.m_pSender && m_pReceiver == unit.m_pReceiver && m_senderFunc == unit.m_senderFunc && m_receiverFunc == unit.m_receiverFunc;
        }

        bool isValid() const
        {
            return m_pSender && m_pReceiver && !m_senderFunc.empty() && !m_receiverFunc.empty();
        }
    };

    class TA_ConnectionsRegister
    {
    public:
        using ReceiversList = std::list<std::tuple<TA_BaseReceiverObject *, TA_MetaObject *, std::string_view, TA_ConnectionType> >;

        bool registConnection(std::string_view &&object, TA_ConnectionUnit &&unit);
        bool removeConnection(std::string_view &&object, TA_ConnectionUnit &&unit);
        void removeConnection(void *pReceiver);

        std::size_t size() const;

        template <EnableConnectObjectType Sender, typename SenderFunc>
        ReceiversList findReceiverWrappers(Sender *&pSender, SenderFunc &&sFunc)
        {
            ReceiversList list {};

            using ParentType = typename FunctionTypeInfo<SenderFunc>::ParentClass;
            if constexpr(!std::is_convertible_v<std::decay_t<Sender> *, ParentType *>)
                return list;
            if constexpr(!Reflex::TA_MemberTypeTrait<SenderFunc>::instanceMethodFlag || !IsReturnTypeEqual<void, std::decay_t<SenderFunc>, std::is_same>::value)
            {
                return list;
            }
            std::string_view senderFuncName {Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<SenderFunc>(sFunc))};
            if(senderFuncName.empty())
                return list;
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            auto && [start, end] = m_connections.equal_range(typeid(ParentType).name());
            if(start == m_connections.end() && end == m_connections.end())
            {
                return list;
            }
            while(start != end)
            {
                auto &&unit {start->second};
                if(unit.m_pSender == pSender && unit.m_senderFunc == senderFuncName)
                {
                    list.emplace_back(std::tuple {unit.m_pReceiverObject.get(), unit.m_pReceiver, unit.m_receiverFunc, unit.m_type});
                }
                start++;
            }
            return list;
        }

        TA_ConnectionsRegister();
        ~TA_ConnectionsRegister();

        TA_ConnectionsRegister(const TA_ConnectionsRegister &reg) = delete;
        TA_ConnectionsRegister(TA_ConnectionsRegister &&reg) = delete;

        TA_ConnectionsRegister & operator = (const TA_ConnectionsRegister &reg) = delete;
        TA_ConnectionsRegister & operator = (TA_ConnectionsRegister &&reg) = delete;

    private:
        void clear();

    private:
        std::unordered_multimap<std::string_view, TA_ConnectionUnit> m_connections;
        std::shared_mutex m_mutex;

    };

    class TA_ConnectionResponder
    {
    public:
        ACTIVITY_FRAMEWORK_EXPORT static TA_ConnectionResponder & GetIns();

        ACTIVITY_FRAMEWORK_EXPORT bool response(TA_ActivityProxy *&pActivity);

    private:
        TA_ConnectionResponder();
        ~TA_ConnectionResponder();

    };

    class TA_ConnectionsRecorder
    {
    public:
        TA_ConnectionsRecorder(void *pReceiver);
        ~TA_ConnectionsRecorder();

        TA_ConnectionsRecorder(const TA_ConnectionsRecorder &recorder) = delete;
        TA_ConnectionsRecorder(TA_ConnectionsRecorder &&recorder) = delete;

        bool record(void *pObject);
        bool remove(void *pObject);

    private:
        void clear();

    private:
        std::unordered_set<void *> m_recordSet;
        void *m_pReceiver;
        std::mutex m_mutex;

    };

}

#endif // TA_CONNECTIONUTILS_H
