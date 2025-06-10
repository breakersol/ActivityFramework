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

#ifndef ITA_CONNECTION_H
#define ITA_CONNECTION_H

#include "Components/TA_Connection.h"

using ConnectionType = CoreAsync::TA_ConnectionType;
using ConnectionHolder = CoreAsync::TA_MetaObject::TA_ConnectionObjectHolder;

namespace CoreAsync
{
    class ITA_Connection
    {
    public:
        template <ConnectionType ct = TA_ConnectionType::Auto, class Sender, typename SenderFunc, typename Receiver, typename ReceiverFunc>
        static constexpr bool connect(Sender *pSender, SenderFunc &&sFunc, Receiver *pReceiver, ReceiverFunc &&rFunc)
        {
            return TA_Connection::connect<ct>(pSender, std::forward<SenderFunc>(sFunc), pReceiver, std::forward<ReceiverFunc>(rFunc));
        }

        template <ConnectionType type = TA_ConnectionType::Auto, class Sender, typename SenderFunc, typename LambdaExp>
        static constexpr auto connect(Sender *pSender, SenderFunc &&sFunc, LambdaExp &&lExp)
        {
            return TA_Connection::connect<type>(pSender, std::forward<SenderFunc>(sFunc), std::forward<LambdaExp>(lExp));
        }

        template <class Sender, typename SenderFunc, class Receiver, typename ReceiverFunc>
        static constexpr bool disconnect(Sender *pSender, SenderFunc &&sFunc, Receiver *pReceiver, ReceiverFunc &&rFunc)
        {
            return TA_Connection::disconnect(pSender, std::forward<SenderFunc>(sFunc), pReceiver, std::forward<ReceiverFunc>(rFunc));
        }

        static bool disconnect(ConnectionHolder &pConnection)
        {
            return TA_Connection::disconnect(pConnection);
        };

        template <class Sender, typename SenderFunc, typename ...FuncPara>
        static constexpr bool active(Sender *pSender, SenderFunc &&sFunc, FuncPara &&...para)
        {
            return TA_Connection::active(pSender, std::forward<SenderFunc>(sFunc), std::forward<FuncPara>(para)...);
        }

    };
}

#endif // ITA_CONNECTION_H
