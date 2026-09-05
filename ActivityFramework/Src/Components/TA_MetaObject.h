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

#include <thread>
#include <algorithm>
#include <any>
#include <functional>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "TA_ThreadPool.h"
#include "TA_MetaReflex.h"
#include "TA_Activity.h"
#include "TA_Coroutine.h"

namespace CoreAsync {

#define TA_Signals public

class TA_MetaObject;
template <typename Owner> class TA_MetaObjectStorage;

template <typename T>
concept EnableConnectObjectType = requires(T *t) {
    { t } -> std::convertible_to<TA_MetaObject *>;
};

template <typename T>
concept ConnectionParameter = std::copy_constructible<T> && std::is_trivially_copyable_v<T>;

template <typename T> struct TA_MetaObjectTraits {
    static constexpr bool isMetaObject = std::is_base_of_v<CoreAsync::TA_MetaObject, T>;
};

template <typename T>
concept EnableMetaObjectType = TA_MetaObjectTraits<T>::isMetaObject;

enum class TA_ConnectionType { Auto, Direct, Queued };

class TA_MetaObject : public std::enable_shared_from_this<TA_MetaObject> {
    class TA_ConnectionObject;
    using SharedConnection = std::shared_ptr<TA_ConnectionObject>;

    struct TA_ConnectionBucket {
        std::vector<SharedConnection> connections{};
        std::size_t emissionDepth{0};
        bool needsCompaction{false};
    };

    class TA_ConnectionStorageAccess {
      public:
        virtual ~TA_ConnectionStorageAccess() = default;
        virtual TA_ConnectionBucket &outputBucket(std::size_t index) = 0;
        virtual TA_ConnectionBucket &inputBucket(std::size_t index) = 0;
        virtual void eraseOutput(std::size_t index, const TA_ConnectionObject *connection) = 0;
        virtual void eraseInput(std::size_t index, const TA_ConnectionObject *connection) = 0;
    };

    struct TA_ResolvedMember {
        TA_ConnectionStorageAccess *storage{nullptr};
        std::size_t index{};
    };

    template <typename> friend class TA_MetaObjectStorage;
    using AsyncTaskRes = TA_ManualCoroutineTask<std::shared_ptr<TA_DefaultVariant>, CorotuineBehavior::Eager>;
  public:
    class TA_ConnectionObjectHolder {
        friend class TA_MetaObject;

      public:
        TA_ConnectionObjectHolder() = default;
        TA_ConnectionObjectHolder(const std::shared_ptr<TA_ConnectionObject> &pConnection)
            : m_pConnection(pConnection ? pConnection->getSharedPtr() : nullptr) {}

        ~TA_ConnectionObjectHolder() = default;

        bool valid() const { return m_pConnection != nullptr; }

        void reset() {
            if (m_pConnection)
                m_pConnection.reset();
            m_pConnection = nullptr;
        }

      private:
        std::shared_ptr<TA_ConnectionObject> m_pConnection{nullptr};
    };

    TA_MetaObject() : m_sourceThread(std::this_thread::get_id()),
                      m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()) {}

    virtual ~TA_MetaObject() = default;

