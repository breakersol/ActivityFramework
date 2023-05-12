#ifndef TA_METAOBJECT_H
#define TA_METAOBJECT_H

#include "TA_ActivityPipeline_global.h"

#include <string_view>

namespace CoreAsync
{
    class TA_ConnectionsRegister;
    class TA_ConnectionUnit;

    class ASYNC_PIPELINE_EXPORT TA_MetaObject
    {
        friend class TA_Connection;
    public:
        TA_MetaObject();
        virtual ~TA_MetaObject();

    private:
        bool insert(std::string_view &&object, TA_ConnectionUnit &&unit);
        bool remove(std::string_view &&object, TA_ConnectionUnit &&unit);
        void clear();

    private:
        TA_ConnectionsRegister *m_pRegister;

    };
}

#endif // TA_METAOBJECT_H
