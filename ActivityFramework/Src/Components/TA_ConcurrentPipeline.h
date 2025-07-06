/*
 * Copyright [2025] [Shuang Zhu / Sol]
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

#ifndef TA_CONCURRENTPIPELINE_H
#define TA_CONCURRENTPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
class ACTIVITY_FRAMEWORK_EXPORT TA_ConcurrentPipeline : public TA_BasicPipeline {
  public:
    TA_ConcurrentPipeline();
    virtual ~TA_ConcurrentPipeline() {}

    TA_ConcurrentPipeline(const TA_ConcurrentPipeline &activity) = delete;
    TA_ConcurrentPipeline(TA_ConcurrentPipeline &&activity) = delete;
    TA_ConcurrentPipeline &operator=(const TA_ConcurrentPipeline &) = delete;

    void clear() override final;
    void reset() override final;

  protected:
    void run() override final;
};

namespace Reflex {
template <>
struct ACTIVITY_FRAMEWORK_EXPORT
    TA_TypeInfo<TA_ConcurrentPipeline> : TA_MetaTypeInfo<TA_ConcurrentPipeline, TA_BasicPipeline> {
    static constexpr TA_MetaFieldList fields = {};
};
} // namespace Reflex
} // namespace CoreAsync

#endif // TA_CONCURRENTPIPELINE_H
