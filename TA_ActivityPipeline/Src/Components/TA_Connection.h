/*
 * Copyright [2023] [Shuang Zhu / Sol]
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

#include "TA_ConnectionUtils.h"

namespace CoreAsync {
    class TA_Connection
    {
    public:
        template <TA_ConnectionType type = TA_ConnectionType::Sync, EnableConnectObjectType Sender, typename SenderFunc, EnableConnectObjectType Receiver, typename ReceiverFunc>
        static constexpr bool connect(Sender *pSender, SenderFunc &&sFunc, Receiver *pReceiver, ReceiverFunc &&rFunc)
        {
            if constexpr(!Reflex::TA_MemberTypeTrait<SenderFunc>::noneStaticMemberFuncFlag || !Reflex::TA_MemberTypeTrait<ReceiverFunc>::noneStaticMemberFuncFlag || !IsReturnTypeEqual<void,SenderFunc,std::is_same>::value)
            {
                return false;
            }

            if constexpr(FunctionTypeInfo<SenderFunc>::paraSize != FunctionTypeInfo<ReceiverFunc>::paraSize)
            {
                return false;
            }

            if constexpr(FunctionTypeInfo<SenderFunc>::paraSize != 0 && FunctionTypeInfo<ReceiverFunc>::paraSize != 0)
            {
                if constexpr(!MetaSame<typename FunctionTypeInfo<SenderFunc>::ParaGroup, typename FunctionTypeInfo<ReceiverFunc>::ParaGroup>::value)
                {
                    return false;
                }
            }

            std::string_view senderFuncName {Reflex::TA_TypeInfo<std::decay_t<Sender>>::findName(std::forward<SenderFunc>(sFunc))};
            std::string_view receiverFuncName {Reflex::TA_TypeInfo<std::decay_t<Receiver>>::findName(std::forward<ReceiverFunc>(rFunc))};
            if(senderFuncName.empty() || receiverFuncName.empty() || !pSender || !pReceiver)
                return false;
            return dynamic_cast<TA_MetaObject *>(pSender)->insert(typeid(std::decay_t<Sender>).name(), std::forward<TA_ConnectionUnit>({pSender, std::forward<SenderFunc>(sFunc), pReceiver, std::forward<ReceiverFunc>(rFunc), type}));
        }

        template<EnableConnectObjectType Sender, typename SenderFunc, EnableConnectObjectType Receiver, typename ReceiverFunc>
        static constexpr bool disconnect(Sender *pSender, SenderFunc &&sFunc, Receiver *pReceiver, ReceiverFunc &&rFunc)
        {
            if constexpr(!Reflex::TA_MemberTypeTrait<SenderFunc>::noneStaticMemberFuncFlag || !Reflex::TA_MemberTypeTrait<ReceiverFunc>::noneStaticMemberFuncFlag || !IsReturnTypeEqual<void,SenderFunc,std::is_same>::value)
            {
                return false;
            }
            std::string_view senderFuncName {Reflex::TA_TypeInfo<std::decay_t<Sender> >::findName(std::forward<SenderFunc>(sFunc))};
            std::string_view receiverFuncName {Reflex::TA_TypeInfo<std::decay_t<Receiver> >::findName(std::forward<ReceiverFunc>(rFunc))};
            if(!pSender || !pReceiver || senderFuncName.empty() || receiverFuncName.empty())
                return false;
            return dynamic_cast<TA_MetaObject *>(pSender)->remove(typeid(std::decay_t<Sender>).name(), std::forward<TA_ConnectionUnit>({pSender, std::forward<SenderFunc>(sFunc), pReceiver, std::forward<ReceiverFunc>(rFunc)}));
        }

        template <EnableConnectObjectType Sender, typename SenderFunc, typename ...FuncPara>
        static constexpr bool active(Sender *pSender, SenderFunc &&sFunc, FuncPara &&...para)
        {
            if constexpr(!std::is_convertible_v<std::decay_t<Sender *>, typename FunctionTypeInfo<SenderFunc>::ParentClass *>)
            {
                return false;
            }
            std::printf("Receive the signal: %s\n",Reflex::TA_TypeInfo<Sender>::findName(sFunc));

            auto receiverList = pSender->m_pRegister->findReceivers(pSender, std::forward<SenderFunc>(sFunc));

            if(!receiverList.empty())
            {
                void **pArgs = new void * [sizeof...(para)] {static_cast<void *>(new std::decay_t<FuncPara>(para))...};

                for(auto &rObj : receiverList)
                {
                    CoreAsync::TA_BasicActivity *pActivity = std::get<0>(rObj)->active(std::get<1>(rObj), std::forward<std::string_view>(std::get<2>(rObj)), pArgs);
                    TA_ConnectionResponder::GetIns().response(pActivity, std::get<3>(rObj));
                }
                return true;
            }
            return false;
        }
    };

}

#endif // TA_CONNECTION_H
