# Activity Framework
![Linux](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml?branch=master&label=Linux&logo=linux&logoColor=white)
![Windows](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml?branch=master&label=Windows&logo=windows&logoColor=white)
![Android](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml?branch=master&label=Android&logo=android&logoColor=white)

Activity Framework is a C++23 toolkit for composing reflective, asynchronous work: a lock-free task queue, work-stealing thread pool, pipelines, coroutine helpers, and a Qt-like signal/slot system built on top of a compile-time reflection layer.

## Highlights
- Compile-time reflection (members, overloads, enums) that powers dynamic invoke by name and metadata-driven serialization.
- Thread-aware meta-object system with Qt-style signals/slots, queued/direct delivery, and safe cross-thread invocation.
- Activity/ActivityProxy wrappers with affinity-aware scheduling, result fetchers, and a lock-free circular queue underneath a platform-specific thread pool.
- Pipeline orchestrators (auto, concurrent, manual, stepped, key-activity) with progress signals and cached results.
- Serialization with endian conversion, property versioning, and deep support for STL containers, pointers, enums, and inheritance.
- Coroutine helpers (manual/eager tasks, generators, activity and signal awaitables) that bridge to the Activity Framework's threading model.
- Small-object optimized variant type used to move results across activities, pipelines, and coroutines.

## Supported Platforms
- Windows (MSVC 17.5+)
- Linux (GCC 13+ or recent Clang)
- Android (arm64-v8a and x86_64 via CMake presets; validated in CI with NDK 27.0.12077973)

Depends only on the C++ standard library; GoogleTest is bundled for optional unit tests.

## Build & Test
### Prerequisites
- CMake 3.20+
- C++23 toolchain (MSVC 17.5+, GCC 13+, or Clang with coroutines enabled)
- Ninja (optional, used by Android presets)
- Android: set `ANDROID_NDK_HOME` (r26d/26.2+ works; CI uses 27.0.12077973)
- Tests: build GoogleTest once under `Test/ThirdParty/googletest` (its output tree is used by the test CMakeLists)

### Configure & Build (desktop)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
On Windows (multi-config):
```bash
cmake -S . -B build-windows -G "Visual Studio 17 2022" -A x64
cmake --build build-windows --config Release -- /m
```
Outputs are placed under `build/ActivityFramework/output` (or the VS configuration directory).

### Configuration Options
- `ACTIVITYFRAMEWORK_BUILD_TESTS` (default: `ON`): build the unit tests. Android arm64 presets turn this off because GoogleTest binaries are not produced there; enable it after providing GoogleTest outputs if you need on-device tests.

### Running the Tests
```bash
cmake --build build --config Release --target ActivityFrameworkTest
ctest --test-dir build -C Release --output-on-failure
```
Executables live under `build/Test/ActivityFrameworkTest` (or `build-windows/Test/Release/ActivityFrameworkTest.exe`).

### Android Builds
Use the supplied presets in `CMakePresets.json`:
```bash
cmake --preset android-arm64-release
cmake --build --preset android-arm64-release

cmake --preset android-x86_64-debug   # runs with tests if GoogleTest is built for x86_64
cmake --build --preset android-x86_64-debug
```
Artifacts land in `build/android-*/<config>/ActivityFramework/output/`.

## Core Components & Usage
### Reflection (TA_MetaReflex)
Register metadata once, then query or invoke by name. Private members require `ENABLE_REFLEX` on the class.
```cpp
#include "Components/TA_MetaReflex.h"

class MetaTest {
    ENABLE_REFLEX
public:
    int sub(int a, int b) { return a - b; }
    static std::string getStr(const std::string &s) { return "123" + s; }
};

DEFINE_TYPE_INFO(MetaTest){
    AUTO_META_FIELDS(
        REGISTER_FIELD(sub),
        REGISTER_FIELD(getStr)
    )
};

MetaTest test;
std::string_view name = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::findName(&MetaTest::sub); // "sub"
auto res = CoreAsync::Reflex::TA_TypeInfo<MetaTest>::invoke(META_STRING("getStr"), "abc");
```
`TA_TypeInfo` can also map enums, static data members, overloads (`REGISTER_METHOD_OVERLOAD_GENERIC`), and base-class members.

