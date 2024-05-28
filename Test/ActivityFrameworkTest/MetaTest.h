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
    ENABLE_REFLEX
public:
    ~M2Test()
    {
        delete px;
        px = nullptr;
    }

    void setMx(int x) {mx = x;}
    int getMx() const {return mx;}

    int mx {5};

private:
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

class M3Test
{
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

template <>
struct TA_TypeInfo<TestA> : TA_MetaTypeInfo<TestA>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::print, META_STRING("print")},
    };
    static constexpr TA_MetaPropertyOperations operations = {};
};

template <>
struct TA_TypeInfo<TestB> : TA_MetaTypeInfo<TestB>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::deduct, META_STRING("deduct")},
    };
    static constexpr TA_MetaPropertyOperations operations = {};
};

template <>
struct TA_TypeInfo<BaseTest> : TA_MetaTypeInfo<BaseTest,TestA>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::sub, META_STRING("sub")},
    };
    static constexpr TA_MetaPropertyOperations operations = {};
};

template <>
struct TA_TypeInfo<OtherTest> : TA_MetaTypeInfo<OtherTest,TestB>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::product, META_STRING("product")},
    };
    static constexpr TA_MetaPropertyOperations operations = {};
};

template <>
struct TA_TypeInfo<MetaTest> : TA_MetaTypeInfo<MetaTest,BaseTest,OtherTest>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {Raw::META_RED, META_STRING("META_RED")},
        TA_MetaField {Raw::META_GREEN, META_STRING("META_GREEN")},
        TA_MetaField {Raw::META_BLUE, META_STRING("META_BLUE")},
        TA_MetaField {static_cast<float(Raw::*)()const>(&Raw::Sum), META_STRING("sum_0")},
        TA_MetaField {static_cast<float(Raw::*)(float)const>(&Raw::Sum),META_STRING("sum_1")},
        TA_MetaField {static_cast<bool(Raw::*)(int)const>(&Raw::contains), META_STRING("contains_0")},
        TA_MetaField {static_cast<bool(Raw::*)(std::string)const>(&Raw::contains),META_STRING("contains_1")},
        TA_MetaField {&Raw::productMM, META_STRING("productMM")},
        TA_MetaField {&Raw::str, META_STRING("str")},
        TA_MetaField {&Raw::getStr, META_STRING("getStr")},
        TA_MetaField {&Raw::startTest, META_STRING("startTest")},
        TA_MetaField {&Raw::printTest, META_STRING("printTest")}
    };
    static constexpr TA_MetaPropertyOperations operations = {};
};

template <>
struct TA_TypeInfo<M2Test> : TA_MetaTypeInfo<M2Test>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::mx, META_STRING("mx")},
        TA_MetaField {&Raw::setMx, META_STRING("setMx")},
        TA_MetaField {&Raw::getMx, META_STRING("getMx")},
        TA_MetaField {&Raw::my, META_STRING("my")},
        TA_MetaField {&Raw::px, META_STRING("px")},
    };
    
    static constexpr TA_MetaPropertyOperations operations = {
        TA_MetaPropertyOperation {META_STRING("getMx"), META_STRING("setMx")}
    };
};

template <>
struct TA_TypeInfo<M3Test> : TA_MetaTypeInfo<M3Test>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::setVec, META_STRING("setVec")},
        TA_MetaField {&Raw::getVec, META_STRING("getVec")},
        TA_MetaField {&Raw::setRawPtr, META_STRING("setRawPtr")},
        TA_MetaField {&Raw::getRawPtr, META_STRING("getRawPtr")},
        TA_MetaField {&Raw::setArray<5>, META_STRING("setArray")},
        TA_MetaField {&Raw::getArray, META_STRING("getArray")},
        TA_MetaField {&Raw::setList, META_STRING("setList")},
        TA_MetaField {&Raw::getList, META_STRING("getList")},
        TA_MetaField {&Raw::setForwardList, META_STRING("setForwardList")},
        TA_MetaField {&Raw::getForwardList, META_STRING("getForwardList")},
        TA_MetaField {&Raw::setDeque, META_STRING("setDeque")},
        TA_MetaField {&Raw::getDeque, META_STRING("getDeque")},
        TA_MetaField {&Raw::setStack, META_STRING("setStack")},
        TA_MetaField {&Raw::getStack, META_STRING("getStack")},
        TA_MetaField {&Raw::setQueue, META_STRING("setQueue")},
        TA_MetaField {&Raw::getQueue, META_STRING("getQueue")},
        TA_MetaField {&Raw::setPrioritQueue, META_STRING("setPrioritQueue")},
        TA_MetaField {&Raw::getPriorityQueue, META_STRING("getPriorityQueue")},
    };

    static constexpr TA_MetaPropertyOperations operations = {
        TA_MetaPropertyOperation {META_STRING("getVec"), META_STRING("setVec")},
        TA_MetaPropertyOperation {META_STRING("getRawPtr"), META_STRING("setRawPtr")},
        TA_MetaPropertyOperation {META_STRING("getArray"), META_STRING("setArray")},
        TA_MetaPropertyOperation {META_STRING("getList"), META_STRING("setList")},
        TA_MetaPropertyOperation {META_STRING("getForwardList"), META_STRING("setForwardList")},
        TA_MetaPropertyOperation {META_STRING("getDeque"), META_STRING("setDeque")},
        TA_MetaPropertyOperation {META_STRING("getStack"), META_STRING("setStack")},
        TA_MetaPropertyOperation {META_STRING("getQueue"), META_STRING("setQueue")},
        TA_MetaPropertyOperation {META_STRING("getPriorityQueue"), META_STRING("setPrioritQueue")},
    };
};

}

#endif // METATEST_H