    TA_MetaObject(const TA_MetaObject &object)
        : m_sourceThread(std::this_thread::get_id()), m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()) {}

    TA_MetaObject(TA_MetaObject &&object) noexcept
        : m_sourceThread(std::this_thread::get_id()), m_affinityThreadIdx(TA_ThreadHolder::get().topPriorityThread()) {}

    TA_MetaObject &operator=(const TA_MetaObject &object) {
        if (this != &object) {
            m_affinityThreadIdx.store(object.affinityThread(), std::memory_order_release);
        }
        return *this;
    }

    TA_MetaObject &operator=(TA_MetaObject &&object) noexcept {
        if (this != &object) {
            m_affinityThreadIdx.store(std::move(object.affinityThread()), std::memory_order_release);
        }
        return *this;
    }

    template <EnableMetaObjectType Object>
    static auto sharedRef(Object *pObject) -> std::shared_ptr<std::remove_cvref_t<Object>> {
        auto weakRef = pObject->weak_from_this();
        if(weakRef.expired())
            return std::shared_ptr<std::remove_cvref_t<Object>>(pObject, [](TA_MetaObject *){});
        return std::dynamic_pointer_cast<std::remove_cvref_t<Object>>(weakRef.lock());
    }

    bool hasSharedRef() {
        auto weakRef = this->weak_from_this();
        return !weakRef.expired();
    }

    void pendingCountIncrement() {
        m_pendingCounter.increment();
    }

    void pendingCountDecrement() {
        m_pendingCounter.decrement();
    }

    bool isIdle() const {
        return m_pendingCounter.isIdle() && !m_isBeingDestroyed.load(std::memory_order_acquire);
    }

    void resetPendingCount() {
        m_pendingCounter.reset();
    }

    bool isBeingDestroyed() const {
        return m_isBeingDestroyed.load(std::memory_order_acquire);
    }

  protected:
    struct PendingCounter {
        std::atomic_size_t m_counter{0};
        void reset() {
            m_counter.store(0, std::memory_order_release);
        }
        void increment() {
            m_counter.fetch_add(1, std::memory_order_acquire);
        }
        void decrement() {
            m_counter.fetch_sub(1, std::memory_order_acquire);
        }
        bool isIdle() const {
            return m_counter.load(std::memory_order_acquire) == 0;
        }
        auto operator ++ () -> PendingCounter & {
            increment();
            return *this;
        }
        auto operator -- () -> PendingCounter & {
            decrement();
            return *this;
        }
    };

    std::weak_ptr<TA_MetaObject> weakPtr() {
        return this->weak_from_this();
    }

    std::shared_ptr<TA_MetaObject> sharedPtr() {
        return this->shared_from_this();
    }

    template <ActivityType Activity>
    [[nodiscard]] inline static AsyncTaskRes invokeActivity(Activity *pActivity, TA_MetaObject *pHost, bool autoDelete = true) {
        if (!pActivity) {
            throw std::invalid_argument("Activity is null");
        }
        if (!pHost) {
            throw std::invalid_argument("Host object is null");
        }
        if (pHost->isBeingDestroyed()) {
            throw std::runtime_error("Host object is being destroyed.");
        }
        pHost->pendingCountIncrement();
        std::size_t idx = pHost->affinityThread();
        if (idx >= TA_ThreadHolder::get().size()) {
            throw std::out_of_range("Thread index out of range.");
        }
        if (pHost->isIdle()) {
            pHost->updateAffinityThread();
            idx = pHost->affinityThread();
        }
        pActivity->moveToThread(idx);
        std::shared_ptr<TA_ActivityFetcherAwaitable> fetcherAwaitable =
            std::make_shared<TA_ActivityFetcherAwaitable>(std::make_shared<TA_ActivityProxy>(pActivity, autoDelete));
        auto res = co_await *fetcherAwaitable;
        pHost->pendingCountDecrement();
        co_return res;
    }

    template <ActivityType Activity>
    inline static TA_AutoCoroutineTask invokeActivityNoAwait(Activity *pActivity, TA_MetaObject *pHost, bool autoDelete = true) {
        if (!pActivity) {
            throw std::invalid_argument("Activity is null");
        }
        if (!pHost) {
            throw std::invalid_argument("Host object is null");
        }
        if (pHost->isBeingDestroyed()) {
            throw std::runtime_error("Host object is being destroyed.");
        }
        pHost->pendingCountIncrement();
        std::size_t idx = pHost->affinityThread();
        if (idx >= TA_ThreadHolder::get().size()) {
            throw std::out_of_range("Thread index out of range.");
        }
        if (pHost->isIdle()) {
            pHost->updateAffinityThread();
            idx = pHost->affinityThread();
        }
        pActivity->moveToThread(idx);
        auto fetcher = TA_ThreadHolder::get().postActivity(pActivity, autoDelete);
        // Have to call the fetcher to ensure the activity is executed temporarily.
        // In the future, we may provide a better way to handle fire-and-forget activities.
        // Need to protect sender's lifetime in the future.
        // fetcher();
        pHost->pendingCountDecrement();
        co_return;
    }

    template <PointerType Ptr>
    inline static bool isOnCurrentThread(Ptr pObject) {
        if (!pObject) {
            return false;
        }
        return TA_ThreadHolder::get().threadId(pObject->affinityThread()) == std::this_thread::get_id();
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

    bool deleteLater() {
        if(hasSharedRef())
            return false;
        m_isBeingDestroyed.store(true, std::memory_order_release);
        auto activity = TA_ActivityCreator::create([this]() -> void {
            delete this;
        });
        activity->setStolenEnabled(false);
        activity->moveToThread(this->affinityThread());
        auto fetcher = TA_ThreadHolder::get().postActivity(activity, true);
        return true;
    }

    std::size_t affinityThread() const { return m_affinityThreadIdx.load(std::memory_order_acquire); }

    bool moveToThread(std::size_t idx) {
        if (m_affinityThreadIdx.load(std::memory_order_acquire) == idx) {
            return false;
        }
        if (isOnCurrentThread(this)) {
            return m_moveToThreadImpl(idx, m_affinityThreadIdx);
        }
        auto activity = TA_ActivityCreator::create(
            std::forward<decltype(m_moveToThreadImpl)>(m_moveToThreadImpl), idx, m_affinityThreadIdx);
        activity->setStolenEnabled(false);
        AsyncTaskRes res = invokeActivity(activity, this);
        auto taskResult = res.get();
        return taskResult->template get<bool>();
    }

    template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
    static bool registerConnection(Sender *pSender, Signal &&signal, Receiver *pReceiver, Slot &&slot,
                                   TA_ConnectionType type) {
        if (!connectionTypesCompatible<Signal, Slot>() || !pSender || !pReceiver) {
            return false;
        }
        auto signalMember = resolveMemberDynamic(pSender, signal);
        auto slotMember = resolveMemberDynamic(pReceiver, slot);
        if (!signalMember || !slotMember) {
            return false;
        }
        return registerResolvedConnection(sharedRef(pSender), *signalMember, sharedRef(pReceiver),
                                          std::decay_t<Slot>(slot), *slotMember, type);
    }

    template <auto Signal, auto Slot, EnableConnectObjectType Sender, EnableConnectObjectType Receiver>
    static bool registerConnection(Sender *pSender, Receiver *pReceiver, TA_ConnectionType type) {
        using SignalType = decltype(Signal);
        using SlotType = decltype(Slot);
        static_assert(connectionTypesCompatible<SignalType, SlotType>(), "Signal and slot signatures are incompatible.");
        if (!pSender || !pReceiver) {
            return false;
        }
        return registerResolvedConnection(sharedRef(pSender), resolveMemberStatic<Signal>(pSender),
                                          sharedRef(pReceiver), Slot, resolveMemberStatic<Slot>(pReceiver), type);
    }

    template <EnableConnectObjectType Sender, typename Signal, LambdaExpType LambdaExp>
    static TA_ConnectionObjectHolder registerConnection(Sender *pSender, Signal &&signal, LambdaExp &&exp,
                                                         TA_ConnectionType type, bool autoDestroy = false) {
        if (!lambdaConnectionTypesCompatible<Signal, LambdaExp>() || !pSender) {
            return {nullptr};
        }
        auto signalMember = resolveMemberDynamic(pSender, signal);
        if (!signalMember) {
            return {nullptr};
        }
        return registerResolvedLambda(sharedRef(pSender), *signalMember, std::forward<LambdaExp>(exp), type,
                                      autoDestroy);
    }

    template <auto Signal, EnableConnectObjectType Sender, LambdaExpType LambdaExp>
    static TA_ConnectionObjectHolder registerConnection(Sender *pSender, LambdaExp &&exp,
                                                         TA_ConnectionType type, bool autoDestroy = false) {
        using SignalType = decltype(Signal);
        static_assert(lambdaConnectionTypesCompatible<SignalType, LambdaExp>(),
                      "Signal and lambda signatures are incompatible.");
        if (!pSender) {
            return {nullptr};
        }
        return registerResolvedLambda(sharedRef(pSender), resolveMemberStatic<Signal>(pSender),
                                      std::forward<LambdaExp>(exp), type, autoDestroy);
    }

    template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
    static bool unregisterConnection(Sender *pSender, Signal &&signal, Receiver *pReceiver, Slot &&slot) {
        if (!connectionTypesCompatible<Signal, Slot>() || !pSender || !pReceiver) {
            return false;
        }
        auto signalMember = resolveMemberDynamic(pSender, signal);
        auto slotMember = resolveMemberDynamic(pReceiver, slot);
        if (!signalMember || !slotMember) {
            return false;
        }
        return unregisterResolvedConnection(sharedRef(pSender), *signalMember,
                                            sharedRef(pReceiver), *slotMember);
    }

    template <auto Signal, auto Slot, EnableConnectObjectType Sender, EnableConnectObjectType Receiver>
    static bool unregisterConnection(Sender *pSender, Receiver *pReceiver) {
        using SignalType = decltype(Signal);
        using SlotType = decltype(Slot);
        static_assert(connectionTypesCompatible<SignalType, SlotType>(), "Signal and slot signatures are incompatible.");
        if (!pSender || !pReceiver) {
            return false;
        }
        return unregisterResolvedConnection(sharedRef(pSender), resolveMemberStatic<Signal>(pSender),
                                            sharedRef(pReceiver), resolveMemberStatic<Slot>(pReceiver));
    }

    static bool unregisterConnection(TA_ConnectionObjectHolder &holder) {
        if (!holder.valid() || !holder.m_pConnection) {
            return false;
        }
        auto connection = holder.m_pConnection;
        auto *pSender = connection->sender();
        if (!pSender) {
            holder.reset();
            return false;
        }
        auto remove = [connection]() -> bool {
            connection->removeConnectionReferences();
            return true;
        };
        bool result = true;
        if (isOnCurrentThread(pSender)) {
            result = remove();
        } else {
            auto activity = TA_ActivityCreator::create(std::move(remove));
            activity->setStolenEnabled(false);
            result = invokeActivity(activity, pSender).get()->template get<bool>();
        }
        holder.reset();
        return result;
    }

    template <EnableConnectObjectType Sender, typename Signal, typename... ConnectionParameter>
    static bool emitSignal(Sender *pSender, Signal &&signal, ConnectionParameter &&...args) {
        using SignalType = std::decay_t<Signal>;
        static_assert(validSignalType<SignalType, Sender>(), "The signal must be a registered void instance method of Sender.");
        static_assert((std::is_copy_constructible_v<std::remove_cvref_t<ConnectionParameter>> && ...),
                      "Signal arguments must be copy-constructible.");
        if (!pSender) {
            return false;
        }
        auto signalMember = resolveMemberDynamic(pSender, signal);
        if (!signalMember) {
            return false;
        }
        emitResolved(pSender, *signalMember, std::forward<ConnectionParameter>(args)...);
        return true;
    }

    template <auto Signal, EnableConnectObjectType Sender, typename... ConnectionParameter>
    static bool emitSignal(Sender *pSender, ConnectionParameter &&...args) {
        using SignalType = decltype(Signal);
        static_assert(validSignalType<SignalType, Sender>(), "The signal must be a registered void instance method of Sender.");
        static_assert((std::is_copy_constructible_v<std::remove_cvref_t<ConnectionParameter>> && ...),
                      "Signal arguments must be copy-constructible.");
        if (!pSender) {
            return false;
        }
        emitResolved(pSender, resolveMemberStatic<Signal>(pSender), std::forward<ConnectionParameter>(args)...);
        return true;
    }

    template <EnableConnectObjectType Sender, typename Signal, EnableConnectObjectType Receiver, typename Slot>
    static bool isConnectionExisted(Sender *pSender, Signal &&signal, Receiver *pReceiver, Slot &&slot) {
        if (!connectionTypesCompatible<Signal, Slot>() || !pSender || !pReceiver) {
            return false;
        }
        auto signalMember = resolveMemberDynamic(pSender, signal);
        auto slotMember = resolveMemberDynamic(pReceiver, slot);
        if (!signalMember || !slotMember) {
            return false;
        }
        return isResolvedConnectionExisted(sharedRef(pSender), *signalMember,
                                           sharedRef(pReceiver), *slotMember);
    }

    template <auto Signal, auto Slot, EnableConnectObjectType Sender, EnableConnectObjectType Receiver>
    static bool isConnectionExisted(Sender *pSender, Receiver *pReceiver) {
        static_assert(connectionTypesCompatible<decltype(Signal), decltype(Slot)>(),
                      "Signal and slot signatures are incompatible.");
        if (!pSender || !pReceiver) {
            return false;
        }
        return isResolvedConnectionExisted(sharedRef(pSender), resolveMemberStatic<Signal>(pSender),
                                           sharedRef(pReceiver), resolveMemberStatic<Slot>(pReceiver));
    }

  private:
    template <typename Signal, typename Sender>
    static consteval bool validSignalType() {
        using SignalType = std::decay_t<Signal>;
        if constexpr (!std::is_member_function_pointer_v<SignalType>) {
            return false;
        } else {
            using Owner = typename MethodTypeInfo<SignalType>::ParentClass;
            return std::is_same_v<typename MethodTypeInfo<SignalType>::RetType, void> &&
                   std::is_convertible_v<std::remove_cvref_t<Sender> *, Owner *>;
        }
    }

    template <typename Signal, typename Slot>
    static consteval bool connectionTypesCompatible() {
        using SignalType = std::decay_t<Signal>;
        using SlotType = std::decay_t<Slot>;
        if constexpr (!std::is_member_function_pointer_v<SignalType> ||
                      !std::is_member_function_pointer_v<SlotType>) {
            return false;
        } else if constexpr (!std::is_same_v<typename MethodTypeInfo<SignalType>::RetType, void> ||
                             !std::is_same_v<typename MethodTypeInfo<SlotType>::RetType, void>) {
            return false;
        } else {
            return MetaSame<typename MethodTypeInfo<SignalType>::ArgGroup,
                            typename MethodTypeInfo<SlotType>::ArgGroup>::value;
        }
    }

    template <typename Signal, typename LambdaExp>
    static consteval bool lambdaConnectionTypesCompatible() {
        using SignalType = std::decay_t<Signal>;
        using ExpType = std::decay_t<LambdaExp>;
        if constexpr (!std::is_member_function_pointer_v<SignalType>) {
            return false;
        } else if constexpr (!std::is_same_v<typename MethodTypeInfo<SignalType>::RetType, void> ||
                             !std::is_same_v<typename LambdaExpTraits<ExpType>::RetType, void>) {
            return false;
        } else {
            return MetaSame<typename MethodTypeInfo<SignalType>::ArgGroup,
                            typename LambdaExpTraits<ExpType>::ArgGroup>::value;
        }
    }

    template <auto Member, EnableConnectObjectType Object>
    static constexpr TA_ResolvedMember resolveMemberStatic(Object *pObject) {
        using MemberType = std::decay_t<decltype(Member)>;
        static_assert(std::is_member_function_pointer_v<MemberType>,
                      "Connection members must be instance methods.");
        using Owner = typename MethodTypeInfo<MemberType>::ParentClass;
        static_assert(std::is_base_of_v<TA_MetaObjectStorage<Owner>, Owner>,
                      "The member owner must inherit TA_MetaObjectStorage<Owner>.");
        constexpr auto index = Reflex::TA_TypeInfo<Owner>::fields.template valueIndex<Member>();
        static_assert(index < Reflex::TA_TypeInfo<Owner>::fields.size(), "The member is not locally registered.");

        static_assert(std::is_convertible_v<Object *, Owner *>, "The member does not belong to this object.");
        auto *pOwner = static_cast<Owner *>(pObject);
        return {static_cast<TA_MetaObjectStorage<Owner> *>(pOwner), index};
    }

    template <EnableConnectObjectType Object, typename Member>
    static std::optional<TA_ResolvedMember> resolveMemberDynamic(Object *pObject, Member &&member) {
        using MemberType = std::decay_t<Member>;
        if constexpr (!std::is_member_function_pointer_v<MemberType>) {
            return std::nullopt;
        } else {
            using Owner = typename MethodTypeInfo<MemberType>::ParentClass;
            static_assert(std::is_base_of_v<TA_MetaObjectStorage<Owner>, Owner>,
                          "The member owner must inherit TA_MetaObjectStorage<Owner>.");
            static_assert(std::is_convertible_v<Object *, Owner *>, "The member does not belong to this object.");
            const auto index = Reflex::TA_TypeInfo<Owner>::fields.findValueIndex(member);
            if (index >= Reflex::TA_TypeInfo<Owner>::fields.size()) {
                return std::nullopt;
            }
            auto *pOwner = static_cast<Owner *>(pObject);
            return TA_ResolvedMember{static_cast<TA_MetaObjectStorage<Owner> *>(pOwner), index};
        }
    }

    static void compactBucket(TA_ConnectionBucket &bucket) {
        if (bucket.emissionDepth != 0 || !bucket.needsCompaction) {
            return;
        }
        std::erase(bucket.connections, SharedConnection{});
        bucket.needsCompaction = false;
    }

    static void eraseFromBucket(TA_ConnectionBucket &bucket, const TA_ConnectionObject *connection) {
        auto found = std::ranges::find_if(bucket.connections, [connection](const auto &candidate) {
            return candidate.get() == connection;
        });
        if (found == bucket.connections.end()) {
            return;
        }
        if (bucket.emissionDepth != 0) {
            found->reset();
            bucket.needsCompaction = true;
        } else {
            *found = std::move(bucket.connections.back());
            bucket.connections.pop_back();
        }
    }

    template <typename Operation>
    static void runOnAffinityThread(TA_MetaObject *pObject, Operation &&operation) {
        if (!pObject || isOnCurrentThread(pObject)) {
            std::invoke(std::forward<Operation>(operation));
            return;
        }
        auto activity = TA_ActivityCreator::create(std::forward<Operation>(operation));
        activity->moveToThread(pObject->affinityThread());
        activity->setStolenEnabled(false);
        auto fetcher = TA_ThreadHolder::get().postActivity(activity, true);
        fetcher();
    }

    class TA_ConnectionObject : public std::enable_shared_from_this<TA_ConnectionObject> {
        using SlotExpType = std::function<void()>;
      public:
        TA_ConnectionObject() = default;
        template <EnableConnectObjectType Sender>
        TA_ConnectionObject(Sender *pSender, TA_ResolvedMember signal, TA_ConnectionType type,
                            bool autoDestroy = false)
            : m_pSender(pSender), m_pReceiver(pSender),
              m_senderStorage(signal.storage), m_signalIndex(signal.index), m_type(type),
              m_autoDestroy(autoDestroy) {
            if (pSender->hasSharedRef()) {
                m_wpSender = pSender->weak_from_this();
                m_wpReceiver = pSender->weak_from_this();
            }
        }

        TA_ConnectionObject(const TA_ConnectionObject &object) = delete;
        TA_ConnectionObject(TA_ConnectionObject &&object) = delete;

        TA_ConnectionObject &operator=(const TA_ConnectionObject &object) = delete;
        TA_ConnectionObject &operator=(TA_ConnectionObject &&object) = delete;

        ~TA_ConnectionObject() = default;

        template <EnableConnectObjectType Receiver, typename Slot>
        void initSlotObject(Receiver *pReceiver, Slot &&slot, TA_ResolvedMember member) {
            m_pReceiver = pReceiver;
            m_receiverStorage = member.storage;
            m_slotIndex = member.index;
            if (pReceiver->hasSharedRef()) {
                m_wpReceiver = pReceiver->weak_from_this();
            }
            using SlotParaTuple = typename MethodTypeInfo<Slot>::ArgGroup::Tuple;
            m_para = SlotParaTuple{};
            auto weakRef = getWeakPtr();
            m_slotExp = [weakRef, slot]() -> void {
                auto sharedRef = weakRef.lock();
                if (!sharedRef) {
                    return;
                }
                auto *pRawReceiver = sharedRef->resolveReceiver();
                if(!pRawReceiver) {
                    return;
                }
                decltype(auto) rObj{dynamic_cast<std::decay_t<Receiver> *>(pRawReceiver)};
                if constexpr (IsInstanceMethod<Slot>::value)
                    std::apply(slot, std::move(std::tuple_cat(std::make_tuple(rObj),
                                                              std::any_cast<SlotParaTuple>(sharedRef->m_para))));
            };
        }

        template <LambdaExpType LambdaExp>
        void initSlotObject(LambdaExp &&exp) {
            using SlotParaTuple = typename LambdaExpTraits<std::decay_t<LambdaExp>>::ArgGroup::Tuple;
            m_para = SlotParaTuple{};
            auto weakRef = getWeakPtr();
            m_slotExp = [weakRef, exp = std::forward<LambdaExp>(exp)]() mutable -> void {
                auto sharedRef = weakRef.lock();
                if (!sharedRef) {
                    return;
                }
                std::apply(exp, std::any_cast<SlotParaTuple>(sharedRef->m_para));
            };
        }

        std::shared_ptr<TA_ConnectionObject> getSharedPtr() { return this->shared_from_this(); }
        std::weak_ptr<TA_ConnectionObject> getWeakPtr() { return this->weak_from_this(); }

        template <typename... Args> void setPara(Args &&...args) {
            using ArgsTypes = std::tuple<std::decay_t<Args>...>;
            if constexpr (sizeof...(Args) != 0)
                m_para.emplace<ArgsTypes>(std::move(std::make_tuple(std::forward<Args>(args)...)));
        }

        void callSlot() {
            auto slotExp = m_slotExp;
            auto activity = TA_ActivityCreator::create(std::forward<SlotExpType>(slotExp));
            activity->setStolenEnabled(false);
            auto *pRealSender = resolveSender();
            auto *pRealReceiver = resolveReceiver();
            if(pRealSender && pRealReceiver) {
                activity->moveToThread(pRealReceiver->affinityThread());
                if (pRealSender->affinityThread() == pRealReceiver->affinityThread() &&
                    (m_type == TA_ConnectionType::Direct || m_type == TA_ConnectionType::Auto)) {
                    activity->operator()();
                } else {
                    auto fetcher = TA_ThreadHolder::get().postActivity(activity, true);
                }
            }
            if (m_autoDestroy) {
                removeConnectionReferences();
            }
        }

        void removeConnectionReferences() {
            auto keepAlive = getSharedPtr();
            auto *pRealSender = resolveSender();
            auto *pRealReceiver = resolveReceiver();
            auto *senderStorage = std::exchange(m_senderStorage, nullptr);
            auto *receiverStorage = std::exchange(m_receiverStorage, nullptr);
            if (senderStorage) {
                runOnAffinityThread(pRealSender, [senderStorage, index = m_signalIndex, connection = this]() {
                    senderStorage->eraseOutput(index, connection);
                });
            }
            if (receiverStorage) {
                runOnAffinityThread(pRealReceiver, [receiverStorage, index = m_slotIndex, connection = this]() {
                    receiverStorage->eraseInput(index, connection);
                });
            }
        }

        // Forget only endpoints owned by this storage; this does not erase bucket entries.
        // The caller drains its local buckets, leaving removeConnectionReferences()
        // responsible for any endpoint in a different storage segment.
        void detachStorage(TA_ConnectionStorageAccess *storage) {
            if (m_senderStorage == storage) {
                m_senderStorage = nullptr;
            }
            if (m_receiverStorage == storage) {
                m_receiverStorage = nullptr;
            }
        }

        bool matches(TA_MetaObject *receiver, const TA_ResolvedMember &slot) const {
            return resolveReceiver() == receiver && m_receiverStorage == slot.storage && m_slotIndex == slot.index;
        }

        TA_MetaObject *sender() const { return resolveSender(); }

        TA_ConnectionStorageAccess *senderStorage() const { return m_senderStorage; }
        TA_ConnectionStorageAccess *receiverStorage() const { return m_receiverStorage; }

        bool isSync() const {
            auto *pRealSender = resolveSender();
            auto *pRealReceiver = resolveReceiver();
            if (!pRealSender || !pRealReceiver) return false;

            return pRealSender->affinityThread() == pRealReceiver->affinityThread() &&
                   (m_type == TA_ConnectionType::Auto || m_type == TA_ConnectionType::Direct);
        }

        bool isAutoDestroy() const { return m_autoDestroy; }

      private:
        //Users need to ensure the validity of the returned pointer and check for nullptr before use.
        TA_MetaObject *resolveSender() const {
            bool isEmpty = !m_wpSender.owner_before(std::weak_ptr<TA_MetaObject>{}) &&
                           !std::weak_ptr<TA_MetaObject>{}.owner_before(m_wpSender);
            if (isEmpty) return m_pSender;
            return m_wpSender.lock().get();
        }

        //Users need to ensure the validity of the returned pointer and check for nullptr before use.
        TA_MetaObject *resolveReceiver() const {
            bool isEmpty = !m_wpReceiver.owner_before(std::weak_ptr<TA_MetaObject>{}) &&
                           !std::weak_ptr<TA_MetaObject>{}.owner_before(m_wpReceiver);
            if (isEmpty) return m_pReceiver;
            return m_wpReceiver.lock().get();
        }

      private:
        TA_MetaObject *m_pSender{nullptr}, *m_pReceiver{nullptr};
        std::weak_ptr<TA_MetaObject> m_wpSender{}, m_wpReceiver{};
        TA_ConnectionStorageAccess *m_senderStorage{nullptr}, *m_receiverStorage{nullptr};
        std::size_t m_signalIndex{std::numeric_limits<std::size_t>::max()};
        std::size_t m_slotIndex{std::numeric_limits<std::size_t>::max()};
        TA_ConnectionType m_type;
        std::any m_para;
        SlotExpType m_slotExp {};
        const bool m_autoDestroy{false};
    };

    template <typename Sender, typename Receiver, typename Slot>
    static SharedConnection createConnection(Sender pSender, TA_ResolvedMember signalMember, Receiver pReceiver,
                                             Slot slot, TA_ResolvedMember slotMember,
                                             TA_ConnectionType type) {
        auto &bucket = signalMember.storage->outputBucket(signalMember.index);
        if (std::ranges::any_of(bucket.connections, [&](const auto &connection) {
                return connection && connection->matches(pReceiver.get(), slotMember);
            })) {
            return nullptr;
        }
        auto connection = std::make_shared<TA_ConnectionObject>(pSender.get(), signalMember, type);
        connection->initSlotObject(pReceiver.get(), std::move(slot), slotMember);
        bucket.connections.push_back(connection);
        return connection;
    }

    template <typename Sender, typename Receiver, typename Slot>
    static bool registerResolvedConnection(Sender pSender, TA_ResolvedMember signalMember, Receiver pReceiver,
                                           Slot slot, TA_ResolvedMember slotMember,
                                           TA_ConnectionType type) {
        auto registerSender = [pSender, signalMember, pReceiver, slot, slotMember, type]() mutable {
            return createConnection(pSender, signalMember, pReceiver, slot, slotMember, type);
        };
        SharedConnection connection;
        if (isOnCurrentThread(pSender)) {
            connection = registerSender();
        } else {
            auto activity = TA_ActivityCreator::create(std::move(registerSender));
            activity->setStolenEnabled(false);
            connection = invokeActivity(activity, pSender.get()).get()->template get<SharedConnection>();
        }
        if (!connection) {
            return false;
        }
        auto registerReceiver = [connection, slotMember]() {
            slotMember.storage->inputBucket(slotMember.index).connections.push_back(connection);
        };
        if (isOnCurrentThread(pReceiver)) {
            registerReceiver();
        } else {
            auto activity = TA_ActivityCreator::create(std::move(registerReceiver));
            activity->setStolenEnabled(false);
            invokeActivity(activity, pReceiver.get()).get();
        }
        return true;
    }

    template <typename Sender, typename LambdaExp>
    static TA_ConnectionObjectHolder registerResolvedLambda(Sender pSender, TA_ResolvedMember signalMember,
                                                             LambdaExp &&exp,
                                                             TA_ConnectionType type, bool autoDestroy) {
        using ExpType = std::decay_t<LambdaExp>;
        auto registerLambda = [pSender, signalMember, exp = ExpType(std::forward<LambdaExp>(exp)),
                               type, autoDestroy]() mutable -> TA_ConnectionObjectHolder {
            auto &bucket = signalMember.storage->outputBucket(signalMember.index);
            auto connection = std::make_shared<TA_ConnectionObject>(pSender.get(), signalMember, type, autoDestroy);
            connection->initSlotObject(std::move(exp));
            bucket.connections.push_back(connection);
            return autoDestroy ? TA_ConnectionObjectHolder{nullptr} : TA_ConnectionObjectHolder{connection};
        };
        if (isOnCurrentThread(pSender)) {
            return registerLambda();
        }
        auto activity = TA_ActivityCreator::create(std::move(registerLambda));
        activity->setStolenEnabled(false);
        return invokeActivity(activity, pSender.get()).get()->template get<TA_ConnectionObjectHolder>();
    }

    template <typename Sender, typename Receiver>
    static bool isResolvedConnectionExisted(Sender pSender, TA_ResolvedMember signalMember,
                                            Receiver pReceiver, TA_ResolvedMember slotMember) {
        auto find = [pReceiver, signalMember, slotMember]() {
            auto &bucket = signalMember.storage->outputBucket(signalMember.index);
            return std::ranges::any_of(bucket.connections, [&](const auto &connection) {
                return connection && connection->matches(pReceiver.get(), slotMember);
            });
        };
        if (isOnCurrentThread(pSender)) {
            return find();
        }
        auto activity = TA_ActivityCreator::create(std::move(find));
        activity->setStolenEnabled(false);
        return invokeActivity(activity, pSender.get()).get()->template get<bool>();
    }

    template <typename Sender, typename Receiver>
    static bool unregisterResolvedConnection(Sender pSender, TA_ResolvedMember signalMember,
                                             Receiver pReceiver, TA_ResolvedMember slotMember) {
        auto findConnection = [pReceiver, signalMember, slotMember]() -> SharedConnection {
            auto &bucket = signalMember.storage->outputBucket(signalMember.index);
            auto found = std::ranges::find_if(bucket.connections, [&](const auto &connection) {
                return connection && connection->matches(pReceiver.get(), slotMember);
            });
            return found == bucket.connections.end() ? nullptr : *found;
        };
        SharedConnection connection;
        if (isOnCurrentThread(pSender)) {
            connection = findConnection();
        } else {
            auto activity = TA_ActivityCreator::create(std::move(findConnection));
            activity->setStolenEnabled(false);
            connection = invokeActivity(activity, pSender.get()).get()->template get<SharedConnection>();
        }
        if (!connection) {
            return false;
        }
        connection->removeConnectionReferences();
        return true;
    }

    template <typename... Args>
    static void emitBucket(TA_ResolvedMember signalMember, Args &&...args) {
        auto &bucket = signalMember.storage->outputBucket(signalMember.index);
        ++bucket.emissionDepth;
        const auto count = bucket.connections.size();
        for (std::size_t index = 0; index < count; ++index) {
            auto connection = bucket.connections[index];
            if (!connection) {
                continue;
            }
            if (index + 1 < count) {
                connection->setPara(args...);
            } else {
                connection->setPara(std::forward<Args>(args)...);
            }
            connection->callSlot();
        }
        --bucket.emissionDepth;
        compactBucket(bucket);
    }

    template <EnableConnectObjectType Sender, typename... Args>
    static void emitResolved(Sender *pSender, TA_ResolvedMember signalMember, Args &&...args) {
        if (isOnCurrentThread(pSender)) {
            emitBucket(signalMember, std::forward<Args>(args)...);
            return;
        }
        auto protectedSender = sharedRef(pSender);
        auto parameters = std::make_tuple(std::forward<Args>(args)...);
        auto emit = [protectedSender, signalMember, parameters = std::move(parameters)]() mutable {
            std::apply([&](auto &&...values) {
                emitBucket(signalMember, std::forward<decltype(values)>(values)...);
            }, std::move(parameters));
        };
        auto activity = TA_ActivityCreator::create(std::move(emit));
        activity->setStolenEnabled(false);
        invokeActivityNoAwait(activity, pSender);
    }

    void updateAffinityThread() {
        m_affinityThreadIdx.store(TA_ThreadHolder::get().topPriorityThread(), std::memory_order_release);
    }

    const std::thread::id m_sourceThread;
    std::atomic_size_t m_affinityThreadIdx;
    PendingCounter m_pendingCounter{};
    std::atomic_bool m_isBeingDestroyed{false};

    bool(*m_moveToThreadImpl)(std::size_t idx, std::atomic_size_t &affinityThread) =
        [](std::size_t idx, std::atomic_size_t &affinityThread) -> bool {
            if (idx >= TA_ThreadHolder::get().size()) {
                return false;
            }
            affinityThread.store(idx, std::memory_order_release);
            return true;
        };

};