### Serialization & Versioning
Properties opt-in via metadata; versions gate reads/writes automatically and endian conversion is handled for primitive types.
```cpp
class Data {
    ENABLE_REFLEX
public:
    int value{42};
    std::vector<int> nums{1,2,3};
};
DEFINE_TYPE_INFO(Data){
    AUTO_META_FIELDS(
        REGISTER_FIELD(value, TA_PROPERTY(2)),  // serialized when version >= 2
        REGISTER_FIELD(nums, TA_DEFAULT_PROPERTY)
    )
};

Data in, out;
CoreAsync::TA_Serializer writer("data.afw", 2);
writer << in;
CoreAsync::TA_Serializer<CoreAsync::BufferReader> reader("data.afw", 2);
reader >> out;
```
Supported: STL containers/adaptors, arrays, enums, pointers (to serializable types), inheritance hierarchies, and custom types with metadata.

### Meta-Object, Signals/Slots, and Dynamic Invoke
`TA_MetaObject` provides thread-aware connections and an `invokeMethod` helper to run member functions or callables (optionally queued on the framework thread pool).
```cpp
class Sender : public CoreAsync::TA_MetaObject {
public:
    TA_Signals: void fired(int) {}
};
class Receiver : public CoreAsync::TA_MetaObject {
public:
    void onFired(int v) { std::printf("%d\n", v); }
};
DEFINE_TYPE_INFO(Sender){AUTO_META_FIELDS(REGISTER_FIELD(fired))};
DEFINE_TYPE_INFO(Receiver){AUTO_META_FIELDS(REGISTER_FIELD(onFired))};

Sender s; Receiver r;
CoreAsync::ITA_Connection::connect(&s, &Sender::fired, &r, &Receiver::onFired); // auto/queued based on threads
CoreAsync::TA_MetaObject::invokeMethod(META_STRING("fired"), &s, 5)();            // returns an activity fetcher
```
Connections can be direct, queued, or auto; lambdas are supported; `TA_SignalAwaitable` lets coroutines wait a signal emission.

### Activities, Thread Pool, and Variants
Activities wrap callables or reflected method names. They carry thread affinity, an ID, and a `stolenEnabled` flag for work stealing. `TA_ThreadPool` (and the singleton `TA_ThreadHolder`) schedule them on a lock-free queue with platform-specific threads.
```cpp
CoreAsync::TA_ThreadHolder::create(); // ensure global pool exists
auto activity = CoreAsync::TA_ActivityCreator::create(&MetaTest::sub, &test, 7, 2);
auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(activity, true);
int result = fetcher().get<int>();
```
Results travel as `TA_DefaultVariant` (small-object optimized, smart pointer backed for larger types). `TA_ActivityFetcherAwaitable` and `TA_ActivityExecutingAwaitable` bridge activities to coroutines.

### Pipelines
Create pipelines through `ITA_PipelineCreator`:
- Auto chain: run activities in order.
- Concurrent: run all activities in parallel.
- Manual chain: advance one activity per `execute()` call.
- Manual steps: run a fixed step count per call.
- Manual key-activity: repeat a marked key activity until skipped.

APIs: `add/remove/clear`, `execute(Async|Sync)`, `result(index, out)`, `reset`, `setSteps`, `setKeyActivityIndex`, `skipKeyActivity`. Signals: `pipelineStateChanged`, `pipelineReady`, and `activityCompleted` (index + `TA_Variant` result).

### Coroutine Utilities
`TA_ManualCoroutineTask` (lazy/eager) and `TA_CoroutineGenerator` wrap standard coroutines with blocking `get()`/`value()` helpers. Awaitables include waiting on signals, executing activities (sync/async), and fetching results, letting coroutine code integrate with the ActivityFramework scheduler.

## Releases
- [v0.5.0](https://github.com/breakersol/ActivityFramework/releases/tag/v0.5.0)
- [v0.4.1](https://github.com/breakersol/ActivityFramework/releases/tag/v0.4.1)
- [v0.4.0](https://github.com/breakersol/ActivityFramework/releases/tag/v0.4.0)

## Authors
- Sol Zhu — breakersol@outlook.com

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for how to propose changes or improvements.

## License
Licensed under the Apache-2.0 License. See [LICENSE](LICENSE) for details.
