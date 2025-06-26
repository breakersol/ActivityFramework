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

#ifndef TA_COROUTINE_H
#define TA_COROUTINE_H

#include <coroutine>
#include <optional>
#include <exception>

#include "TA_MetaObject.h"
#include "TA_Connection.h"

namespace CoreAsync {
enum CorotuineBehavior { Lazy, Eager };

template <EnableConnectObjectType Sender, typename... Args> class TA_SignalAwaitable {
  public:
    TA_SignalAwaitable(Sender *pObject, void (std::decay_t<Sender>::*signal)(Args...))
        : m_pObject(pObject), m_signal(signal) {}

    ~TA_SignalAwaitable() {}

    constexpr bool await_ready() const noexcept { return false; }

    constexpr void await_suspend(std::coroutine_handle<> handle) noexcept {
        TA_Connection::connect(
            m_pObject, std::move(m_signal),
            [this, handle](Args... args) {
                if constexpr (sizeof...(Args) != 0)
                    m_args = std::make_tuple(args...);
                handle.resume();
            },
            true);
    }

    constexpr auto await_resume() const noexcept {
        if constexpr (sizeof...(Args) != 0) {
            if constexpr (sizeof...(Args) == 1) {
                return std::get<0>(m_args);
            } else {
                return m_args;
            }
        }
    }

    TA_SignalAwaitable operator=(const TA_SignalAwaitable &) = delete;

  protected:
    Sender *m_pObject{nullptr};
    void (std::decay_t<Sender>::*m_signal)(Args...);
    std::tuple<Args...> m_args{};
};

class TA_ActivityResultAwaitable {
  public:
    TA_ActivityResultAwaitable() = delete;

    explicit TA_ActivityResultAwaitable(const TA_ActivityResultFetcher &fetcher) : m_fetcher(fetcher) {
        if (!m_fetcher.pProxy->isValid()) {
            throw std::runtime_error("TA_ActivityResultAwaitable: Fetcher is not valid!");
        }
    }

    ~TA_ActivityResultAwaitable() = default;

    bool await_ready() const noexcept { return m_fetcher.pProxy->isExecuted() || !m_fetcher.pProxy->isValid(); }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        auto activity = TA_ActivityCreator::create([this, handle]() {
            if (m_fetcher.pProxy->isExecuted()) {
                handle.resume();
            } else {
                m_res = m_fetcher();
                handle.resume();
            }
        });
        auto fetcher = TA_ThreadHolder::get().postActivity(activity, true);
    }

    auto await_resume() noexcept { return m_res; }

  private:
    TA_ActivityResultFetcher m_fetcher{};
    TA_DefaultVariant m_res{};
};

class TA_ActivityExecutingAwaitable {
  public:
    enum class ExecuteType { Async, Sync };

    TA_ActivityExecutingAwaitable(TA_ActivityProxy *pActivity, ExecuteType type)
        : m_pProxy(pActivity), m_executeType(type) {
        if (!m_pProxy) {
            throw std::runtime_error("TA_ActivityResultAwaitable: pActivity is null!");
        }
    }

    ~TA_ActivityExecutingAwaitable() {
        if (m_pProxy && !m_pProxy->isExecuted()) {
            delete m_pProxy;
            m_pProxy = nullptr;
        }
    }

    bool await_ready() const noexcept { return m_pProxy->isExecuted() || !m_pProxy->isValid(); }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        if (m_executeType == ExecuteType::Async) {
            m_fetcher = TA_ThreadHolder::get().postActivity(m_pProxy);
        } else {
            m_pProxy->operator()();
            m_fetcher = {std::make_shared<TA_ActivityProxy>(m_pProxy)};
        }
        handle.resume();
    }

    auto await_resume() const noexcept { return m_fetcher; }

  private:
    TA_ActivityProxy *m_pProxy{nullptr};
    ExecuteType m_executeType{ExecuteType::Async};
    TA_ActivityResultFetcher m_fetcher{};
};

