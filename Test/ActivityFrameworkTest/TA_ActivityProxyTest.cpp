#include "TA_ActivityProxyTest.h"
#include "Components/TA_ActivityProxy.h"
#include "Components/TA_MetaActivity.h"
#include "Components/TA_ThreadPool.h"

TA_ActivityProxyTest::TA_ActivityProxyTest()
{

}

TA_ActivityProxyTest::~TA_ActivityProxyTest()
{

}

void TA_ActivityProxyTest::SetUp()
{
    m_pTest = new MetaTest();
}

void TA_ActivityProxyTest::TearDown()
{
    if(m_pTest)
        delete m_pTest;
    m_pTest = nullptr;
}

TEST_F(TA_ActivityProxyTest, MetaActivityTest)
{
    decltype(auto) pActivity = new CoreAsync::TA_MetaActivity<decltype(META_STRING("sub")), MetaTest *&, int, int>(META_STRING("sub"), m_pTest, 9, 8);
    CoreAsync::TA_ActivityProxy *pProxy = new CoreAsync::TA_ActivityProxy(pActivity);
    auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(pProxy);
    EXPECT_EQ(fetcher().get<int>(), 1);
}

TEST_F(TA_ActivityProxyTest, SingleActivityTest)
{
    decltype(auto) pActivity = new CoreAsync::TA_SingleActivity<decltype(&MetaTest::sub), MetaTest *, int, int ,int>(&MetaTest::sub, m_pTest, 9, 8);
    CoreAsync::TA_ActivityProxy *pProxy = new CoreAsync::TA_ActivityProxy(pActivity);
    auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(pProxy);
    EXPECT_EQ(fetcher().get<int>(), 1);
}
