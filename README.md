# Activity Framework
![Linux](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml?branch=master&label=Linux&logo=linux&logoColor=white)
![Windows](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml?branch=master&label=Windows&logo=windows&logoColor=white)
![Android](https://img.shields.io/github/actions/workflow/status/breakersol/ActivityFramework/cmake.yml?branch=master&label=Android&logo=android&logoColor=white)

Activity Framework is a C++23 toolkit for composing reflective, asynchronous work: bounded task queues, a work-stealing thread pool, pipelines, coroutine helpers, and a Qt-like signal/slot system built on top of a compile-time reflection layer.

## Highlights

- Compile-time reflection (members, overloads, enums, inheritance) with lookup by name or value, indexed metadata access, and metadata-driven serialization.
- Thread-aware meta-object system with Qt-style signals/slots, queued/direct delivery, and asynchronous method invocation.
- Activity/ActivityProxy wrappers with affinity-aware scheduling, lifecycle states, result fetchers, and atomic circular queue storage.
- Pipeline orchestrators (auto, concurrent, manual, stepped, key-activity) with progress signals and cached results.
- AFWS revision 3 binary serialization with endian conversion, property versioning, strings, STL containers, nullable raw pointers, and cyclic/shared meta-object graphs.
- Process-local `std::uint64_t` identities for meta-objects and activities through `TA_UniqueId`.
- Coroutine helpers (manual/eager tasks, generators, activity and signal awaitables) that bridge to the Activity Framework's threading model.
- Small-object optimized variant type used to move results across activities, pipelines, and coroutines.

## Platforms and Dependencies

- Windows: MSVC; the checked-in CI workflow uses a Visual Studio 2026 runner.
- Linux: GCC; the checked-in CI workflow uses GCC 14 on Ubuntu 24.04.
- Android: `arm64-v8a` and `x86_64` presets targeting API 24; the checked-in CI workflow builds `x86_64` with NDK `30.0.14904198`. Its emulator test step is currently disabled.

The shared library uses the C++ standard library and the platform thread library (`Threads::Threads`); it does not require Qt. GoogleTest sources and platform-specific test libraries are bundled under `Test/ThirdParty/googletest`.

## Build & Test

### Prerequisites

- CMake 3.20+ for direct configuration, or **3.24+ for the supplied presets** (preset schema version 5).
- A C++23 compiler and standard library with coroutines, ranges, semaphores, and threading support.
- Ninja for all supplied Windows and Android presets.
- Windows presets: an x64 MSVC developer environment with `cl.exe` and the MSVC `link.exe` on `PATH`.
- Android presets: set `ANDROID_NDK_HOME` to the NDK directory containing `build/cmake/android.toolchain.cmake`.

### Build the Library

For a desktop build without the test dependency:

```bash
cmake -S . -B build/desktop-release -DCMAKE_BUILD_TYPE=Release -DACTIVITYFRAMEWORK_BUILD_TESTS=OFF
cmake --build build/desktop-release --config Release --parallel
```

The target is `ActivityFramework`, built as a shared library. For a build directory `<build-dir>`, library artifacts are placed under `<build-dir>/ActivityFramework/output/`; multi-configuration generators add a configuration subdirectory. MSVC Debug library names have the `_d` suffix.

### Windows Presets and IDE Tasks

From an x64 MSVC developer shell:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --test-dir build/windows-msvc/debug --output-on-failure

cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --test-dir build/windows-msvc/release --output-on-failure
```

The PowerShell helper used by the Windows tasks locates Visual Studio, CMake, and Ninja and loads the developer environment automatically:

```powershell
./.vscode/scripts/cmake-msvc.ps1 configure Debug
./.vscode/scripts/cmake-msvc.ps1 build Debug
./.vscode/scripts/cmake-msvc.ps1 test Debug
```

Use `Release` in place of `Debug` for a release build. The test action runs `ActivityFrameworkTest.exe` directly. If an existing build cache selects a different toolchain's linker, reconfigure with `CMAKE_LINKER` pointing to the MSVC `link.exe`.

### Linux Build and Tests

There are currently no Linux presets in `CMakePresets.json`; the Linux tasks in `.vscode/tasks.json` refer to undefined presets. Configure directly, as in CI:

```bash
cmake -S . -B build/linux-release -G Ninja -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux-release --parallel
ctest --test-dir build/linux-release --output-on-failure
```

### Test Configuration

`ACTIVITYFRAMEWORK_BUILD_TESTS` defaults to `ON`. Tests link against GoogleTest libraries in the following source-tree directories; GoogleTest is not built automatically by the root project:

| Platform | Expected GoogleTest output directory |
| --- | --- |
| Windows Debug | `Test/ThirdParty/googletest/output/Windows/Debug` |
| Windows Release | `Test/ThirdParty/googletest/output/Windows/Release` |
| Linux | `Test/ThirdParty/googletest/output/Linux` |
| Android arm64 | `Test/ThirdParty/googletest/output/Android_arm64` |
| Android x86_64 | `Test/ThirdParty/googletest/output/Android_x86_64` |

Use binaries compatible with your compiler, ABI, and configuration. If needed, build the bundled GoogleTest sources separately and place the outputs in the matching directory. On Windows, the test build also expects `gtest.dll` there and copies it and the framework DLL beside the test executable.

For any configured desktop build with tests enabled:

```bash
cmake --build <build-dir> --config Release --target ActivityFrameworkTest
ctest --test-dir <build-dir> -C Release --output-on-failure
```

The executable is `<build-dir>/Test/ActivityFrameworkTest` (`.exe` on Windows), with an extra configuration directory for multi-configuration generators. CTest registers one test that runs the complete GoogleTest executable.

### Android Builds

Use the supplied configure and build presets:

```bash
cmake --preset android-arm64-release
cmake --build --preset android-arm64-release

cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug
```

Both ABIs also have the other configuration (`android-arm64-debug` and `android-x86_64-release`). The arm64 presets disable tests; the x86_64 presets enable them. Override `ACTIVITYFRAMEWORK_BUILD_TESTS` at configure time if needed. These presets select the static C++ runtime.

Artifacts land in `build/android-arm64-v8a/<config>/ActivityFramework/output/` or `build/android-x86_64/<config>/ActivityFramework/output/`. Android test binaries must run on a matching device or emulator with the framework shared library available; building an Android test executable does not run it on the host.

### Use from Another CMake Project

The current source tree provides a CMake target, but no installed package configuration or public include-directory propagation. Add the library subdirectory and headers explicitly:

```cmake
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(path/to/ActivityFramework/ActivityFramework activityframework-build)
target_include_directories(my_app PRIVATE path/to/ActivityFramework/ActivityFramework/Src)
target_link_libraries(my_app PRIVATE ActivityFramework)
```

Make the framework shared library available at runtime. Public headers are under `ActivityFramework/Src`, primarily in `Components/`, and APIs use the `CoreAsync` namespace.

## Core Components & Usage

The examples below show class/metadata declarations at namespace scope and usage statements inside a function. Include the headers shown in each example and link against `ActivityFramework`.

### Reflection (TA_MetaReflex)

Register metadata once, then query or invoke by name. Private members require `ENABLE_REFLEX` on the class.

```cpp
#include <string>
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
using MetaInfo = CoreAsync::Reflex::TA_TypeInfo<MetaTest>;
constexpr auto name = MetaInfo::findName<&MetaTest::sub>(); // "sub"
constexpr auto index = MetaInfo::valueIndex<&MetaTest::sub>();
static_assert(MetaInfo::nameAt<index>() == "sub");
auto res = MetaInfo::invoke(META_STRING("getStr"), std::string{"abc"});
```

`TA_TypeInfo` also supports runtime-value lookup with `findName(value)`, name lookup with `nameIndex`, indexed access with `valueAt`/`nameValueAt`, enums, static data members, overloads (`REGISTER_METHOD_OVERLOAD_GENERIC`), and registered base-class members.

### Serialization & Versioning

Properties opt in through metadata; a property's version determines whether it participates in the file's schema version.

```cpp
#include <cstdint>
#include <vector>
#include "Components/TA_Serialization.h"

class Data {
    ENABLE_REFLEX
public:
    std::int32_t value{42};
    std::vector<std::int32_t> nums{1, 2, 3};
};
DEFINE_TYPE_INFO(Data){
    AUTO_META_FIELDS(
        REGISTER_FIELD(value, TA_PROPERTY(2)), // participates in schema version >= 2
        REGISTER_FIELD(nums, TA_DEFAULT_PROPERTY)
    )
};

Data in, out;
CoreAsync::TA_Serializer<> writer("data.afw", 2);
writer << in;
writer.close(); // report flush/close errors before opening the reader
CoreAsync::TA_Serializer<CoreAsync::BufferReader> reader("data.afw", 2);
reader >> out;
```

Supported values include arithmetic types, enums, `std::string`, supported STL containers/adaptors, pairs, arrays, raw pointers, and reflected types with eligible properties. Strings store a 64-bit byte count followed by exactly those bytes, preserving embedded nulls without a terminator or encoding conversion. Raw pointers use a presence marker, including null pointers and `nullptr` literals.

The [binary format specification](docs/serialization-format.md) defines the 16-byte AFWS header, **format revision 3**, big-endian fields, 64-bit counts, graph storage rules, and schema compatibility. Use fixed-width integer fields and explicit enum underlying types for portable schemas; native-width types such as `long` and `size_t` retain their platform-dependent widths.

The writer's version selects the file schema. The reader's version is its maximum supported schema version (default `1`); `version()` reports the actual file version. Skipped properties retain their existing values. Preserve existing property order and encodings when adding higher-version properties. Revision 1/2 files and older native-header files require migration; there is no automatic fallback.

Call `close()` before reporting a successful save. Decoding is not transactional: failures can leave partial changes, and the serializer should be discarded after a decoding error.

### Graph Serialization and Object Identity

Reflected classes publicly derived from `TA_MetaObject` support cycles and shared references through raw pointers. Each node's first occurrence writes its source ID and properties; later occurrences write only its ID. The reader registers nodes before reading their properties and maps file references to destination objects, preserving their runtime identities.

```cpp
#include <cassert>
#include <cstdint>
#include "Components/TA_Serialization.h"

struct Node : CoreAsync::TA_MetaObject {
    std::uint64_t value{7};
    Node *next{};
};
DEFINE_TYPE_INFO(Node){
    AUTO_META_FIELDS(
        REGISTER_FIELD(value, TA_DEFAULT_PROPERTY),
        REGISTER_FIELD(next, TA_DEFAULT_PROPERTY)
    )
};

Node source;
source.next = &source;
CoreAsync::TA_Serializer<> graphWriter("graph.afw");
graphWriter << &source;
graphWriter.close();

Node restored;
Node *root = &restored; // supply stable destination storage
CoreAsync::TA_Serializer<CoreAsync::BufferReader> graphReader("graph.afw");
graphReader >> root;
assert(root == &restored && root->next == root);
assert(root->id() != source.id());
```

The graph cache is non-owning. A null destination pointer may cause allocation with `new`; callers must release each allocated node exactly once, including partially decoded nodes after errors. Reads can rebind pointers or set them to null without deleting previous pointees. Keep registered objects alive at stable addresses for the reader's lifetime. Graph values are decoded in final storage and are rejected in associative keys/set elements and adaptors; use graph-node pointers in those positions. See the format specification for value-ordering and container restrictions.

`TA_UniqueId` supplies nonzero, process-local `std::uint64_t` IDs from a shared atomic allocator. Meta-objects and activities use the same allocator, while each instance has its own identity. Meta-object copy/move construction allocates a fresh ID; assignment preserves the destination ID. `TA_UniqueId` itself is neither copyable nor movable. IDs are not persistent: saved graph IDs identify references within a file and are never assigned back to runtime objects.

### Meta-Object, Signals/Slots, and Dynamic Invoke

Connection-capable classes inherit `TA_MetaObjectStorage<Class>` and register their signals and slots. Emit through `ITA_Connection::active`; calling a signal's empty member body directly does not dispatch connections.

```cpp
#include <cstdio>
#include "ITA_Connection.h"
#include "Components/TA_MetaObject.h"

class Sender : public CoreAsync::TA_MetaObjectStorage<Sender> {
public:
    TA_Signals: void fired(int) {}
};
class Receiver : public CoreAsync::TA_MetaObjectStorage<Receiver> {
public:
    void onFired(int v) { std::printf("%d\n", v); }
};
DEFINE_TYPE_INFO(Sender){AUTO_META_FIELDS(REGISTER_FIELD(fired))};
DEFINE_TYPE_INFO(Receiver){AUTO_META_FIELDS(REGISTER_FIELD(onFired))};

Sender s;
Receiver r;
CoreAsync::ITA_Connection::connect<&Sender::fired, &Receiver::onFired,
                                  CoreAsync::TA_ConnectionType::Direct>(&s, &r);
CoreAsync::ITA_Connection::active<&Sender::fired>(&s, 5);
CoreAsync::ITA_Connection::disconnect<&Sender::fired, &Receiver::onFired>(&s, &r);

auto invocation = CoreAsync::TA_MetaObject::invokeMethod(META_STRING("onFired"), &r, 6);
invocation(); // wait for the posted method invocation
```

Registered signals and slots use dense per-member buckets. The compile-time overload resolves bucket indices without a runtime metadata scan or hash lookup; pointer-based overloads remain available for dynamically selected members. Connections support `Direct`, `Queued`, and `Auto` delivery, plus lambda receivers. Keep objects and captured state alive until queued work completes. `TA_SignalAwaitable` lets coroutines wait for a signal emission.

### Activities, Thread Pool, and Variants

Activities wrap callables or reflected method names and expose thread affinity, an ID, and a `stolenEnabled` flag. `TA_ThreadPool` and the global `TA_ThreadHolder` schedule them using bounded activity queues and work stealing. Desktop workers use `std::jthread`; Android workers use `std::thread` with explicit stop state.

```cpp
#include "Components/TA_ThreadPool.h"

CoreAsync::TA_ThreadHolder::create(); // optionally initialize the global pool explicitly
auto activity = CoreAsync::TA_ActivityCreator::create([](int a, int b) { return a - b; }, 7, 2);
auto fetcher = CoreAsync::TA_ThreadHolder::get().postActivity(activity, true);
int result = fetcher().get<int>(); // 5; true transfers activity cleanup to the proxy
```

`moveToThread(index)` chooses a worker affinity; disable stealing with `setStolenEnabled(false)` when work must remain on that worker. Null activities, unavailable submissions, and queue insertion failures are reported with exceptions. Results travel as `TA_DefaultVariant` (small-object storage with smart-pointer storage for larger values). `TA_ActivityFetcherAwaitable` and `TA_ActivityExecutingAwaitable` bridge activities to coroutines.

Activities expose `TA_ActivityState::Configuring`, `Queued`, `Running`, and `Completed`. Configure affinity, arguments, and stealing before submission; configuration changes return `false` once the activity leaves `Configuring`. Activities execute once, and duplicate submissions are rejected. Queue stealing selects only eligible activities, leaving non-stealable work on its owning worker.

### Pipelines

Create pipeline holders through `ITA_PipelineCreator`:

| Factory | Execution model |
| --- | --- |
| `createAutoChainPipeline()` | Run activities in order |
| `createConcurrentPipeline()` | Run activities concurrently |
| `createManualChainPipeline()` | Advance one activity per execution |
| `createManualStepsChainPipeline()` | Advance a configured number of activities |
| `createManualKeyActivityChainPipeline()` | Repeat a marked key activity until skipped |

Common APIs include `add`, `remove`, `clear`, `execute`, `result(index, out)`, `reset`, and `setStartIndex`. `execute` accepts `TA_BasicPipeline::ExecuteType::Async` or `Sync` and returns a callable waiter. `add` takes activity pointer variables and clears them as ownership transfers to the pipeline. Configure the corresponding manual holder with `setSteps`, `setKeyActivityIndex`, or `skipKeyActivity`.

Holder signals are `pipelineStateChanged`, `pipelineReady`, and `activityCompleted` (activity index and `TA_DefaultVariant` result). Pipeline states are `Waiting`, `Busy`, and `Ready`.

### Coroutine Utilities

`TA_ManualCoroutineTask<T, Lazy/Eager>` and `TA_CoroutineGenerator<T, Lazy/Eager>` wrap standard coroutines with `get()`/`value()` helpers. `TA_AutoCoroutineTask` provides automatically managed coroutine execution. Awaitables include waiting on signals, executing activities synchronously or asynchronously, and fetching results, integrating coroutine code with the framework scheduler.

## Repository Layout

- `ActivityFramework/Src/`: library headers and implementations.
- `Test/ActivityFrameworkTest/`: GoogleTest coverage for reflection, connections, activities, queues, pipelines, serialization, variants, coroutines, and threading.
- `Test/ThirdParty/googletest/`: bundled GoogleTest sources and platform outputs.
- `docs/serialization-format.md`: binary format and graph ownership contract.
- `Benchmark/`: separate benchmark project using bundled Google Benchmark; it is not part of the root CMake build and currently has Windows-specific library paths.
- `.vscode/`: IDE tasks and the Windows MSVC build helper.
- `.github/workflows/cmake.yml`: desktop build/test jobs and the Android build job.

## Releases

- [v0.6.0](https://github.com/breakersol/ActivityFramework/releases/tag/v0.6.0)
- [v0.5.1](https://github.com/breakersol/ActivityFramework/releases/tag/v0.5.1)
- [v0.5.0](https://github.com/breakersol/ActivityFramework/releases/tag/v0.5.0)

## Authors

- Sol Zhu — breakersol@outlook.com

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for how to propose changes or improvements.

## License

Licensed under the Apache-2.0 License. See [LICENSE](LICENSE) for details.