template <typename T, CorotuineBehavior = Lazy> struct TA_CoroutineTask {
    struct promise_type {
        std::optional<T> m_result{};
        std::exception_ptr m_exception{};

        TA_CoroutineTask get_return_object() {
            return TA_CoroutineTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        ::std::suspend_always final_suspend() noexcept { return {}; }

        void return_value(const T &value) { m_result = value; }

        void unhandled_exception() { m_exception = std::current_exception(); }
    };

    using HandleType = std::coroutine_handle<promise_type>;

    explicit TA_CoroutineTask(HandleType handle) : m_coroutineHandle(handle) {}

    TA_CoroutineTask(const TA_CoroutineTask &) = delete;

    TA_CoroutineTask &operator=(const TA_CoroutineTask &) = delete;

    TA_CoroutineTask(TA_CoroutineTask &&task) noexcept : m_coroutineHandle(std::move(task.m_coroutineHandle)) {
        task.m_coroutineHandle = nullptr;
    }

    TA_CoroutineTask &operator=(TA_CoroutineTask &&task) noexcept {
        if (this != &task) {
            if (m_coroutineHandle)
                m_coroutineHandle.destroy();
            m_coroutineHandle = std::move(task.m_coroutineHandle);
            task.m_coroutineHandle = nullptr;
        }
        return *this;
    }

    ~TA_CoroutineTask() {
        if (m_coroutineHandle) {
            m_coroutineHandle.destroy();
        }
    }

    void start() {
        if (m_coroutineHandle) {
            m_coroutineHandle.resume();
        }
    }

    T get() {
        if (m_coroutineHandle.done()) {
            if (m_coroutineHandle.promise().m_exception) {
                std::rethrow_exception(m_coroutineHandle.promise().m_exception);
            }
            return std::move(*m_coroutineHandle.promise().m_result);
        }
        m_coroutineHandle.resume();
        if (m_coroutineHandle.promise().m_exception) {
            std::rethrow_exception(m_coroutineHandle.promise().m_exception);
        }
        return std::move(*m_coroutineHandle.promise().m_result);
    }

    HandleType m_coroutineHandle;
};

template <typename T> struct TA_CoroutineTask<T, Eager> {
    struct promise_type {
        std::optional<T> m_result{};
        std::exception_ptr m_exception{};

        TA_CoroutineTask get_return_object() {
            return TA_CoroutineTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_value(const T &value) { m_result = value; }

        void unhandled_exception() { m_exception = std::current_exception(); }
    };

    using HandleType = std::coroutine_handle<promise_type>;
    HandleType m_coroutineHandle;

    explicit TA_CoroutineTask(HandleType handle) : m_coroutineHandle(handle) {}

    TA_CoroutineTask(const TA_CoroutineTask &) = delete;

    TA_CoroutineTask &operator=(const TA_CoroutineTask &) = delete;

    TA_CoroutineTask(TA_CoroutineTask &&task) noexcept : m_coroutineHandle(std::move(task.m_coroutineHandle)) {
        task.m_coroutineHandle = nullptr;
    }

    TA_CoroutineTask &operator=(TA_CoroutineTask &&task) noexcept {
        if (this != &task) {
            if (m_coroutineHandle)
                m_coroutineHandle.destroy();
            m_coroutineHandle = std::move(task.m_coroutineHandle);
            task.m_coroutineHandle = nullptr;
        }
        return *this;
    }

    ~TA_CoroutineTask() {
        if (m_coroutineHandle) {
            m_coroutineHandle.destroy();
        }
    }

    T get() {
        if (m_coroutineHandle.done()) {
            if (m_coroutineHandle.promise().m_exception) {
                std::rethrow_exception(m_coroutineHandle.promise().m_exception);
            }
            return std::move(*m_coroutineHandle.promise().m_result);
        }
        m_coroutineHandle.resume();
        if (m_coroutineHandle.promise().m_exception) {
            std::rethrow_exception(m_coroutineHandle.promise().m_exception);
        }
        return std::move(*m_coroutineHandle.promise().m_result);
    }
};

template <typename T, CorotuineBehavior = Lazy> struct TA_CoroutineGenerator {
    struct promise_type {
        T m_currentValue{};
        std::exception_ptr m_exception{};

        TA_CoroutineGenerator get_return_object() {
            return TA_CoroutineGenerator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(const T &value) {
            m_currentValue = value;
            return {};
        }

        void return_void() {};

        void unhandled_exception() { m_exception = std::current_exception(); }
    };

    using HandleType = std::coroutine_handle<promise_type>;

    explicit TA_CoroutineGenerator(HandleType handle) : m_coroutineHandle(handle) {}

    TA_CoroutineGenerator(const TA_CoroutineGenerator &) = delete;

    TA_CoroutineGenerator &operator=(const TA_CoroutineGenerator &) = delete;

    TA_CoroutineGenerator(TA_CoroutineGenerator &&generator) noexcept
        : m_coroutineHandle(std::move(generator.m_coroutineHandle)) {
        generator.m_coroutineHandle = nullptr;
    }

    TA_CoroutineGenerator &operator=(TA_CoroutineGenerator &&generator) noexcept {
        if (this != &generator) {
            if (m_coroutineHandle)
                m_coroutineHandle.destroy();
            m_coroutineHandle = std::move(generator.m_coroutineHandle);
            generator.m_coroutineHandle = nullptr;
        }
        return *this;
    }

    ~TA_CoroutineGenerator() {
        if (m_coroutineHandle) {
            m_coroutineHandle.destroy();
        }
    }

    void start() {
        if (m_coroutineHandle) {
            m_coroutineHandle.resume();
        }
    }

    bool next() {
        if (!m_coroutineHandle || m_coroutineHandle.done()) {
            if (m_coroutineHandle.promise().m_exception) {
                std::rethrow_exception(m_coroutineHandle.promise().m_exception);
            }
            return false;
        }
        m_coroutineHandle.resume();
        return !m_coroutineHandle.done();
    }

    T value() { return std::move(m_coroutineHandle.promise().m_currentValue); }

    HandleType m_coroutineHandle;
};

template <typename T> struct TA_CoroutineGenerator<T, Eager> {
    struct promise_type {
        T m_currentValue{};
        std::exception_ptr m_exception{};

        TA_CoroutineGenerator get_return_object() {
            return TA_CoroutineGenerator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(const T &value) {
            m_currentValue = value;
            return {};
        }

        void return_void() {};

        void unhandled_exception() { m_exception = std::current_exception(); }
    };

    using HandleType = std::coroutine_handle<promise_type>;

    explicit TA_CoroutineGenerator(HandleType handle) : m_coroutineHandle(handle) {}

    TA_CoroutineGenerator(const TA_CoroutineGenerator &) = delete;

    TA_CoroutineGenerator &operator=(const TA_CoroutineGenerator &) = delete;

    TA_CoroutineGenerator(TA_CoroutineGenerator &&generator) noexcept
        : m_coroutineHandle(std::move(generator.m_coroutineHandle)) {
        generator.m_coroutineHandle = nullptr;
    }

    TA_CoroutineGenerator &operator=(TA_CoroutineGenerator &&generator) noexcept {
        if (this != &generator) {
            if (m_coroutineHandle)
                m_coroutineHandle.destroy();
            m_coroutineHandle = std::move(generator.m_coroutineHandle);
            generator.m_coroutineHandle = nullptr;
        }
        return *this;
    }

    ~TA_CoroutineGenerator() {
        if (m_coroutineHandle) {
            m_coroutineHandle.destroy();
        }
    }

    bool next() {
        if (!m_coroutineHandle || m_coroutineHandle.done()) {
            if (m_coroutineHandle.promise().m_exception) {
                std::rethrow_exception(m_coroutineHandle.promise().m_exception);
            }
            return false;
        }
        m_coroutineHandle.resume();
        return !m_coroutineHandle.done();
    }

    T value() { return std::move(m_coroutineHandle.promise().m_currentValue); }

    HandleType m_coroutineHandle;
};
} // namespace CoreAsync

#endif // TA_COROUTINE_H
