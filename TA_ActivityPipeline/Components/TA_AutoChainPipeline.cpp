#include "TA_AutoChainPipeline.h"
#include "TA_CommonTools.h"

#include <future>

namespace CoreAsync {
    TA_AutoChainPipeline::TA_AutoChainPipeline() : TA_BasicPipeline()
    {

    }

    void TA_AutoChainPipeline::run()
    {
        auto exFunc = [&](){
            for(int i = startIndex();i < m_pActivityList.size();++i)
            {
                TA_Variant var = TA_CommonTools::at<TA_BasicActivity *>(m_pActivityList, i)->execute();
                TA_CommonTools::replace(m_resultList, i, var);
                TA_Connection::active(this, &TA_AutoChainPipeline::activityCompleted, i);
            }
        };
//        std::future<void> ft = std::async(std::launch::async,exFunc);
//        ft.get();
        exFunc();
        setState(State::Ready);
    }
}
