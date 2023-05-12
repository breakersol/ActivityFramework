#ifndef ITA_CONNECTION_H
#define ITA_CONNECTION_H

#include "Components/TA_Connection.h"

using ConnectionType = CoreAsync::TA_ConnectionType;

namespace CoreAsync
{
    class ITA_Connection
    {
    public:
        template <ConnectionType = TA_ConnectionType::Sync, class Sender, typename SenderFunc, typename Receiver, typename ReceiverFunc>
        static constexpr bool connect(Sender *&pSender, SenderFunc &&sFunc, Receiver *&pReceiver, ReceiverFunc &&rFunc)
        {
            return TA_Connection::connect(pSender, std::forward<SenderFunc>(sFunc), pReceiver, std::forward<ReceiverFunc>(rFunc));
        }

        template <class Sender, typename SenderFunc, class Receiver, typename ReceiverFunc>
        static constexpr bool disconnect(Sender *&pSender, SenderFunc &&sFunc, Receiver *&pReceiver, ReceiverFunc &&rFunc)
        {
            return TA_Connection::disconnect(pSender, std::forward<SenderFunc>(sFunc), pReceiver, std::forward<ReceiverFunc>(rFunc));
        }

        template <class Sender, typename SenderFunc, typename ...FuncPara>
        static constexpr bool active(Sender *&pSender, SenderFunc &&sFunc, FuncPara &&...para)
        {
            return TA_Connection::active(pSender, std::forward<SenderFunc>(sFunc), std::forward<FuncPara>(para)...);
        }

    };
}

#endif // ITA_CONNECTION_H
