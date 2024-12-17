# Activity Framework ![Build Status](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml)

**Activity Framework** is an advanced, evolving lightweight C++ development framework optimized for efficiency and flexibility. Engineered to augment the software development process, it incorporates a rich set of tools that streamline and enhance the development of complex applications. Below is a detailed overview of its features, platform support, and dependencies.

### Features

| Feature                          | Description                                                                 |
|----------------------------------|-----------------------------------------------------------------------------|
| Compile-time Reflection          | Facilitates metadata manipulation at compile-time, enhancing adaptability.  |
| Serialization & Deserialization  | Ensures robust data persistence and retrieval, crucial for data integrity.  |
| Lock-free Circular Queue         | Implements an efficient circular data structure for improved data handling. |
| Thread Pool                      | Optimizes concurrent task executions through effective thread management.   |
| Qt-like Signal and Slot Mechanism| Provides a dynamic event-handling mechanism inspired by the Qt framework.   |

### Platform Support

- **Windows**: Fully optimized and supported.
- **Linux**: Ensures full compatibility and performance.

### Dependencies

- Exclusively relies on the C++ Standard Library, minimizing external dependencies and simplifying integration.

This framework is ideal for developers seeking a robust, scalable, and easily integrated solution for sophisticated C++ applications.

## Getting Started

These instructions will guide you through setting up the project on your local machine for development and testing purposes.

### Prerequisites

- CMake: Version 3.20 or higher
- Windows: MSVC 17.5.33530.505 or higher
- Linux: GCC13 or higher

### Installing

To build the project, use the `CMakeList.txt` at **/ActivityFramework/ActivityFramework/CMakeList.txt** and set the build directory to **/ActivityFramework/build**.

### Running the Tests

