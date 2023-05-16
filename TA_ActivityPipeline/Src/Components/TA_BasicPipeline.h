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

#ifndef TA_ACTIVITYPIPELINE_H
#define TA_ACTIVITYPIPELINE_H

#include <future>
#include <atomic>
#include <cassert>

#include "TA_LinkedActivity.h"
#include "TA_MetaReflex.h"
#include "TA_MetaObject.h"
#include "TA_Connection.h"

namespace CoreAsync {

    ///Activity管线基类
    class ASYNC_PIPELINE_EXPORT TA_BasicPipeline : public TA_MetaObject
    {  
    protected:
        using Milliseconds = std::chrono::duration<int,std::milli>;

    public:
        using ActivityIndex = unsigned int;
        enum class State
        {
            Waiting = 0,            //空闲
            Busy,                   //繁忙
            Ready                   //完成
        };

        enum class ExecuteType
        {
            Async,
            Sync
        };

        TA_BasicPipeline() : TA_MetaObject()
        {

        }
        TA_BasicPipeline(const TA_BasicPipeline &activity) = delete;
        TA_BasicPipeline(TA_BasicPipeline &&activity) = delete;
        TA_BasicPipeline & operator = (const TA_BasicPipeline &) = delete;

        virtual ~TA_BasicPipeline()
        {
            clear();
        }

        virtual bool waitingComplete();

        virtual void setStartIndex(unsigned int index);

        bool switchActivityBranch(int activityIndex, std::deque<unsigned int> branches);

        template <typename T>
        using EnableActivityType = typename std::enable_if<std::is_pointer<typename std::remove_reference<T>::type>::value, T>::type;
        template<typename Activity,typename ...Activities>
        void add(EnableActivityType<Activity> &pActivity,EnableActivityType<Activities> &...pActivities)
        {
            if(State::Waiting != m_state.load(std::memory_order_acquire))
            {
                assert(State::Waiting == m_state.load(std::memory_order_acquire));
                std::printf("Add activity failed!");
                return;
            }
            m_mutex.lock();
            push(pActivity,pActivities...);
            m_resultList.resize(m_pActivityList.size());
            m_mutex.unlock();
        }

        virtual bool remove(ActivityIndex index);
        virtual void clear();
        void execute(ExecuteType type = ExecuteType::Async);
        virtual void reset();

        std::size_t activitySize() const;

        template <typename Res>
        bool result(int index, Res &res)
        {
            if(State::Busy == m_state.load(std::memory_order_acquire))
            {
                assert(State::Busy != m_state.load(std::memory_order_acquire));
                std::printf("Get result from pipeline failed!");
                return false;
            }
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            if(m_resultList.size() > index)
            {
                auto var = m_resultList.at(index);
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
        template<typename Activity,typename ...Activities>
        void push(Activity &activity,Activities & ...activities)
        {
            push(activity);
            push(activities...);
        }

        template <typename Activity>
        void push(Activity &activity)
        {
            if(activity)
            {
                m_pActivityList.push_back(activity);
                activity = nullptr;
            }
        }

    protected:
        std::list<TA_BasicActivity *> m_pActivityList;
        std::vector<TA_Variant> m_resultList;
        std::recursive_mutex m_mutex;

    private:
        std::atomic<State> m_state {State::Waiting};
        std::atomic<unsigned int> m_startIndex {0};
        std::future<void> m_ft;

    TA_Signals:
        void stateChanged(TA_BasicPipeline::State st) { std::ignore = st; };
        void activityCompleted(unsigned int index) { std::ignore = index; };

    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_BasicPipeline> : TA_MetaTypeInfo<TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {Raw::State::Busy, META_STRING("Busy")},
                TA_MetaField {Raw::State::Ready, META_STRING("Ready")},
                TA_MetaField {Raw::State::Waiting, META_STRING("Waiting")},
                TA_MetaField {Raw::ExecuteType::Async, META_STRING("Async")},
                TA_MetaField {Raw::ExecuteType::Sync, META_STRING("Sync")},
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::switchActivityBranch, META_STRING("switchActivityBranch")},
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
