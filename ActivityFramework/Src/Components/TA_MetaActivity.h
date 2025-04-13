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

#ifndef TA_METAACTIVITY_H
#define TA_METAACTIVITY_H

#include "TA_MetaReflex.h"
#include "TA_ActivityComponments.h"

namespace CoreAsync
{
    template <typename T>
    concept Method = requires(T t)
    {
        typename T::RawType;
        {T::data()};
    };

    template <Method MethodName, typename ...Paras>
    class TA_MetaActivity
    {
    public:
        template <typename Method, typename Ins, typename ...RemainedParas>
        struct ExpParser
        {
            using Instance = std::decay_t<Ins>;
            static constexpr auto exp {Reflex::TA_TypeInfo<Instance>::findType(Method {})};
            using Func = std::decay_t<decltype(exp)>;
            static constexpr auto isStaticMethod {IsStaticMethod<Func>::value};
            static_assert(!std::is_same_v<std::nullptr_t, Func>, "Cannot find the method from the class.");
            static constexpr auto lambdaExp {[](Instance &ins, const RemainedParas &...paras)->FunctionTypeInfo<Func>::RetType {
                if constexpr(IsStaticMethod<Func>::value)
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
                else
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (ins.*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (ins.*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
            }};
        };

        template <typename Method, typename Ins, typename ...RemainedParas>
        struct ExpParser<Method, Ins *, RemainedParas...>
        {
            using Instance = std::remove_cvref_t<Ins>;
            static constexpr auto exp {Reflex::TA_TypeInfo<Instance>::findType(Method {})};
            static_assert(!std::is_same_v<std::nullptr_t, std::decay_t<decltype(exp)>>, "Cannot find the method from the class.");
            using Func = std::decay_t<decltype(exp)>;
            static constexpr auto isStaticMethod {IsStaticMethod<Func>::value};
            static constexpr auto lambdaExp {[](Instance *pIns, const RemainedParas &...paras)->FunctionTypeInfo<Func>::RetType {
                if constexpr(IsStaticMethod<Func>::value)
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
                else
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (pIns->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (pIns->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
            }};
        };

        template <typename Method, typename Ins, typename ...RemainedParas>
        struct ExpParser<Method, std::shared_ptr<Ins>, RemainedParas...>
        {
            using Instance = std::remove_cvref_t<Ins>;
            static constexpr auto exp {Reflex::TA_TypeInfo<Instance>::findType(Method {})};
            using Func = std::decay_t<decltype(exp)>;
            static constexpr auto isStaticMethod {IsStaticMethod<Func>::value};
            static_assert(!std::is_same_v<std::nullptr_t, Func>, "Cannot find the method from the class.");
            static constexpr auto lambdaExp {[](std::shared_ptr<Instance> &pIns, const RemainedParas &...paras)->FunctionTypeInfo<Func>::RetType {
                if constexpr(IsStaticMethod<Func>::value)
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
                else
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (pIns.get()->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (pIns.get()->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
            }};
        };

        template <typename Method, typename Ins, typename ...RemainedParas>
        struct ExpParser<Method, std::unique_ptr<Ins>, RemainedParas...>
        {
            using Instance = std::remove_cvref_t<Ins>;
            static constexpr auto exp {Reflex::TA_TypeInfo<Instance>::findType(Method {})};
            using Func = std::decay_t<decltype(exp)>;
            static constexpr auto isStaticMethod {IsStaticMethod<Func>::value};
            static_assert(!std::is_same_v<std::nullptr_t, Func>, "Cannot find the method from the class.");
            static constexpr auto lambdaExp {[](std::unique_ptr<Instance> &pIns, const RemainedParas &...paras)->FunctionTypeInfo<Func>::RetType {
                if constexpr(IsStaticMethod<Func>::value)
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (*Reflex::TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
                else
                {
                    if constexpr(std::is_same_v<typename FunctionTypeInfo<Func>::RetType, void>)
                        (pIns.get()->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (pIns.get()->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
            }};
        };

    public:
        TA_MetaActivity() = delete;
        TA_MetaActivity(const TA_MetaActivity &activity) = delete;
        TA_MetaActivity(TA_MetaActivity &&activity) = delete;

        TA_MetaActivity & operator = (const TA_MetaActivity &activity) = delete;
        TA_MetaActivity & operator = (TA_MetaActivity &&activity) = delete;

        TA_MetaActivity(MethodName, Paras &&...para) : m_paras(std::forward<Paras>(para)...), m_affinityThread(TA_ThreadHolder::get().topPriorityThread())
        {

        }

        TA_MetaActivity(MethodName, Paras &&...para, std::size_t affinityThread) : m_paras(std::forward<Paras>(para)...), m_affinityThread(affinityThread)
        {

        }

        decltype(auto) operator()()
        {
            if constexpr(ExpParser<MethodName, std::remove_reference_t<Paras>...>::isStaticMethod)
            {
                return std::apply(ExpParser<MethodName, std::remove_reference_t<Paras>...>::exp, getStaticMethodParas(m_paras, std::make_index_sequence<sizeof...(Paras)> {}));
            }
            else
                return std::apply(ExpParser<MethodName, std::remove_reference_t<Paras>...>::exp, m_paras);
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

        void setParas(Paras &&...paras)
        {
            m_paras = std::make_tuple(std::forward<Paras>(paras)...);
        }

    private:
        template<typename T>
        using StorageType = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;

        template <std::size_t Idx, std::size_t ...Idxs>
        static constexpr auto getStaticMethodParas(const std::tuple<StorageType<Paras>...> &paras, std::index_sequence<Idx, Idxs...>)
        {
            if constexpr(sizeof...(Idxs) != 0)
                return std::make_tuple(std::get<Idxs>(paras)...);
            else
                return std::make_tuple();
        }

    private:
        const std::tuple<StorageType<Paras>...> m_paras;
        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};

    };
}

#endif // TA_METAACTIVITY_H
