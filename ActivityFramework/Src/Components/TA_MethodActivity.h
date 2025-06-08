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

#ifndef TA_METHODACTIVITY_H
#define TA_METHODACTIVITY_H

// #include <functional>

// #include "TA_TypeFilter.h"
// #include "TA_ActivityComponments.h"

// namespace CoreAsync {
//     template <typename T>
//     using EnableRightReference = std::enable_if_t<std::is_rvalue_reference_v<std::decay_t<T>>, std::decay_t<T>>;

//     template <typename Ret,typename ...FuncPara>
//     using NonMemberFunctionPtr = Ret(*)(FuncPara...);

//     template <typename Ret,typename ...FuncPara>
//     using LambdaType = std::function<Ret(FuncPara...)>;

//     template <typename Ret>
//     using LambdaTypeWithoutPara = std::function<Ret()>;

//     template <typename Fn>
//     using SupportMemberFunction = typename std::enable_if<std::is_member_function_pointer<Fn>::value,Fn>::type;

//     template <typename Fn>
//     using SupportNonMemberFunction = typename std::enable_if<!std::is_member_function_pointer<Fn>::value,Fn>::type;

//     struct INVALID_INS {};

//     template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
//     class TA_MethodActivity
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) :  m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&ins,func,&para...]()->Ret{return (ins.*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->Ret{return (m_object.*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->Ret{return (m_object.*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_object;
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         Fn &m_funcPtr;
//         Ins &m_object;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};

//     };

//     template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
//     class TA_MethodActivity<Fn,Ins *,Ret,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&ins,func,&para...]()->Ret{return (ins->*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->Ret{return (m_object->*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->Ret{return (m_object->*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_object;
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         Fn &m_funcPtr;
//         Ins *&m_object;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Fn,typename Ins,typename... FuncPara>
//     class TA_MethodActivity<Fn,Ins,void,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&ins,func,&para...]()->void{(ins.*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->void{(m_object.*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->void{(m_object.*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_object;
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         Fn &m_funcPtr;
//         Ins &m_object;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Fn,typename Ins,typename... FuncPara>
//     class TA_MethodActivity<Fn,Ins *,void,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&ins,func,&para...]()->void{(ins->*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, Ins *&ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->void{(m_object->*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->void{(m_object->*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_object;
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         Fn &m_funcPtr;
//         Ins *&m_object;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
//     class TA_MethodActivity<Fn,std::shared_ptr<Ins>,Ret,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&ins,func,&para...]()->Ret{return (ins.get()->*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->Ret{return (m_object.get()->*m_funcPtr)(para...);});
//         }


//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->Ret{return (m_object.get()->*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_object.get();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         Fn &m_funcPtr;
//         std::shared_ptr<Ins> &m_object;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Fn,typename Ins,typename... FuncPara>
//     class TA_MethodActivity<Fn,std::shared_ptr<Ins>,void,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&ins,func,&para...]()->void{(ins.get()->*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::shared_ptr<Ins> &ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_object(ins), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),ins,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->void{(m_object.get()->*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->void{(m_object.get()->*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_object.get();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         Fn &m_funcPtr;
//         std::shared_ptr<Ins> &m_object;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Fn,typename Ins, typename Ret,typename... FuncPara>
//     class TA_MethodActivity<Fn,std::unique_ptr<Ins>,Ret,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [this,func,&para...]()->Ret{return (m_pObject->*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),m_pObject,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {
//             if(m_pObject)
//             {
//                 delete m_pObject;
//                 m_pObject = nullptr;
//             }
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->Ret{return (m_pObject->*m_funcPtr)(para...);});
//         }


//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->Ret{return (m_pObject->*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_pObject;
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         Fn &m_funcPtr;
//         Ins *m_pObject;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Fn,typename Ins,typename... FuncPara>
//     class TA_MethodActivity<Fn,std::unique_ptr<Ins>,void,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [this,func,&para...]()->void{return (m_pObject->*func)(para...);};
//         }

//         TA_MethodActivity(SupportMemberFunction<Fn> &&func, std::unique_ptr<Ins> ins, FuncPara &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func),m_pObject(ins.release()), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),m_pObject,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {
//             if(m_pObject)
//             {
//                 delete m_pObject;
//                 m_pObject = nullptr;
//             }
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->void{return (m_pObject->*m_funcPtr)(para...);});
//         }


//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->void{return (m_pObject->*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         decltype(auto) caller() const
//         {
//             return m_pObject;
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         Fn &m_funcPtr;
//         Ins *m_pObject;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Ret,typename... FuncPara>
//     class TA_MethodActivity<NonMemberFunctionPtr<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(NonMemberFunctionPtr<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [func,&para...]()->Ret{return (*func)(para...);};
//         }

//         TA_MethodActivity(NonMemberFunctionPtr<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([this,&para...]()->Ret{return (*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap(std::bind(m_funcPtr,std::forward<FuncPara>(para)...));
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         NonMemberFunctionPtr<Ret,FuncPara...> &m_funcPtr;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename... FuncPara>
//     class TA_MethodActivity<NonMemberFunctionPtr<void,FuncPara...>,INVALID_INS,void,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(NonMemberFunctionPtr<void,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [func, &para...]()->void{(func)(para...);};
//         }

//         TA_MethodActivity(NonMemberFunctionPtr<void,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([this,&para...]()->void{(*m_funcPtr)(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap([this,para...]()->void{(*m_funcPtr)(para...);});
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//     protected:
//         decltype(auto) run()
//         {
//             m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         NonMemberFunctionPtr<void,FuncPara...> &m_funcPtr;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Ret,typename... FuncPara>
//     class TA_MethodActivity<LambdaType<Ret,FuncPara...>,INVALID_INS,Ret,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(LambdaType<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&,func]()->Ret{return func(para...);};
//         }

//         TA_MethodActivity(LambdaType<Ret,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(std::forward<decltype(func)>(func),std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->Ret{return m_funcPtr(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap(std::bind(m_funcPtr,std::forward<FuncPara>(para)...));
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         LambdaType<Ret,FuncPara...> m_funcPtr;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename... FuncPara>
//     class TA_MethodActivity<LambdaType<void,FuncPara...>,INVALID_INS,void,FuncPara...>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(LambdaType<void,FuncPara...> &&func, std::decay_t<FuncPara> &... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = [&,func]()->void{func(para...);};
//         }

//         TA_MethodActivity(LambdaType<void,FuncPara...> &&func, std::decay_t<FuncPara> &&... para, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr),m_funcPtr(func), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = std::bind(func,std::forward<FuncPara>(para)...);
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &...para)
//         {
//             m_funcActivity.swap([&]()->void{return m_funcPtr(para...);});
//         }

//         template <typename ...NewPara>
//         void setPara(NewPara &&...para)
//         {
//             m_funcActivity.swap(std::bind(m_funcPtr,std::forward<FuncPara>(para)...));
//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         std::function<void(FuncPara...)> m_funcPtr;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <typename Ret>
//     class TA_MethodActivity<LambdaTypeWithoutPara<Ret>,INVALID_INS,Ret,INVALID_INS>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(LambdaTypeWithoutPara<Ret> &&func, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = func;
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         decltype(auto) operator()()
//         {
//             return run();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         decltype(auto) run()
//         {
//             return m_funcActivity();
//         }

//     protected:
//         std::function<Ret()> m_funcActivity;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };

//     template <>
//     class TA_MethodActivity<LambdaTypeWithoutPara<void>,INVALID_INS,void,INVALID_INS>
//     {
//     public:
//         TA_MethodActivity() = delete;
//         TA_MethodActivity(const TA_MethodActivity &activity) = delete;
//         TA_MethodActivity(TA_MethodActivity &&activity) = delete;
//         TA_MethodActivity & operator = (const TA_MethodActivity &) = delete;

//         TA_MethodActivity(LambdaTypeWithoutPara<void> &&func, std::size_t affinityThread = std::numeric_limits<std::size_t>::max()) : m_funcActivity(nullptr), m_affinityThread(affinityThread)
//         {
//             m_funcActivity = func;
//         }

//         virtual ~TA_MethodActivity()
//         {

//         }

//         decltype(auto) operator()()
//         {
//             run();
//         }

//         std::size_t affinityThread() const
//         {
//             return m_affinityThread.affinityThread();
//         }

//         bool moveToThread(std::size_t thread)
//         {
//             return m_affinityThread.moveToThread(thread);
//         }

//         std::int64_t id() const
//         {
//             return m_id.id();
//         }

//     protected:
//         void run()
//         {
//             m_funcActivity();
//         }

//     protected:
//         std::function<void()> m_funcActivity;

//         TA_ActivityAffinityThread m_affinityThread {};
//         TA_ActivityId m_id {};
//     };
// }

#endif // TA_MethodActivity_H
