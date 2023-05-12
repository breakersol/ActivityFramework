#ifndef TA_MANUALCHAINPIPELINE_H
#define TA_MANUALCHAINPIPELINE_H

#include "TA_BasicPipeline.h"

namespace CoreAsync {
    class TA_ManualChainPipeline : public TA_BasicPipeline
    {
    public:
        explicit ASYNC_PIPELINE_EXPORT TA_ManualChainPipeline();
        virtual ~TA_ManualChainPipeline(){}

        TA_ManualChainPipeline(const TA_ManualChainPipeline &activity) = delete;
        TA_ManualChainPipeline(TA_ManualChainPipeline &&activity) = delete;
        TA_ManualChainPipeline & operator = (const TA_ManualChainPipeline &) = delete;

        int currentIndex() const {return m_currentIndex.load(std::memory_order_acquire);}

        virtual bool remove(ActivityIndex index) override final;
        virtual void clear() override;
        virtual void reset() override;

        virtual bool waitingComplete() override final;

        void setStartIndex(unsigned int index) override;

    protected:
        virtual void run() override;

    protected:
        std::atomic<int> m_currentIndex {0};

    };

    namespace Reflex
    {
        template <>
        struct ASYNC_PIPELINE_EXPORT TA_TypeInfo<TA_ManualChainPipeline> : TA_MetaTypeInfo<TA_ManualChainPipeline,TA_BasicPipeline>
        {
            static constexpr TA_MetaFieldList fields = {
                TA_MetaField {&Raw::currentIndex, META_STRING("currentIndex")},
                TA_MetaField {&Raw::remove, META_STRING("remove")},
                TA_MetaField {&Raw::clear, META_STRING("clear")},
                TA_MetaField {&Raw::reset, META_STRING("reset")},
                TA_MetaField {&Raw::waitingComplete, META_STRING("waitingComplete")},
                TA_MetaField {&Raw::setStartIndex, META_STRING("setStartIndex")},
            };
        };
    }
}

#endif // TA_MANUALCHAINPIPELINE_H
