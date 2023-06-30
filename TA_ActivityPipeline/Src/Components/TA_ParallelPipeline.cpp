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

#include "Components/TA_ParallelPipeline.h"
#include "Components/TA_CommonTools.h"
#include "Components/TA_ThreadPool.h"

#include <future>

namespace CoreAsync {
    TA_ParallelPipeline::TA_ParallelPipeline() : TA_BasicPipeline(), m_waitingCount(0)
    {
        TA_Connection::connect(&TA_ThreadHolder::get(), &TA_ThreadPool::taskCompleted, this, &TA_ParallelPipeline::taskCompleted);
    }

    void TA_ParallelPipeline::run()
    {
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
        }
    }

    void TA_ParallelPipeline::taskCompleted(std::size_t id, TA_Variant var)
    {  
        for(std::size_t idx = 0;idx < m_activityIds.size();++idx)
        {
            if(m_activityIds[idx] == id)
            {
                TA_CommonTools::replace(m_resultList, idx, std::forward<TA_Variant>(var));
                TA_Connection::active(this, &TA_ParallelPipeline::activityCompleted, idx, var);
                break;
            }
        }
        if(--m_waitingCount == 0)
        {
            setState(State::Ready);
        }
    }
}