template <typename Owner>
class TA_MetaObjectStorage : public virtual TA_MetaObject, public TA_MetaObject::TA_ConnectionStorageAccess {
    using ConnectionBucket = TA_MetaObject::TA_ConnectionBucket;

  public:
    TA_MetaObjectStorage() = default;
    TA_MetaObjectStorage(const TA_MetaObjectStorage &) {}
    TA_MetaObjectStorage(TA_MetaObjectStorage &&) noexcept {}

    TA_MetaObjectStorage &operator=(const TA_MetaObjectStorage &) {
        disconnectAll();
        return *this;
    }

    TA_MetaObjectStorage &operator=(TA_MetaObjectStorage &&) noexcept {
        disconnectAll();
        return *this;
    }

    ~TA_MetaObjectStorage() override { disconnectAll(); }

  private:
    void ensureBuckets() {
        const auto fieldCount = Reflex::TA_TypeInfo<Owner>::fields.size();
        if (m_outputBuckets.empty()) {
            m_outputBuckets.resize(fieldCount);
            m_inputBuckets.resize(fieldCount);
        }
    }

    ConnectionBucket &outputBucket(std::size_t index) override {
        ensureBuckets();
        return m_outputBuckets.at(index);
    }

    ConnectionBucket &inputBucket(std::size_t index) override {
        ensureBuckets();
        return m_inputBuckets.at(index);
    }

