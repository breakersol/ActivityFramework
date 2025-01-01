cmake_minimum_required(VERSION 3.20)

# Set the project name
project(ActivityFrameworkProject LANGUAGES CXX)

# Set the C++ standard and its requirement
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compile the AsyncPipeline library
add_subdirectory(./ActivityFramework)

# Compile the PipelineTest project
add_subdirectory(./Test/)

add_dependencies(ActivityFrameworkTest ActivityFramework)
