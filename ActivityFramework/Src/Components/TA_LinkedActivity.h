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

#ifndef TA_LINKEDACTIVITY_H
#define TA_LINKEDACTIVITY_H

#include "TA_SingleActivity.h"
#include "TA_CommonTools.h"

#include <memory>
#include <future>
#include <list>

namespace CoreAsync {
    template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
    class TA_LinkedActivity : public TA_SingleActivity<Fn,Ins,Ret,FuncPara...>
    {
    public:
        TA_LinkedActivity() = delete;
        TA_LinkedActivity(const TA_LinkedActivity &activity) = delete;
        TA_LinkedActivity(TA_LinkedActivity &&activity) = delete;
        TA_LinkedActivity & operator = (const TA_LinkedActivity &) = delete;

        TA_LinkedActivity(Fn &&func, Ins &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<Fn, Ins, Ret, FuncPara...> (std::move(func),ins,para..., affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        TA_LinkedActivity(Fn &&func, Ins &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<Fn, Ins, Ret, FuncPara...> (std::move(func),ins,std::forward<FuncPara>(para)..., affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        virtual ~TA_LinkedActivity()
        {
            for(auto pBranchActivity : m_pBranchList)
            {
                if(pBranchActivity)
                    delete pBranchActivity;
                pBranchActivity = nullptr;
            }
        }

        bool hasBranch() const override
        {
            return !m_pBranchList.empty();
        }

        bool selectBranch(std::deque<unsigned int> branches) override
        {
            if(branches.empty() || m_pBranchList.empty() || branches.front() > m_pBranchList.size())
                return false;
            TA_BasicActivity *pActivity = nullptr;
            m_branchSelected = branches.front();
            if(m_branchSelected == 0)
                return true;
            pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1);
            branches.pop_front();
            pActivity->selectBranch(branches);
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool link(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pLinkedActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool parallel(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pParallelActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template<typename Activity,typename ...Activities>
        void branch(Activity *&activity, Activities *&...activities)
        {
            branch(activity);
            branch(activities...);
        }

        template <typename Activity>
        void branch(Activity *&activity)
        {
            if(!activity)
                return;
            Activity *pActivity = activity;
            activity = nullptr;
            m_pBranchList.push_back(pActivity);
        }

    protected:
        TA_Variant run() override
        {
            if(m_branchSelected == 0)
            {
                using BasicType = TA_SingleActivity<Fn,Ins,Ret,FuncPara...>;
                TA_Variant varNext, varLinked, varParallel, varBasic;
                std::future<TA_Variant> ft;
                if(m_pParallelActivity)
                {
                    ft = std::async(std::launch::async,[&]()-> TA_Variant{return m_pParallelActivity();});
                }
                if(m_branchSelected == 0)
                    varBasic = BasicType::run();
                else
                    varBasic = (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
                if(m_pLinkedActivity)
                {
                    varLinked = m_pLinkedActivity();
                }
                return varLinked.isValid() ? varLinked : ft.valid() ? ft.get() : varBasic;
            }
            return (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
        }

    private:
        std::function<TA_Variant()> m_pLinkedActivity;
        std::function<TA_Variant()> m_pParallelActivity;
        std::list<TA_BasicActivity *> m_pBranchList;
        unsigned int m_branchSelected;

    };

    template <typename Ret,typename... FuncPara>
    class TA_LinkedActivity<NonMemberFunctionPtr<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...> : public TA_SingleActivity<NonMemberFunctionPtr<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>
    {
    public:
        TA_LinkedActivity() = delete;
        TA_LinkedActivity(const TA_LinkedActivity &activity) = delete;
        TA_LinkedActivity(TA_LinkedActivity &&activity) = delete;
        TA_LinkedActivity & operator = (const TA_LinkedActivity &) = delete;

        TA_LinkedActivity(NonMemberFunctionPtr<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<NonMemberFunctionPtr<Ret,FuncPara...>, INVALID_INS, Ret, FuncPara...> (std::move(func),para..., affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        TA_LinkedActivity(NonMemberFunctionPtr<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<NonMemberFunctionPtr<Ret,FuncPara...>, INVALID_INS, Ret, FuncPara...> (std::move(func),std::forward<FuncPara>(para)..., affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        virtual ~TA_LinkedActivity()
        {
            for(auto pBranchActivity : m_pBranchList)
            {
                if(pBranchActivity)
                    delete pBranchActivity;
                pBranchActivity = nullptr;
            }
        }

        bool hasBranch() const override
        {
            return !m_pBranchList.empty();
        }

        bool selectBranch(std::deque<unsigned int> branches) override
        {
            if(branches.empty() || m_pBranchList.empty() || branches.front() > m_pBranchList.size())
                return false;
            TA_BasicActivity *pActivity = nullptr;
            m_branchSelected = branches.front();
            if(m_branchSelected == 0)
                return true;
            pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1);
            branches.pop_front();
            pActivity->selectBranch(branches);
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool link(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pLinkedActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool parallel(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pParallelActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template<typename Activity,typename ...Activities>
        void branch(Activity *&activity, Activities *&...activities)
        {
            branch(activity);
            branch(activities...);
        }

        template <typename Activity>
        void branch(Activity *&activity)
        {
            if(!activity)
                return;
            Activity *pActivity = activity;
            activity = nullptr;
            m_pBranchList.push_back(pActivity);
        }

    protected:
        TA_Variant run() override
        {
            if(m_branchSelected == 0)
            {
                using BasicType = TA_SingleActivity<NonMemberFunctionPtr<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>;
                TA_Variant varNext, varLinked, varParallel, varBasic;
                std::future<TA_Variant> ft;
                if(m_pParallelActivity)
                {
                    ft = std::async(std::launch::async,[&]()-> TA_Variant{return m_pParallelActivity();});
                }
                if(m_branchSelected == 0)
                    varBasic = BasicType::run();
                else
                    varBasic = (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
                if(m_pLinkedActivity)
                {
                    varLinked = m_pLinkedActivity();
                }
                return varLinked.isValid() ? varLinked : ft.valid() ? ft.get() : varBasic;
            }
            return (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
        }

    private:
        std::function<TA_Variant()> m_pLinkedActivity;
        std::function<TA_Variant()> m_pParallelActivity;
        std::list<TA_BasicActivity *> m_pBranchList;
        unsigned int m_branchSelected;

    };

    template <typename Ret,typename... FuncPara>
    class TA_LinkedActivity<LambdaType<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...> : public TA_SingleActivity<LambdaType<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>
    {
    public:
        TA_LinkedActivity() = delete;
        TA_LinkedActivity(const TA_LinkedActivity &activity) = delete;
        TA_LinkedActivity(TA_LinkedActivity &&activity) = delete;
        TA_LinkedActivity & operator = (const TA_LinkedActivity &) = delete;

        TA_LinkedActivity(LambdaType<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<LambdaType<Ret,FuncPara...>, INVALID_INS, Ret, FuncPara...> (std::move(func),para..., affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        TA_LinkedActivity(LambdaType<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<LambdaType<Ret,FuncPara...>, INVALID_INS, Ret, FuncPara...> (std::move(func),std::forward<FuncPara>(para)..., affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        virtual ~TA_LinkedActivity()
        {
            for(auto pBranchActivity : m_pBranchList)
            {
                if(pBranchActivity)
                    delete pBranchActivity;
                pBranchActivity = nullptr;
            }
        }

        bool hasBranch() const override
        {
            return !m_pBranchList.empty();
        }

        bool selectBranch(std::deque<unsigned int> branches) override
        {
            if(branches.empty() || m_pBranchList.empty() || branches.front() > m_pBranchList.size())
                return false;
            TA_BasicActivity *pActivity = nullptr;
            m_branchSelected = branches.front();
            if(m_branchSelected == 0)
                return true;
            pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1);
            branches.pop_front();
            pActivity->selectBranch(branches);
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool link(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pLinkedActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool parallel(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pParallelActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template<typename Activity,typename ...Activities>
        void branch(Activity *&activity, Activities *&...activities)
        {
            branch(activity);
            branch(activities...);
        }

        template <typename Activity>
        void branch(Activity *&activity)
        {
            if(!activity)
                return;
            Activity *pActivity = activity;
            activity = nullptr;
            m_pBranchList.push_back(pActivity);
        }

    protected:
        TA_Variant run() override
        {
            if(m_branchSelected == 0)
            {
                using BasicType = TA_SingleActivity<LambdaType<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>;
                TA_Variant varNext, varLinked, varParallel, varBasic;
                std::future<TA_Variant> ft;
                if(m_pParallelActivity)
                {
                    ft = std::async(std::launch::async,[&]()-> TA_Variant{return m_pParallelActivity();});
                }
                if(m_branchSelected == 0)
                    varBasic = BasicType::run();
                else
                    varBasic = (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
                if(m_pLinkedActivity)
                {
                    varLinked = m_pLinkedActivity();
                }
                return varLinked.isValid() ? varLinked : ft.valid() ? ft.get() : varBasic;
            }
            return (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
        }

    private:
        std::function<TA_Variant()> m_pLinkedActivity;
        std::function<TA_Variant()> m_pParallelActivity;
        std::list<TA_BasicActivity *> m_pBranchList;
        unsigned int m_branchSelected;

    };

    template <typename Ret>
    class TA_LinkedActivity<LambdaTypeWithoutPara<Ret>,INVALID_INS,Ret,INVALID_INS> : public TA_SingleActivity<LambdaTypeWithoutPara<Ret>,INVALID_INS,Ret,INVALID_INS>
    {
    public:
        TA_LinkedActivity() = delete;
        TA_LinkedActivity(const TA_LinkedActivity &activity) = delete;
        TA_LinkedActivity(TA_LinkedActivity &&activity) = delete;
        TA_LinkedActivity & operator = (const TA_LinkedActivity &) = delete;

        TA_LinkedActivity(LambdaTypeWithoutPara<Ret> &&func, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : TA_SingleActivity<LambdaTypeWithoutPara<Ret>, INVALID_INS, Ret, INVALID_INS> (std::move(func), affinityThread),m_pLinkedActivity(nullptr),m_pParallelActivity(nullptr),m_branchSelected(0)
        {

        }

        virtual ~TA_LinkedActivity()
        {
            for(auto pBranchActivity : m_pBranchList)
            {
                if(pBranchActivity)
                    delete pBranchActivity;
                pBranchActivity = nullptr;
            }
        }

        bool hasBranch() const override
        {
            return !m_pBranchList.empty();
        }

        bool selectBranch(std::deque<unsigned int> branches) override
        {
            if(branches.empty() || m_pBranchList.empty() || branches.front() > m_pBranchList.size())
                return false;
            TA_BasicActivity *pActivity = nullptr;
            m_branchSelected = branches.front();
            if(m_branchSelected == 0)
                return true;
            pActivity = TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1);
            branches.pop_front();
            pActivity->selectBranch(branches);
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool link(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pLinkedActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template <typename FnNext,typename InsNext, typename RetNext,typename... FuncParaNext>
        bool parallel(TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> *&pNext)
        {
            if(!pNext)
                return false;
            std::shared_ptr<TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...> > pSharedNext = nullptr;
            pSharedNext.reset(pNext);
            m_pParallelActivity = std::bind(&TA_LinkedActivity<FnNext,InsNext,RetNext,FuncParaNext...>::operator(),pSharedNext);
            pNext = nullptr;
            return true;
        }

        template<typename Activity,typename ...Activities>
        void branch(Activity *&activity, Activities *&...activities)
        {
            branch(activity);
            branch(activities...);
        }

        template <typename Activity>
        void branch(Activity *&activity)
        {
            if(!activity)
                return;
            Activity *pActivity = activity;
            activity = nullptr;
            m_pBranchList.push_back(pActivity);
        }

    protected:
        TA_Variant run() override
        {
            if(m_branchSelected == 0)
            {
                using BasicType = TA_SingleActivity<LambdaTypeWithoutPara<Ret>,INVALID_INS,Ret,INVALID_INS>;
                TA_Variant varNext, varLinked, varParallel, varBasic;
                std::future<TA_Variant> ft;
                if(m_pParallelActivity)
                {
                    ft = std::async(std::launch::async,[&]()-> TA_Variant{return m_pParallelActivity();});
                }
                if(m_branchSelected == 0)
                    varBasic = BasicType::run();
                else
                    varBasic = (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
                if(m_pLinkedActivity)
                {
                    varLinked = m_pLinkedActivity();
                }
                return varLinked.isValid() ? varLinked : ft.valid() ? ft.get() : varBasic;
            }
            return (*TA_CommonTools::at<TA_BasicActivity *>(m_pBranchList, m_branchSelected - 1))();
        }

    private:
        std::function<TA_Variant()> m_pLinkedActivity;
        std::function<TA_Variant()> m_pParallelActivity;
        std::list<TA_BasicActivity *> m_pBranchList;
        unsigned int m_branchSelected;

    };
}

#endif // TA_LINKEDACTIVITY_H