    void eraseOutput(std::size_t index, const TA_MetaObject::TA_ConnectionObject *connection) override {
        if (index < m_outputBuckets.size()) {
            TA_MetaObject::eraseFromBucket(m_outputBuckets[index], connection);
        }
    }

    void eraseInput(std::size_t index, const TA_MetaObject::TA_ConnectionObject *connection) override {
        if (index < m_inputBuckets.size()) {
            TA_MetaObject::eraseFromBucket(m_inputBuckets[index], connection);
        }
    }

    void disconnectAll() noexcept {
        auto disconnectBuckets = [this](auto &buckets) {
            for (auto &bucket : buckets) {
                while (!bucket.connections.empty()) {
                    auto connection = bucket.connections.back();
                    // Remove the local entry first, retaining ownership during cleanup.
                    bucket.connections.pop_back();
                    if (connection) {
                        // Skip callbacks into this storage, which we are already draining.
                        // Pointers to other storage segments remain available for removal.
                        connection->detachStorage(this);
                        connection->removeConnectionReferences();
                    }
                }
            }
        };
        // If both endpoints share this storage, detachStorage() clears both pointers.
        // Draining both bucket collections removes the remaining local entry as well.
        disconnectBuckets(m_outputBuckets);
        disconnectBuckets(m_inputBuckets);
    }

