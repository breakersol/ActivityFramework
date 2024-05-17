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

#include "TA_AutoChainPipeline.h"
#include "TA_CommonTools.h"

#include <future>

namespace CoreAsync {
    TA_AutoChainPipeline::TA_AutoChainPipeline() : TA_BasicPipeline()
    {

    }

    void TA_AutoChainPipeline::run()
    {
        auto exFunc = [&](){
            for(int i = startIndex();i < m_pActivityList.size();++i)
            {
                TA_Variant var = (*TA_CommonTools::at<TA_BasicActivity *>(m_pActivityList, i))();
                TA_CommonTools::replace(m_resultList, i, var);
                TA_Connection::active(this, &TA_AutoChainPipeline::activityCompleted, i, var);
            }
        };
//        std::future<void> ft = std::async(std::launch::async,exFunc);
//        ft.get();
        exFunc();
        setState(State::Ready);
    }
}
