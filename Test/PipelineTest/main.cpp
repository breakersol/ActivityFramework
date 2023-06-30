#include "gtest/gtest.h"
#include "MetaTest.h"
#include "Components/TA_ThreadPool.h"

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc,argv);

    return RUN_ALL_TESTS();

//    static CoreAsync::TA_ThreadPool theadPool;
//    std::function<void()> func = []() {std::printf("123456");};
//    auto activity = CoreAsync::ITA_ActivityCreator::create(func);
//    theadPool.postActivity(activity);

    return 0;
}
