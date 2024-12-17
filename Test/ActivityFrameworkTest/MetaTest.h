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

#ifndef METATEST_H
#define METATEST_H

#include <iostream>
#include <string>
#include <forward_list>
#include <stack>
#include <queue>

#include "Components/TA_MetaReflex.h"
#include "Components/TA_MetaObject.h"
#include "Components/TA_ConnectionUtils.h"

class TestA : public virtual CoreAsync::TA_MetaObject
{
public:
    void print() {std::printf("We are the champion\n");}

};

class TestB : public virtual CoreAsync::TA_MetaObject
{
public:
    void deduct() {std::printf("We are superman\n");}
};

class BaseTest : public TestA
{
public:
    int sub(int a, int b)
    {
        int res = a - b;
        std::printf("sub: %d\n", res);
        return res;
    }
};

class OtherTest : public TestB
{
public:
    void product(int a, int b) {auto x = a * b;}
};

class M2Test : public CoreAsync::TA_MetaObject
{
public:
    ~M2Test()
    {
        delete px;
        px = nullptr;
    }

    std::vector<int> m_vec {0,0,0,0};

    int mx {5};
    int my {10};
    int *px = new int(10);

};

class MetaTest : public BaseTest, public OtherTest
{
public:
    enum MetaColor
    {
        META_RED,
        META_BLUE,
        META_GREEN
    };

    float Sum() const {
        return 0;
    }
    float Sum(float z) const {
        return z + 3;
    }

    bool contains(int m) const
    {
        return true;
    }

    bool contains(std::string n) const
    {
        return false;
    }

    void productMM(int a, int b)
    {
        std::printf("The numbers are: %d, %d\n.",a,b);
    }

    int xx {3};

    static std::string str;

    static std::string getStr(const std::string &str)
    {
        return std::string("123") + str;
    }

TA_Signals:
    void startTest(int,int) {}
    void printTest() {}

};

class M3Test : public M2Test
{
    ENABLE_REFLEX
public:
    void setVec(const std::vector<int> &v)
    {
        vec = v;
    }

    const std::vector<int> & getVec() const
    {
        return vec;
    }

    void setRawPtr(float *&ptr)
    {
        if(pFloatPtr)
            delete pFloatPtr;
        pFloatPtr = ptr;
    }

    float * getRawPtr() const
    {
        return pFloatPtr;
    }

    template <std::size_t N = 5>
    void setArray(const std::array<uint32_t, N> &array)
    {
        m_array = array;
    }

    const std::array<uint32_t, 5> & getArray() const
    {
        return m_array;
    }

    void setList(const std::list<uint16_t> &list)
    {
        m_list = list;
    }

    const std::list<uint16_t> & getList() const
    {
        return m_list;
    }

    void setForwardList(const std::forward_list<uint64_t> &list)
    {
        m_forwardList = list;
    }

    const std::forward_list<uint64_t> & getForwardList() const
    {
        return m_forwardList;
    }

    void setDeque(const std::deque<int16_t> &deque)
    {
        m_deque = deque;
    }

    const std::deque<int16_t> & getDeque() const
    {
        return m_deque;
    }

    void setStack(const std::stack<int32_t> &stack)
    {
        m_stack = stack;
    }

    const std::stack<int32_t> & getStack() const
    {
        return m_stack;
    }

    void setQueue(const std::queue<int64_t> &queue)
    {
        m_queue = queue;
    }

    const std::queue<int64_t> & getQueue() const
    {
        return m_queue;
    }

    void setPrioritQueue(const std::priority_queue<double> &queue)
    {
        m_prioritQueue = queue;
    }

    const std::priority_queue<double> & getPriorityQueue() const
    {
        return m_prioritQueue;
    }

private:
    std::vector<int> vec {1, 2, 3, 4};
    float *pFloatPtr = new float(1.5);
    std::array<uint32_t, 5> m_array {1, 2, 3, 4, 5};
    std::list<uint16_t> m_list {8,8,8,8,8,8};
    std::forward_list<uint64_t> m_forwardList {100000, 10000};
    std::deque<int16_t> m_deque {1,2,3,4,5};
    std::stack<int32_t> m_stack;
    std::queue<int64_t> m_queue;
    std::priority_queue<double> m_prioritQueue;

};

namespace CoreAsync::Reflex {

DEFINE_TYPE_INFO(TestA)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(print)
    )
};

DEFINE_TYPE_INFO(TestB)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(deduct)
    )
};

DEFINE_TYPE_INFO(BaseTest, TestA)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(sub)
        )
};

DEFINE_TYPE_INFO(OtherTest, TestB)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(product)
    )
};

DEFINE_TYPE_INFO(MetaTest, BaseTest, OtherTest)
{
    AUTO_META_FIELDS(
        REGISTER_ENUM(META_RED),
        REGISTER_ENUM(META_GREEN),
        REGISTER_ENUM(META_BLUE),
        REGISTER_METHOD_OVERLOAD_GENERIC(Sum, const, float, float),
        REGISTER_METHOD_OVERLOAD_GENERIC(Sum, const, float),
        REGISTER_METHOD_OVERLOAD_GENERIC(contains, const, bool, int),
        REGISTER_METHOD_OVERLOAD_GENERIC(contains, const, bool, std::string),
        REGISTER_FIELD(productMM),
        REGISTER_FIELD(str),
        REGISTER_FIELD(getStr),
        REGISTER_FIELD(startTest),
        REGISTER_FIELD(printTest),
        )
};

DEFINE_TYPE_INFO(M2Test)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(m_vec, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(mx, TA_PROPERTY(2)),
        REGISTER_FIELD(my, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(px),
        )
};

DEFINE_TYPE_INFO(M3Test, M2Test)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(vec, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_array, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(pFloatPtr, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_list, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_forwardList, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_deque, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_stack, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_queue, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(m_prioritQueue, TA_DEFAULT_PROPERTY),
    )
};

}

#endif // METATEST_H
