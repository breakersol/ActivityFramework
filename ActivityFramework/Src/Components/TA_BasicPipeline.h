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

#ifndef TA_ACTIVITYPIPELINE_H
#define TA_ACTIVITYPIPELINE_H

#include <atomic>
#include <cassert>

#include "TA_ActivityProxy.h"
#include "TA_MetaReflex.h"
#include "TA_MetaObject.h"
#include "TA_Connection.h"
#include "TA_Coroutine.h"

namespace CoreAsync {
    class ACTIVITY_FRAMEWORK_EXPORT TA_BasicPipeline : public TA_MetaObject
    {
    protected:
        using Milliseconds = std::chrono::duration<int,std::milli>;
        using AsyncTask = TA_CoroutineTask<TA_ActivityResultFetcher, CorotuineBehavior::Eager>;
    public:
        using ActivityIndex = unsigned int;
        enum class State
        {
            Waiting = 0,
            Busy,
            Ready
        };

        enum class ExecuteType
        {
            Async,
            Sync
        };

        struct Waiter
        {
            Waiter(AsyncTask &&fetcher) : m_fetcher(std::move(fetcher))
            {

            }

            Waiter(const Waiter &) = delete;
            Waiter(Waiter &&other) noexcept : m_fetcher(std::move(other.m_fetcher))
            {

            }

            Waiter & operator = (const Waiter &) = delete;
            Waiter & operator = (Waiter &&other) noexcept
            {
                if(this != &other)
                {
                    m_fetcher = std::move(other.m_fetcher);
                }
                return *this;
            }

            ~Waiter() = default;

            TA_DefaultVariant operator()()
            {
                return m_fetcher.get()();
            }

        private:
            AsyncTask m_fetcher;
        };

    private:
        class TA_RunningAwaitable
        {
        public:
            TA_RunningAwaitable(TA_ActivityProxy *pActivity, ExecuteType type) : m_pProxy(pActivity), m_executeType(type)
            {
                if(!m_pProxy)
                {
                    throw std::runtime_error("TA_ActivityResultAwaitable: pActivity is null!");
                }
            }

            ~TA_RunningAwaitable()
            {
                if(m_pProxy && !m_pProxy->isExecuted())
                {
                    delete m_pProxy;
                    m_pProxy = nullptr;
                }
            }

            bool await_ready() const noexcept
            {
                return m_pProxy->isExecuted() || !m_pProxy->isValid();
            }

            void await_suspend(std::coroutine_handle<> handle) noexcept
            {
                if(m_executeType == ExecuteType::Async)
                {
                    m_fetcher = TA_ThreadHolder::get().postActivity(m_pProxy);
                }
                else
                {
                    m_pProxy->operator()();
                    m_fetcher = {std::make_shared<TA_ActivityProxy>(m_pProxy)};
                }
                handle.resume();
            }

            auto await_resume() const noexcept
            {
                return m_fetcher;
            }

        private:
            TA_ActivityProxy *m_pProxy {nullptr};
            ExecuteType m_executeType {ExecuteType::Async};
            TA_ActivityResultFetcher m_fetcher {};

        };

    public:
        TA_BasicPipeline() : TA_MetaObject(), m_pRunningActivity(new TA_MethodActivity<std::function<void()>>(std::function<void()>([this]() { run(); })))
        {

        }
        TA_BasicPipeline(const TA_BasicPipeline &other) = delete;
        TA_BasicPipeline(TA_BasicPipeline &&other) = delete;
        TA_BasicPipeline & operator = (const TA_BasicPipeline &) = delete;

        virtual ~TA_BasicPipeline()
        {
            destroy();
        }

        virtual void setStartIndex(unsigned int index);

        template<typename Activity,typename ...Activities>
        void add(Activity &pActivity, Activities &...pActivities)
        {
            if(State::Waiting != m_state.load(std::memory_order_acquire))
            {
                assert(State::Waiting == m_state.load(std::memory_order_acquire));
                TA_CommonTools::debugInfo(META_STRING("Add activity failed!"));
                return;
            }
            m_mutex.lock();
            push(pActivity,pActivities...);
            m_resultList.resize(m_pActivityList.size());
            m_mutex.unlock();
        }

        virtual bool remove(ActivityIndex index);
        virtual void clear();

        Waiter execute(ExecuteType type = ExecuteType::Async)
        {
            return executeHelperFunc(type);
        }

        virtual void reset();

        std::size_t activitySize() const;

        template <typename Res>
        bool result(int index, Res &res)
        {
            if(State::Busy == m_state.load(std::memory_order_acquire))
            {
                TA_CommonTools::debugInfo(META_STRING("Get result from pipeline failed!"));
                assert(State::Busy != m_state.load(std::memory_order_acquire));             
                return false;
            }
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            if(m_resultList.size() > index)
            {
                decltype(auto) var = m_resultList.at(index);
                res = var.template get<Res>();
                return true;
            }
            return false;
        }

        State state() const;

    protected:
        virtual void run() = 0;
        void setState(State state);
        unsigned int startIndex() const;

    private:
        AsyncTask executeHelperFunc(ExecuteType type = ExecuteType::Async)
        {
            if(State::Waiting != m_state.load(std::memory_order_consume))
            {
                assert(State::Waiting == m_state.load(std::memory_order_consume));
                TA_CommonTools::debugInfo(META_STRING("Execute pipeline failed!"));
                co_return {};
            }
            setState(State::Busy);
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            auto fetcher = co_await TA_RunningAwaitable(new TA_ActivityProxy(m_pRunningActivity, false), type);
            co_return fetcher;
        }

        template<ActivityType Activity,ActivityType ...Activities>
        void push(Activity *&activity,Activities *& ...activities)
        {
            push(activity);
            push(activities...);
        }

        template <ActivityType Activity>
        void push(Activity *&activity)
        {
            if(activity)
            {
                m_pActivityList.emplace_back(std::move(new TA_ActivityProxy(activity)));
                activity = nullptr;
            }
        }

        void destroy();

    protected:
        std::list<TA_ActivityProxy *> m_pActivityList;
        std::vector<TA_DefaultVariant> m_resultList;
        std::recursive_mutex m_mutex;
        TA_MethodActivity<std::function<void()>> *m_pRunningActivity {nullptr};

    private:
        std::atomic<State> m_state {State::Waiting};
        std::atomic<unsigned int> m_startIndex {0};

    TA_Signals:
        void stateChanged(TA_BasicPipeline::State st) { std::ignore = st; };
        void activityCompleted(unsigned int index, TA_DefaultVariant res) { std::ignore = index; std::ignore = res;};

    };

    namespace Reflex
    {
        template <>
        struct ACTIVITY_FRAMEWORK_EXPORT TA_TypeInfo<TA_BasicPipeline> : TA_MetaTypeInfo<TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {Raw::State::Busy, META_STRING("Busy")},
                TA_MetaField {Raw::State::Ready, META_STRING("Ready")},
                TA_MetaField {Raw::State::Waiting, META_STRING("Waiting")},
                TA_MetaField {Raw::ExecuteType::Async, META_STRING("Async")},
                TA_MetaField {Raw::ExecuteType::Sync, META_STRING("Sync")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::execute, META_STRING("execute")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::activitySize, META_STRING("activitySize")},
                TA_MetaField {&Raw::state, META_STRING("state")},
                TA_MetaField {&Raw::stateChanged, META_STRING("stateChanged")},
                TA_MetaField {&Raw::activityCompleted, META_STRING("activityCompleted")}
            };
        };
    }
}

#endif // TA_ACTIVITYPIPELINE_H
