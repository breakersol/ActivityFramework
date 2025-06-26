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

#ifndef TA_SLOTOBJECT_H
#define TA_SLOTOBJECT_H

#include "TA_MetaReflex.h"
#include "TA_MetaObject.h"

namespace CoreAsync {
class TA_BaseReceiverObject {
  public:
    virtual TA_ActivityProxy *active(TA_MetaObject *&pReceiver, std::string_view &&rFunc, void **args) = 0;
    virtual void call(TA_MetaObject *&pReceiver, std::string_view &&rFunc, void **args) = 0;
    virtual ~TA_BaseReceiverObject() = default;
};

template <typename Receiver> class TA_ReceiverObject : public TA_BaseReceiverObject {
    using RecType = std::decay_t<Receiver>;

  public:
    virtual TA_ActivityProxy *active(TA_MetaObject *&pReceiver, std::string_view &&rFunc, void **args) override {
        if (!pReceiver)
            return nullptr;
        auto receiverFunction{Reflex::TA_TypeInfo<RecType>::findTypeValue(rFunc)};
        auto vistor = [&, this](auto &&value) -> auto {
            using FuncType = std::decay_t<decltype(value)>;
            if constexpr (std::is_member_function_pointer_v<FuncType>) {
                auto pObj = dynamic_cast<typename MethodTypeInfo<FuncType>::ParentClass *>(pReceiver);
                return wrapper(pObj, std::forward<std::decay_t<decltype(value)>>(value),
                               std::make_index_sequence<MethodTypeInfo<FuncType>::argSize>{}, args);
            } else {
                TA_ActivityProxy *pActivity{nullptr};
                return pActivity;
            }
        };

        return std::visit(vistor, receiverFunction);
    }

    virtual void call(TA_MetaObject *&pReceiver, std::string_view &&rFunc, void **args) override {
        if (!pReceiver)
            return;
        auto receiverFunction{Reflex::TA_TypeInfo<RecType>::findTypeValue(rFunc)};
        auto vistor = [&, this](auto &&value) {
            using FuncType = std::decay_t<decltype(value)>;
            if constexpr (std::is_member_function_pointer_v<FuncType>) {
                using ReturnType = typename MethodTypeInfo<FuncType>::RetType;
                auto pObj = dynamic_cast<typename MethodTypeInfo<FuncType>::ParentClass *>(pReceiver);
                if constexpr (std::is_same_v<void, ReturnType>) {
                    if constexpr (MethodTypeInfo<FuncType>::argSize != 0)
                        invoke(pObj, std::forward<FuncType>(value),
                               std::make_index_sequence<MethodTypeInfo<FuncType>::argSize>{}, args);
                    else
                        invoke(pObj, std::forward<FuncType>(value));
                } else {
                    if constexpr (MethodTypeInfo<FuncType>::argSize != 0)
                        invoke(pObj, std::forward<FuncType>(value),
                               std::make_index_sequence<MethodTypeInfo<FuncType>::argSize>{}, args);
                    else
                        invoke(pObj, std::forward<FuncType>(value));
                }
            }
        };
        std::visit(vistor, receiverFunction);
    }

  private:
    template <typename Ret, typename RClass, typename... Args, typename std::size_t... IDXS>
    constexpr auto wrapper(RClass *pReceiver, Ret (RClass::*&&rFunc)(Args...), std::index_sequence<IDXS...>,
                           void **args) {
        auto funcWrapper = [=]() -> Ret {
            if constexpr (std::is_same_v<void, Ret>) {
                (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
                freeParas<sizeof...(Args)>(args, TA_MetaTypelist<Args...>{});
            } else {
                auto res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
                freeParas<sizeof...(Args)>(args, TA_MetaTypelist<Args...>{});
                return res;
            }
        };
        return new TA_ActivityProxy{TA_ActivityCreator::create(std::move(funcWrapper), pReceiver->affinityThread())};
    }

    template <typename Ret, typename RClass, typename... Args, typename std::size_t... IDXS>
    constexpr auto wrapper(RClass *pReceiver, Ret (RClass::*&&rFunc)(Args...) const, std::index_sequence<IDXS...>,
                           void **args) {
        auto funcWrapper = [=]() -> Ret {
            if constexpr (std::is_same_v<void, Ret>) {
                (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
                freeParas<sizeof...(Args)>(args, TA_MetaTypelist<Args...>{});
            } else {
                auto res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
                freeParas<sizeof...(Args)>(args, TA_MetaTypelist<Args...>{});
                return res;
            }
        };
        return new TA_ActivityProxy{TA_ActivityCreator::create(std::move(funcWrapper), pReceiver->affinityThread())};
    }

    template <typename Ret, typename RClass, typename... Args, typename std::size_t... IDXS>
    constexpr decltype(auto) invoke(RClass *pReceiver, Ret (RClass::*&&rFunc)(Args...), std::index_sequence<IDXS...>,
                                    void **args) {
        Ret res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
        return res;
    }

    template <typename Ret, typename RClass>
    constexpr decltype(auto) invoke(RClass *pReceiver, Ret (RClass::*&&rFunc)()) {
        Ret res = (pReceiver->*rFunc)();
        return res;
    }

    template <typename RClass, typename... Args, typename std::size_t... IDXS>
    constexpr void invoke(RClass *pReceiver, void (RClass::*&&rFunc)(Args...), std::index_sequence<IDXS...>,
                          void **args) {
        (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
    }

    template <typename RClass> constexpr void invoke(RClass *pReceiver, void (RClass::*&&rFunc)()) {
        (pReceiver->*rFunc)();
    }

    template <typename Ret, typename RClass, typename... Args, typename std::size_t... IDXS>
    constexpr decltype(auto) invoke(RClass *pReceiver, Ret (RClass::*&&rFunc)(Args...) const,
                                    std::index_sequence<IDXS...>, void **args) {
        Ret res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
        return res;
    }

    template <typename Ret, typename RClass>
    constexpr decltype(auto) invoke(RClass *pReceiver, Ret (RClass::*&&rFunc)() const) {
        Ret res = (pReceiver->*rFunc)();
        return res;
    }

    template <typename RClass, typename... Args, typename std::size_t... IDXS>
    constexpr decltype(auto) invoke(RClass *pReceiver, void (RClass::*&&rFunc)(Args...) const,
                                    std::index_sequence<IDXS...>, void **args) {
        (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<Args> *>(args[IDXS])...);
    }

    template <typename RClass> constexpr decltype(auto) invoke(RClass *pReceiver, void (RClass::*&&rFunc)() const) {
        (pReceiver->*rFunc)();
    }

    template <std::size_t N> static constexpr void freeParas(void **pArgs, TA_MetaTypelist<>) { return; }

    template <std::size_t N, typename Para, typename... Paras>
    static constexpr void freeParas(void **pArgs, TA_MetaTypelist<Para, Paras...>) {
        delete reinterpret_cast<Para *>(pArgs[0]);
        freeParas<N>(&pArgs[1], TA_MetaTypelist<Paras...>{});
        if constexpr (N == TA_MetaTypelist<Para, Paras...>::size) {
            delete[] pArgs;
            pArgs = nullptr;
        }
    }
};
} // namespace CoreAsync

#endif // TA_SLOTOBJECT_H
