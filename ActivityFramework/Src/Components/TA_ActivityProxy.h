#ifndef TA_ACTIVITYPROXY_H
#define TA_ACTIVITYPROXY_H

#include <future>
#include <utility>

#include "TA_TypeFilter.h"
#include "TA_Variant.h"

namespace CoreAsync
{
    template <typename T>
    concept ActivityType = requires(T t, const T ct)
    {
        { t() };
        { ct.affinityThread() } -> std::convertible_to<std::size_t>;
        { t.moveToThread(std::size_t{}) } -> std::same_as<bool>;
        { ct.id() } -> std::convertible_to<std::int64_t>;
        requires !IsTrivalCopyable<std::decay_t<T>>;
    };

    class TA_ActivityProxy
    {
    public:
        TA_ActivityProxy() = delete;

        template <ActivityType Activity>
        explicit TA_ActivityProxy(Activity *pActivity, bool autoDelete = true) : m_pActivity(pActivity, autoDelete ? [](void *pActivity)->void{delete static_cast<Activity *>(pActivity);} : [](void *pActivity)->void{})
        {
            using RawActivity = std::remove_cvref_t<Activity>;
            if(pActivity)
            {
                m_pExecuteExp = [](auto &pObj, std::promise<TA_Variant> &&promise)->void {
                    TA_Variant var;
                    var.set(static_cast<RawActivity *>(pObj.get())->operator()());
                    promise.set_value(std::move(var));
                };
                m_pAffinityThreadExp = [](auto const &pObj)->std::size_t {
                    return static_cast<RawActivity *>(pObj.get())->affinityThread();
                };

                m_pIdExp = [](auto const &pObj)->int64_t {
                    return static_cast<RawActivity *>(pObj.get())->id();
                };

                m_pMoveThreadExp = [](auto &pObj, std::size_t thread)->bool {
                    return static_cast<RawActivity *>(pObj.get())->moveToThread(thread);
                };
            }
        }

        ~TA_ActivityProxy() = default;

        TA_ActivityProxy(const TA_ActivityProxy &other) = delete;
        TA_ActivityProxy(TA_ActivityProxy &&other) : m_pActivity(std::exchange(other.m_pActivity, nullptr)), m_pExecuteExp(std::exchange(other.m_pExecuteExp, nullptr)), m_pAffinityThreadExp(std::exchange(other.m_pAffinityThreadExp, nullptr)), m_pIdExp(std::exchange(other.m_pIdExp, nullptr)), m_pMoveThreadExp(std::exchange(other.m_pMoveThreadExp, nullptr)), m_future(std::move(other.m_future))
        {

        }

        TA_ActivityProxy & operator = (const TA_ActivityProxy &other) = delete;
        TA_ActivityProxy & operator = (TA_ActivityProxy &&other)
        {
            if(this != &other)
            {
                m_pActivity = std::exchange(other.m_pActivity, nullptr);
                m_pExecuteExp = std::exchange(other.m_pExecuteExp, nullptr);
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
            if (!m_isExecuted.load(std::memory_order_acquire))
            {
                m_pExecuteExp(m_pActivity, std::forward<std::promise<TA_Variant>>(m_promise));
                m_isExecuted.store(true, std::memory_order_release);
            }    
        }

        bool isExecuted() const
        {
            return m_isExecuted.load(std::memory_order_acquire);
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
        std::unique_ptr<void, void(*)(void *)> m_pActivity;
        void (*m_pExecuteExp)(std::unique_ptr<void, void(*)(void *)> &, std::promise<TA_Variant> &&promise);
        std::size_t (*m_pAffinityThreadExp)(std::unique_ptr<void, void(*)(void *)> const &);
        int64_t (*m_pIdExp)(std::unique_ptr<void, void(*)(void *)> const &);
        bool (*m_pMoveThreadExp)(std::unique_ptr<void, void(*)(void *)> &, std::size_t);
        std::promise<TA_Variant> m_promise{};
        std::shared_future<TA_Variant> m_future { m_promise.get_future().share()};
        std::atomic_bool m_isExecuted{ false };

    };

    struct TA_ActivityResultFetcher
    {
        std::shared_ptr<TA_ActivityProxy> pProxy{nullptr};

        TA_Variant operator()()
        {
            return pProxy->result();
        }
    };
}

#endif // TA_ACTIVITYPROXY_H
