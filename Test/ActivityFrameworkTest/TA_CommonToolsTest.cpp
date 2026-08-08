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

#include "TA_CommonToolsTest.h"
#include "Components/TA_CommonTools.h"

TA_CommonToolsTest::TA_CommonToolsTest() {}

TA_CommonToolsTest::~TA_CommonToolsTest() {}

void TA_CommonToolsTest::SetUp() {}

void TA_CommonToolsTest::TearDown() {}

TEST_F(TA_CommonToolsTest, decToBinTest) {
    std::string res = CoreAsync::TA_CommonTools::decimalToBinary(1991817);
    EXPECT_EQ(res, "111100110010010001001");
}

TEST_F(TA_CommonToolsTest, midTest) {
    std::list<int> l{1, 2, 3, 4, 5, 6, 7, 8};
    auto res = CoreAsync::ContainerUtils::subRanges<int>(l, 2, 4);
    EXPECT_EQ(res.front(), 3);
    EXPECT_EQ(res.back(), 6);
}

TEST_F(TA_CommonToolsTest, atTest) {
    const std::list<int> l{1, 2, 3};
    const auto value = CoreAsync::ContainerUtils::at<int>(l, 1);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 2);
    EXPECT_FALSE(CoreAsync::ContainerUtils::at<int>(l, l.size()).has_value());
}

TEST_F(TA_CommonToolsTest, refTest) {
    std::list<int> l{1, 2, 3};
    auto value = CoreAsync::ContainerUtils::ref<int>(l, 1);

    ASSERT_TRUE(value.has_value());
    value->get() = 20;
    EXPECT_EQ(*std::next(l.begin()), 20);
    EXPECT_FALSE(CoreAsync::ContainerUtils::ref<int>(l, l.size()).has_value());
}

TEST_F(TA_CommonToolsTest, insertTest) {
    std::list<int> l{1, 2};

    CoreAsync::ContainerUtils::insert(l, l.size(), 3);
    EXPECT_EQ(l.back(), 3);
    EXPECT_THROW(CoreAsync::ContainerUtils::insert(l, l.size() + 1, 4), std::out_of_range);
}

TEST_F(TA_CommonToolsTest, takeAtTest) {
    std::list<int> l{1, 2, 3, 4, 5, 6, 7, 8};
    auto res = CoreAsync::ContainerUtils::takeAt<int>(l, 2);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, 3);
    EXPECT_FALSE(CoreAsync::ContainerUtils::takeAt<int>(l, l.size()).has_value());
}

TEST_F(TA_CommonToolsTest, mapUtilsValueTest) {
    std::multimap<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}, {1, "111"}};
    auto res = CoreAsync::MapUtils::values(map, 1);
    EXPECT_EQ(res.front() == "1" && res.back() == "111", true);
}

TEST_F(TA_CommonToolsTest, mapUtilsSingleValueTest) {
    const std::map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    const auto value = CoreAsync::MapUtils::value(map, 2);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "2");
    EXPECT_FALSE(CoreAsync::MapUtils::value(map, 4).has_value());
}

TEST_F(TA_CommonToolsTest, mapUtilsRemoveTest) {
    std::multimap<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}, {1, "111"}};
    CoreAsync::MapUtils::remove(map, 1);
    EXPECT_EQ(map.contains(2) && map.contains(3), true);
}

TEST_F(TA_CommonToolsTest, mapUtilsKeyTest) {
    std::multimap<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}, {1, "111"}};
    auto res = CoreAsync::MapUtils::key(map, {"111"});

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, 1);
    EXPECT_FALSE(CoreAsync::MapUtils::key(map, {"missing"}).has_value());
}

TEST_F(TA_CommonToolsTest, mapUtilsContainsTest) {
    std::multimap<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}, {1, "111"}};
    auto res = CoreAsync::MapUtils::contains(map, 1);
    EXPECT_EQ(res == true, true);
}

TEST_F(TA_CommonToolsTest, splitTest) {
    std::string source{"123,345,456"};
    auto res = CoreAsync::ContainerUtils::split(source, ',');
    EXPECT_EQ(res.front(), "123");
    EXPECT_EQ(res.back(), "456");
}

TEST_F(TA_CommonToolsTest, stringUtilsSplitCompatibilityTest) {
    std::string source{"123,345,456"};
    auto res = CoreAsync::StringUtils::split(source, ',');
    EXPECT_EQ(res.front(), "123");
    EXPECT_EQ(res.back(), "456");
}
