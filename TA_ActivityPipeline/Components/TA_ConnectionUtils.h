#ifndef TA_CONNECTIONUTILS_H
#define TA_CONNECTIONUTILS_H

#include <type_traits>
#include <unordered_map>
#include <tuple>
#include <memory>

#include "TA_ReceiverObject.h"
#include "TA_MetaObject.h"
#include "TA_ActivityQueue.h"

#define TA_Signals  public  

namespace CoreAsync
{
    template <typename T>
    using EnableConnectObjectType = std::enable_if_t<!std::is_pointer_v<T> && std::is_convertible_v<T *, TA_MetaObject *>, T>;

    enum class TA_ConnectionType
    {
        Async,
        Sync
    };

    class TA_ConnectionUnit
    {
        friend class TA_ConnectionsRegister;

        void *m_pSender {nullptr};
        void *m_pReceiver {nullptr};
        std::string_view m_senderFunc {};
        std::string_view m_receiverFunc {};
        std::shared_ptr<TA_BaseReceiverObject> m_pReceiverObject {nullptr};
        TA_ConnectionType m_type {TA_ConnectionType::Sync};

    public:
        TA_ConnectionUnit() {}

        template <typename Sender, typename SenderFunc, typename Receiver, typename RecRet, typename RClass, typename ...RPara>
        TA_ConnectionUnit(Sender *&pSender, SenderFunc &&sFunc, Receiver *&pReceiver, RecRet(RClass::*&&rFunc)(RPara...), TA_ConnectionType type = TA_ConnectionType::Sync) : m_pSender(pSender),m_pReceiver(pReceiver),m_senderFunc(Reflex::TA_TypeInfo<std::decay_t<Sender> >::findName(std::forward<SenderFunc>(sFunc))),m_receiverFunc(Reflex::TA_TypeInfo<std::decay_t<Receiver> >::findName(std::forward<RecRet(RClass::*)(RPara...)>(rFunc))),m_pReceiverObject(new TA_ReceiverObject<std::decay_t<Receiver>>()),m_type(type)
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
        using ReceiversList = std::list<std::tuple<TA_BaseReceiverObject *, void *, std::string_view, TA_ConnectionType> >;

        bool insert(std::string_view &&object, TA_ConnectionUnit &&unit);
        bool remove(std::string_view &&object, TA_ConnectionUnit &&unit);
        void clear();

        std::size_t size() const;

        template <typename Sender, typename SenderFunc, typename = EnableConnectObjectType<Sender> >
        ReceiversList findReceivers(Sender *&pSender, SenderFunc &&sFunc)
        {
            ReceiversList list {};

            using ParentType = typename FunctionTypeInfo<SenderFunc>::ParentClass;
            if(!std::is_convertible_v<std::decay_t<Sender> *, ParentType *>)
                return list;
            if constexpr(!Reflex::TA_MemberTypeTrait<SenderFunc>::noneStaticMemberFuncFlag || !IsReturnTypeEqual<void, std::decay_t<SenderFunc>, std::is_same>::value)
            {
                return list;
            }
            std::string_view senderFuncName {Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<SenderFunc>(sFunc))};
            if(senderFuncName.empty())
                return list;
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
            if(end != m_connections.end())
            {
                auto &&endUnit {end->second};
                if(endUnit.m_pSender == pSender && endUnit.m_senderFunc == senderFuncName)
                {
                    list.emplace_back(std::tuple {endUnit.m_pReceiverObject.get(), endUnit.m_pReceiver, endUnit.m_receiverFunc, endUnit.m_type});
                }
            }
            return list;
        }

        TA_ConnectionsRegister();
        ~TA_ConnectionsRegister();

    private:
        std::unordered_multimap<std::string_view, TA_ConnectionUnit> m_connections;

    };

    class ASYNC_PIPELINE_EXPORT TA_ConnectionResponder
    {
    public:
        static TA_ConnectionResponder & GetIns();

        bool response(TA_BasicActivity *&pActivity, TA_ConnectionType type);

    private:
        TA_ConnectionResponder();
        ~TA_ConnectionResponder();

        void consume();

    private:
        ActivityQueue m_queue {};
        std::atomic<bool> m_enableConsume {true};

    };

}

#endif // TA_CONNECTIONUTILS_H
