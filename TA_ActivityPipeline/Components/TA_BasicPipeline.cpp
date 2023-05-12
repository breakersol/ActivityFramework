#include "Components/TA_BasicPipeline.h"
#include "Components/TA_CommonTools.h"

#include <chrono>
#include <cassert>

namespace CoreAsync {
    bool TA_BasicPipeline::remove(ActivityIndex index)
    {
        if(State::Waiting != m_state.load(std::memory_order_consume))
        {
            assert(State::Waiting == m_state.load(std::memory_order_consume));
            std::printf("Remove activity failed!");
            return false;
        }
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if(index < m_pActivityList.size())
        {
            TA_CommonTools::remove<TA_BasicActivity *>(m_pActivityList, index);
            return true;
        }
        return false;
    }

    void TA_BasicPipeline::clear()
    {
        if(State::Busy == m_state.load(std::memory_order_consume))
        {
            assert(State::Busy != m_state.load(std::memory_order_consume));
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
        m_startIndex.store(0,std::memory_order_release);
        setState(State::Waiting);
    }

    void TA_BasicPipeline::reset()
    {
        if(State::Ready != m_state.load(std::memory_order_consume))
        {
            assert(State::Ready == m_state.load(std::memory_order_consume));
            std::printf("Reset pipeline failed!");
            return;
        }
        m_mutex.lock();
        m_resultList.clear();
        m_resultList.resize(m_pActivityList.size());
        m_mutex.unlock();
        m_startIndex.store(0,std::memory_order_release);
        setState(State::Waiting);
    }

    void TA_BasicPipeline::execute(ExecuteType type)
    {
        if(State::Waiting != m_state.load(std::memory_order_consume))
        {
            assert(State::Waiting == m_state.load(std::memory_order_consume));
            std::printf("Execute pipeline failed!");
            return;
        }
        setState(State::Busy);
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if(type == ExecuteType::Async)
        {
            auto ft = std::async(std::launch::async,[this]()->void{this->run();});
            std::swap(m_ft,ft);
        }
        else
        {
            this->run();
        }
    }

    void TA_BasicPipeline::setState(State state)
    {
        m_state.store(state,std::memory_order_release);
        TA_Connection::active(this, &TA_BasicPipeline::stateChanged, m_state.load(std::memory_order_consume));
    }

    TA_BasicPipeline::State TA_BasicPipeline::state() const
    {
        return m_state.load(std::memory_order_consume);
    }

    std::size_t TA_BasicPipeline::activitySize() const
    {
        return m_pActivityList.size();
    }

    bool TA_BasicPipeline::waitingComplete()
    {
        std::chrono::steady_clock::time_point asyncStartTime;
        constexpr int interval = 100;
        while (State::Ready != m_state.load(std::memory_order_consume))
        {
            asyncStartTime = std::chrono::steady_clock::now();
            while (std::chrono::duration_cast<Milliseconds>(std::chrono::steady_clock::now() - asyncStartTime).count() < interval){}
        }

        return true;
    }

    void TA_BasicPipeline::setStartIndex(unsigned int index)
    {
        if(State::Waiting != m_state.load(std::memory_order_consume))
        {
            assert(State::Waiting == m_state.load(std::memory_order_consume));
            std::printf("Set start index failed!");
            return;
        }
        m_startIndex.store(index,std::memory_order_release);
    }

    bool TA_BasicPipeline::switchActivityBranch(int activityIndex, std::deque<unsigned int> branches)
    {
        if(State::Waiting != m_state.load(std::memory_order_consume) || activityIndex >= m_pActivityList.size())
            return false;
        auto pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pActivityList, activityIndex);
        if(!pActivity)
            return false;
        return pActivity->selectBranch(branches);
    }

    unsigned int TA_BasicPipeline::startIndex() const
    {
        return m_startIndex.load(std::memory_order_acquire);
    }
}
