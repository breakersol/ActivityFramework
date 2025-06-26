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
    TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Lazy> runningGenerator(TA_ManualStepsChainPipeline *pPipeline)
    {
        auto step {pPipeline->steps()};
        if(step <= pPipeline->m_pActivityList.size())
        {
            for(auto i = pPipeline->startIndex(); i < pPipeline->m_pActivityList.size(); ++i)
            {
                decltype(auto) pActivity {TA_CommonTools::at<TA_ActivityProxy *>(pPipeline->m_pActivityList, i)};
                (*pActivity)();
                auto var {pActivity->result()};
                TA_CommonTools::replace(pPipeline->m_resultList, i, var);
                TA_Connection::active(pPipeline, &TA_ManualStepsChainPipeline::activityCompleted, i, var);
                if(--step == 0)
                {
                    co_yield var;
                    step = pPipeline->steps();
                    if(i + step >= pPipeline->m_pActivityList.size())
                    {
                        break;
                    }
                }
            }
        }
        co_return;
    }

    TA_ManualStepsChainPipeline::TA_ManualStepsChainPipeline() :TA_ManualChainPipeline()
    {
        m_runningGenerator = runningGenerator(this);
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
        m_runningGenerator = runningGenerator(this);
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
        m_runningGenerator = runningGenerator(this);
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

