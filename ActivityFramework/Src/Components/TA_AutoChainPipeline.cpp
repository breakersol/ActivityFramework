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

#include "TA_AutoChainPipeline.h"

namespace CoreAsync {
    TA_AutoChainPipeline::TA_AutoChainPipeline() : TA_BasicPipeline()
    {

    }

    void TA_AutoChainPipeline::run()
    {
        auto generator {this->runningGenerator()};
        while(generator.next());
    }

    TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Eager> TA_AutoChainPipeline::runningGenerator()
    {
        for(auto i = startIndex(); i < m_pActivityList.size(); ++i)
        {
            decltype(auto) pActivity {TA_CommonTools::at<TA_ActivityProxy *>(m_pActivityList, i)};
            (*pActivity)();
            auto var {pActivity->result()};
            TA_CommonTools::replace(m_resultList, i, var);
            co_yield var;
        }
        setState(State::Ready);
        co_return;
    }
}
