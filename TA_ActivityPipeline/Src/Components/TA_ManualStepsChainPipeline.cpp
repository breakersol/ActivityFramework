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

#include "Components/TA_ManualStepsChainPipeline.h"
#include "Components/TA_CommonTools.h"

namespace CoreAsync {
    TA_ManualStepsChainPipeline::TA_ManualStepsChainPipeline() :TA_ManualChainPipeline()
    {

    }

    void TA_ManualStepsChainPipeline::run()
    {
        switch (m_steps.load(std::memory_order_acquire)) {
        case 0:
            break;
        case 1:
            TA_ManualChainPipeline::run();
            break;
        default:
        {      
            if(!m_pActivityList.empty() && m_steps.load(std::memory_order_acquire) <= m_pActivityList.size() && m_currentIndex.load(std::memory_order_acquire) + m_steps.load(std::memory_order_acquire) <= m_pActivityList.size())
            {
                int endIndex = m_currentIndex.load(std::memory_order_acquire) + m_steps.load(std::memory_order_acquire);
                do
                {
                    auto pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pActivityList, m_currentIndex.load(std::memory_order_acquire));
                    if(pActivity)
                    {
                        TA_Variant var = (*pActivity)();
                        TA_CommonTools::replace(m_resultList, m_currentIndex.load(std::memory_order_acquire), var);
                        TA_Connection::active(this, &TA_ManualStepsChainPipeline::activityCompleted, m_currentIndex.load(std::memory_order_acquire), var);
                    }
                    m_currentIndex.fetch_add(1);
                }while(m_currentIndex.load(std::memory_order_acquire) < endIndex);
                if(m_currentIndex.load(std::memory_order_acquire) < m_pActivityList.size())
                {
                    setState(State::Waiting);
                }
                else
                {
                    setState(State::Ready);
                }     
            }
            else
            {
                assert(!m_pActivityList.empty());
                assert(m_steps.load(std::memory_order_acquire) <= m_pActivityList.size());
                assert(m_currentIndex.load(std::memory_order_acquire) + m_steps.load(std::memory_order_acquire) <= m_pActivityList.size());
            }
            break;
        }
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
        m_currentIndex.store(0,std::memory_order_release);
        m_mutex.lock();
        m_resultList.clear();
        m_resultList.resize(m_pActivityList.size());
        m_mutex.unlock();
        m_steps.store(1,std::memory_order_release);
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
        setState(State::Waiting);
        setStartIndex(0);
        m_currentIndex.store(0,std::memory_order_release);
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

