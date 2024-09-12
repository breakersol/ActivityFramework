#ifndef TA_ASYNCACTIVITYPROXY_H
#define TA_ASYNCACTIVITYPROXY_H

#include <concepts>

#include "TA_MetaActivity.h"
#include "TA_SingleActivity.h"

namespace CoreAsync
{
    template <typename T>
    concept ActivityType = requires(T t) {
        std::decay_t<T>::operator();
        std::decay_t<T>::affinityThread();
        std::decay_t<T>::moveToThread();
        std::decay_t<T>::id();
        !IsTrivalCopyable<T>;
    };

    class TA_AsyncActivityProxy
    {
        template <typename Activity>
        struct AutoDeleter
        {
            using type = std::remove_cvref_t<Activity>;
            void operator()(Activity *&pActivity)
            {
                if(pActivity)
                {
                    delete pActivity;
                    pActivity = nullptr;
                }
            }
        };

        using AsyncRet = std::shared_future<TA_Variant>;
    public:
        TA_AsyncActivityProxy() = delete;

        template <ActivityType Activity, bool AutoDelete = true>
        TA_AsyncActivityProxy(Activity *pActivity) : m_pActivity(pActivity)
        {
            using RawActivity = std::remove_cvref_t<Activity>;
            if(pActivity)
            {
                if constexpr(AutoDelete)
                {
                    m_pExecuteExp = [](void *&pObj, std::promise<TA_Variant> &promise)->void {
                        std::unique_ptr<RawActivity, AutoDeleter<RawActivity>> ptr {static_cast<RawActivity *>(pObj)};
                        pObj = nullptr;
                        TA_Variant var;
                        var.set(ptr->operator()());
                        promise.set_value(var);
                    };
                }
                else
                {
                    m_pExecuteExp = [](void *&pObj, std::promise<TA_Variant> &promise)->void {
                        TA_Variant var;
                        var.set(static_cast<RawActivity *>(pObj)->operator()());
                        promise.set_value(var);
                    };
                }
                m_pDestructorExp = [](void *&pObj)->void {
                    if constexpr(AutoDelete)
                    {
                        if(pObj)
                        {
                            delete static_cast<RawActivity *>(pObj);
                            pObj = nullptr;
                        }
                    }
                };
            }
        }

        ~TA_AsyncActivityProxy()
        {
            m_pDestructorExp(m_pActivity);
        }

        TA_AsyncActivityProxy(const TA_AsyncActivityProxy &other) = delete;
        TA_AsyncActivityProxy(TA_AsyncActivityProxy &&other) = delete;

        TA_AsyncActivityProxy & operator = (const TA_AsyncActivityProxy &other) = delete;
        TA_AsyncActivityProxy & operator = (TA_AsyncActivityProxy &&other) = delete;

        AsyncRet execute()
        {
            //Post into thread pool...
            m_pExecuteExp(m_pActivity, m_promise);
            return m_promise.get_future();
        }

    private:
        void (*m_pExecuteExp)(void *, std::promise<TA_Variant> &promise);
        void (*m_pDestructorExp)(void *);
        void *m_pActivity;
        std::promise<TA_Variant> m_promise {};

    };
}

#endif // TA_ASYNCACTIVITYPROXY_H