1. Build the Google Test framework under **ActivityFramework/Test/ThirdParty/googletest/**.
2. Utilize **ActivityFramework/Test/CMakeList.txt** to prepare the unit test project.
3. Execute `ActivityFrameworkTest` to run the tests.

### Versioning

Recent versions can be accessed at:
- [v0.2.1](https://github.com/breakersol/ActivityFramework/releases/tag/v0.2.1)
- [v0.2.2](https://github.com/breakersol/ActivityFramework/releases/tag/v0.2.2)
- [v0.3.0](https://github.com/breakersol/ActivityFramework/releases/tag/v0.3.0)
- [v0.3.1](https://github.com/breakersol/ActivityFramework/releases/tag/v0.3.1)

### Authors

- **Sol** - Initial work - [breakersol](https://github.com/breakersol)
Email: breakersol@outlook.com

### License

This project is licensed under the Apache-2.0 License - see the [LICENSE.md](https://github.com/breakersol/ActivityPipeline/blob/master/LICENSE) file for details.

## Basic Usage

### Meta Reflex
This module implements a simple compile-time reflection mechanism and it is part of the infrastructure of **Connection**. It supports reflection of member variables, member functions, static member variables, static member variables, and member enumeration types, and it can reflect the base class information under single-level inheritance relationship, multi-level inheritance relationship. But for diamond inheritance there are still some problems existed now. 

<br/>Here are an example of using it:
```cpp
#include "Components/TA_MetaReflex.h"

class BaseTest
{
public:
    int sub(int a, int b)
    {
        return a - b;
    }
};

class OtherTest
{
public:
    void product(int a, int b) {auto x = a * b;}
};

class MetaTest : public BaseTest, public OtherTest
{
    ENABLE_REFLEX    //If you want to reflect private members, then declaring this macro is a must

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
};

namespace CoreAsync::Reflex {

DEFINE_TYPE_INFO(BaseTest)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(sub)
        )
};

DEFINE_TYPE_INFO(OtherTest)
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
}

```
<br/>First, _TA_TypeInfo_ is necessary in this mechanism. You must write type info like the above for the types that need to be reflected. And then, you can try to use it as follow.
- **Function -> Name**
```cpp
    //find member function name: sum_1
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(static_cast<float(MetaTest::*)(float)const>(&MetaTest::Sum));
```
```cpp
    //find member variable name: xx
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(&MetaTest::xx);
```
```cpp
    //find non member variable name: str
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(&MetaTest::str);
```
```cpp
    //find enum name: META_RED
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(MetaTest::META_RED);
```
```cpp
    //find non member function name: getStr
    std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(&MetaTest::getStr);
```
- **Name -> Function**
```cpp
    MetaTest *pTest = new MetaTest();
```
```cpp
    //invoke sum by the name "sum_1", res = 4.5
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("sum_1"),pTest,1.5);
```
```cpp
    //invoke getStr by the name "getStr", res = "123123"
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("getStr"), "123");
```
```cpp
    //invoke non member variable, res = "123"
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("str"));
```
```cpp
    //invoke non member variable, res = 3
    auto res = CoreAsync::Reflex::TA_TypeInfo<M2Test>::invoke(META_STRING("xx"),pTest);
```
```cpp
    //invoke enum, res = MetaTest::Red
    auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("META_RED"));
```
**Note: If a type is exported to use, then the meta info for that type needs to be exported as well.**
### Serialization & Deserialization
This module is responsible for both serialization and deserialization of data types to and from a buffer. It provides a robust foundation for serialization tasks, accommodating a variety of data types and structures, and ensuring data integrity and type safety through compile-time checks and runtime validations.<br/>
<br/>- **Supported Types**:<br/>
| Category              | Supported Types Examples                                          |
|-----------------------|-------------------------------------------------------------------|
| **Custom Types**      | Any user-defined type that adheres to the serialization interface |
| **Standard Containers** | `std::vector`, `std::deque`, `std::list`, `std::map`, `std::set`, `std::multimap`, `std::multiset`, `std::unordered_map`, `std::unordered_set`, `std::unordered_multimap`, `std::unordered_multiset`, `std::forward_list` |
| **Stack-like Adaptors** | `std::stack`, `std::queue`, `std::priority_queue`                |
| **Basic Data Types**   | `int`, `float`, `double`, etc.                                    |
| **Pairs**              | `std::pair<T1, T2>`                                               |
| **Arrays**             | `std::array`, C-style arrays, e.g., `int[10]`                                   |
| **Enums**              | Enumerated types                                                  |
| **Pointers**           | Raw pointers to types that are serializable                       |

<br/>- **Endianess Conversion**:<br/>
Serializer handles endianess conversions for basic data types, ensuring that data is correctly serialized and deserialized across different hardware architectures.
<br/>- **Version Management**:<br/>
The serializer supports versioning, allowing for backward compatibility in serialized data. This feature is crucial for applications that may evolve over time, ensuring that older data formats can still be read by newer versions of the software.
<br/>- **Support for class inheritance**:<br/>
When serializing/de-serializing a subclass object, the attribute data of the parent class is automatically serialized/de-serialized.
<br/>- **Interfaces**:<br/>
| Function Signature                                 | Description                                                  |
|----------------------------------------------------|--------------------------------------------------------------|
| `explicit TA_Serializer(const std::string &path, std::size_t version = 1, std::size_t bufferSize = 1024 * 1024)` | Constructor initializes the serializer with a file path, optional version, and buffer size. |
| `~TA_Serializer()`                                 | Destructor that handles resource cleanup.                     |
| `std::size_t version() const`                      | Returns the serialization version.                           |
| `void flush()`                                     | Flushes the buffer to ensure all data is written.             |
| `void close()`                                     | Closes the data operator (file), ensuring all data is finalized. |

When using this module, you must implement the TA_TypeInfo of the object that is going to be serialized/de-serialized and extend the attribute information in it.
```cpp
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
```
From the snippet above, it is evident that **TA_DEFAULT_PROPERTY** is passed into *TA_MetaField*. The corresponding class member variable is defined as a property within the domain, which allows it to be automatically identified by the serialization module. Consequently, serialization and deserialization processes are applied to this property. Any class member variables not defined as properties are automatically excluded during serialization and deserialization.
**TA_DEFAULT_PROPERTY** indicates that the serialization version of the attribute is set to 1. You can also define the serialization version of the property as some other value greater than 1 using **TA_PROPERTY(VALUE)**.

```cpp
    M3Test p1, p2;
    {
        CoreAsync::TA_Serializer output("./test.afw", 2);    //Set the serialization version as 2.
        output << p1;
    }
    {
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input("./test.afw", 2); //Set the de-serialization version as 2.
        input >> p2;
    }
```
In the demo above, properties with a serialized version of 2 or lower are eligible for serialization. The same criteria apply to deserialization.
### Connection
This is a simplified version of the Qt-like signal-slot mechanism.It allows objects to communicate with each other in a loosely coupled way, by emitting signals and connecting them to slots. When a signal is emitted, all connected slots are called, allowing objects to respond to events in a flexible and decoupled manner.
The condition for using this mechanism is to make the target class inherit from **TA_MetaObject** and to implement type info of this type.
```cpp
class MetaTest : public CoreAsync::TA_MetaObject
{
public:
    void printSlot() {std::printf("We are superman\n");}
    void printNums(int a, int b)
    {
        std::printf("The numbers are: %d, %d\n.",a,b);
    }
  
TA_Signals:
    void startTest(int,int) {}
    void printTest() {}

};

DEFINE_TYPE_INFO(MetaTest)
{
    AUTO_META_FIELDS(
        REGISTER_FIELD(printSlot),
        REGISTER_FIELD(printNums),
        REGISTER_FIELD(startTest),
        REGISTER_FIELD(printTest)
    )
};
```
<br/>From the code above, you can see there are two signal function _void startTest(int,int)_ and _void printTest()_ have been defined, and they have been filled into the type info. Note that the signal function does not require any implementation, and its return type must be **void**.

<br/>Next, you can use the interface **ITA_Connection::connect** as follow:
```cpp
    CoreAsync::ITA_Connection::connect(pTest, &MetaTest::startTest, pTest, &MetaTest::printNums);
    CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Queue>(pTest, &MetaTest::printTest, pTest, &MetaTest::printSlot);
```
<br/>**ITA_Connection::connect** requires the same number of arguments for both of sender function and receiver function.

<br/>Connection currently offers three types of connections: 
```cpp
    enum class TA_ConnectionType
    {
        Auto,
        Direct,
        Queued
    };
```
<br/>The default connection type is _Auto_, which automatically determines whether to use a Direct Connection or a Queued Connection based on whether the sender and receiver are in the same thread. This is the default type when connecting signals and slots.

<br/>If you'd like to active the signal function by calling **ITA_Connection::active**. 
```cpp
    CoreAsync::ITA_Connection::active(pTest,&MetaTest::startTest,2,3);
```
Finally, if the connection is unnecessary anymore, you can **disconnect** it.
```cpp
    CoreAsync::ITA_Connection::disconnect(pTest, &MetaTest::printTest, ppTest, &MetaTest::printSlot);
```
### Activity
Activity is the basic unit in a pipeline, and _ITA_ActivityCreator_ provides several methods to create them, you can learn about them from the source code of the unit test. This is one of the methods  :
```cpp
    auto acivity = CoreAsync::ITA_ActivityCreator::create<int>(&MetaTest::printNums, pTest, m,n);
```
<br/>If all the parameters binded when creating an activity are left-value, modifying the original left-valued object before executing the activity will affect the final execution result.
<br/>Activity also provides some additional features:
<br/>1. Link an activity to another activity. 
```cpp
    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(2,1);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    activity_1->link(activity_2);
    auto var = (*activity_1)();
```
<br/>When the execution of activity_1 is finished, activity_2 will be executed automatically, and the result of activity_2 will be returned.

<br/>2. Make two activities execute in concurrency.
```cpp
    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(8,1);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    activity_1->parallel(activity_2);
    auto var = (*activity_1)();
    auto res = var.get<int>();
```
<br/>The execution of activity_1 will execute actvity_2 in concurrency and finally return the result of activtiy_2.

<br/>3. Create branches on selected activity.
```cpp
    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(1,2);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    std::function<int()> func_3 = [&]()->int {return m_pTest->sub(99,1);};
    auto activity_3 = CoreAsync::ITA_ActivityCreator::create(func_3);
    std::function<int()> func_4 = [&]()->int {return m_pTest->sub(58,33);};
    auto activity_4 = CoreAsync::ITA_ActivityCreator::create(func_4);
    std::function<std::string()> func_5 = [&]()->std::string {return MetaTest::getStr("321");};
    auto activity_5 = CoreAsync::ITA_ActivityCreator::create(func_5);

    activity_3->branch(activity_4,activity_5);
    activity_1->branch(activity_2,activity_3);

    activity_1->selectBranch({2,2});
    auto var = (*activity_1)();
    auto res = var.get<std::string>();
```
<br/>The above code indicates that we first create two branches activity_4 and activity_5 on activity_3. At this layer, activity_3 represents branch index 0, while activity_4 and activity_5 represent 1 and 2, respectively. Similarly, we created the branch on activity_1 activity_2 and activity_3. **selectBranch({2,2})** means that when activity_1 is executed(**(*activity_1)()**), it will be executed in the order from activity_3 to activity_5 and return the result of activity_5. And activity_1 itself will not be executed.

### Thread Pool
A lightweight thread pool has been implemented within the framework, which is associated with Activities. The main interfaces for the thread pool are as follows.
- **postActivity**: This interface allows us to post tasks to the thread pool with a flag of type bool indicating whether the task object can be automatically released after being executed. This function returns a std::future object and an activity id, which you can use to get the result of the execution.
```cpp
    CoreAsync::TA_ThreadPool threadPool;
    std::vector<std::future<CoreAsync::TA_Variant>> testVec;
    std::vector<int> validVec(1024);
    for(int i = 0;i < activities.size();++i)
    {
        testVec.emplace_back(threadPool.postActivity(activities[i]).first);
        validVec[i] = i;
    }
    for(int i = 0;i < testVec.size();++i)
    {
        EXPECT_EQ(testVec[i].get().get<int>(), validVec[i]);
    }
```
- **size**: Return the number of threads.
- **shutDown**: Request to shut down and clear all of threads.
---

### Pipelines
Activity Pipeline currently offers five types of pipelines to use: **Auto Chain Pipeline, Concurrent Pipeline, Manual Chain Pipeline, Manual Steps Chain Pipeline, and Manual Key Activity Chain Pipeline**, and all types of pipelines can be created through **ITA_PipelineCreator**.
1. _Auto Chain Pipeline_: The pipeline will automatically execute all activites in order.
2. _Concurrent Pipeline_: All the activities in this pipeline will be executed in concurrency.
3. _Manual Chain Pipeline_: Calling the execute function will only execute the next activity in order.
4. _Manual Steps Chain Pipeline_: This pipeline inherits from _Manual Chain Pipeline_. The difference with _Manual Chain Pipeline_ is that the user can set the step value so that the pipeline executes as many activites as the step value each time.
5. _Manual Key Activity Chain Pipeline_: This pipeline inherits from _Manual Chain Pipeline_. The difference with _Manual Chain Pipeline_ is that the user can set the key activity to make the pipeline repeat selected activity.

<br/>List some of the APIs about pipeline here:
- **waitingComplete**: Continuous waiting until the pipeline execution is finished.This function will block the current thread.
- **void setStartIndex(unsigned int index)**: Set the execution to start from the _index_ activity. Valid only in the **Waiting** state.
- **bool switchActivityBranch(int activityIndex, std::deque<unsigned int> branches)**: Switching the branch of the _activityIndex_ activity.Valid only in the **Waiting** state.
- **add**: Adding activites into the pipeline. Valid only in the **Waiting** state.
- **bool remove(ActivityIndex index)**: Delete the _index_ activity. Valid only in the **Waiting** state.
- **void clear()**: Clear all activities and cached data, and reset the pipeline status to **Waiting**. Invalid when called in **Busy** state.
- **bool execute()**: Start the execution and set the pipeline to **Busy** state. Valid only in the **Waiting** state.
- **void reset()**: Resets the pipeline status to **Waiting** and clears all cached data. Valid only in the **Ready** state. 
- **bool result(int index, Res &res)**: Get the result of the specified activity by index. Invalid when called in **Busy** state.
- **void setSteps(unsigned int steps)**: Set the steps for a single execution. API specific to **Manual Steps Chain Pipeline**. Invalid when called in **Busy** state.
- **void setKeyActivityIndex(int index)**: Set the _index_ activity as the key activity, and the pipeline will not continue backwards at this position, but will repeat the execution of this key activity unless the **skipKeyActivity** is called. Invalid when called in **Busy** state. API specific to **Manual Key Activity Chain Pipeline**.
- **void skipKeyActivity()**: Skip the key activity to the next activity. API specific to **Manual Steps Chain Pipeline**. Invalid when called in **Busy** state.
---
#### Signals:
- **pipelineStateChanged(TA_BasicPipeline::State)**
    <br/>This signal will be activated when the state of pipeline changed.
- **pipelineReady()**
    <br/>This signal will be activated when the pipeline execution finished.
- **activityCompleted(unsigned int index, TA_Variant var)**
    <br/>The signal will be activated when a single activity completed. _index_ is the index of the activity, and _var_ is the result returned from the activity.
---
Pipeline status description:
- **Waiting**: Pipeline is idle or suspended.
- **Busy**: Pipeline is being executed.
- **Ready**: Pipeline execution completed.
---
***Note: Pipeline Creator has ownership of all pipeline objects. And once an activity is added into a pipeline, its ownership would be transferred to the pipeline and the original activity pointer will be set as nullptr.***
