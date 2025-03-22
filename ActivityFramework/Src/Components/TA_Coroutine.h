#ifndef TA_COROUTINE_H
#define TA_COROUTINE_H

#include <coroutine>
#include <optional>
#include <exception>

namespace CoreAsync
{
    enum CorotuineBehavior
    {
        Lazy,
        Eager
    };

    struct TA_BaseAwaitable
    {
        virtual bool await_ready() const noexcept = 0;
        virtual void await_suspend(std::coroutine_handle<>) const noexcept = 0;
        virtual void await_resume() const noexcept = 0;
    };

    template <typename T, CorotuineBehavior = Lazy>
    struct TA_CoroutineTask
    {
        using handle_type = std::coroutine_handle<typename std::coroutine_traits<TA_CoroutineTask, T>::promise_type>;

        struct promise_type
        {
            std::optional<T> m_result {};
            std::exception_ptr m_exception {};

            TA_CoroutineTask get_return_object()
            {
                return TA_CoroutineTask {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() noexcept {return {};}
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

        handle_type m_coroutineHandle;

        explicit TA_CoroutineTask(handle_type handle) : m_coroutineHandle(handle) {}

        TA_CoroutineTask(const TA_CoroutineTask &) = delete;

        TA_CoroutineTask & operator = (const TA_CoroutineTask &) = delete;

        TA_CoroutineTask(TA_CoroutineTask &&task) noexcept : m_coroutineHandle(std::move(task.m_coroutineHandle))
        {
            task.m_coroutineHandle = nullptr;
        }

        TA_CoroutineTask &operator = (TA_CoroutineTask &&task) noexcept
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
        using handle_type = std::coroutine_handle<typename std::coroutine_traits<TA_CoroutineTask, T>::promise_type>;

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

        handle_type m_coroutineHandle;

        explicit TA_CoroutineTask(handle_type handle) : m_coroutineHandle(handle) {}

        TA_CoroutineTask(const TA_CoroutineTask &) = delete;

        TA_CoroutineTask & operator = (const TA_CoroutineTask &) = delete;

        TA_CoroutineTask(TA_CoroutineTask &&task) noexcept : m_coroutineHandle(std::move(task.m_coroutineHandle))
        {
            task.m_coroutineHandle = nullptr;
        }

        TA_CoroutineTask &operator = (TA_CoroutineTask &&task) noexcept
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
}

#endif // TA_COROUTINE_H
