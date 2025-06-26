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

#include "Components/TA_ConcurrentPipeline.h"
#include "Components/TA_CommonTools.h"
#include "Components/TA_ThreadPool.h"

namespace CoreAsync {
    TA_ConcurrentPipeline::TA_ConcurrentPipeline() : TA_BasicPipeline()
    {
        
    }

    TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Eager> runningGenerator(TA_ConcurrentPipeline *pPipeline)
    {
        for(auto i = pPipeline->startIndex();i < pPipeline->m_pActivityList.size();++i)
        {
            decltype(auto) pActivity {TA_CommonTools::at<TA_ActivityProxy *>(pPipeline->m_pActivityList, i)};
            auto fetcher = co_await TA_ActivityExecutingAwaitable(pActivity, TA_ActivityExecutingAwaitable::ExecuteType::Async);
            auto res = co_await TA_ActivityResultAwaitable(fetcher);
            TA_CommonTools::replace(pPipeline->m_resultList, i, res);
            co_yield fetcher;
        }
        co_return;
    }

    void TA_ConcurrentPipeline::run()
    {
        std::size_t sIndex(std::move(startIndex()));
        std::size_t activitySize = m_pActivityList.size();
        if(activitySize > 0)
        {
            m_resultFetchers.resize(activitySize);
            for(std::size_t i = sIndex;i < activitySize;++i)
            {
                m_resultFetchers[i] = TA_ThreadHolder::get().postActivity(TA_CommonTools::ref<TA_ActivityProxy *>(m_pActivityList, i));
            }
            std::size_t idx {0};
            for(auto &fetcher : m_resultFetchers)
            {
                m_resultList[idx] = fetcher();
                TA_Connection::active(this, &TA_ConcurrentPipeline::activityCompleted, idx, std::forward<TA_DefaultVariant>(m_resultList[idx]));
                idx++;
            }
            m_pActivityList.clear();
            setState(State::Ready);
        }  
    }

    void TA_ConcurrentPipeline::clear()
    {
        if(State::Busy == state())
        {
            assert(State::Busy != state());
            TA_CommonTools::debugInfo(META_STRING("Clear pipeline failed!"));
            return;
        }
        m_mutex.lock();
        for(auto &pActivity : m_pActivityList)
        {
            if(pActivity)
            {
                delete pActivity;
                pActivity = nullptr;
            }
        }
        m_pActivityList.clear();
        m_resultList.clear();
        m_resultFetchers.clear();
        m_mutex.unlock();
        setState(State::Waiting);
    }

    void TA_ConcurrentPipeline::reset()
    {
        if(State::Ready != state())
        {
            assert(State::Ready == state());
            TA_CommonTools::debugInfo(META_STRING("Reset pipeline failed!"));
            return;
        }
        m_mutex.lock();
        m_resultList.clear();
        m_resultList.resize(m_pActivityList.size());
        m_resultFetchers.clear();
        m_mutex.unlock();
        setState(State::Waiting);
    }
}
