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

#include "TA_VariantTest.h"
#include "Components/TA_Variant.h"

#include <array>
#include <stdexcept>

namespace {
struct ThrowingCopyValue {
    inline static bool throwOnCopy = false;
    inline static int liveCount = 0;
    int value = 42;

    ThrowingCopyValue() { ++liveCount; }
    ThrowingCopyValue(const ThrowingCopyValue &other) : value(other.value) {
        if (throwOnCopy)
            throw std::runtime_error("Payload copy failed");
        ++liveCount;
    }
    ThrowingCopyValue(ThrowingCopyValue &&other) noexcept : value(other.value) { ++liveCount; }
    ~ThrowingCopyValue() { --liveCount; }
};
} // namespace

TA_VariantTest::TA_VariantTest() {}

TA_VariantTest::~TA_VariantTest() {}

void TA_VariantTest::SetUp() { m_pTest = new MetaTest(); }

void TA_VariantTest::TearDown() {
    if (m_pTest) {
        delete m_pTest;
        m_pTest = nullptr;
    }
}

TEST_F(TA_VariantTest, constructTest) {
    CoreAsync::TA_Variant var{m_pTest};
    auto res = var.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, setTest) {
    CoreAsync::TA_Variant var{};
    var.set(m_pTest);
    auto res = var.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, validTest) {
    CoreAsync::TA_Variant var{};
    EXPECT_EQ(var.isValid(), false);
    var.set(m_pTest);
    EXPECT_EQ(var.isValid(), true);
}

TEST_F(TA_VariantTest, assignmentOperatorTest) {
    CoreAsync::TA_Variant var_1{m_pTest};
    CoreAsync::TA_Variant var2;
    var2 = var_1;
    auto res = var2.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, copyTest) {
    CoreAsync::TA_Variant var_1{m_pTest};
    CoreAsync::TA_Variant var2{var_1};
    auto res = var2.get<MetaTest *>()->sub(1, 2);
    EXPECT_EQ(res, -1);
}

TEST_F(TA_VariantTest, throwingCopyAssignmentLeavesEmptyAndReusable) {
    for (bool heapDestination : {false, true}) {
        CoreAsync::TA_Variant source{ThrowingCopyValue{}};
        CoreAsync::TA_Variant destination;
        auto owned = std::make_shared<int>(7);
        std::weak_ptr<int> previousValue = owned;
        if (heapDestination) {
            std::array<std::shared_ptr<int>, 16> largeValue{};
            largeValue[0] = owned;
            destination.set(std::move(largeValue));
        } else {
            destination.set(owned);
        }
        owned.reset();

        ThrowingCopyValue::throwOnCopy = true;
        EXPECT_THROW(destination = source, std::runtime_error);
        ThrowingCopyValue::throwOnCopy = false;

        ASSERT_FALSE(destination.isValid());
        EXPECT_FALSE(destination.isSameType<ThrowingCopyValue>());
        EXPECT_EQ(destination.typeId(), typeid(std::nullptr_t).hash_code());
        EXPECT_TRUE(previousValue.expired());
        EXPECT_EQ(ThrowingCopyValue::liveCount, 1);
        EXPECT_EQ(source.get<ThrowingCopyValue>().value, 42);

        CoreAsync::TA_Variant copied{destination};
        CoreAsync::TA_Variant moved{std::move(destination)};
        EXPECT_FALSE(copied.isValid());
        EXPECT_FALSE(moved.isValid());
        copied = source;
        moved.set(17);
        destination = source;
        EXPECT_EQ(copied.get<ThrowingCopyValue>().value, 42);
        EXPECT_EQ(moved.get<int>(), 17);
        EXPECT_EQ(destination.get<ThrowingCopyValue>().value, 42);
    }
    EXPECT_EQ(ThrowingCopyValue::liveCount, 0);
}
