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

#include "Components/TA_BasicPipeline.h"
#include "Components/TA_CommonTools.h"

#include <cassert>

namespace CoreAsync {
bool TA_BasicPipeline::remove(ActivityIndex index) {
    if (State::Waiting != m_state.load(std::memory_order_consume)) {
        assert(State::Waiting == m_state.load(std::memory_order_consume));
        TA_CommonTools::debugInfo(META_STRING("Remove activity failed!"));
        return false;
    }
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (index < m_pActivityList.size()) {
        TA_CommonTools::remove<TA_ActivityProxy *>(m_pActivityList, index);
        return true;
    }
    return false;
}

void TA_BasicPipeline::clear() {
    if (State::Busy == m_state.load(std::memory_order_consume)) {
        assert(State::Busy != m_state.load(std::memory_order_consume));
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
    m_startIndex.store(0, std::memory_order_release);
    setState(State::Waiting);
}

void TA_BasicPipeline::destroy() {
    m_mutex.lock();
    for (auto &pActivity : m_pActivityList) {
        if (pActivity) {
            delete pActivity;
            pActivity = nullptr;
        }
    }
    m_pActivityList.clear();
    m_resultList.clear();
    if (m_pRunningActivity) {
        delete m_pRunningActivity;
        m_pRunningActivity = nullptr;
    }
    m_mutex.unlock();
}

void TA_BasicPipeline::reset() {
    if (State::Ready != m_state.load(std::memory_order_consume)) {
        assert(State::Ready == m_state.load(std::memory_order_consume));
        TA_CommonTools::debugInfo(META_STRING("Reset pipeline failed!"));
        return;
    }
    m_mutex.lock();
    m_resultList.clear();
    m_resultList.resize(m_pActivityList.size());
    m_mutex.unlock();
    m_startIndex.store(0, std::memory_order_release);
    setState(State::Waiting);
}

void TA_BasicPipeline::setState(State state) {
    m_state.store(state, std::memory_order_release);
    TA_Connection::active(this, &TA_BasicPipeline::stateChanged, m_state.load(std::memory_order_consume));
}

TA_BasicPipeline::State TA_BasicPipeline::state() const { return m_state.load(std::memory_order_consume); }

std::size_t TA_BasicPipeline::activitySize() const { return m_pActivityList.size(); }

void TA_BasicPipeline::setStartIndex(ActivityIndex index) {
    if (State::Waiting != m_state.load(std::memory_order_consume)) {
        assert(State::Waiting == m_state.load(std::memory_order_consume));
        TA_CommonTools::debugInfo(META_STRING("Set start index failed!"));
        return;
    }
    m_startIndex.store(index, std::memory_order_release);
}

TA_BasicPipeline::ActivityIndex TA_BasicPipeline::startIndex() const {
    return m_startIndex.load(std::memory_order_acquire);
}
} // namespace CoreAsync
