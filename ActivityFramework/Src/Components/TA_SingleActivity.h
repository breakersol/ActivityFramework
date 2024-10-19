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

#ifndef TA_SINGLEACTIVITY_H
#define TA_SINGLEACTIVITY_H

#include <functional>
#include <mutex>

#include "TA_TypeFilter.h"
#include "TA_Variant.h"
#include "TA_ActivityComponments.h"

namespace CoreAsync {
    template <typename T>
    using EnableRightReference = std::enable_if_t<std::is_rvalue_reference_v<std::decay_t<T>>, std::decay_t<T>>;

    template <typename Ret,typename ...FuncPara>
    using NonMemberFunctionPtr = Ret(*)(FuncPara...);

    template <typename Ret,typename ...FuncPara>
    using LambdaType = std::function<Ret(FuncPara...)>;

    template <typename Ret>
    using LambdaTypeWithoutPara = std::function<Ret()>;

    template <typename Fn>
    using SupportMemberFunction = typename std::enable_if<std::is_member_function_pointer<Fn>::value,Fn>::type;

    template <typename Fn>
    using SupportNonMemberFunction = typename std::enable_if<!std::is_member_function_pointer<Fn>::value,Fn>::type;

    struct INVALID_INS {};

    template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
    class TA_SingleActivity
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) :  m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&ins,func,&para...]()->Ret{return (ins.*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->Ret{return (m_object.*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->Ret{return (m_object.*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<Ins>(m_object);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        Ins &m_object;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};

    };

    template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
    class TA_SingleActivity<Fn,Ins *,Ret,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&ins,func,&para...]()->Ret{return (ins->*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->Ret{return (m_object->*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->Ret{return (m_object->*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::decay_t<Ins> *>(const_cast<std::decay_t<Ins> *>(m_object));
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        Ins *&m_object;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Fn,typename Ins,typename... FuncPara>
    class TA_SingleActivity<Fn,Ins,void,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {   
            m_funcActivity = [&ins,func,&para...]()->void{(ins.*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->void{(m_object.*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->void{(m_object.*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<Ins>(m_object);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        Ins &m_object;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Fn,typename Ins,typename... FuncPara>
    class TA_SingleActivity<Fn,Ins *,void,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&ins,func,&para...]()->void{(ins->*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->void{(m_object->*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->void{(m_object->*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::decay_t<Ins> *>(const_cast<std::decay_t<Ins> *>(m_object));
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        Ins *&m_object;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
    class TA_SingleActivity<Fn,std::shared_ptr<Ins>,Ret,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&ins,func,&para...]()->Ret{return (ins.get()->*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->Ret{return (m_object.get()->*m_funcPtr)(para...);});
        }


        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->Ret{return (m_object.get()->*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<Ins *>(m_object.get());
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        std::shared_ptr<Ins> &m_object;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Fn,typename Ins,typename... FuncPara>
    class TA_SingleActivity<Fn,std::shared_ptr<Ins>,void,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&ins,func,&para...]()->void{(ins.get()->*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->void{(m_object.get()->*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->void{(m_object.get()->*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<Ins *>(m_object.get());
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        std::shared_ptr<Ins> &m_object;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
    class TA_SingleActivity<Fn,std::unique_ptr<Ins>,Ret,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
        {
            m_funcActivity = [this,func,&para...]()->Ret{return (m_pObject->*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),m_pObject,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {
            if(m_pObject)
            {
                delete m_pObject;
                m_pObject = nullptr;
            }
        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->Ret{return (m_pObject->*m_funcPtr)(para...);});
        }


        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->Ret{return (m_pObject->*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<Ins *>(m_pObject);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        Ins *m_pObject;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Fn,typename Ins,typename... FuncPara>
    class TA_SingleActivity<Fn,std::unique_ptr<Ins>,void,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
        {
            m_funcActivity = [this,func,&para...]()->void{return (m_pObject->*func)(para...);};
        }

        TA_SingleActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),m_pObject,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {
            if(m_pObject)
            {
                delete m_pObject;
                m_pObject = nullptr;
            }
        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->void{return (m_pObject->*m_funcPtr)(para...);});
        }


        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->void{return (m_pObject->*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<Ins *>(m_pObject);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {      
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        Fn &m_funcPtr;
        Ins *m_pObject;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Ret,typename... FuncPara>
    class TA_SingleActivity<NonMemberFunctionPtr<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(NonMemberFunctionPtr<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = [func,&para...]()->Ret{return (*func)(para...);};
        }

        TA_SingleActivity(NonMemberFunctionPtr<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,&para...]()->Ret{return (*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap(std::bind(m_funcPtr,std::forward<FuncPara>(para)...));
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::nullptr_t>(nullptr);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        NonMemberFunctionPtr<Ret,FuncPara...> &m_funcPtr;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename... FuncPara>
    class TA_SingleActivity<NonMemberFunctionPtr<void,FuncPara...>,INVALID_INS,void,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(NonMemberFunctionPtr<void,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = [func, &para...]()->void{(func)(para...);};
        }

        TA_SingleActivity(NonMemberFunctionPtr<void,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,&para...]()->void{(*m_funcPtr)(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([this,para...]()->void{(*m_funcPtr)(para...);});
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::nullptr_t>(nullptr);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        NonMemberFunctionPtr<void,FuncPara...> &m_funcPtr;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Ret,typename... FuncPara>
    class TA_SingleActivity<LambdaType<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(LambdaType<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&,func]()->Ret{return func(para...);};
        }

        TA_SingleActivity(LambdaType<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(std::forward<decltype(func)>(func),std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->Ret{return m_funcPtr(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap(std::bind(m_funcPtr,std::forward<FuncPara>(para)...));
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::nullptr_t>(nullptr);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        LambdaType<Ret,FuncPara...> m_funcPtr;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename... FuncPara>
    class TA_SingleActivity<LambdaType<void,FuncPara...>,INVALID_INS,void,FuncPara...>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(LambdaType<void,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&,func]()->void{func(para...);};
        }

        TA_SingleActivity(LambdaType<void,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
        {
            m_funcActivity = std::bind(func,std::forward<FuncPara>(para)...);
        }

        virtual ~TA_SingleActivity()
        {

        }

        template <typename ...NewPara>
        void setPara(NewPara &...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap([&]()->void{return m_funcPtr(para...);});
        }

        template <typename ...NewPara>
        void setPara(NewPara &&...para)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity.swap(std::bind(m_funcPtr,std::forward<FuncPara>(para)...));
        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::nullptr_t>(nullptr);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        std::function<void(FuncPara...)> m_funcPtr;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <typename Ret>
    class TA_SingleActivity<LambdaTypeWithoutPara<Ret>,INVALID_INS,Ret,INVALID_INS>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(LambdaTypeWithoutPara<Ret> &&func, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&,func]()->Ret{return func();};
        }

        virtual ~TA_SingleActivity()
        {

        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::nullptr_t>(nullptr);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            TA_Variant var;
            var.template set<Ret>(m_funcActivity());
            return var;
        }

    protected:
        std::function<Ret()> m_funcActivity;
        std::mutex m_mutex;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

    template <>
    class TA_SingleActivity<LambdaTypeWithoutPara<void>,INVALID_INS,void,INVALID_INS>
    {
    public:
        TA_SingleActivity() = delete;
        TA_SingleActivity(const TA_SingleActivity &activity) = delete;
        TA_SingleActivity(TA_SingleActivity &&activity) = delete;
        TA_SingleActivity & operator = (const TA_SingleActivity &) = delete;

        TA_SingleActivity(LambdaTypeWithoutPara<void> &&func, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr), m_affinityThread(affinityThread)
        {
            m_funcActivity = [&,func]()->void{func();};
        }

        virtual ~TA_SingleActivity()
        {

        }

        TA_Variant operator()()
        {
            return run();
        }

        TA_Variant caller() const
        {
            TA_Variant caller;
            caller.template set<std::nullptr_t>(nullptr);
            return caller;
        }

        std::size_t affinityThread() const
        {
            return m_affinityThread.affinityThread();
        }

        bool moveToThread(std::size_t thread)
        {
            return m_affinityThread.moveToThread(thread);
        }

        std::int64_t id() const
        {
            return m_id.id();
        }

    protected:
        virtual TA_Variant run()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_funcActivity();
            TA_Variant var;
            var.template set<std::nullptr_t>(nullptr);
            return var;
        }

    protected:
        std::function<void()> m_funcActivity;
        std::mutex m_mutex;

        TA_ActivityAffinityThread m_affinityThread {};
        TA_ActivityId m_id {};
    };

}

#endif // TA_SingleActivity_H
