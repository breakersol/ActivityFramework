# Activity Pipeline
The project focuses on providing a multi-functional pipeline supporting both synchronous and asynchronous processes with functions as the basic task unit. In addition to pipeline function, compile-time reflection, a simple Qt-like signal and slot mechanism, and some other related features have been implemented. The Activity Pipeline is currently supported on Windows and Linux, and I hope more people will suggest and help improve it!
## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.
### Prerequisites
- CMake: Not less than 3.14
- Windows: MSVC 17.5.33530.505 or higher
- Linux: GCC11 or higher
### Installing
Using **/ActivityPipeline/CMakeList.txt** to build the project ActivityPipeline, and set the build dir as **/ActivityPipeline/build**.
### Running the tests
1. Build the Google Test under **ActivityPipeline/Test/hirdParty/googletest/**.
2. Using **ActivityPipeline/Test/CMakeList.txt** to unit test project.
3. Running PipelineTest.
### Versioning

### Authors
- **Sol** - Initial work - [breakersol](https://github.com/breakersol) E-mail:breakersol@outlook.com
### License
This project is licensed under the Apache-2.0 license License - see the [LICENSE.md](https://github.com/breakersol/ActivityPipeline/blob/master/LICENSE) file for details
### Basic Usage
#### Meta Reflex
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

class MetaTest
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
};

namespace CoreAsync::Reflex {

template <>
struct TA_TypeInfo<BaseTest> : TA_MetaTypeInfo<BaseTest>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::sub, META_STRING("sub")},
    };
};

template <>
struct TA_TypeInfo<OtherTest> : TA_MetaTypeInfo<OtherTest>
{
    static constexpr TA_MetaFieldList fields = {
        TA_MetaField {&Raw::product, META_STRING("product")},
    };
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
        TA_MetaField {&Raw::xx, META_STRING("xx")},
        TA_MetaField {&Raw::getStr, META_STRING("getStr")},
        TA_MetaField {&Raw::startTest, META_STRING("startTest")},
        TA_MetaField {&Raw::printTest, META_STRING("printTest")}
    };
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
#### Connection
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

namespace CoreAsync::Reflex 
{
    template <>
    struct TA_TypeInfo<MetaTest> : TA_MetaTypeInfo<MetaTest>
    {
        static constexpr TA_MetaFieldList fields = {
            TA_MetaField {&Raw::printSlot, META_STRING("printSlot")},
            TA_MetaField {&Raw::printNums, META_STRING("printNums")},
            TA_MetaField {&Raw::startTest, META_STRING("startTest")},
            TA_MetaField {&Raw::printTest, META_STRING("printTest")}
        };
    };
}
```
<br/>From the code above, you can see there are two signal function _void startTest(int,int)_ and _void printTest()_ have been defined, and they have been filled into the type info. Note that the signal function does not require any implementation, and its return type must be **void**.

<br/>Next, you can use the interface **ITA_Connection::connect** as follow:
```cpp
    CoreAsync::ITA_Connection::connect(pTest, &MetaTest::startTest, pTest, &MetaTest::printNums);
    CoreAsync::ITA_Connection::connect<CoreAsync::TA_ConnectionType::Async>(pTest, &MetaTest::printTest, pTest, &MetaTest::printSlot);
```
<br/>**ITA_Connection::connect** requires the same number of arguments for both of sender function and receiver function.

<br/>Connection currently offers only two types of connections: 
```cpp
    enum class TA_ConnectionType
    {
        Async,
        Sync
    };
```
<br/>The default connection type is _Sync_, which means the receiver function won't be executed immediately, but added into the activity queue to wait for call. _Async_ means that the activity will be executed immediately in another thread.

<br/>If you'd like to active the signal function by calling **ITA_Connection::active**. 
```cpp
    CoreAsync::ITA_Connection::active(pTest,&MetaTest::startTest,2,3);
```
Finally, if the connection is unnecessary anymore, you can **disconnect** it.
```cpp
    CoreAsync::ITA_Connection::disconnect(pTest, &MetaTest::printTest, ppTest, &MetaTest::printSlot);
```
#### Activity
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
    auto var = activity_1->execute();
```
<br/>When the execution of activity_1 is finished, activity_2 will be executed automatically, and the result of activity_2 will be returned.

<br/>2. Make two activities execute in parallel.
```cpp
    std::function<int()> func_1 = [&]()->int {return m_pTest->sub(5,5);};
    auto activity_1 = CoreAsync::ITA_ActivityCreator::create(func_1);
    std::function<int()> func_2 = [&]()->int {return m_pTest->sub(8,1);};
    auto activity_2 = CoreAsync::ITA_ActivityCreator::create(func_2);
    activity_1->parallel(activity_2);
    auto var = activity_1->execute();
    auto res = var.get<int>();
```
<br/>The execution of activity_1 will execute actvity_2 in parallel and finally return the result of activtiy_2.

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
    auto var = activity_1->execute();
    auto res = var.get<std::string>();
```
<br/>The above code indicates that we first create two branches activity_4 and activity_5 on activity_3. At this layer, activity_3 represents branch index 0, while activity_4 and activity_5 represent 1 and 2, respectively. Similarly, we created the branch on activity_1 activity_2 and activity_3. **selectBranch({2,2})** means that when activity_1 is executed(activity_1->execute()), it will be executed in the order from activity_3 to activity_5 and return the result of activity_5. And activity_1 itself will not be executed.

#### Pipelines
Activity Pipeline currently offers five types of pipelines to use: **Auto Chain Pipeline, Parallel Pipeline, Manual Chain Pipeline, Manual Steps Chain Pipeline, and Manual Key Activity Chain Pipeline**, and all types of pipelines can be created through **ITA_PipelineCreator**.
1. _Auto Chain Pipeline_: The pipeline will automatically execute all activites in order.
2. _Parallel Pipeline_: All the activities in this pipeline will be executed in parallel.
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
##### Signals:
- **pipelineStateChanged(PipelineState state)**
    <br/>This signal will be activated when the state of pipeline changed.
- **pipelineReady()**
    <br/>This signal will be activated when the pipeline execution finished.
- **activityCompleted(unsigned int index)**
    <br/>The signal will be activated when a single activity completed.
---
Pipeline status description:
- **Waiting**: Pipeline is idle or suspended.
- **Busy**: Pipeline is being executed.
- **Ready**: Pipeline execution completed.
---
***Note: Pipeline Creator has ownership of all pipeline objects. And once an activity is added into a pipeline, its ownership would be transferred to the pipeline and the original activity pointer will be set as nullptr.***
