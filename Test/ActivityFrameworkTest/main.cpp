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

#include "gtest/gtest.h"
#include "MetaTest.h"
#include "Components/TA_Activity.h"
#include "ITA_PipelineCreator.h"
#include "Components/TA_BasicPipeline.h"
#include "Components/TA_MetaReflex.h"
#include "Components/TA_Coroutine.h"

CoreAsync::TA_CoroutineTask<int, CoreAsync::Eager> testCoroutineTask(CoroutineTestSender *pSender)
{
    auto val = co_await CoreAsync::TA_SignalAwaitable(pSender, &CoroutineTestSender::sendSignal);
    co_return val * 3;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc,argv);

    return RUN_ALL_TESTS();
    
  //   std::function<void()> testFunction = []() {
  //       std::cout << "Test function executed!" << std::endl;
        // };

  //   auto testActivity = CoreAsync::TA_ActivityCreator::create(std::move(testFunction));
  //   testActivity->operator()();
  //   std::cout << "Test completed!" << std::endl;

 //   CoroutineTestSender sender;

 //   auto task_1 = testCoroutineTask(&sender);
 //   CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 3);
 //   auto r1 = task_1.get();
 //   EXPECT_EQ(r1, 9);

 //   auto task_2 = testCoroutineTask(&sender);
 //   CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 4);
 //   auto r2 = task_2.get();
 //   EXPECT_EQ(r2, 12);

	//std::cout << "Test completed!" << std::endl;

    //return 0;
}
