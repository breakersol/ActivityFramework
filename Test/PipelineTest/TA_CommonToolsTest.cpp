#include "TA_CommonToolsTest.h"
#include "Components/TA_CommonTools.h"

TA_CommonToolsTest::TA_CommonToolsTest()
{

}

TA_CommonToolsTest::~TA_CommonToolsTest()
{

}

void TA_CommonToolsTest::SetUp()
{

}

void TA_CommonToolsTest::TearDown()
{

}

TEST_F(TA_CommonToolsTest, decToBinTest)
{
    std::string res = CoreAsync::TA_CommonTools::decimalToBinary(1991817);
    EXPECT_EQ(res, "111100110010010001001");
}

TEST_F(TA_CommonToolsTest, midTest)
{
    std::list<int> l {1,2,3,4,5,6,7,8};
    auto res = CoreAsync::TA_CommonTools::mid<int>(l, 2, 4);
    EXPECT_EQ(res.front(), 3);
    EXPECT_EQ(res.back(), 5);
}

TEST_F(TA_CommonToolsTest, takeAtTest)
{
    std::list<int> l {1,2,3,4,5,6,7,8};
    auto res = CoreAsync::TA_CommonTools::takeAt<int>(l, 2);
    EXPECT_EQ(res, 3);
}

TEST_F(TA_CommonToolsTest, mapUtilsValueTest)
{
    std::multimap<int, std::string> map{{1,"1"},{2,"2"}, {3,"3"}, {1,"111"}};
    auto res = CoreAsync::MapUtils::values(map, 1);
    EXPECT_EQ(res.front() == "1" && res.back() == "111", true);
}

TEST_F(TA_CommonToolsTest, mapUtilsRemoveTest)
{
    std::multimap<int, std::string> map{{1,"1"},{2,"2"}, {3,"3"}, {1,"111"}};
    CoreAsync::MapUtils::remove(map, 1);
    EXPECT_EQ(map.contains(2) && map.contains(3), true);
}

TEST_F(TA_CommonToolsTest, mapUtilsKeyTest)
{
    std::multimap<int, std::string> map{{1,"1"},{2,"2"}, {3,"3"}, {1,"111"}};
    auto res = CoreAsync::MapUtils::key(map, {"111"});
    EXPECT_EQ(res == 1, true);
}

TEST_F(TA_CommonToolsTest, splitTest)
{
    std::string source {"123,345,456"};
    auto res = CoreAsync::StringUtils::split(source, ',');
    EXPECT_EQ(res.front(), "123");
    EXPECT_EQ(res.back(), "456");
}
