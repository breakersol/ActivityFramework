cmake_minimum_required(VERSION 3.5)

project(PipelineTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PipelineTest 
    PipelineTest/main.cpp
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

include_directories(../TA_ActivityPipeline/Src)
include_directories(./ThirdParty/googletest/googletest/include)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries(PipelineTest PRIVATE
        $<$<CONFIG:Debug>:AsyncPipeline_d>
        $<$<CONFIG:Release>:AsyncPipeline>
    )
    target_link_libraries(PipelineTest PRIVATE
        $<$<CONFIG:Debug>:gtest>
        $<$<CONFIG:Release>:gtest>
    )
    target_link_directories(PipelineTest PRIVATE ThirdParty/googletest/output/win32)
    target_link_directories(PipelineTest PRIVATE ../build/TA_ActivityPipeline/output)

    set(DLL_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(ASYNC_PIPELINE_DIR ../TA_ActivityPipeline/output)
    set(GTEST_DIR ../../Test/ThirdParty/googletest/output/win32)

    add_custom_command(
        TARGET PipelineTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<IF:$<CONFIG:Release>,${ASYNC_PIPELINE_DIR}/Release/AsyncPipeline.dll,${ASYNC_PIPELINE_DIR}/Debug/AsyncPipeline_d.dll>"
            "$<IF:$<CONFIG:Release>,${DLL_TARGET_DIR}/Release,${DLL_TARGET_DIR}/Debug>"
    )

    add_custom_command(
        TARGET PipelineTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<IF:$<CONFIG:Release>,${GTEST_DIR}/Release/gtest.dll,${GTEST_DIR}/Debug/gtest.dll>"
            "$<IF:$<CONFIG:Release>,${DLL_TARGET_DIR}/Release,${DLL_TARGET_DIR}/Debug>"
    )

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    link_libraries(AsyncPipeline)
    link_directories(ThirdParty/googletest/output)
    link_libraries(gtest)
    link_directories(../build/TA_ActivityPipeline/output)
endif()

install(TARGETS PipelineTest
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
