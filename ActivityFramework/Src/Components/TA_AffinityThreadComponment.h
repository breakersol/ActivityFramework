#ifndef TA_AFFINITYTHREADCOMPONMENT_H
#define TA_AFFINITYTHREADCOMPONMENT_H

#include <atomic>
#include <thread>

#include "TA_ThreadPool.h"

namespace CoreAsync
{
    class TA_AffinityThreadIdx
    {
    public:
        explicit TA_AffinityThreadIdx(std::size_t affinityThread = TA_ThreadHolder::get().topPriorityThread()) : m_sourceThread(std::this_thread::get_id()), m_affinityThread(affinityThread)
        {

        }

        ~TA_AffinityThreadIdx() {}

        TA_AffinityThreadIdx(const TA_AffinityThreadIdx &another) : m_sourceThread(another.m_sourceThread), m_affinityThread(another.affinityThread())
        {

        }

        TA_AffinityThreadIdx(TA_AffinityThreadIdx &&another) : m_sourceThread(std::move(another.m_sourceThread)), m_affinityThread(std::move(another.affinityThread()))
        {

        }

        TA_AffinityThreadIdx & operator = (const TA_AffinityThreadIdx &another)
        {
            m_affinityThread.store(another.affinityThread(), std::memory_order_release);
            return *this;
        }

        TA_AffinityThreadIdx & operator = (TA_AffinityThreadIdx &&another)
        {
            m_affinityThread.store(std::move(another.affinityThread()), std::memory_order_release);
            return *this;
        }

        const std::thread::id sourceThread() const
        {
            return m_sourceThread;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.load(std::memory_order_acquire);
        }

        bool moveToThread(std::size_t thread)
        {
            if(thread >= TA_ThreadHolder::get().size())
                return false;
            m_affinityThread.store(thread, std::memory_order_release);
            return true;
        }

    private:
        const std::thread::id m_sourceThread;
        std::atomic_size_t m_affinityThread;
    };
}

#endif // TA_AFFINITYTHREADCOMPONMENT_H
