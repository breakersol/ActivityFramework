cmake_minimum_required(VERSION 3.5)

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

include_directories(${PROJECT_SOURCE_DIR}/Src)

add_library(AsyncPipeline SHARED
    Src/TA_ActivityPipeline_global.h
    Src/ITA_PipelineCreator.h
    Src/ITA_PipelineCreator.cpp
    Src/ITA_Connection.h
    Src/ITA_ActivityCreator.h
    Src/Components/TA_ActivityQueue.h
    Src/Components/TA_AutoChainPipeline.cpp
    Src/Components/TA_AutoChainPipeline.h
    Src/Components/TA_BasicActivity.h
    Src/Components/TA_BasicPipeline.cpp
    Src/Components/TA_BasicPipeline.h
    Src/Components/TA_CommonTools.h
    Src/Components/TA_ParallelPipeline.cpp
    Src/Components/TA_ParallelPipeline.h
    Src/Components/TA_PipelineCreator.cpp
    Src/Components/TA_PipelineCreator.h
    Src/Components/TA_PipelineHolder.h
    Src/Components/TA_ReceiverObject.h
    Src/Components/TA_SingleActivity.h
    Src/Components/TA_TypeFilter.h
    Src/Components/TA_TypeList.h
    Src/Components/TA_Variant.cpp
    Src/Components/TA_Variant.h
    Src/Components/TA_Connection.h
    Src/Components/TA_ConnectionUtils.cpp
    Src/Components/TA_ConnectionUtils.h
    Src/Components/TA_LinkedActivity.h
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

)

target_compile_definitions(AsyncPipeline PRIVATE ASYNCPIPELINE_LIBRARY)

