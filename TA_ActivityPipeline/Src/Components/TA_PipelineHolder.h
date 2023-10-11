/*
 * Copyright [2023] [Shuang Zhu / Sol]
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

#ifndef TA_PIPELINEHOLDER_H
#define TA_PIPELINEHOLDER_H

#include "TA_AutoChainPipeline.h"
#include "TA_ManualChainPipeline.h"
#include "TA_ParallelPipeline.h"
#include "TA_ManualStepsChainPipeline.h"
#include "TA_ManualKeyActivityChainPipeline.h"
#include "TA_TypeList.h"
#include "TA_MetaReflex.h"

namespace CoreAsync {

    using Pipelines = TA_MetaTypelist<TA_AutoChainPipeline,TA_ManualChainPipeline,TA_ParallelPipeline,TA_ManualStepsChainPipeline,TA_ManualKeyActivityChainPipeline>;

    template <typename Holder>
    class ASYNC_PIPELINE_EXPORT TA_MainPipelineHolder : public TA_MetaObject
    {
        friend class TA_PipelineCreator;
    public:
        virtual ~TA_MainPipelineHolder()
        {
            destroy();
        }

        void execute(TA_BasicPipeline::ExecuteType type = TA_BasicPipeline::ExecuteType::Async)
        {
            m_pBasicPipeline->execute(type);
        }

        void reset()
        {
            m_pBasicPipeline->reset();
        }

        void clear()
        {
            m_pBasicPipeline->clear();
        }

        bool remove(unsigned int index)
        {
            return m_pBasicPipeline->remove(index);
        }

        bool waitingComplete()
        {
            return m_pBasicPipeline->waitingComplete();
        }

        void setStartIndex(unsigned int index)
        {
            m_pBasicPipeline->setStartIndex(index);
        }

        std::size_t activitySize() const
        {
            return m_pBasicPipeline->activitySize();
        }

        void receivePipelineState(TA_BasicPipeline::State st)
        {
            TA_CommonTools::debugInfo(META_STRING("Receive pipeline state %d\n"),st);
            TA_Connection::active(this, &TA_MainPipelineHolder::pipelineStateChanged, st);
            if(st == TA_BasicPipeline::State::Ready)
            {
                TA_Connection::active(this, &TA_MainPipelineHolder::pipelineReady);
            }
        }

        TA_BasicPipeline::State state() const
        {
            return m_pBasicPipeline->state();
        }

        bool switchActivityBranch(int activityIndex, std::deque<unsigned int> branches)
        {
            return m_pBasicPipeline->switchActivityBranch(activityIndex,branches);
        }

        template <typename Activity, typename ...Activities>
        void add(Activity &activity, Activities &...activites)
        {
            m_pBasicPipeline->add<Activity,Activities...>(activity,activites...);
        }

        template <typename Res>
        bool result(int index,Res &res)
        {
            return m_pBasicPipeline->result(index,res);
        }

        void setSteps(unsigned int steps)
        {
            static_cast<Holder *>(this)->setSteps(steps);
        }

        void setKeyActivityIndex(int index)
        {
            static_cast<Holder *>(this)->setKeyActivityIndex(index);
        }

        void skipKeyActivity()
        {
            static_cast<Holder *>(this)->skipKeyActivity();
        }

    protected:
        template <typename Pip>
        Pip * raw() const
        {
            Pip *pObj = dynamic_cast<Pip *>(m_pBasicPipeline);
            return pObj;
        }

    TA_Signals:
        void pipelineStateChanged(TA_BasicPipeline::State state) { std::ignore = state; };
        void pipelineReady() {}
        void activityCompleted(unsigned int index, TA_Variant var) {
            TA_CommonTools::debugInfo(META_STRING("Activity completed: %d\n"),index);
        };

    protected:
        explicit TA_MainPipelineHolder(TA_BasicPipeline *pLine = nullptr) : TA_MetaObject(), m_pBasicPipeline(pLine)
        {
            if(m_pBasicPipeline)
            {
                TA_Connection::connect(m_pBasicPipeline,&TA_BasicPipeline::stateChanged,this,&TA_MainPipelineHolder<Holder>::receivePipelineState);
                TA_Connection::connect(m_pBasicPipeline,&TA_BasicPipeline::activityCompleted,this,&TA_MainPipelineHolder<Holder>::activityCompleted);
            }
        }

    private:
        void destroy()
        {
            static_cast<Holder *>(this)->destroy();
        }

    private:
        TA_BasicPipeline *m_pBasicPipeline;

    };

    template <typename P>
    concept ValidHolder = MetaContains<Pipelines, std::decay_t<P> >::value && !std::is_pointer_v<P>;

    template <typename P>
    concept EnableHolderType = requires (P p)
    {
        {p}->ValidHolder;
    };

    template <EnableHolderType Pip>
    class ASYNC_PIPELINE_EXPORT TA_PipelineHolder : public TA_MainPipelineHolder<TA_PipelineHolder<Pip> >
    {
    public:
        TA_PipelineHolder() : TA_MainPipelineHolder<TA_PipelineHolder<Pip> >(new std::decay_t<Pip>()), m_pPipeline(TA_MainPipelineHolder<TA_PipelineHolder<Pip> >::template raw<std::decay_t<Pip> >())
        {

        }

        void setSteps(unsigned int steps)
        {
            static_assert(std::is_same_v<std::decay_t<Pip>, TA_ManualStepsChainPipeline>, "This kind of pipeline doesn't support steps setting.");
            m_pPipeline->setSteps(steps);
        }

        void setKeyActivityIndex(int index)
        {
            static_assert(std::is_same_v<std::decay_t<Pip>, TA_ManualKeyActivityChainPipeline>, "This kind of pipeline doesn't support key activity setting.");
            m_pPipeline->setKeyActivity(index);
        }

        void skipKeyActivity()
        {
            static_assert(std::is_same_v<std::decay_t<Pip>, TA_ManualKeyActivityChainPipeline>, "This kind of pipeline doesn't support skipping key activity function.");
            m_pPipeline->skipKeyActivity();
        }

        void destroy()
        {
            if(m_pPipeline)
            {
                delete m_pPipeline;
                m_pPipeline = nullptr;
            }
        }

    private:
        Pip *m_pPipeline;
    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> > > : TA_MetaTypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> >>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::switchActivityBranch, META_STRING("switchActivityBranch")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::execute, META_STRING("execute")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::activitySize, META_STRING("activitySize")},
                TA_MetaField {&Raw::state, META_STRING("state")},
                TA_MetaField {&Raw::receivePipelineState, META_STRING("receivePipelineState")},
                TA_MetaField {&Raw::pipelineStateChanged, META_STRING("pipelineStateChanged")},
                TA_MetaField {&Raw::pipelineReady, META_STRING("pipelineReady")},
                TA_MetaField {&Raw::activityCompleted, META_STRING("activityCompleted")}
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_PipelineHolder<TA_AutoChainPipeline> > : TA_MetaTypeInfo<TA_PipelineHolder<TA_AutoChainPipeline>, TA_MainPipelineHolder<TA_PipelineHolder<TA_AutoChainPipeline> > >
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::destroy, META_STRING("destroy")},
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ParallelPipeline> > > : TA_MetaTypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ParallelPipeline> >>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::switchActivityBranch, META_STRING("switchActivityBranch")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::execute, META_STRING("execute")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::activitySize, META_STRING("activitySize")},
                TA_MetaField {&Raw::state, META_STRING("state")},
                TA_MetaField {&Raw::receivePipelineState, META_STRING("receivePipelineState")},
                TA_MetaField {&Raw::pipelineStateChanged, META_STRING("pipelineStateChanged")},
                TA_MetaField {&Raw::pipelineReady, META_STRING("pipelineReady")},
                TA_MetaField {&Raw::activityCompleted, META_STRING("activityCompleted")}
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_PipelineHolder<TA_ParallelPipeline> > : TA_MetaTypeInfo<TA_PipelineHolder<TA_ParallelPipeline>, TA_MainPipelineHolder<TA_PipelineHolder<TA_ParallelPipeline> > >
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::destroy, META_STRING("destroy")},
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> > > : TA_MetaTypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> >>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::switchActivityBranch, META_STRING("switchActivityBranch")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::execute, META_STRING("execute")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::activitySize, META_STRING("activitySize")},
                TA_MetaField {&Raw::state, META_STRING("state")},
                TA_MetaField {&Raw::receivePipelineState, META_STRING("receivePipelineState")},
                TA_MetaField {&Raw::pipelineStateChanged, META_STRING("pipelineStateChanged")},
                TA_MetaField {&Raw::pipelineReady, META_STRING("pipelineReady")},
                TA_MetaField {&Raw::activityCompleted, META_STRING("activityCompleted")}
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_PipelineHolder<TA_ManualChainPipeline> > : TA_MetaTypeInfo<TA_PipelineHolder<TA_ManualChainPipeline>, TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualChainPipeline> > >
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::destroy, META_STRING("destroy")},
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> > > : TA_MetaTypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> >>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::switchActivityBranch, META_STRING("switchActivityBranch")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::execute, META_STRING("execute")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::activitySize, META_STRING("activitySize")},
                TA_MetaField {&Raw::state, META_STRING("state")},
                TA_MetaField {&Raw::receivePipelineState, META_STRING("receivePipelineState")},
                TA_MetaField {&Raw::setSteps, META_STRING("setSteps")},
                TA_MetaField {&Raw::pipelineStateChanged, META_STRING("pipelineStateChanged")},
                TA_MetaField {&Raw::pipelineReady, META_STRING("pipelineReady")},
                TA_MetaField {&Raw::activityCompleted, META_STRING("activityCompleted")}
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_PipelineHolder<TA_ManualStepsChainPipeline> > : TA_MetaTypeInfo<TA_PipelineHolder<TA_ManualStepsChainPipeline>, TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualStepsChainPipeline> > >
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::destroy, META_STRING("destroy")},
                TA_MetaField {&Raw::setSteps, META_STRING("setSteps")}
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> > > : TA_MetaTypeInfo<TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> >>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
                TA_MetaField {&Raw::switchActivityBranch, META_STRING("switchActivityBranch")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::execute, META_STRING("execute")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::activitySize, META_STRING("activitySize")},
                TA_MetaField {&Raw::state, META_STRING("state")},
                TA_MetaField {&Raw::receivePipelineState, META_STRING("receivePipelineState")},
                TA_MetaField {&Raw::setKeyActivityIndex, META_STRING("setKeyActivityIndex")},
                TA_MetaField {&Raw::skipKeyActivity, META_STRING("skipKeyAcitivty")},
                TA_MetaField {&Raw::pipelineStateChanged, META_STRING("pipelineStateChanged")},
                TA_MetaField {&Raw::pipelineReady, META_STRING("pipelineReady")},
                TA_MetaField {&Raw::activityCompleted, META_STRING("activityCompleted")}
            };
        };

        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> > : TA_MetaTypeInfo<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline>, TA_MainPipelineHolder<TA_PipelineHolder<TA_ManualKeyActivityChainPipeline> > >
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::destroy, META_STRING("destroy")},
                TA_MetaField {&Raw::setKeyActivityIndex, META_STRING("setKeyActivityIndex")},
                TA_MetaField {&Raw::skipKeyActivity, META_STRING("skipKeyActivity")}
            };
        };
    }
}

#endif // TA_PIPELINEHOLDER_H
