/*
 * Copyright [2024] [Shuang Zhu / Sol]
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

#include <future>

namespace CoreAsync {
    TA_ConcurrentPipeline::TA_ConcurrentPipeline() : TA_BasicPipeline(), m_waitingCount(0)
    {
        
    }

    void TA_ConcurrentPipeline::run()
    {
        TA_Connection::connect(&TA_ThreadHolder::get(), &TA_ThreadPool::taskCompleted, this, &TA_ConcurrentPipeline::taskCompleted);
        std::size_t sIndex(std::move(startIndex()));
        std::size_t activitySize = m_pActivityList.size();
        if(activitySize > 0)
        {
            m_activityIds.resize(activitySize);
            m_waitingCount = activitySize;
            for(std::size_t i = sIndex;i < activitySize;++i)
            {
                auto [ft, id] = TA_ThreadHolder::get().postActivity(TA_CommonTools::at<TA_BasicActivity *>(m_pActivityList, i));
                m_activityIds[i] = id;
            }
            m_postSemaphore.release();
        }  
    }

    void TA_ConcurrentPipeline::taskCompleted(std::size_t id, TA_Variant var)
    {
        m_postSemaphore.acquire();
        for(std::size_t idx = 0;idx < m_activityIds.size();++idx)
        {
            if(m_activityIds[idx] == id)
            {
                m_resultList[idx] = var;
                TA_Connection::active(this, &TA_ConcurrentPipeline::activityCompleted, idx, std::forward<TA_Variant>(m_resultList[idx]));
                if (--m_waitingCount == 0)
                {
                    setState(State::Ready);
                    TA_Connection::disconnect(&TA_ThreadHolder::get(), &TA_ThreadPool::taskCompleted, this, &TA_ConcurrentPipeline::taskCompleted);
                }
                break;
            }
        }
        if(m_waitingCount != 0)
            m_postSemaphore.release();
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
        m_activityIds.clear();
        m_waitingCount = 0;
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
        m_activityIds.clear();
        m_waitingCount = 0;
        m_mutex.unlock();
        setState(State::Waiting);
    }
}
