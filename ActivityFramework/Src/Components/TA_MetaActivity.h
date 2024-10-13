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
            using Instance = std::remove_cvref_t<Ins>;
            static constexpr auto exp {Reflex::TA_TypeInfo<Instance>::findType(Method {})};
           using Func = std::decay_t<decltype(exp)>;
            static_assert(std::is_same_v<std::nullptr_t, Func>, "Cannot find the method from the class.");
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
            static_assert(std::is_same_v<std::nullptr_t, Func>, "Cannot find the method from the class.");
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
                        (pIns->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (pIns->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
            }};
        };

        template <typename Method, typename Ins, typename ...RemainedParas>
        struct ExpParser<Method, std::unique_ptr<Ins>, RemainedParas...>
        {
            using Instance = std::remove_cvref_t<Ins>;
            static constexpr auto exp {Reflex::TA_TypeInfo<Instance>::findType(Method {})};
            using Func = std::decay_t<decltype(exp)>;
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
                        (pIns->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                    else
                        return (pIns->*Reflex::template TA_TypeInfo<Instance>::findType(Method {}))(paras...);
                }
            }};
        };

    public:
        TA_MetaActivity() = delete;
        TA_MetaActivity(const TA_MetaActivity &activity) = delete;
        TA_MetaActivity(TA_MetaActivity &&activity) = delete;

        TA_MetaActivity & operator = (const TA_MetaActivity &activity) = delete;
        TA_MetaActivity & operator = (TA_MetaActivity &&activity) = delete;

        TA_MetaActivity(MethodName, const Paras &...para) : m_paras(para...), m_affinityThread(TA_ThreadHolder::get().topPriorityThread())
        {

        }

        TA_MetaActivity(MethodName, const Paras &...para, std::size_t affinityThread) : m_paras(para...), m_affinityThread(affinityThread)
        {

        }

        decltype(auto) operator()()
        {
            return std::apply(ExpParser<MethodName, Paras...>::exp, m_paras);
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

    private:
        const std::tuple<const Paras &...> m_paras;
        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};

    };
}

#endif // TA_METAACTIVITY_H
