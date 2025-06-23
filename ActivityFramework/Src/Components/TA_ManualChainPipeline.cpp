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

#include "Components/TA_ManualChainPipeline.h"
#include "Components/TA_CommonTools.h"

namespace CoreAsync {
    TA_ManualChainPipeline::TA_ManualChainPipeline() : TA_BasicPipeline()
    {

    }

    void TA_ManualChainPipeline::run()
    {
        if(m_runningGenerator.next())
        {
            setState(State::Waiting);
        }
        else
        {
            setState(State::Ready);
        }
    }

    TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Lazy> TA_ManualChainPipeline::runningGenerator()
    {
        for(auto i = startIndex(); i < m_pActivityList.size(); ++i)
        {
            decltype(auto) pActivity {TA_CommonTools::at<TA_ActivityProxy *>(m_pActivityList, i)};
            (*pActivity)();
            auto var {pActivity->result()};
            TA_CommonTools::replace(m_resultList, i, var);
            TA_Connection::active(this, &TA_ManualChainPipeline::activityCompleted, i, var);
            co_yield var;
        }
        co_return;
    }

    void TA_ManualChainPipeline::setStartIndex(ActivityIndex index)
    {
        TA_BasicPipeline::setStartIndex(index);
    }

    bool TA_ManualChainPipeline::remove(ActivityIndex index)
    {
        std::ignore = index;
        return false;
    }

    void TA_ManualChainPipeline::reset()
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            TA_CommonTools::debugInfo(META_STRING("Reset pipeline failed!"));
            return;
        }
        m_mutex.lock();
        m_resultList.clear();
        m_resultList.resize(m_pActivityList.size());
        m_runningGenerator = runningGenerator();
        m_mutex.unlock();
        setState(State::Waiting);
        setStartIndex(0);
    }

    void TA_ManualChainPipeline::clear()
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
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
        m_runningGenerator = runningGenerator();
        m_mutex.unlock();
        setState(State::Waiting);
        setStartIndex(0);
    }
}

