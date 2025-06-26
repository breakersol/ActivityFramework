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

#include "Components/TA_ManualKeyActivityChainPipeline.h"
#include "Components/TA_CommonTools.h"

namespace CoreAsync {
    TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Lazy> runningGenerator(TA_ManualKeyActivityChainPipeline *pPipeline)
    {
        bool isAtKey {false};
        for(auto i = pPipeline->startIndex(); i < pPipeline->m_pActivityList.size();)
        {
            decltype(auto) pActivity {TA_CommonTools::at<TA_ActivityProxy *>(pPipeline->m_pActivityList, i)};
            (*pActivity)();
            auto var {pActivity->result()};
            TA_CommonTools::replace(pPipeline->m_resultList, i, var);
            TA_Connection::active(pPipeline, &TA_ManualKeyActivityChainPipeline::activityCompleted, i, var);
            co_yield var;
            isAtKey = (i == pPipeline->keyIndex());
            if(!isAtKey)
            {
                i++;
            }
        }
        co_return;
    }

    TA_ManualKeyActivityChainPipeline::TA_ManualKeyActivityChainPipeline() : TA_ManualChainPipeline()
    {
        m_runningGenerator = runningGenerator(this);
    }

    void TA_ManualKeyActivityChainPipeline::setKeyActivity(int index)
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            TA_CommonTools::debugInfo(META_STRING("Set key activity failed!"));
            return;
        }
        m_keyIndex.store(index,std::memory_order_release);
    }

    void TA_ManualKeyActivityChainPipeline::skipKeyActivity()
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            TA_CommonTools::debugInfo(META_STRING("Skip key activity failed!"));
            return;
        }
        m_keyIndex.store(-1,std::memory_order_release);
    }

    void TA_ManualKeyActivityChainPipeline::run()
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

    void TA_ManualKeyActivityChainPipeline::reset()
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
        m_mutex.unlock();
        m_keyIndex.store(-1,std::memory_order_release);
        m_runningGenerator = runningGenerator(this);
        setState(State::Waiting);
        setStartIndex(0);  
    }

    void TA_ManualKeyActivityChainPipeline::clear()
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            TA_CommonTools::debugInfo(META_STRING("Clear pipeline failed!"));
            return;
        }
        m_mutex.lock();
        for(auto pActivity : m_pActivityList)
        {
            if(pActivity)
            {
                delete pActivity;
                pActivity = nullptr;
            }
        }
        m_pActivityList.clear();
        m_resultList.clear();
        m_mutex.unlock();
        m_runningGenerator = runningGenerator(this);
        setState(State::Waiting);
        setStartIndex(0);
        m_keyIndex.store(-1,std::memory_order_release);
    }
}

