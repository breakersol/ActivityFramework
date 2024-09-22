#ifndef TA_ACTIVITYPROXY_H
#define TA_ACTIVITYPROXY_H

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

    class TA_ActivityProxy
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
    public:
        TA_ActivityProxy() = delete;

        template <ActivityType Activity>
        explicit TA_ActivityProxy(Activity *pActivity, bool autoDelete = true) : m_pActivity(pActivity)
        {
            using RawActivity = std::remove_cvref_t<Activity>;
            if(pActivity)
            {
                if(autoDelete)
                {
                    m_pExecuteExp = [](void *&pObj, std::promise<TA_Variant> &&promise)->void {
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

        ~TA_ActivityProxy()
        {
            if(m_pDestructorExp)
                m_pDestructorExp(m_pActivity);
        }

        TA_ActivityProxy(const TA_ActivityProxy &other) = delete;
        TA_ActivityProxy(TA_ActivityProxy &&other) : m_pActivity(std::exchange(other.m_pActivity, nullptr)), m_pExecuteExp(std::exchange(other.m_pExecuteExp, nullptr)), m_pDestructorExp(std::exchange(other.m_pDestructorExp, nullptr)), m_future(std::move(other.m_future))
        {

        }

        TA_ActivityProxy & operator = (const TA_ActivityProxy &other) = delete;
        TA_ActivityProxy & operator = (TA_ActivityProxy &&other)
        {
            if(this != &other)
            {
                m_pActivity = std::exchange(other.m_pActivity, nullptr);
                m_pExecuteExp = std::exchange(other.m_pExecuteExp, nullptr);
                m_pDestructorExp = std::exchange(other.m_pDestructorExp, nullptr);
                m_future = std::move(other.m_future);
            }
            return *this;
        }

        bool isValid() const
        {
            return m_future.valid();
        }

        TA_Variant result() const
        {
            return m_future.get();
        }

        void operator()()
        {
            std::promise<TA_Variant> promise;
            m_future = promise.get_future().share();
            m_pExecuteExp(m_pActivity, std::move(promise));
        }

    private:
        void *m_pActivity;
        void (*m_pExecuteExp)(void *, std::promise<TA_Variant> &&promise);
        void (*m_pDestructorExp)(void *);
        std::shared_future<TA_Variant> m_future {};

    };
}

#endif // TA_ACTIVITYPROXY_H
