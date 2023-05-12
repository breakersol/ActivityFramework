#include "TA_ConnectionTest.h"
#include "ITA_Connection.h"
#include "MetaTest.h"

TA_ConnectionTest::TA_ConnectionTest()
{

}

TA_ConnectionTest::~TA_ConnectionTest()
{

}

void TA_ConnectionTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ConnectionTest::TearDown()
{
//    if(m_pTest)
//        delete m_pTest;
//    m_pTest = nullptr;
}

TEST_F(TA_ConnectionTest, connectSyncTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    CoreAsync::ITA_Connection::disconnect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM);
}

TEST_F(TA_ConnectionTest, connectAsyncTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Async>(m_pTest, &MetaTest::printTest, m_pTest, &MetaTest::print));
    EXPECT_FALSE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::printTest, m_pTest, &MetaTest::print));
    CoreAsync::ITA_Connection::disconnect(m_pTest, &MetaTest::printTest, m_pTest, &MetaTest::print);
}

TEST_F(TA_ConnectionTest, disconnectTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::disconnect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_FALSE(CoreAsync::ITA_Connection::active(m_pTest,&MetaTest::startTest,2,3));
}

TEST_F(TA_ConnectionTest, activeTest)
{
    EXPECT_TRUE(CoreAsync::ITA_Connection::connect(m_pTest, &MetaTest::startTest, m_pTest, &MetaTest::productMM));
    EXPECT_TRUE(CoreAsync::ITA_Connection::active(m_pTest,&MetaTest::startTest,5,5));
}
