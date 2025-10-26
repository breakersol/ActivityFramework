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

#include "Components/TA_ConcurrentPipeline.h"
#include "Components/TA_CommonTools.h"

namespace CoreAsync {
TA_CoroutineGenerator<TA_DefaultVariant, CoreAsync::Eager> runningGenerator(TA_ConcurrentPipeline *pPipeline) {
    std::vector<TA_ActivityResultFetcher> resultFetchers(pPipeline->m_pActivityList.size());
    for (auto i = pPipeline->startIndex(); i < pPipeline->m_pActivityList.size(); ++i) {
        decltype(auto) pActivity{TA_CommonTools::at<TA_ActivityProxy *>(pPipeline->m_pActivityList, i)};
        std::shared_ptr<TA_ActivityExecutingAwaitable> executingAwaitable =
            std::make_shared<TA_ActivityExecutingAwaitable>(pActivity, TA_ActivityExecutingAwaitable::ExecuteType::Async);
        resultFetchers[i] = co_await *executingAwaitable;
    }
    for (std::size_t idx = 0; idx < resultFetchers.size(); ++idx) {
        pPipeline->m_resultList[idx] = resultFetchers[idx]();
        TA_Connection::active(pPipeline, &TA_ConcurrentPipeline::activityCompleted, idx,
                              std::forward<TA_DefaultVariant>(pPipeline->m_resultList[idx]));
    }
    pPipeline->m_pActivityList.clear();
    pPipeline->setState(TA_BasicPipeline::State::Ready);
    co_return;
}

TA_ConcurrentPipeline::TA_ConcurrentPipeline() : TA_BasicPipeline() {}

void TA_ConcurrentPipeline::run() {
    auto generator{runningGenerator(this)};
}

void TA_ConcurrentPipeline::clear() {
    if (State::Busy == state()) {
        assert(State::Busy != state());
        TA_CommonTools::debugInfo(META_STRING("Clear pipeline failed!"));
        return;
    }
    m_mutex.lock();
    for (auto &pActivity : m_pActivityList) {
        if (pActivity) {
            delete pActivity;
            pActivity = nullptr;
        }
    }
    m_pActivityList.clear();
    m_resultList.clear();
    m_mutex.unlock();
    setState(State::Waiting);
}

void TA_ConcurrentPipeline::reset() {
    if (State::Ready != state()) {
        assert(State::Ready == state());
        TA_CommonTools::debugInfo(META_STRING("Reset pipeline failed!"));
        return;
    }
    m_mutex.lock();
    m_resultList.clear();
    m_resultList.resize(m_pActivityList.size());
    m_mutex.unlock();
    setState(State::Waiting);
}
} // namespace CoreAsync