    std::vector<ConnectionBucket> m_outputBuckets{};
    std::vector<ConnectionBucket> m_inputBuckets{};
};

template <EnableConnectObjectType Sender, typename... Args>
class TA_SignalAwaitable : public std::enable_shared_from_this<TA_SignalAwaitable<Sender, Args...>> {
  public:
    TA_SignalAwaitable(Sender *pObject, void (std::decay_t<Sender>::*signal)(Args...))
        : m_pObject(pObject), m_signal(signal) {}

    ~TA_SignalAwaitable() {}

    constexpr bool await_ready() const noexcept { return false; }

    template <typename PromiseType>
    constexpr void await_suspend(std::coroutine_handle<PromiseType> handle) noexcept {
        TA_MetaObject::registerConnection(
            m_pObject, std::move(m_signal),
            [this, handle](Args... args) {
                if constexpr (sizeof...(Args) != 0)
                    m_args = std::make_tuple(args...);
                handle.resume();
            }, TA_ConnectionType::Auto, true);
    }

    constexpr auto await_resume() const noexcept {
        if constexpr (sizeof...(Args) != 0) {
            if constexpr (sizeof...(Args) == 1) {
                return std::get<0>(m_args);
            } else {
                return m_args;
            }
        }
    }

    TA_SignalAwaitable operator=(const TA_SignalAwaitable &) = delete;

  protected:
    Sender *m_pObject{nullptr};
    void (std::decay_t<Sender>::*m_signal)(Args...);
    std::tuple<Args...> m_args{};
};

} // namespace CoreAsync

#endif // TA_METAOBJECT_H
