#ifndef TA_ASYNCACTIVITYPROXY_H
#define TA_ASYNCACTIVITYPROXY_H

#include <concepts>
#include <future>

#include "TA_TypeFilter.h"
#include "TA_Variant.h"

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

        template <ActivityType Activity>
        explicit TA_AsyncActivityProxy(Activity *pActivity, bool autoDelete = true) : m_pActivity(pActivity)
        {
            using RawActivity = std::remove_cvref_t<Activity>;
            if(pActivity)
            {
                if(autoDelete)
                {
                    m_pExecuteExp = [](void *&pObj, std::promise<TA_Variant> &promise)->void {
                        std::unique_ptr<RawActivity, AutoDeleter<RawActivity>> ptr {static_cast<RawActivity *>(pObj)};
                        pObj = nullptr;
                        TA_Variant var;
                        var.set(ptr->operator()());
                        promise.set_value(var);
                    };
                    m_pDestructorExp = [&autoDelete](void *&pObj)->void {
                        if(pObj)
                        {
                            delete static_cast<RawActivity *>(pObj);
                            pObj = nullptr;
                        }
                    };
                }
                else
                {
                    m_pExecuteExp = [](void *&pObj, std::promise<TA_Variant> &promise)->void {
                        TA_Variant var;
                        var.set(static_cast<RawActivity *>(pObj)->operator()());
                        promise.set_value(var);
                    };
                    m_pDestructorExp = nullptr;
                }
            }
        }

        ~TA_AsyncActivityProxy()
        {
            if(m_pDestructorExp)
                m_pDestructorExp(m_pActivity);
        }

        TA_AsyncActivityProxy(const TA_AsyncActivityProxy &other) = delete;
        TA_AsyncActivityProxy(TA_AsyncActivityProxy &&other) : m_pActivity(std::exchange(other.m_pActivity, nullptr)), m_pExecuteExp(std::exchange(other.m_pExecuteExp, nullptr)), m_pDestructorExp(std::exchange(other.m_pDestructorExp, nullptr)), m_promise(std::move(other.m_promise)), m_future(std::move(other.m_future))
        {

        }

        TA_AsyncActivityProxy & operator = (const TA_AsyncActivityProxy &other) = delete;
        TA_AsyncActivityProxy & operator = (TA_AsyncActivityProxy &&other)
        {
            if(this != &other)
            {
                m_pActivity = std::exchange(other.m_pActivity, nullptr);
                m_pExecuteExp = std::exchange(other.m_pExecuteExp, nullptr);
                m_pDestructorExp = std::exchange(other.m_pDestructorExp, nullptr);
                m_promise = std::move(other.m_promise);
                m_future = std::move(other.m_future);
            }
            return *this;
        }

        AsyncRet asyncResult() const
        {
            return m_future;
        }

        void operator()()
        {
            m_pExecuteExp(m_pActivity, m_promise);
        }

    private:
        void *m_pActivity;
        void (*m_pExecuteExp)(void *, std::promise<TA_Variant> &promise);
        void (*m_pDestructorExp)(void *);
        std::promise<TA_Variant> m_promise {};
        std::shared_future<TA_Variant> m_future {m_promise.get_future()};

    };
}

#endif // TA_ASYNCACTIVITYPROXY_H
