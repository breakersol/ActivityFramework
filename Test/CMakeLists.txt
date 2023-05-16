cmake_minimum_required(VERSION 3.5)

project(PipelineTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../TA_ActivityPipeline/Src)
include_directories(./ThirdParty/googletest/googletest/include)
link_directories(../build/TA_ActivityPipeline/output)

if(MSVC)
    if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        link_directories(ThirdParty/googletest/output/win32/Debug)
        link_libraries(AsyncPipeline_d)
    elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
            link_directories(ThirdParty/googletest/output/win32/Release)
            link_libraries(AsyncPipeline)
    endif()
else()
    link_libraries(AsyncPipeline)
    link_directories(ThirdParty/googletest/output)
endif()

link_libraries(gtest)

add_executable(PipelineTest PipelineTest/main.cpp
    PipelineTest/TA_ActivityTest.cpp
    PipelineTest/TA_ActivityTest.h
    PipelineTest/TA_AutoChainPipelineTest.cpp
    PipelineTest/TA_AutoChainPipelineTest.h
    PipelineTest/TA_BasePipelineTest.cpp
    PipelineTest/TA_BasePipelineTest.h
    PipelineTest/TA_ConnectionTest.cpp
    PipelineTest/TA_ConnectionTest.h
    PipelineTest/TA_ManualChainPipelineTest.cpp
    PipelineTest/TA_ManualChainPipelineTest.h
    PipelineTest/TA_ManualKeyActivityChainPipelineTest.cpp
    PipelineTest/TA_ManualKeyActivityChainPipelineTest.h
    PipelineTest/TA_ManualStepsChainPipelineTest.cpp
    PipelineTest/TA_ManualStepsChainPipelineTest.h
    PipelineTest/TA_MetaReflexTest.cpp
    PipelineTest/TA_MetaReflexTest.h
    PipelineTest/TA_ParallelPipelineTest.cpp
    PipelineTest/TA_ParallelPipelineTest.h
    PipelineTest/main.cpp
    PipelineTest/MetaTest.cpp
    PipelineTest/MetaTest.h
    PipelineTest/TA_ActivityQueueTest.cpp
    PipelineTest/TA_ActivityQueueTest.h

)

install(TARGETS PipelineTest
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
