cmake_minimum_required(VERSION 3.20)

# Set the project name
project(ActivityFrameworkProject LANGUAGES CXX)

# Set the C++ standard and its requirement
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(_default_build_tests ON)
if(ANDROID)
    set(_default_build_tests ON)
endif()
option(ACTIVITYFRAMEWORK_BUILD_TESTS "Build Activity Framework unit tests" ${_default_build_tests})

# Compile the AsyncPipeline library
add_subdirectory(./ActivityFramework)

if(ACTIVITYFRAMEWORK_BUILD_TESTS)
    add_subdirectory(./Test/)
    add_dependencies(ActivityFrameworkTest ActivityFramework)
endif()
