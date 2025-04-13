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

#include "TA_CoroutineTest.h"

TA_CoroutineTest::TA_CoroutineTest()
{

}

TA_CoroutineTest::~TA_CoroutineTest()
{

}

void TA_CoroutineTest::SetUp()
{

}

void TA_CoroutineTest::TearDown()
{

}

TEST_F(TA_CoroutineTest, testCoroutineTask)
{
    CoroutineTestSender sender;

    auto task_1 = testCoroutineTask(&sender);
    CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 3);
    auto r1 = task_1.get();
    EXPECT_EQ(r1, 9);

    auto task_2 = testCoroutineTask(&sender);
    CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 4);
    auto r2 = task_2.get();
    EXPECT_EQ(r2, 12);
}

TEST_F(TA_CoroutineTest, testCoroutineGenerator)
{
    CoroutineTestSender sender;
    auto gen = testCoroutineGenerator(&sender);

    int r1, r2, r3;
    CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 3);
    if(gen.next())
        r1 = gen.value();
    CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 4);
    if (gen.next())
        r2 = gen.value();
    CoreAsync::TA_Connection::active(&sender, &CoroutineTestSender::sendSignal, 5);
    if (gen.next())
        r3 = gen.value();
    EXPECT_EQ(r1, 9);
    EXPECT_EQ(r2, 12);
    EXPECT_EQ(r3, 15);
}
