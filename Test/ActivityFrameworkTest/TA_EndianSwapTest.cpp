/*
 * Copyright [2024] [Shuang Zhu / Sol]
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

#include "TA_EndianSwapTest.h"
#include "Components/TA_EndianConversion.h"

TA_EndianSwapTest::TA_EndianSwapTest()
{

}

TA_EndianSwapTest::~TA_EndianSwapTest()
{

}

void TA_EndianSwapTest::SetUp()
{

}

void TA_EndianSwapTest::TearDown()
{

}

TEST_F(TA_EndianSwapTest, SwapEndianUint16) {
    uint16_t input = 0x1234;
    uint16_t expected = CoreAsync::TA_EndianConversion::isSystemLittleEndian() ? 0x3412 : input;
    ASSERT_EQ(CoreAsync::TA_EndianConversion::swapEndian(input), expected);
    CoreAsync::TA_EndianConversion::swapEndian(&input);
    ASSERT_EQ(input, expected);
}

TEST_F(TA_EndianSwapTest, SwapEndianUint32) {
    uint32_t input = 0x12345678;
    uint32_t expected = CoreAsync::TA_EndianConversion::isSystemLittleEndian() ? 0x78563412 : input;
    ASSERT_EQ(CoreAsync::TA_EndianConversion::swapEndian(input), expected);
    CoreAsync::TA_EndianConversion::swapEndian(&input);
    ASSERT_EQ(input, expected);
}

TEST_F(TA_EndianSwapTest, SwapEndianUint64) {
    uint64_t input = 0x123456789ABCDEF0;
    uint64_t expected = CoreAsync::TA_EndianConversion::isSystemLittleEndian() ? 0xF0DEBC9A78563412 : input;
    ASSERT_EQ(CoreAsync::TA_EndianConversion::swapEndian(input), expected);
    CoreAsync::TA_EndianConversion::swapEndian(&input);
    ASSERT_EQ(input, expected);
}

TEST_F(TA_EndianSwapTest, SwapEndianFloat) {
    float input = 1.23f;
    uint32_t inputBits = *reinterpret_cast<uint32_t *>(&input);
    uint32_t swappedBits = CoreAsync::TA_EndianConversion::swapEndian(inputBits);
    float expected = *reinterpret_cast<float *>(&swappedBits);
    if (!CoreAsync::TA_EndianConversion::isSystemLittleEndian())
        expected = input;
    ASSERT_FLOAT_EQ(CoreAsync::TA_EndianConversion::swapEndian(input), expected);
    CoreAsync::TA_EndianConversion::swapEndian(&input);
    ASSERT_EQ(input, expected);
}

TEST_F(TA_EndianSwapTest, SwapEndianDouble) {
    double input = 1.23;
    uint64_t inputBits = *reinterpret_cast<uint64_t *>(&input);
    uint64_t swappedBits = CoreAsync::TA_EndianConversion::swapEndian(inputBits);
    double expected = *reinterpret_cast<double *>(&swappedBits);
    if (!CoreAsync::TA_EndianConversion::isSystemLittleEndian())
        expected = input;
    ASSERT_DOUBLE_EQ(CoreAsync::TA_EndianConversion::swapEndian(input), expected);
    CoreAsync::TA_EndianConversion::swapEndian(&input);
    ASSERT_EQ(input, expected);
}
