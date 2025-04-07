#ifndef TA_COROUTINE_H
#define TA_COROUTINE_H

#include <coroutine>
#include <optional>
#include <exception>

#include "TA_MetaObject.h"
#include "TA_Connection.h"

namespace CoreAsync
{
    enum CorotuineBehavior
    {
        Lazy,
        Eager
    };

    struct TA_BaseAwaitable : public TA_MetaObject
    {
        virtual bool await_ready() const noexcept = 0;
        virtual void await_suspend(std::coroutine_handle<>) const noexcept = 0;
        virtual void await_resume() const noexcept = 0;
    };

    DEFINE_TYPE_INFO(TA_BaseAwaitable)
    {
        AUTO_META_FIELDS(
            REGISTER_FIELD(await_ready),
            REGISTER_FIELD(await_suspend),
            REGISTER_FIELD(await_resume)
        )
    };

    template <EnableConnectObjectType Sender, typename ...Args>
    class TA_SignalAwaitable : public TA_BaseAwaitable
    {
    public:
        TA_SignalAwaitable(Sender *pObject, void(std::decay_t<Sender>::*signal)(Args...))
        {

        }

        virtual bool await_ready() const noexcept override
        {
            return true;
        }

        virtual void await_suspend(std::coroutine_handle<> handle) const noexcept override
        {
            m_connectionHolder = TA_Connection::connect(m_pObject, m_signal, [handle](Args... args)
            {
                handle.resume();
            });
        }

        virtual void await_resume() const noexcept override
        {
            if(m_connectionHolder.valid())
            {
                TA_Connection::disconnect(m_connectionHolder);
            }
        }

    private:
        TA_ConnectionObjectHolder m_connectionHolder {nullptr};
        Sender *m_pObject {nullptr};
        void (std::decay_t<Sender>::*m_signal)(Args...);

    };

    template <typename T, CorotuineBehavior = Lazy>
    struct TA_CoroutineTask
    {
        using HandleType = std::coroutine_handle<typename std::coroutine_traits<TA_CoroutineTask, T>::promise_type>;

        struct promise_type
        {
            std::optional<T> m_result {};
            std::exception_ptr m_exception {};

            TA_CoroutineTask get_return_object()
            {
                return TA_CoroutineTask {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() noexcept {return {};}
            ::std::suspend_always final_suspend() noexcept {return {};}

            void return_value(const T &value)
            {
                m_result = value;
            }

            void unhanded_exception()
            {
                m_exception = std::current_exception();
            }
        };

        HandleType m_coroutineHandle;

        explicit TA_CoroutineTask(HandleType handle) : m_coroutineHandle(handle) {}

        TA_CoroutineTask(const TA_CoroutineTask &) = delete;

        TA_CoroutineTask & operator = (const TA_CoroutineTask &) = delete;

        TA_CoroutineTask(TA_CoroutineTask &&task) noexcept : m_coroutineHandle(std::move(task.m_coroutineHandle))
        {
            task.m_coroutineHandle = nullptr;
        }

        TA_CoroutineTask & operator = (TA_CoroutineTask &&task) noexcept
        {
            if(this != &task)
            {
                m_coroutineHandle = std::move(task.m_coroutineHandle);
                task.m_coroutineHandle = nullptr;
            }
            return *this;
        }

        ~TA_CoroutineTask()
        {
            if(m_coroutineHandle)
            {
                m_coroutineHandle.destroy();
            }
        }

        T get()
        {
            if(m_coroutineHandle.done())
            {
                if(m_coroutineHandle.promise().m_exception)
                {
                    std::rethrow_exception(m_coroutineHandle.promise().m_exception);
                }
                return std::move(*m_coroutineHandle.promise().m_result);
            }
            m_coroutineHandle.resume();
            if(m_coroutineHandle.promise().m_exception)
            {
                std::rethrow_exception(m_coroutineHandle.promise().m_exception);
            }
            return std::move(*m_coroutineHandle.promise().m_result);
        }
    };

    template <typename T>
    struct TA_CoroutineTask<T, Eager>
    {
        using HandleType = std::coroutine_handle<typename std::coroutine_traits<TA_CoroutineTask, T>::promise_type>;

        struct promise_type
        {
            std::optional<T> m_result {};
            std::exception_ptr m_exception {};

            TA_CoroutineTask get_return_object()
            {
                return TA_CoroutineTask {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_never initial_suspend() noexcept {return {};}
            std::suspend_always final_suspend() noexcept {return {};}

            void return_value(const T &value)
            {
                m_result = value;
            }

            void unhanded_exception()
            {
                m_exception = std::current_exception();
            }
        };

        HandleType m_coroutineHandle;

        explicit TA_CoroutineTask(HandleType handle) : m_coroutineHandle(handle) {}

        TA_CoroutineTask(const TA_CoroutineTask &) = delete;

        TA_CoroutineTask & operator = (const TA_CoroutineTask &) = delete;

        TA_CoroutineTask(TA_CoroutineTask &&task) noexcept : m_coroutineHandle(std::move(task.m_coroutineHandle))
        {
            task.m_coroutineHandle = nullptr;
        }

        TA_CoroutineTask & operator = (TA_CoroutineTask &&task) noexcept
        {
            if(this != &task)
            {
                m_coroutineHandle = std::move(task.m_coroutineHandle);
                task.m_coroutineHandle = nullptr;
            }
            return *this;
        }

        ~TA_CoroutineTask()
        {
            if(m_coroutineHandle)
            {
                m_coroutineHandle.destroy();
            }
        }

        T get()
        {
            if(m_coroutineHandle.done())
            {
                if(m_coroutineHandle.promise().m_exception)
                {
                    std::rethrow_exception(m_coroutineHandle.promise().m_exception);
                }
                return std::move(*m_coroutineHandle.promise().m_result);
            }
            m_coroutineHandle.resume();
            if(m_coroutineHandle.promise().m_exception)
            {
                std::rethrow_exception(m_coroutineHandle.promise().m_exception);
            }
            return std::move(*m_coroutineHandle.promise().m_result);
        }
    };

