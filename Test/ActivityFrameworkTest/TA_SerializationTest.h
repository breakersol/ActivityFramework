#ifndef TA_SERIALIZATIONTEST_H
#define TA_SERIALIZATIONTEST_H

#include "gtest/gtest.h"
#include "MetaTest.h"

class TA_SerializationTest : public ::testing::Test
{
public:
    TA_SerializationTest();
    ~TA_SerializationTest();

    void SetUp() override;
    void TearDown() override;

    template<typename T, typename Container = std::vector<T>, typename Compare = std::less<typename Container::value_type>>
    bool arePriorityQueueEqual(std::priority_queue<T, Container, Compare> &pq1, std::priority_queue<T, Container, Compare> &pq2)
    {
        if (pq1.size() != pq2.size())
            return false;

        while (!pq1.empty() && !pq2.empty())
        {
            if (pq1.top() != pq2.top())
            {
                return false;
            }
            pq1.pop();
            pq2.pop();
        }

        return pq1.empty() && pq2.empty();
    }

    M3Test testObj;

};

#endif // TA_SERIALIZATIONTEST_H
