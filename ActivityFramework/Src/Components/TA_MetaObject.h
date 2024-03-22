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

#ifndef TA_METAOBJECT_H
#define TA_METAOBJECT_H

#include "TA_ActivityFramework_global.h"

#include <string_view>

namespace CoreAsync
{
    class TA_ConnectionUnit;
    class TA_ConnectionsRegister;
    class TA_ConnectionsRecorder;

    class ACTIVITY_FRAMEWORK_EXPORT TA_MetaObject
    {
        friend class TA_Connection;
        friend class TA_ConnectionsRegister;
        friend class TA_ConnectionsRecorder;
    public:
        TA_MetaObject();
        virtual ~TA_MetaObject();

    private:
        bool registConnection(std::string_view &&object, TA_ConnectionUnit &&unit);
        bool removeConnection(std::string_view &&object, TA_ConnectionUnit &&unit);

        bool recordSender(TA_MetaObject *pSender);
        bool removeSender(TA_MetaObject *pSender);

    private:
        TA_ConnectionsRegister *m_pRegister;
        TA_ConnectionsRecorder *m_pRecorder;

    };
}

#endif // TA_METAOBJECT_H
