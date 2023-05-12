#ifndef TA_BASICACTIVITY_H
#define TA_BASICACTIVITY_H

#include <deque>

#include "TA_Variant.h"

namespace CoreAsync {
    class TA_BasicActivity
    {
    public:
        virtual ~TA_BasicActivity() = default;
        virtual TA_Variant execute() = 0;

        virtual bool hasBranch() const = 0;

        virtual bool selectBranch(std::deque<unsigned int> branches) = 0;

        virtual TA_Variant caller() const = 0;

    };
}

#endif // TA_BASICACTIVITY_H
