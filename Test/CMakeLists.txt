cmake_minimum_required(VERSION 3.5)

project(PipelineTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE_DEBUG "Debug")
set(CMAKE_BUILD_TYPE_RELEASE "Release")

include_directories(../TA_ActivityPipeline/Src)
include_directories(./ThirdParty/googletest/googletest/include)

add_executable(PipelineTest PipelineTest/main.cpp
    PipelineTest/TA_ActivityTest.cpp
    PipelineTest/TA_ActivityTest.h
    PipelineTest/TA_PipelineTest.cpp
    PipelineTest/TA_PipelineTest.h
    PipelineTest/TA_ConnectionTest.cpp
    PipelineTest/TA_ConnectionTest.h
    PipelineTest/TA_MetaReflexTest.cpp
    PipelineTest/TA_MetaReflexTest.h
    PipelineTest/main.cpp
    PipelineTest/MetaTest.cpp
    PipelineTest/MetaTest.h
    PipelineTest/TA_ActivityQueueTest.cpp
    PipelineTest/TA_ActivityQueueTest.h
    PipelineTest/TA_ThreadPoolTest.h
    PipelineTest/TA_ThreadPoolTest.cpp
    PipelineTest/TA_CommonToolsTest.h
    PipelineTest/TA_CommonToolsTest.cpp
)

if(MSVC)
    message(STATUS "Configuring for MSVC")
    target_link_libraries(PipelineTest PRIVATE
        $<$<CONFIG:Debug>:AsyncPipeline_d>
        $<$<CONFIG:Release>:AsyncPipeline>
    )
    target_link_libraries(PipelineTest PRIVATE
        $<$<CONFIG:Debug>:gtest>
        $<$<CONFIG:Release>:gtest>
    )
    if(CMAKE_BUILD_TYPE STREQUAL ${CMAKE_BUILD_TYPE_DEBUG})
        target_link_directories(PipelineTest PRIVATE ThirdParty/googletest/output/win32/Debug)
    else()
        target_link_directories(PipelineTest PRIVATE ThirdParty/googletest/output/win32/Release)
    endif()
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
elseif(UNIX)
    message(STATUS "Configuring for GCC")
    target_link_libraries(PipelineTest PRIVATE
        $<$<CONFIG:Debug>:AsyncPipeline>
        $<$<CONFIG:Release>:AsyncPipeline>
    )
    target_link_libraries(PipelineTest PRIVATE
        $<$<CONFIG:Debug>:gtest>
        $<$<CONFIG:Release>:gtest>
    )
    target_link_directories(PipelineTest PRIVATE ThirdParty/googletest/output)
    target_link_directories(PipelineTest PRIVATE ../build/TA_ActivityPipeline/output)
endif()

install(TARGETS PipelineTest
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
