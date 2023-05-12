cmake_minimum_required(VERSION 3.14)

project(AsyncPipeline LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/build/debug)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/build/release)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "_d")
endif()

include_directories(${PROJECT_SOURCE_DIR}/TA_ActivityPipeline)

add_library(AsyncPipeline SHARED
    TA_ActivityPipeline/TA_ActivityPipeline_global.h
    TA_ActivityPipeline/ITA_PipelineCreator.h
    TA_ActivityPipeline/ITA_PipelineCreator.cpp
    TA_ActivityPipeline/ITA_Connection.h
    TA_ActivityPipeline/ITA_ActivityCreator.h
    TA_ActivityPipeline/Components/TA_ActivityQueue.h
    TA_ActivityPipeline/Components/TA_AutoChainPipeline.cpp
    TA_ActivityPipeline/Components/TA_AutoChainPipeline.h
    TA_ActivityPipeline/Components/TA_BasicActivity.h
    TA_ActivityPipeline/Components/TA_BasicPipeline.cpp
    TA_ActivityPipeline/Components/TA_BasicPipeline.h
    TA_ActivityPipeline/Components/TA_CommonTools.h
    TA_ActivityPipeline/Components/TA_ParallelPipeline.cpp
    TA_ActivityPipeline/Components/TA_ParallelPipeline.h
    TA_ActivityPipeline/Components/TA_PipelineCreator.cpp
    TA_ActivityPipeline/Components/TA_PipelineCreator.h
    TA_ActivityPipeline/Components/TA_PipelineHolder.h
    TA_ActivityPipeline/Components/TA_ReceiverObject.h
    TA_ActivityPipeline/Components/TA_SingleActivity.h
    TA_ActivityPipeline/Components/TA_TypeFilter.h
    TA_ActivityPipeline/Components/TA_TypeList.h
    TA_ActivityPipeline/Components/TA_Variant.cpp
    TA_ActivityPipeline/Components/TA_Variant.h
    TA_ActivityPipeline/Components/TA_Connection.h
    TA_ActivityPipeline/Components/TA_ConnectionUtils.cpp
    TA_ActivityPipeline/Components/TA_ConnectionUtils.h
    TA_ActivityPipeline/Components/TA_LinkedActivity.h
    TA_ActivityPipeline/Components/TA_ManualChainPipeline.cpp
    TA_ActivityPipeline/Components/TA_ManualChainPipeline.h
    TA_ActivityPipeline/Components/TA_ManualKeyActivityChainPipeline.cpp
    TA_ActivityPipeline/Components/TA_ManualKeyActivityChainPipeline.h
    TA_ActivityPipeline/Components/TA_ManualStepsChainPipeline.cpp
    TA_ActivityPipeline/Components/TA_ManualStepsChainPipeline.h
    TA_ActivityPipeline/Components/TA_MetaObject.cpp
    TA_ActivityPipeline/Components/TA_MetaObject.h
    TA_ActivityPipeline/Components/TA_MetaReflex.h
    TA_ActivityPipeline/Components/TA_MetaStringView.h

)

target_compile_definitions(AsyncPipeline PRIVATE ASYNCPIPELINE_LIBRARY)

