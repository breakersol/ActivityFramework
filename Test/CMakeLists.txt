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
    ActivityFrameworkTest/TA_CoroutineTest.h ActivityFrameworkTest/TA_CoroutineTest.cpp
    ActivityFrameworkTest/TA_MetaObjectTest.h  ActivityFrameworkTest/TA_MetaObjectTest.cpp
)

if(MSVC)
    message(STATUS "Configuring for MSVC")
    target_link_libraries(ActivityFrameworkTest PRIVATE ActivityFramework gtest)

    target_link_directories(ActivityFrameworkTest PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest/output/Windows/Debug>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest/output/Windows/Release>
    )

    set(DLL_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(GTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest/output/Windows)

    add_custom_command(
        TARGET ActivityFrameworkTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:ActivityFramework>
            "${DLL_TARGET_DIR}"
    )

    add_custom_command(
        TARGET ActivityFrameworkTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<IF:$<CONFIG:Release>,${GTEST_DIR}/Release/gtest.dll,${GTEST_DIR}/Debug/gtest.dll>"
            "${DLL_TARGET_DIR}"
    )
elseif(ANDROID)
    message(STATUS "Configuring for Android")
    target_link_libraries(ActivityFrameworkTest PRIVATE ActivityFramework gtest)
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        target_link_directories(ActivityFrameworkTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest/output/Android_arm64)
    else()
        target_link_directories(ActivityFrameworkTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest/output/Android_x86_64)
    endif()
elseif(UNIX)
    message(STATUS "Configuring for GCC")
    target_link_libraries(ActivityFrameworkTest PRIVATE ActivityFramework gtest)
    target_link_directories(ActivityFrameworkTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest/output/Linux)
endif()

install(TARGETS ActivityFrameworkTest
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
