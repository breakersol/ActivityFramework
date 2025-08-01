/*
 * Copyright [2025] [Shuang Zhu / Sol]
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

#include <string_view>
#include <thread>
#include <unordered_map>
#include <any>

#include "TA_ThreadPool.h"
#include "TA_MetaReflex.h"
#include "TA_Activity.h"

namespace CoreAsync {
#define TA_Signals public

class TA_MetaObject;

template <typename T>
concept EnableConnectObjectType = requires(T *t) {
    { t } -> std::convertible_to<TA_MetaObject *>;
};

enum class TA_ConnectionType { Auto, Direct, Queued };

class TA_MetaObject {
    class TA_ConnectionObject;

  public:
    class TA_ConnectionObjectHolder {
        friend class TA_MetaObject;

      public:
        TA_ConnectionObjectHolder() = default;
        TA_ConnectionObjectHolder(const std::shared_ptr<TA_ConnectionObject> &pConnection)
            : m_pConnection(pConnection ? pConnection->getSharedPtr() : nullptr) {}

        ~TA_ConnectionObjectHolder() {}

        bool valid() const { return m_pConnection != nullptr; }

        void reset() {
            if (m_pConnection)
                m_pConnection.reset();
        }

      private:
        std::shared_ptr<TA_ConnectionObject> m_pConnection{nullptr};
    };

    TA_MetaObject() : m_sourceThread(std::this_thread::get_id()),
                      m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()) {}

    virtual ~TA_MetaObject() { destroyConnections(); }

    TA_MetaObject(const TA_MetaObject &object)
        : m_sourceThread(std::this_thread::get_id()), m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()),
          m_outputConnections(object.m_outputConnections), m_inputConnections(object.m_inputConnections) {}

    TA_MetaObject(TA_MetaObject &&object) noexcept
        : m_sourceThread(std::this_thread::get_id()), m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()),
          m_outputConnections(std::move(object.m_outputConnections)),
          m_inputConnections(std::move(object.m_inputConnections)) {}

    TA_MetaObject &operator=(const TA_MetaObject &object) {
        if (this != &object) {
            m_outputConnections = object.m_outputConnections;
            m_inputConnections = object.m_inputConnections;
            m_affinityThreadIdx.store(object.affinityThread(), std::memory_order_release);
        }
        return *this;
    }

    TA_MetaObject &operator=(TA_MetaObject &&object) noexcept {
        if (this != &object) {
            m_outputConnections = std::move(object.m_outputConnections);
            m_inputConnections = std::move(object.m_inputConnections);
            m_affinityThreadIdx.store(std::move(object.affinityThread()), std::memory_order_release);
        }
        return *this;
    }

  private:
    template <ActivityType Activity>
    inline static auto invokeActivity(Activity *pActivity, std::size_t idx, bool autoDelete = true) {
        if (idx >= TA_ThreadHolder::get().size()) {
            throw std::out_of_range("Thread index out of range.");
        }
        pActivity->moveToThread(idx);
        return TA_ThreadHolder::get().postActivity(pActivity, autoDelete);
    }

  public:
    const std::thread::id &sourceThread() const { return m_sourceThread; }

    template <LambdaExpType LambdaExp, typename... Args>
    static constexpr auto invokeMethod(LambdaExp &&exp, Args &&...args) {
        static_assert(LambdaExpTraits<std::decay_t<LambdaExp>>::argSize == sizeof...(Args),
                      "The number of arguments does not match the lambda expression.");
        return TA_ThreadHolder::get().postActivity(
            TA_ActivityCreator::create(std::forward<LambdaExp>(exp), std::forward<Args>(args)...), true);
    }

    template <LambdaExpType LambdaExp, typename... Args>
    static constexpr auto invokeMethod(LambdaExp &exp, Args &&...args) {
        static_assert(LambdaExpTraits<std::decay_t<LambdaExp>>::argSize == sizeof...(Args),
                      "The number of arguments does not match the lambda expression.");
        return TA_ThreadHolder::get().postActivity(
            TA_ActivityCreator::create(std::forward<LambdaExp>(exp), std::forward<Args>(args)...), true);
    }

    template <InstanceMethodType Method, typename Ins, typename... Args>
    static constexpr auto invokeMethod(Method &&method, Ins &ins, Args &&...args) {
        static_assert(MethodTypeInfo<std::decay_t<Method>>::argSize == sizeof...(Args),
                      "The number of arguments does not match the method signature.");
        using Ret = typename MethodTypeInfo<std::decay_t<Method>>::RetType;
        return TA_ThreadHolder::get().postActivity(
            TA_ActivityCreator::create(std::forward<Method>(method),
                                       std::forward<Ins>(ins),
                                       std::forward<Args>(args)...),
            true);
    }

    template <StaticMethodType Method, typename... Args>
    static constexpr auto invokeMethod(Method &&method, Args &&...args) {
        static_assert(MethodTypeInfo<std::decay_t<Method>>::argSize == sizeof...(Args),
                      "The number of arguments does not match the method signature.");
        using Ret = typename MethodTypeInfo<std::decay_t<Method>>::RetType;
        return TA_ThreadHolder::get().postActivity(
            TA_ActivityCreator::create(std::forward<Method>(method), std::forward<Args>(args)...), true);
    }

    template <MetaStringType Method, typename... Args> static constexpr auto invokeMethod(Method, Args &&...args) {
        return TA_ThreadHolder::get().postActivity(
            TA_ActivityCreator::create(Method{}, std::forward<Args>(args)...), true);
    }

    void deleteLater() {
        invokeMethod([this]() { delete this; });
    }

    std::size_t affinityThread() const { return m_affinityThreadIdx.load(std::memory_order_acquire); }

    bool moveToThread(std::size_t idx) {
        auto activity = TA_ActivityCreator::create(
            std::forward<bool(*)(std::size_t, std::atomic_size_t &)>(m_moveToThreadImpl),
            idx, m_affinityThreadIdx);
        auto fetcher = invokeActivity(activity, affinityThread());
        return fetcher().template get<bool>();
    }

    template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
    static constexpr bool registerConnection(Sender *pSender, Signal &&signal, Receiver *pReceiver, Slot &&slot,
                                             TA_ConnectionType type) {
        if constexpr (!Reflex::TA_MemberTypeTrait<Signal>::instanceMethodFlag ||
                      !Reflex::TA_MemberTypeTrait<Slot>::instanceMethodFlag ||
                      !IsReturnTypeEqual<void, Signal, std::is_same>::value ||
                      !IsReturnTypeEqual<void, Slot, std::is_same>::value) {
            return false;
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != MethodTypeInfo<Slot>::argSize) {
            return false;
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != 0 && MethodTypeInfo<Slot>::argSize != 0) {
            if constexpr (!MetaSame<typename MethodTypeInfo<Signal>::ArgGroup,
                                    typename MethodTypeInfo<Slot>::ArgGroup>::value) {
                return false;
            }
        }
        if (!pSender || !pReceiver) {
            return false;
        }
        return m_registerConnectionImpl<Sender, Signal, Receiver, Slot>(
            pSender, std::forward<Signal>(signal),
            pReceiver, std::forward<Slot>(slot), type);
    }

    template <EnableConnectObjectType Sender, typename Signal, LambdaExpType LambdaExp>
    static constexpr TA_ConnectionObjectHolder registerConnection(Sender *pSender, Signal &&signal, LambdaExp &&exp,
                                                                  TA_ConnectionType type, bool autoDestroy = false) {
        if constexpr (!Reflex::TA_MemberTypeTrait<Signal>::instanceMethodFlag ||
                      !IsReturnTypeEqual<void, Signal, std::is_same>::value ||
                      !std::is_same_v<typename LambdaExpTraits<std::decay_t<LambdaExp>>::RetType, void>) {
            return {nullptr};
        }
        if (!pSender) {
            return {nullptr};
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != LambdaExpTraits<std::decay_t<LambdaExp>>::argSize) {
            return {nullptr};
        }
        using ExpType = decltype(m_registerLambdaConnectionImpl<Sender, Signal, LambdaExp>);
        auto registerActivity = TA_ActivityCreator::create(
            std::forward<ExpType>(m_registerLambdaConnectionImpl<Sender, Signal, LambdaExp>),
            pSender, signal, exp, type, autoDestroy);
        auto fetcher = invokeActivity(registerActivity, pSender->affinityThread());
        auto res = fetcher();
        return res.template get<TA_ConnectionObjectHolder>();
    }

    template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
    static constexpr bool unregisterConnection(Sender *pSender, Signal &&signal, Receiver *pReceiver, Slot &&slot) {
        if constexpr (!Reflex::TA_MemberTypeTrait<Signal>::instanceMethodFlag ||
                      !Reflex::TA_MemberTypeTrait<Slot>::instanceMethodFlag ||
                      !IsReturnTypeEqual<void, Signal, std::is_same>::value) {
            return false;
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != MethodTypeInfo<Slot>::argSize) {
            return false;
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != 0 && MethodTypeInfo<Slot>::argSize != 0) {
            if constexpr (!MetaSame<typename MethodTypeInfo<Signal>::ArgGroup,
                                    typename MethodTypeInfo<Slot>::ArgGroup>::value) {
                return false;
            }
        }
        if (!pSender || !pReceiver) {
            return false;
        }
        TA_ConnectionObject::FuncMark signalMark{
            Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal))};
        TA_ConnectionObject::FuncMark slotMark{
            Reflex::TA_TypeInfo<std::decay_t<Receiver>>::findName(std::forward<Slot>(slot))};
        return m_unregisterConnectionImpl<Sender, Receiver>(
            pSender, std::forward<TA_ConnectionObject::FuncMark>(signalMark),
            pReceiver, std::forward<TA_ConnectionObject::FuncMark>(slotMark));
    }

    static bool unregisterConnection(const TA_ConnectionObjectHolder &holder) {
        using ExpType = decltype(m_unregisterConnectionHolderImpl<TA_MetaObject>);
        auto activity = TA_ActivityCreator::create(
            std::forward<ExpType>(m_unregisterConnectionHolderImpl<TA_MetaObject>), holder);
        auto fetcher = invokeActivity(activity, holder.m_pConnection->sender()->affinityThread());
        auto res = fetcher();
        return res.template get<bool>();
    }

    template <EnableConnectObjectType Sender, typename Signal, typename... Args>
    static constexpr bool emitSignal(Sender *pSender, Signal &&signal, Args &&...args) {
        if constexpr (!Reflex::TA_MemberTypeTrait<Signal>::instanceMethodFlag ||
                      !IsReturnTypeEqual<void, Signal, std::is_same>::value) {
            return false;
        }
        if constexpr (!std::is_convertible_v<std::decay_t<Sender *>, typename MethodTypeInfo<Signal>::ParentClass *>) {
            return false;
        }
        if (!pSender) {
            return false;
        }

        auto fetcher = invokeMethod(m_emitSignalImpl<Sender, Signal, Args...>,
                                    pSender, std::forward<Signal>(signal),
                                    std::forward<Args>(args)...);
        auto res = fetcher();
        return res.template get<bool>();
    }

    template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
    static constexpr bool isConnectionExisted(Sender *pSender, Signal &&signal, Receiver *pReceiver, Slot &&slot) {
        if constexpr (!Reflex::TA_MemberTypeTrait<Signal>::instanceMethodFlag ||
                      !Reflex::TA_MemberTypeTrait<Slot>::instanceMethodFlag ||
                      !IsReturnTypeEqual<void, Signal, std::is_same>::value) {
            return false;
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != MethodTypeInfo<Slot>::argSize) {
            return false;
        }
        if constexpr (MethodTypeInfo<Signal>::argSize != 0 && MethodTypeInfo<Slot>::argSize != 0) {
            if constexpr (!MetaSame<typename MethodTypeInfo<Signal>::ArgGroup,
                                    typename MethodTypeInfo<Slot>::ArgGroup>::value) {
                return false;
            }
        }
        if (!pSender || !pReceiver) {
            return false;
        }

        TA_ConnectionObject::FuncMark signalMark{
            Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal))};
        TA_ConnectionObject::FuncMark slotMark{
            Reflex::TA_TypeInfo<std::decay_t<Receiver>>::findName(std::forward<Slot>(slot))};

        auto activity = TA_ActivityCreator::create(m_isConnectionExistedImpl<Sender, Receiver>,
                                                   pSender, signalMark, pReceiver, slotMark);
        auto fetcher = TA_ThreadHolder::get().postActivity(activity, true);
        return fetcher().template get<bool>();
    }

  private:
    class TA_ConnectionObject : public std::enable_shared_from_this<TA_ConnectionObject> {
        using SlotExpType = std::function<void()>;

      public:
        TA_ConnectionObject() = default;
        template <EnableConnectObjectType Sender, typename Signal>
        TA_ConnectionObject(Sender *pSender, Signal &&signal, TA_ConnectionType type)
            : m_pSender(pSender),
              m_senderFunc(Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal))),
              m_type(type), m_autoDestroy(false) {}

        template <EnableConnectObjectType Sender, typename Signal>
        TA_ConnectionObject(Sender *pSender, Signal &&signal, TA_ConnectionType type, bool autoDestroy)
            : m_pSender(pSender), m_pReceiver(pSender),
              m_senderFunc(Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal))),
              m_type(type), m_autoDestroy(autoDestroy) {}

        TA_ConnectionObject(const TA_ConnectionObject &object) = delete;
        TA_ConnectionObject(TA_ConnectionObject &&object) = delete;

        TA_ConnectionObject &operator=(const TA_ConnectionObject &object) = delete;
        TA_ConnectionObject &operator=(TA_ConnectionObject &&object) = delete;

        ~TA_ConnectionObject() {
            if (m_pSlotProxy) {
                delete m_pSlotProxy;
                m_pSlotProxy = nullptr;
            }
            if (m_pActivity) {
                delete m_pActivity;
                m_pActivity = nullptr;
            }
        }

        template <EnableConnectObjectType Receiver, typename Slot>
        void initSlotObject(Receiver *pReceiver, Slot &&slot) {
            m_pReceiver = pReceiver;
            m_receiverFunc = Reflex::TA_TypeInfo<std::decay_t<Receiver>>::findName(std::forward<Slot>(slot));
            using SlotParaTuple = typename MethodTypeInfo<Slot>::ArgGroup::Tuple;
            using Ret = typename MethodTypeInfo<Slot>::RetType;
            m_para = SlotParaTuple{};
            auto sharedRef = getSharedPtr();
            SlotExpType slotExp = [sharedRef, slot]() -> void {
                decltype(auto) rObj{dynamic_cast<std::decay_t<Receiver> *>(sharedRef->m_pReceiver)};
                if constexpr (IsInstanceMethod<Slot>::value)
                    std::apply(slot, std::move(std::tuple_cat(std::make_tuple(rObj),
                                                              std::any_cast<SlotParaTuple>(sharedRef->m_para))));
            };
            {
                m_pActivity = TA_ActivityCreator::create(std::move(slotExp));
                m_pActivity->moveToThread(pReceiver->affinityThread());
                m_pSlotProxy = new TA_ActivityProxy(m_pActivity, false);
            }
        }

        template <LambdaExpType LambdaExp>
        void initSlotObject(LambdaExp &&exp) {
            m_receiverFunc = typeid(LambdaExp).name();
            using SlotParaTuple = typename LambdaExpTraits<std::decay_t<LambdaExp>>::ArgGroup::Tuple;
            using Ret = typename LambdaExpTraits<std::decay_t<LambdaExp>>::RetType;
            m_para = SlotParaTuple{};
            auto sharedRef = getSharedPtr();
            SlotExpType slotExp = [sharedRef, exp]() -> void {
                std::apply(exp, std::any_cast<SlotParaTuple>(sharedRef->m_para));
            };
            {
                m_pActivity = TA_ActivityCreator::create(std::move(slotExp));
                // Don't need to ensure the activity is moved to the sender's affinity thread
                // m_pActivity->moveToThread(m_pSender->affinityThread());
                m_pSlotProxy = new TA_ActivityProxy(m_pActivity, false);
            }
        }

        std::shared_ptr<TA_ConnectionObject> getSharedPtr() { return this->shared_from_this(); }
        std::weak_ptr<TA_ConnectionObject> getWeakPtr() { return this->weak_from_this(); };

        template <typename... Args> void setPara(Args &&...args) {
            using ArgsTypes = std::tuple<std::decay_t<Args>...>;
            if constexpr (sizeof...(Args) != 0)
                m_para.emplace<ArgsTypes>(std::move(std::make_tuple(std::forward<Args>(args)...)));
        }

        void callSlot() {
            if (m_pActivity) {
                TA_MetaObject *pSender = reinterpret_cast<TA_MetaObject *>(m_pSender);
                TA_MetaObject *pReceiver = reinterpret_cast<TA_MetaObject *>(m_pReceiver);
                if (pReceiver->affinityThread() == pSender->affinityThread() &&
                    (m_type == TA_ConnectionType::Direct || m_type == TA_ConnectionType::Auto)) {
                    m_pActivity->operator()();
                } else {
                    auto fetcher = TA_ThreadHolder::get().postActivity(m_pSlotProxy);
                    m_pSlotProxy = new TA_ActivityProxy(m_pActivity, false);
                }
            }
            if (m_autoDestroy) {
                removeConnectionReferences();
            }
        }

        void removeConnectionReferences() {
            m_removeConnectionReferenceImpl<TA_MetaObject, TA_MetaObject>(
                m_pSender, m_pReceiver,
                std::forward<FuncMark>(m_senderFunc), std::forward<FuncMark>(m_receiverFunc));
        }

        TA_MetaObject *sender() const { return m_pSender; }
        TA_MetaObject *receiver() const { return m_pReceiver; }

        using FuncMark = std::string_view;

        const FuncMark &signalMark() const { return m_senderFunc; }
        const FuncMark &slotMark() const { return m_receiverFunc; }

        bool isSync() const {
            return m_pSender->affinityThread() == m_pReceiver->affinityThread() &&
                   (m_type == TA_ConnectionType::Auto || m_type == TA_ConnectionType::Direct);
        }

        bool isAutoDestroy() const { return m_autoDestroy; }

      private:
        TA_MetaObject *m_pSender{nullptr}, *m_pReceiver{nullptr};
        FuncMark m_senderFunc{}, m_receiverFunc{};
        TA_ConnectionType m_type;
        std::any m_para;
        TA_MethodActivity<SlotExpType> *m_pActivity{nullptr};
        TA_ActivityProxy *m_pSlotProxy{nullptr};
        const bool m_autoDestroy{false};
    };

  private:
    void destroyConnections() {
        for (auto &&[signal, obj] : m_outputConnections) {
            auto &&[start, end] = obj->receiver()->m_inputConnections.equal_range(obj->slotMark());
            while (start != end) {
                if (start->second == obj) {
                    obj->receiver()->m_inputConnections.erase(start);
                    break;
                }
                ++start;
            }
        }
        m_outputConnections.clear();

        for (auto &&[slot, obj] : m_inputConnections) {
            auto &&[start, end] = obj->sender()->m_outputConnections.equal_range(obj->signalMark());
            while (start != end) {
                if (start->second == obj) {
                    obj->sender()->m_outputConnections.erase(start);
                    break;
                }
                ++start;
            }
        }
        m_inputConnections.clear();
    }

  private:
    const std::thread::id m_sourceThread;
    std::atomic_size_t m_affinityThreadIdx;

    std::unordered_multimap<TA_ConnectionObject::FuncMark, std::shared_ptr<TA_ConnectionObject>> m_outputConnections{};
    std::unordered_multimap<TA_ConnectionObject::FuncMark, std::shared_ptr<TA_ConnectionObject>> m_inputConnections{};

  private:
    template <typename Sender, typename Receiver>
    inline static auto m_isConnectionExistedImpl =
        [](Sender *pSender, TA_ConnectionObject::FuncMark &&signal,
           Receiver *pReceiver, TA_ConnectionObject::FuncMark &&slot) -> bool {
        auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signal);
        while (startSendIter != endSendIter) {
            if (startSendIter->second->receiver() == pReceiver &&
                startSendIter->second->slotMark() == slot) {
                return true;
            }
            startSendIter++;
        }
        return false;
    };

    template <typename Sender, typename Signal, typename... Args>
    inline static auto m_emitSignalImpl = [](Sender *pSender, Signal signal, Args... args) {
        auto [startIter, endIter] = pSender->m_outputConnections.equal_range(
            Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal)));
        if (startIter == endIter)
            return false;
        while (startIter != endIter) {
            auto obj = startIter++->second;
            obj->setPara(std::forward<Args>(args)...);
            obj->callSlot();
        }
        return true;
    };

    template <typename Sender>
    inline static auto m_unregisterConnectionHolderImpl = [](const TA_ConnectionObjectHolder &holder) -> bool {
        if (!holder.valid()) {
            return false;
        }
        auto &&pConnection = holder.m_pConnection;
        if (!pConnection) {
            return false;
        }
        Sender *pSender = pConnection->sender();
        auto &&signalMark = pConnection->signalMark();
        auto &&slotMark = pConnection->slotMark();
        auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signalMark);
        if (startSendIter == endSendIter)
            return false;
        while (startSendIter != endSendIter) {
            if (startSendIter->second == pConnection) {
                pSender->m_outputConnections.erase(startSendIter);
                break;
            }
            startSendIter++;
        }
        return true;
    };

    template <typename Sender, typename Receiver>
    inline static auto m_unregisterConnectionImpl = [](Sender *pSender, TA_ConnectionObject::FuncMark &&signal,
                                                        Receiver *pReceiver, TA_ConnectionObject::FuncMark &&slot) -> bool {
        std::shared_ptr<TA_ConnectionObject> pConnection{nullptr};
        auto senderActivity = TA_ActivityCreator::create([&pConnection, pSender, &signal, pReceiver, &slot]() ->bool {
            auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signal);
            if (startSendIter == endSendIter)
                return false;
            while (startSendIter != endSendIter) {
                if (startSendIter->second->receiver() == pReceiver &&
                    startSendIter->second->slotMark() == slot) {
                    pConnection = startSendIter->second;
                    pSender->m_outputConnections.erase(startSendIter);
                    break;
                }
                startSendIter++;
            }
            return true;
        });
        auto opFetcher = invokeActivity(senderActivity, pSender->affinityThread());
        auto res = opFetcher().template get<bool>();
        if(!res) {
            return false;
        }
        auto receiverActivity = TA_ActivityCreator::create([&pConnection, pReceiver, &slot]() -> bool {
            auto &&[startRecIter, endRecIter] = pReceiver->m_inputConnections.equal_range(slot);
            if (startRecIter == endRecIter)
                return false;
            while (startRecIter != endRecIter) {
                if (startRecIter->second == pConnection) {
                    pReceiver->m_inputConnections.erase(startRecIter);
                    break;
                }
                startRecIter++;
            }
            return true;
        });
        opFetcher = invokeActivity(receiverActivity, pReceiver->affinityThread());
        return opFetcher().template get<bool>();
    };

    template <typename Sender, typename Signal, LambdaExpType Exp>
    inline static auto m_registerLambdaConnectionImpl = [](Sender *pSender, Signal signal,
                                                           Exp exp, TA_ConnectionType type,
                                                           bool autoDestroy) -> TA_ConnectionObjectHolder {
        TA_ConnectionObject::FuncMark signalMark{
            Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal))};
        TA_ConnectionObject::FuncMark slotMark{typeid(Exp).name()};
        auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signalMark);
        while (startSendIter != endSendIter) {
            if (startSendIter->second->receiver() == pSender && startSendIter->second->slotMark() == slotMark) {
                return {nullptr};
            }
            startSendIter++;
        }

        auto &&conn = std::make_shared<TA_ConnectionObject>(pSender, std::move(signal), type, autoDestroy);
        conn->initSlotObject(std::move(exp));
        pSender->m_outputConnections.emplace(signalMark, conn->getSharedPtr());
        if (autoDestroy)
            return {nullptr};
        return {conn};
    };

    template <typename Sender, typename Signal, typename Receiver, typename Slot>
    inline static auto m_registerConnectionImpl = [](Sender *pSender, Signal &&signal,
                                                     Receiver *pReceiver, Slot &&slot,
                                                     TA_ConnectionType type) -> bool {
        TA_ConnectionObject::FuncMark signalMark{
            Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<Signal>(signal))};
        TA_ConnectionObject::FuncMark slotMark{
            Reflex::TA_TypeInfo<std::decay_t<Receiver>>::findName(std::forward<Slot>(slot))};
        auto searchActivity = TA_ActivityCreator::create([pSender, signalMark, pReceiver, slotMark]() -> bool {
            auto &&[startSendIter, endSendIter] = pSender->m_outputConnections.equal_range(signalMark);
            while (startSendIter != endSendIter) {
                if (startSendIter->second->receiver() == pReceiver &&
                    startSendIter->second->slotMark() == slotMark) {
                    return false;
                }
                startSendIter++;
            }
            return true;
        });
        auto searchFetcher = invokeActivity(searchActivity, pSender->affinityThread());
        bool res = searchFetcher().template get<bool>();
        if(!res)
            return false;
        auto &&conn = std::make_shared<TA_ConnectionObject>(pSender, std::move(signal), type);
        conn->initSlotObject(pReceiver, std::forward<Slot>(slot));
        auto addIntoSenderActivity = TA_ActivityCreator::create(
            [pSender, &signalMark, &conn]() -> void {
                pSender->m_outputConnections.emplace(signalMark, conn->getSharedPtr());
            }
        );
        auto addIntoSenderFetcher = invokeActivity(addIntoSenderActivity, pSender->affinityThread());
        addIntoSenderFetcher();
        auto addIntoReceiverActivity = TA_ActivityCreator::create([pReceiver, &slotMark, &conn]() -> void {
            pReceiver->m_inputConnections.emplace(slotMark, conn->getSharedPtr());
        });
        auto addIntoReceiverFetcher = invokeActivity(addIntoReceiverActivity, pReceiver->affinityThread());
        addIntoReceiverFetcher();
        return true;
    };

    bool(*m_moveToThreadImpl)(std::size_t idx, std::atomic_size_t &affintyThread) =
        [](std::size_t idx, std::atomic_size_t &affintyThread) -> bool {
        if (idx >= TA_ThreadHolder::get().size()) {
            return false;
        }
        affintyThread.store(idx, std::memory_order_release);
        return true;
    };

    template <typename Sender, typename Receiver>
    inline static auto m_removeConnectionReferenceImpl = [](Sender *pSender, Receiver *pReceiver,
                                                             TA_ConnectionObject::FuncMark &&signal,
                                                             TA_ConnectionObject::FuncMark &&slot) -> void {
       auto senderExp =
            [](Sender *pSender, TA_ConnectionObject::FuncMark &&signal) -> void {
            auto rangeSender = pSender->m_outputConnections.equal_range(signal);
            for (auto iter = rangeSender.first; iter != rangeSender.second; ++iter) {
                if (iter->second->sender() == pSender) {
                    pSender->m_outputConnections.erase(iter);
                    break;
                }
            }
        };
        senderExp(pSender, std::forward<TA_ConnectionObject::FuncMark>(signal));
        if(pSender == pReceiver) {
            return;
        }
        auto receiverExp = [pReceiver, &slot]() -> void {
            auto rangeReceiver = pReceiver->m_inputConnections.equal_range(slot);
            for (auto iter = rangeReceiver.first; iter != rangeReceiver.second; ++iter) {
                if (iter->second->receiver() == pReceiver) {
                    pReceiver->m_inputConnections.erase(iter);
                    break;
                }
            }
        };
        auto receiverActivity = TA_ActivityCreator::create(std::move(receiverExp));
        receiverActivity->moveToThread(pReceiver->affinityThread());
        auto fetcher = TA_ThreadHolder::get().postActivity(receiverActivity, true);
        fetcher();
    };
};
} // namespace CoreAsync

template <typename T> struct TA_MetaObjectTraits {
    static constexpr bool isMetaObject = std::is_base_of_v<CoreAsync::TA_MetaObject, T>;
};
template <typename T>
concept EnableMetaObjectType = TA_MetaObjectTraits<T>::isMetaObject;

#endif // TA_METAOBJECT_H
