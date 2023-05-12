#ifndef TA_ACTIVITYPIPELINE_GLOBAL_H
#define TA_ACTIVITYPIPELINE_GLOBAL_H

#pragma warning(disable: 4251)

#if defined( _WIN32 )
#if defined( ASYNCPIPELINE_LIBRARY )
#define ASYNC_PIPELINE_EXPORT __declspec(dllexport)
#else
#define ASYNC_PIPELINE_EXPORT __declspec(dllimport)
#endif
#else
#define ASYNC_PIPELINE_EXPORT
#endif

#endif // TA_ACTIVITYPIPELINE_GLOBAL_H
