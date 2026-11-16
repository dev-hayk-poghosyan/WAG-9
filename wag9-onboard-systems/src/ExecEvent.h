#pragma once

#include <QEvent>
#include <type_traits>
#include <utility>

/**
 * A base class for events carrying a functor to be executed when delivered to an instnce of
 * EventExecutor.
 * @see EventExecutor
 */
class ExecEvent : public QEvent
{
public:
    ExecEvent();

    virtual void exec() = 0;
    
    static Type registeredType();
};

template<typename F>
class PayloadExecEvent : public ExecEvent
{
public:
    template<typename T>
    PayloadExecEvent(T&& func) : m_func(std::forward<T>(func)) {}

    virtual void exec() override { m_func(); }
private:
    F m_func;
};

template<typename F>
ExecEvent* createExecEvent(F&& func)
{
    return new PayloadExecEvent<typename std::remove_reference<F>::type>(std::forward<F>(func));
}
