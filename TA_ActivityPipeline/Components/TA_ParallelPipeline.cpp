#include "Components/TA_ParallelPipeline.h"
#include "Components/TA_CommonTools.h"

#include <future>

namespace CoreAsync {
    TA_ParallelPipeline::TA_ParallelPipeline() : TA_BasicPipeline()
    {

    }

    void TA_ParallelPipeline::run()
    {
        unsigned int sIndex(std::move(startIndex()));
        std::size_t activitySize = m_pActivityList.size();
        if(activitySize > 0)
        {
            std::future<TA_Variant> *pFArray = nullptr;
            if(!m_pActivityList.empty())
            {
                pFArray = new std::future<TA_Variant> [activitySize];
            }
            for(std::size_t i = sIndex;i < activitySize;++i)
            {
                auto pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pActivityList, i);
                pFArray[i] = std::async(std::launch::async,[&,pActivity]()->TA_Variant{return pActivity->execute();});
            }
            for(int index = sIndex;index < activitySize;++index)
            {
                TA_Variant var = pFArray[index].get();
                TA_CommonTools::replace(m_resultList, index, std::forward<TA_Variant>(var));
                TA_Connection::active(this, &TA_ParallelPipeline::activityCompleted, index);;
            }
            delete [] pFArray;
            setState(State::Ready);
        }
    }
}
