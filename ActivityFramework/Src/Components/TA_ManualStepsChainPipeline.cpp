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

#include "Components/TA_ManualStepsChainPipeline.h"
#include "Components/TA_CommonTools.h"

namespace CoreAsync {
    TA_ManualStepsChainPipeline::TA_ManualStepsChainPipeline() :TA_ManualChainPipeline()
    {
        m_runningGenerator = runningGenerator();
    }

    void TA_ManualStepsChainPipeline::run()
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

    TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Lazy> TA_ManualStepsChainPipeline::runningGenerator()
    {
        auto step {m_steps.load(std::memory_order_acquire)};
        if(step <= m_pActivityList.size())
        {
            for(auto i = startIndex(); i < m_pActivityList.size(); ++i)
            {
                decltype(auto) pActivity {TA_CommonTools::at<TA_ActivityProxy *>(m_pActivityList, i)};
                (*pActivity)();
                auto var {pActivity->result()};
                TA_CommonTools::replace(m_resultList, i, var);
                TA_Connection::active(this, &TA_ManualStepsChainPipeline::activityCompleted, i, var);
                if(--step == 0)
                {
                    co_yield var;
                    step = m_steps.load(std::memory_order_acquire);
                    if(i + step >= m_pActivityList.size())
                    {
                        break;
                    }
                }
            }
        }
        co_return;
    }

    void TA_ManualStepsChainPipeline::reset()
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            std::printf("Reset pipeline failed!");
            return;
        }
        m_mutex.lock();
        m_resultList.clear();
        m_resultList.resize(m_pActivityList.size());
        m_mutex.unlock();
        m_steps.store(1,std::memory_order_release);
        m_runningGenerator = runningGenerator();
        setState(State::Waiting);
        setStartIndex(0); 
    }

    void TA_ManualStepsChainPipeline::clear()
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            std::printf("Clear pipeline failed!");
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
        m_mutex.unlock();
        m_steps.store(1,std::memory_order_release);
        m_runningGenerator = runningGenerator();
        setState(State::Waiting);
        setStartIndex(0);
    }

    void TA_ManualStepsChainPipeline::setSteps(unsigned int steps)
    {
        if(state() == State::Busy)
        {
            assert(state() != State::Busy);
            std::printf("Set steps failed!");
            return;
        }
        m_steps.store(steps,std::memory_order_release);
    }
}

