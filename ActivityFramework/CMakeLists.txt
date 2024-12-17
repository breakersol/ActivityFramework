cmake_minimum_required(VERSION 3.5)

project(ActivityFramework LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/build/debug)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/build/release)
set(CMAKE_BUILD_TYPE_DEBUG "Debug")
set(CMAKE_BUILD_TYPE_RELEASE "Release")

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "_d")
    if(CMAKE_BUILD_TYPE STREQUAL ${CMAKE_BUILD_TYPE_DEBUG})
        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build/Debug)
        set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
    else()
        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build/Release)
        set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
    endif()
elseif(UNIX)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
endif()

include_directories(${PROJECT_SOURCE_DIR}/Src)

add_library(ActivityFramework SHARED
    Src/TA_ActivityFramework_global.h
    Src/ITA_PipelineCreator.h
    Src/ITA_Connection.h
    Src/ITA_ActivityCreator.h
    Src/Components/TA_ActivityQueue.h
    Src/Components/TA_AutoChainPipeline.cpp
    Src/Components/TA_AutoChainPipeline.h
    Src/Components/TA_BasicPipeline.cpp
    Src/Components/TA_BasicPipeline.h
    Src/Components/TA_CommonTools.h
    Src/Components/TA_ConcurrentPipeline.cpp
    Src/Components/TA_ConcurrentPipeline.h
    Src/Components/TA_PipelineCreator.cpp
    Src/Components/TA_PipelineCreator.h
    Src/Components/TA_PipelineHolder.h
    Src/Components/TA_ReceiverObject.h
    Src/Components/TA_SingleActivity.h
    Src/Components/TA_TypeFilter.h
    Src/Components/TA_TypeList.h
    Src/Components/TA_Variant.h
    Src/Components/TA_Connection.h
    Src/Components/TA_ConnectionUtils.cpp
    Src/Components/TA_ConnectionUtils.h
    Src/Components/TA_ManualChainPipeline.cpp
    Src/Components/TA_ManualChainPipeline.h
    Src/Components/TA_ManualKeyActivityChainPipeline.cpp
    Src/Components/TA_ManualKeyActivityChainPipeline.h
    Src/Components/TA_ManualStepsChainPipeline.cpp
    Src/Components/TA_ManualStepsChainPipeline.h
    Src/Components/TA_MetaObject.cpp
    Src/Components/TA_MetaObject.h
    Src/Components/TA_MetaReflex.h
    Src/Components/TA_MetaStringView.h
    Src/Components/TA_ThreadPool.h
    Src/Components/TA_CommonTools.cpp
    Src/Components/TA_EndianConversion.h
    Src/Components/TA_Serialization.h
    Src/Components/TA_Buffer.h
    Src/Components/TA_MetaActivity.h
    Src/Components/TA_ActivityComponments.h
    Src/Components/TA_ActivityProxy.h

)

target_compile_definitions(ActivityFramework PRIVATE ACTIVITY_FRAMEWORK_LIBRARY)
# target_compile_definitions(ActivityFramework PRIVATE DEBUG_INFO_ON)
