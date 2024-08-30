cmake_minimum_required(VERSION 3.16)

project(Benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

include_directories(../ActivityFramework/Src)
include_directories(../../benchmark-1.9.0/include ../../benchmark-1.9.0/src)

add_executable(Benchmark main.cpp)

target_link_directories(Benchmark PRIVATE ../../benchmark-1.9.0/build/src/Release)

target_link_libraries(Benchmark PRIVATE benchmark)
target_link_libraries(Benchmark PRIVATE benchmark_main)
target_link_libraries(Benchmark PRIVATE Shlwapi.lib)

target_compile_definitions(Benchmark PRIVATE BENCHMARK_STATIC_DEFINE)

include(GNUInstallDirs)
install(TARGETS Benchmark
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
