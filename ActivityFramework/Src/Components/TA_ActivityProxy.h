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

#ifndef TA_ACTIVITYPROXY_H
#define TA_ACTIVITYPROXY_H

#include <future>
#include <utility>

#include "TA_TypeFilter.h"
#include "TA_Variant.h"

namespace CoreAsync {
template <typename T>
concept ActivityType = requires(T t, const T ct) {
    { t() };
    { ct.affinityThread() } -> std::convertible_to<std::size_t>;
    { t.moveToThread(std::size_t{}) } -> std::same_as<bool>;
    { ct.id() } -> std::convertible_to<std::int64_t>;
    requires !IsTrivalCopyable<std::decay_t<T>>;
};

class TA_ActivityProxy {
    template <typename Ret, typename... Args> using Executor = Ret (*)(Args...);

  public:
    TA_ActivityProxy() = delete;

    template <ActivityType Activity>
    explicit TA_ActivityProxy(Activity *pActivity, bool autoDelete = true)
        : m_pActivity(
              pActivity,
              autoDelete ? [](void *pActivity) -> void { delete static_cast<Activity *>(pActivity); }
                         : [](void *pActivity) -> void {}) {
        using RawActivity = std::remove_cvref_t<Activity>;
        if (pActivity) {
            using Ret = std::invoke_result_t<decltype(&RawActivity::operator()), RawActivity>;
            m_pExecuteExp = [](auto &pObj, std::promise<TA_DefaultVariant> &&promise) -> void {
                TA_DefaultVariant var;
                if constexpr (std::is_void_v<Ret>) {
                    static_cast<RawActivity *>(pObj.get())->operator()();
                    var.set(nullptr);
                } else
                    var.set(static_cast<RawActivity *>(pObj.get())->operator()());
                promise.set_value(std::move(var));
            };
            m_pAffinityThreadExp = [](auto const &pObj) -> std::size_t {
                return static_cast<RawActivity *>(pObj.get())->affinityThread();
            };

            m_pDependThreadIdExp = [](auto const &pObj) -> std::thread::id {
                return static_cast<RawActivity *>(pObj.get())->dependencyThreadId();
            };

            m_pIdExp = [](auto const &pObj) -> int64_t { return static_cast<RawActivity *>(pObj.get())->id(); };

            m_pMoveThreadExp = [](auto &pObj, std::size_t thread) -> bool {
                return static_cast<RawActivity *>(pObj.get())->moveToThread(thread);
            };
            m_pStolenEnabledExp = [](auto &pObj) -> bool {
                return static_cast<RawActivity *>(pObj.get())->stolenEnabled();
            };
        }
    }

    ~TA_ActivityProxy() = default;

    TA_ActivityProxy(const TA_ActivityProxy &other) = delete;
    TA_ActivityProxy(TA_ActivityProxy &&other) noexcept
        : m_pActivity(std::exchange(other.m_pActivity, nullptr)),
          m_pExecuteExp(std::exchange(other.m_pExecuteExp, nullptr)),
          m_pAffinityThreadExp(std::exchange(other.m_pAffinityThreadExp, nullptr)),
          m_pDependThreadIdExp(std::exchange(other.m_pDependThreadIdExp, nullptr)),
          m_pIdExp(std::exchange(other.m_pIdExp, nullptr)),
          m_pMoveThreadExp(std::exchange(other.m_pMoveThreadExp, nullptr)),
          m_pStolenEnabledExp(std::exchange(other.m_pStolenEnabledExp, nullptr)),
          m_future(std::move(other.m_future)) {}

    TA_ActivityProxy &operator=(const TA_ActivityProxy &other) = delete;
    TA_ActivityProxy &operator=(TA_ActivityProxy &&other) noexcept {
        if (this != &other) {
            m_pActivity = std::exchange(other.m_pActivity, nullptr);
            m_pExecuteExp = std::exchange(other.m_pExecuteExp, nullptr);
            m_pAffinityThreadExp = std::exchange(other.m_pAffinityThreadExp, nullptr);
            m_pDependThreadIdExp = std::exchange(other.m_pDependThreadIdExp, nullptr);
            m_pIdExp = std::exchange(other.m_pIdExp, nullptr);
            m_pMoveThreadExp = std::exchange(other.m_pMoveThreadExp, nullptr);
            m_pStolenEnabledExp = std::exchange(other.m_pStolenEnabledExp, nullptr);
            m_future = std::move(other.m_future);
        }
        return *this;
    }

    bool isValid() const { return m_pActivity && m_future.valid(); }

    TA_DefaultVariant result() const { return m_future.get(); }

    void operator()() {
        if (!m_pExecuteExp || !m_pActivity) {
            throw std::runtime_error("Execute function or activity is null");
        }
        bool expected{false};
        if (m_isExecuted.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            m_pExecuteExp(m_pActivity, std::forward<std::promise<TA_DefaultVariant>>(m_promise));
            m_pExecuteExp = nullptr;
        }
    }

    bool isExecuted() const { return m_isExecuted.load(std::memory_order_acquire); }

    std::size_t affinityThread() const { return m_pAffinityThreadExp(m_pActivity); }

    std::thread::id dependencyThreadId() const { return m_pDependThreadIdExp(m_pActivity); }

    int64_t id() const { return m_pIdExp(m_pActivity); }

    bool moveToThread(std::size_t thread) { return m_pMoveThreadExp(m_pActivity, thread); }

    bool stolenEnabled() const {
        return m_pStolenEnabledExp(m_pActivity);
    }

  private:
    std::unique_ptr<void, void (*)(void *)> m_pActivity;
    Executor<void, std::unique_ptr<void, void (*)(void *)> &, std::promise<TA_DefaultVariant> &&> m_pExecuteExp;
    Executor<std::size_t, std::unique_ptr<void, void (*)(void *)> const &> m_pAffinityThreadExp;
    Executor<std::thread::id, std::unique_ptr<void, void (*)(void *)> const &> m_pDependThreadIdExp;
    Executor<std::int64_t, std::unique_ptr<void, void (*)(void *)> const &> m_pIdExp;
    Executor<bool, std::unique_ptr<void, void (*)(void *)> &, std::size_t> m_pMoveThreadExp;
    Executor<bool, std::unique_ptr<void, void (*)(void *)> const &> m_pStolenEnabledExp;
    std::promise<TA_DefaultVariant> m_promise{};
    std::shared_future<TA_DefaultVariant> m_future{m_promise.get_future().share()};
    std::atomic_bool m_isExecuted{false};
};

class TA_ActivityResultFetcher {
  public:
    TA_ActivityResultFetcher() = default;
    TA_ActivityResultFetcher(std::shared_ptr<TA_ActivityProxy> proxy) : pProxy(proxy) {}

    virtual TA_DefaultVariant operator()() const { return pProxy->result(); }
    bool isValid() const { return pProxy && pProxy->isValid(); }
    bool isExecuted() const { return pProxy && pProxy->isExecuted(); }

  private:
    std::shared_ptr<TA_ActivityProxy> pProxy{nullptr};

};
} // namespace CoreAsync

#endif // TA_ACTIVITYPROXY_H
