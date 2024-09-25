#ifndef TA_ACTIVITYPROXY_H
#define TA_ACTIVITYPROXY_H

#include <future>

#include "TA_TypeFilter.h"
#include "TA_Variant.h"

namespace CoreAsync
{
    template <typename T>
    concept ActivityType = requires(T t, const T ct)
    {
        { t() } -> std::convertible_to<TA_Variant>;
        { ct.affinityThread() } -> std::convertible_to<std::size_t>;
        { t.moveToThread(std::size_t{}) } -> std::same_as<bool>;
        { ct.id() } -> std::convertible_to<std::int64_t>;
        requires !IsTrivalCopyable<std::decay_t<T>>;
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
                m_pAffinityThreadExp = [](void *&pObj)->std::size_t {
                    return static_cast<RawActivity *>(pObj)->affinityThread();
                };

                m_pIdExp = [](void *&pObj)->int64_t {
                    return static_cast<RawActivity *>(pObj)->id();
                };

                m_pMoveThreadExp = [](void *&pObj, std::size_t thread)->bool {
                    return static_cast<RawActivity *>(pObj)->moveToThread(thread);
                };
            }
        }

        ~TA_ActivityProxy()
        {
            if(m_pDestructorExp)
                m_pDestructorExp(m_pActivity);
        }

        TA_ActivityProxy(const TA_ActivityProxy &other) = delete;
        TA_ActivityProxy(TA_ActivityProxy &&other) : m_pActivity(std::exchange(other.m_pActivity, nullptr)), m_pExecuteExp(std::exchange(other.m_pExecuteExp, nullptr)), m_pDestructorExp(std::exchange(other.m_pDestructorExp, nullptr)), m_pAffinityThreadExp(std::exchange(other.m_pAffinityThreadExp, nullptr)), m_pIdExp(std::exchange(other.m_pIdExp, nullptr)), m_pMoveThreadExp(std::exchange(other.m_pMoveThreadExp, nullptr)), m_future(std::move(other.m_future))
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
                m_pAffinityThreadExp = std::exchange(other.m_pAffinityThreadExp, nullptr);   
                m_pIdExp = std::exchange(other.m_pIdExp, nullptr);
                m_pMoveThreadExp = std::exchange(other.m_pMoveThreadExp, nullptr);
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

        std::size_t affinityThread() const
        {
            return m_pAffinityThreadExp(m_pActivity);
        }

        int64_t id() const
        {
            return m_pIdExp(m_pActivity);
        }

        bool moveToThread(std::size_t thread)
        {
            return m_pMoveThreadExp(m_pActivity, thread);
        }

    private:
        void *m_pActivity;
        void (*m_pExecuteExp)(void *, std::promise<TA_Variant> &&promise);
        void (*m_pDestructorExp)(void *);
        std::size_t (*m_pAffinityThreadExp)(void *);
        int64_t (*m_pIdExp)(void *);
        bool (*m_pMoveThreadExp)(void *, std::size_t);
        std::shared_future<TA_Variant> m_future {};

    };

    struct ActivityResultFetcher
    {
        std::shared_ptr<TA_ActivityProxy> pProxy;

        TA_Variant operator()()
        {
            return pProxy->result();
        }
    };
}

#endif // TA_ACTIVITYPROXY_H
