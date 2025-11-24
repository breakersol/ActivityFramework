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

#include "TA_ThreadPool.h"

namespace CoreAsync {
#if defined(__ANDROID__)
void TA_ThreadPool::shutDown() {
    for (std::size_t idx = 0; idx < m_threads.size(); ++idx) {
        m_states[idx].stopRequested.store(true, std::memory_order_release);
        m_states[idx].resource.release();
        m_states[idx].isBusy.store(false, std::memory_order_release);
    }
    for (auto &thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_threads.clear();
}

void  TA_ThreadPool::init() {
    for (std::size_t idx = 0; idx < m_states.size(); ++idx) {
        m_stealIdxs[idx] = (idx + 1) % m_stealIdxs.size();
        m_threads.emplace_back([&, idx]() {
            std::shared_ptr<TA_ActivityProxy> pActivity{nullptr};
            HandleType *handle{nullptr};
            while(!m_states[idx].stopRequested.load(std::memory_order_acquire)) {
                m_states[idx].resource.acquire();
                m_states[idx].isBusy.store(true, std::memory_order_release);
                while (!m_activityQueues[idx].isEmpty()) {
                    if (m_activityQueues[idx].pop(handle)) {
                        pActivity = HandleType::extractActivity(handle);
                        if (pActivity) {
                            (*pActivity)();
                        }
                    }
                }
                if (trySteal(pActivity, idx) && pActivity) {
                    (*pActivity)();
                }
                m_states[idx].isBusy.store(false, std::memory_order_release);
            }
            TA_CommonTools::debugInfo(META_STRING("Shut down successuflly!\n"));
        });
        m_threads.back().join();
    }
}

bool TA_ThreadPool::trySteal(std::shared_ptr<TA_ActivityProxy> &stolenActivity, std::size_t excludedIdx) {
    std::size_t startIdx{m_stealIdxs[excludedIdx]};
    std::size_t idx{(startIdx + 1) % m_threads.size()};
    while (idx != startIdx) {
        if (idx != excludedIdx) {
            HandleType *previewHandle{nullptr};
            if (m_activityQueues[idx].top(previewHandle) && previewHandle && previewHandle->stolenEnabled()) {
                HandleType *handle{nullptr};
                if (m_activityQueues[idx].pop(handle)) {
                    stolenActivity = HandleType::extractActivity(handle);
                    if (stolenActivity) {
                        return true;
                    }
                }
            }
        }
        idx = (idx + 1) % m_threads.size();
    }
    stolenActivity.reset();
    return false;
}
#else
void TA_ThreadPool::shutDown() {
    for (std::size_t idx = 0; idx < m_threads.size(); ++idx) {
        m_threads[idx].request_stop();
        m_states[idx].resource.release();
        m_states[idx].isBusy.store(false, std::memory_order_release);
    }
    m_threads.clear();
}

void  TA_ThreadPool::init() {
    for (std::size_t idx = 0; idx < m_states.size(); ++idx) {
        m_stealIdxs[idx] = (idx + 1) % m_stealIdxs.size();
        m_threads.emplace_back([&, idx](const std::stop_token &st) {
            std::shared_ptr<TA_ActivityProxy> pActivity{nullptr};
            while (!st.stop_requested()) {
                m_states[idx].resource.acquire();
                m_states[idx].isBusy.store(true, std::memory_order_release);
                while (!m_activityQueues[idx].isEmpty()) {
                    if (m_activityQueues[idx].pop(pActivity) && pActivity) {
                        (*pActivity)();
                    }
                }
                if (trySteal(pActivity, idx) && pActivity) {
                    (*pActivity)();
                }
                m_states[idx].isBusy.store(false, std::memory_order_release);
            }
            TA_CommonTools::debugInfo(META_STRING("Shut down successuflly!\n"));
        });
    }
}

bool TA_ThreadPool::trySteal(std::shared_ptr<TA_ActivityProxy> &stolenActivity, std::size_t excludedIdx) {
    std::size_t startIdx{m_stealIdxs[excludedIdx]};
    std::size_t idx{(startIdx + 1) % m_threads.size()};
    while (idx != startIdx) {
        if (idx != excludedIdx) {
            std::shared_ptr<TA_ActivityProxy> activity{};
            if (m_activityQueues[idx].top(activity) && activity && activity->stolenEnabled() &&
                m_activityQueues[idx].pop(stolenActivity)) {
                return true;
            }
        }
        idx = (idx + 1) % m_threads.size();
    }
    stolenActivity = nullptr;
    return false;
}
#endif

}
