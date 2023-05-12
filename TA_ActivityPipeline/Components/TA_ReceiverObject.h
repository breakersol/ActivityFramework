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

#ifndef TA_SLOTOBJECT_H
#define TA_SLOTOBJECT_H

#include "ITA_ActivityCreator.h"
#include "TA_MetaReflex.h"

namespace CoreAsync
{
    class TA_BaseReceiverObject
    {
    public:
        virtual TA_BasicActivity * active(void *&pReceiver, std::string_view &&rFunc,void **args) = 0;
        virtual void call(void *&pReceiver, std::string_view &&rFunc, void **args) = 0;
        virtual ~TA_BaseReceiverObject() = default;
    };

    template <typename Receiver>
    class TA_ReceiverObject : public TA_BaseReceiverObject
    {
        using RecType = std::decay_t<Receiver>;
    public:
        virtual TA_BasicActivity * active(void *&pReceiver, std::string_view &&rFunc,void **args) override
        { 
            if(!pReceiver)
                return nullptr;
            auto receiverFunction {Reflex::TA_TypeInfo<RecType>::findTypeValue(rFunc)};
            auto vistor = [&,this](auto &&value)-> TA_BasicActivity * {
                using FuncType = std::decay_t<decltype(value)>;
                if constexpr(std::is_member_function_pointer_v<FuncType>)
                {
                    auto pObj = reinterpret_cast<typename FunctionTypeInfo<FuncType>::ParentClass *>(pReceiver);
                    return wrapper(pObj, std::forward<std::decay_t<decltype(value)>>(value), std::make_index_sequence<FunctionTypeInfo<FuncType>::paraSize> {}, args);
                }
                else
                {
                    TA_BasicActivity *pActivity {nullptr};
                    return pActivity;
                }
            };

            return std::visit(vistor, receiverFunction);
        }

        virtual void call(void *&pReceiver, std::string_view &&rFunc, void **args) override
        {
            if(!pReceiver)
                return;
            auto receiverFunction {Reflex::TA_TypeInfo<RecType>::findTypeValue(rFunc)};
            auto vistor = [&,this](auto &&value) {
                if constexpr(std::is_member_function_pointer_v<decltype(value)>)
                {
                    using FuncType = std::decay_t<decltype(value)>;
                    using RetType = typename FunctionTypeInfo<decltype(value)>::Ret;
                    auto pObj = reinterpret_cast<typename FunctionTypeInfo<FuncType>::ParentClass *>(pReceiver);
                    if constexpr(std::is_same_v<void, RetType>)
                        invoke(pObj, std::forward<decltype(value)>(value), std::make_index_sequence<FunctionTypeInfo<decltype(value)>::paraSize> {}, args);
                    else
                        auto res = invoke(pReceiver, std::forward<decltype(value)>(value), std::make_index_sequence<FunctionTypeInfo<decltype(value)>::paraSize> {}, args);
                }
            };
            std::visit(vistor, receiverFunction);
        }

    private:
        template <typename Ret, typename RClass, typename ...PARA, typename std::size_t ...IDXS>
        constexpr auto wrapper(RClass *pReceiver, Ret(RClass::*&&rFunc)(PARA...), std::index_sequence<IDXS...>, void **args)
        {
            auto funcWrapper = [=]()->Ret{
                if constexpr(std::is_same_v<void, Ret>)
                {
                    (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
                    freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
                }
                else
                {
                    auto res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
                    freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
                    return res;
                }

            };
            return ITA_ActivityCreator::create<Ret>(funcWrapper);
        }

        template <typename Ret, typename RClass, typename ...PARA, typename std::size_t ...IDXS>
        constexpr auto wrapper(RClass *pReceiver, Ret(RClass::*&&rFunc)(PARA...) const, std::index_sequence<IDXS...>, void **args)
        {
            auto funcWrapper = [=]()->Ret{
                if constexpr(std::is_same_v<void, Ret>)
                {
                    (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
                    freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
                }
                else
                {
                    auto res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
                    freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
                    return res;
                }

            };
            return ITA_ActivityCreator::create<Ret>(funcWrapper);
        }

        template <typename Ret, typename RClass, typename ...PARA, typename std::size_t ...IDXS>
        constexpr decltype(auto) invoke(RClass *pReceiver, Ret(RClass::*&&rFunc)(PARA...), std::index_sequence<IDXS...>, void **args)
        {
            Ret res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
            freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
            return res;
        }

        template <typename RClass, typename ...PARA, typename std::size_t ...IDXS>
        constexpr void invoke(RClass *pReceiver, void(RClass::*&&rFunc)(PARA...), std::index_sequence<IDXS...>, void **args)
        {
            (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
            freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
        }


        template <typename Ret, typename RClass, typename ...PARA, typename std::size_t ...IDXS>
        constexpr decltype(auto) invoke(RClass *pReceiver, Ret(RClass::*&&rFunc)(PARA...) const, std::index_sequence<IDXS...>, void **args)
        {
            Ret res = (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
            freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
            return res;
        }

        template <typename RClass, typename ...PARA, typename std::size_t ...IDXS>
        constexpr decltype(auto) invoke(RClass *pReceiver, void(RClass::*&&rFunc)(PARA...) const, std::index_sequence<IDXS...>, void **args)
        {
            (pReceiver->*rFunc)(*reinterpret_cast<std::decay_t<PARA> *>(args[IDXS])...);
            freeParas<sizeof...(PARA)>(args, TA_MetaTypelist<PARA...> {});
        }

        template <std::size_t N>
        static constexpr void freeParas(void **pArgs, TA_MetaTypelist<>)
        {
            return;
        }

        template <std::size_t N, typename Para, typename ...Paras>
        static constexpr void freeParas(void **pArgs, TA_MetaTypelist<Para, Paras...>)
        {
            delete [] reinterpret_cast<Para *>(pArgs[0]);
            freeParas<N>(&pArgs[1], TA_MetaTypelist<Paras...> {});
            if constexpr(N == TA_MetaTypelist<Para, Paras...>::size)
            {
                delete [] pArgs;
                pArgs = nullptr;
            }
        }

    };
}

#endif // TA_SLOTOBJECT_H
