cmake_minimum_required(VERSION 3.20)

project(ActivityFrameworkTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE_DEBUG "Debug")
set(CMAKE_BUILD_TYPE_RELEASE "Release")

include_directories(../ActivityFramework/Src)
include_directories(./ThirdParty/googletest/googletest/include)

add_executable(ActivityFrameworkTest ActivityFrameworkTest/main.cpp
    ActivityFrameworkTest/TA_ActivityTest.cpp
    ActivityFrameworkTest/TA_ActivityTest.h
    ActivityFrameworkTest/TA_PipelineTest.cpp
    ActivityFrameworkTest/TA_PipelineTest.h
    ActivityFrameworkTest/TA_ConnectionTest.cpp
    ActivityFrameworkTest/TA_ConnectionTest.h
    ActivityFrameworkTest/TA_MetaReflexTest.cpp
    ActivityFrameworkTest/TA_MetaReflexTest.h
    ActivityFrameworkTest/main.cpp
    ActivityFrameworkTest/MetaTest.cpp
    ActivityFrameworkTest/MetaTest.h
    ActivityFrameworkTest/TA_ActivityQueueTest.cpp
    ActivityFrameworkTest/TA_ActivityQueueTest.h
    ActivityFrameworkTest/TA_ThreadPoolTest.h
    ActivityFrameworkTest/TA_ThreadPoolTest.cpp
    ActivityFrameworkTest/TA_CommonToolsTest.h
    ActivityFrameworkTest/TA_CommonToolsTest.cpp
    ActivityFrameworkTest/TA_EndianSwapTest.h
    ActivityFrameworkTest/TA_EndianSwapTest.cpp
    ActivityFrameworkTest/TA_SerializationTest.h
    ActivityFrameworkTest/TA_SerializationTest.cpp
    ActivityFrameworkTest/TA_ActivityProxyTest.h ActivityFrameworkTest/TA_ActivityProxyTest.cpp
    ActivityFrameworkTest/TA_VariantTest.h ActivityFrameworkTest/TA_VariantTest.cpp
)

if(MSVC)
    message(STATUS "Configuring for MSVC")
    target_link_libraries(ActivityFrameworkTest PRIVATE
        $<$<CONFIG:Debug>:ActivityFramework_d>
        $<$<CONFIG:Release>:ActivityFramework>
    )
    target_link_libraries(ActivityFrameworkTest PRIVATE
        $<$<CONFIG:Debug>:gtest>
        $<$<CONFIG:Release>:gtest>
    )
    if(CMAKE_BUILD_TYPE STREQUAL ${CMAKE_BUILD_TYPE_DEBUG})
        target_link_directories(ActivityFrameworkTest PRIVATE ThirdParty/googletest/output/win32/Debug)
        target_link_directories(ActivityFrameworkTest PRIVATE ../build/Debug/ActivityFramework/output)
    else()
        target_link_directories(ActivityFrameworkTest PRIVATE ThirdParty/googletest/output/win32/Release)
        target_link_directories(ActivityFrameworkTest PRIVATE ../build/Release/ActivityFramework/output)
    endif()

    set(DLL_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(ACTIVITY_FRAMEWORK_DIR ../ActivityFramework/output)
    set(GTEST_DIR ../../../Test/ThirdParty/googletest/output/win32)

    add_custom_command(
        TARGET ActivityFrameworkTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<IF:$<CONFIG:Release>,${ACTIVITY_FRAMEWORK_DIR}/ActivityFramework.dll,${ACTIVITY_FRAMEWORK_DIR}/ActivityFramework_d.dll>"
            "$<IF:$<CONFIG:Release>,${DLL_TARGET_DIR},${DLL_TARGET_DIR}>"
    )

    add_custom_command(
        TARGET ActivityFrameworkTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<IF:$<CONFIG:Release>,${GTEST_DIR}/Release/gtest.dll,${GTEST_DIR}/Debug/gtest.dll>"
            "$<IF:$<CONFIG:Release>,${DLL_TARGET_DIR},${DLL_TARGET_DIR}>"
    )
elseif(UNIX)
    message(STATUS "Configuring for GCC")
    target_link_libraries(ActivityFrameworkTest PRIVATE
        $<$<CONFIG:Debug>:ActivityFramework>
        $<$<CONFIG:Release>:ActivityFramework>
    )
    target_link_libraries(ActivityFrameworkTest PRIVATE
        $<$<CONFIG:Debug>:gtest>
        $<$<CONFIG:Release>:gtest>
    )
    target_link_directories(ActivityFrameworkTest PRIVATE ThirdParty/googletest/output)
    target_link_directories(ActivityFrameworkTest PRIVATE ../build/ActivityFramework/output)
endif()

install(TARGETS ActivityFrameworkTest
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
