/*
 * Copyright [2024] [Shuang Zhu / Sol]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TA_ACTIVITYFRAMEWORK_GLOBAL_H
#define TA_ACTIVITYFRAMEWORK_GLOBAL_H

#pragma warning(disable: 4251)

#if defined( _WIN32 )
#if defined( ACTIVITY_FRAMEWORK_LIBRARY )
#define ACTIVITY_FRAMEWORK_EXPORT __declspec(dllexport)
#else
#define ACTIVITY_FRAMEWORK_EXPORT __declspec(dllimport)
#endif
#else
#define ACTIVITY_FRAMEWORK_EXPORT
#endif

#endif // TA_ACTIVITYFRAMEWORK_GLOBAL_H
