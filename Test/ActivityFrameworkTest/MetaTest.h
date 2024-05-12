#ifndef METATEST_H
#define METATEST_H

#include <iostream>
#include <string>

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
    void setMx(int x) {mx = x;}
    int getMx() const {return mx;}

    int mx {5};

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

private:
    std::vector<int> vec {1, 2, 3, 4};
    float *pFloatPtr = new float(1.5);
    std::array<uint32_t, 5> m_array {1, 2, 3, 4, 5};
    std::list<uint16_t> m_list {8,8,8,8,8,8};

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
    };

    static constexpr TA_MetaPropertyOperations operations = {
        TA_MetaPropertyOperation {META_STRING("getVec"), META_STRING("setVec")},
        TA_MetaPropertyOperation {META_STRING("getRawPtr"), META_STRING("setRawPtr")},
        TA_MetaPropertyOperation {META_STRING("getArray"), META_STRING("setArray")},
        TA_MetaPropertyOperation {META_STRING("getList"), META_STRING("setList")}
    };
};

}

#endif // METATEST_H
