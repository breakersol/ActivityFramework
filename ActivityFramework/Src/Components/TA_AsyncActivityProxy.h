#ifndef TA_ASYNCACTIVITYPROXY_H
#define TA_ASYNCACTIVITYPROXY_H

#include <concepts>

#include "TA_MetaActivity.h"
#include "TA_SingleActivity.h"

namespace CoreAsync
{
template <typename T>
    concept ActivityType = requires(T t) {
        std::decay_t<T>::operator();
        std::decay_t<T>::affinityThread();
        std::decay_t<T>::moveToThread();
        std::decay_t<T>::id();
        !IsTrivalCopyable<T>;
    };

    class TA_AsyncActivityProxy
    {
    public:
        TA_AsyncActivityProxy() = delete;

        template <ActivityType Activity>
        TA_AsyncActivityProxy(Activity *pActivity, bool autoDelete = true)
        {

        }
    };
}

#endif // TA_ASYNCACTIVITYPROXY_H
