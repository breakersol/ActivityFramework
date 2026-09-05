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

#ifndef TA_CONNECTION_H
#define TA_CONNECTION_H

#include "TA_MetaObject.h"

namespace CoreAsync {

class TA_Connection {
  public:
    template <TA_ConnectionType type = TA_ConnectionType::Auto, EnableConnectObjectType Sender, typename SenderFunc,
              EnableConnectObjectType Receiver, typename ReceiverFunc>
    static bool connect(Sender *pSender, SenderFunc &&sFunc, Receiver *pReceiver, ReceiverFunc &&rFunc) {
        return TA_MetaObject::registerConnection(pSender, std::forward<SenderFunc>(sFunc), pReceiver,
                                                 std::forward<ReceiverFunc>(rFunc), type);
    }

    template <auto Signal, auto Slot, TA_ConnectionType type = TA_ConnectionType::Auto,
              EnableConnectObjectType Sender, EnableConnectObjectType Receiver>
    static bool connect(Sender *pSender, Receiver *pReceiver) {
        return TA_MetaObject::registerConnection<Signal, Slot>(pSender, pReceiver, type);
    }

    template <TA_ConnectionType type = TA_ConnectionType::Auto, EnableConnectObjectType Sender, typename SenderFunc,
              LambdaExpType LambdaExp>
    static auto connect(Sender *pSender, SenderFunc &&sFunc, LambdaExp &&lExp, bool autoDestroy = false) {
        return TA_MetaObject::registerConnection(pSender, std::forward<SenderFunc>(sFunc),
                                                 std::forward<LambdaExp>(lExp), type, autoDestroy);
    }

    template <auto Signal, TA_ConnectionType type = TA_ConnectionType::Auto,
              EnableConnectObjectType Sender, LambdaExpType LambdaExp>
    static auto connect(Sender *pSender, LambdaExp &&lExp, bool autoDestroy = false) {
        return TA_MetaObject::registerConnection<Signal>(pSender, std::forward<LambdaExp>(lExp), type, autoDestroy);
    }

    template <EnableConnectObjectType Sender, typename SenderFunc, EnableConnectObjectType Receiver,
              typename ReceiverFunc>
    static bool disconnect(Sender *pSender, SenderFunc &&sFunc, Receiver *pReceiver, ReceiverFunc &&rFunc) {
        return TA_MetaObject::unregisterConnection(pSender, std::forward<SenderFunc>(sFunc), pReceiver,
                                                   std::forward<ReceiverFunc>(rFunc));
    }

    template <auto Signal, auto Slot, EnableConnectObjectType Sender, EnableConnectObjectType Receiver>
    static bool disconnect(Sender *pSender, Receiver *pReceiver) {
        return TA_MetaObject::unregisterConnection<Signal, Slot>(pSender, pReceiver);
    }

    static bool disconnect(TA_MetaObject::TA_ConnectionObjectHolder &pConnection) {
        return TA_MetaObject::unregisterConnection(pConnection);
    };

    template <EnableConnectObjectType Sender, typename SenderFunc, typename... FuncPara>
    static bool active(Sender *pSender, SenderFunc &&sFunc, FuncPara &&...para) {
        return TA_MetaObject::emitSignal(pSender, std::forward<SenderFunc>(sFunc), std::forward<FuncPara>(para)...);
    }

    template <auto Signal, EnableConnectObjectType Sender, typename... FuncPara>
    static bool active(Sender *pSender, FuncPara &&...para) {
        return TA_MetaObject::emitSignal<Signal>(pSender, std::forward<FuncPara>(para)...);
    }
};
} // namespace CoreAsync

#endif // TA_CONNECTION_H
