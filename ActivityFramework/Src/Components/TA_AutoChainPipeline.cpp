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
TA_AutoChainPipeline::TA_AutoChainPipeline() : TA_BasicPipeline() {}

TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Eager> runningGenerator(TA_AutoChainPipeline *pPipeline) {
    for (auto i = pPipeline->startIndex(); i < pPipeline->m_pActivityList.size(); ++i) {
        decltype(auto) pActivity{TA_CommonTools::at<TA_ActivityProxy *>(pPipeline->m_pActivityList, i)};
        (*pActivity)();
        auto var{pActivity->result()};
        TA_CommonTools::replace(pPipeline->m_resultList, i, var);
        co_yield var;
    }
    pPipeline->setState(TA_BasicPipeline::State::Ready);
    co_return;
}

void TA_AutoChainPipeline::run() {
    auto generator{runningGenerator(this)};
    while (generator.next());
}
} // namespace CoreAsync