    template <typename T, CorotuineBehavior = Lazy>
    struct TA_CoroutineGenerator
    {
        using HandleType = std::coroutine_handle<typename std::coroutine_traits<TA_CoroutineGenerator, T>::promise_type>;

        struct promise_type
        {
            T m_currentValue {};
            std::exception_ptr m_exception {};

            TA_CoroutineGenerator get_return_object()
            {
                return TA_CoroutineGenerator {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() noexcept {return {};}
            std::suspend_always final_suspend() noexcept {return {};}

            std::suspend_always yield_value(const T &value)
            {
                m_currentValue = value;
                return {};
            }

            void return_void() {};

            void unhanded_exception()
            {
                m_exception = std::current_exception();
            }
        };

        HandleType m_coroutineHandle;

        explicit TA_CoroutineGenerator(HandleType handle) : m_coroutineHandle(handle) {}

        TA_CoroutineGenerator(const TA_CoroutineGenerator &) = delete;

        TA_CoroutineGenerator & operator = (const TA_CoroutineGenerator &) = delete;

        TA_CoroutineGenerator(TA_CoroutineGenerator &&generator) noexcept : m_coroutineHandle(std::move(generator.m_coroutineHandle))
        {
            generator.m_coroutineHandle = nullptr;
        }

        TA_CoroutineGenerator & operator = (TA_CoroutineGenerator &&generator) noexcept
        {
            if(this != &generator)
            {
                m_coroutineHandle = std::move(generator.m_coroutineHandle);
                generator.m_coroutineHandle = nullptr;
            }
            return *this;
        }

        ~TA_CoroutineGenerator()
        {
            if(m_coroutineHandle)
            {
                m_coroutineHandle.destroy();
            }
        }

        bool next()
        {
            m_coroutineHandle.resume();
            if(m_coroutineHandle.done())
            {
                if(m_coroutineHandle.promise().m_exception)
                {
                    std::rethrow_exception(m_coroutineHandle.promise().m_exception);
                }
                return false;
            }
            return true;
        }

        T value()
        {
            return std::move(m_coroutineHandle.promise().m_currentValue);
        }
    };

    template <typename T>
    struct TA_CoroutineGenerator<T, Eager>
    {
        using HandleType = std::coroutine_handle<typename std::coroutine_traits<TA_CoroutineGenerator, T>::promise_type>;

        struct promise_type
        {
            T m_currentValue {};
            std::exception_ptr m_exception {};

            TA_CoroutineGenerator get_return_object()
            {
                return TA_CoroutineGenerator {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_never initial_suspend() noexcept {return {};}
            std::suspend_always final_suspend() noexcept {return {};}

            std::suspend_always yield_value(const T &value)
            {
                m_currentValue = value;
                return {};
            }

            void return_void() {};

            void unhanded_exception()
            {
                m_exception = std::current_exception();
            }
        };

        HandleType m_coroutineHandle;

        explicit TA_CoroutineGenerator(HandleType handle) : m_coroutineHandle(handle) {}

        TA_CoroutineGenerator(const TA_CoroutineGenerator &) = delete;

        TA_CoroutineGenerator & operator = (const TA_CoroutineGenerator &) = delete;

        TA_CoroutineGenerator(TA_CoroutineGenerator &&generator) noexcept : m_coroutineHandle(std::move(generator.m_coroutineHandle))
        {
            generator.m_coroutineHandle = nullptr;
        }

        TA_CoroutineGenerator & operator = (TA_CoroutineGenerator &&generator) noexcept
        {
            if(this != &generator)
            {
                m_coroutineHandle = std::move(generator.m_coroutineHandle);
                generator.m_coroutineHandle = nullptr;
            }
            return *this;
        }

        ~TA_CoroutineGenerator()
        {
            if(m_coroutineHandle)
            {
                m_coroutineHandle.destroy();
            }
        }

        bool next()
        {
            m_coroutineHandle.resume();
            if(m_coroutineHandle.done())
            {
                if(m_coroutineHandle.promise().m_exception)
                {
                    std::rethrow_exception(m_coroutineHandle.promise().m_exception);
                }
                return false;
            }
            return true;
        }

        T value()
        {
            return std::move(m_coroutineHandle.promise().m_currentValue);
        }
    };
}

#endif // TA_COROUTINE_H
