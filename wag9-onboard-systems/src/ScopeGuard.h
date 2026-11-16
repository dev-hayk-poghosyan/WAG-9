#pragma once

#include <boost/optional.hpp>
#include <type_traits>
#include <utility>

template<typename F>
class ScopeGuard
{
public:
    template<typename T>
    ScopeGuard(T&& func) : m_func(std::forward<T>(func)) {}

    ScopeGuard(ScopeGuard&& other)
        : m_func(std::move(other.m_func))
    {
    }

    ScopeGuard& operator=(ScopeGuard&&) = delete;

    ~ScopeGuard()
    {
        if (m_func)
        {
            (*m_func)();
        }
    }

    /**
     * Prevents the guarding code running at scope exit.
     */
    void disarm()
    {
        m_func = boost::none;
    }

private:
    boost::optional<F> m_func;
};

template<typename F>
ScopeGuard<F> createScopeGuard(F&& func)
{
    return ScopeGuard<typename std::remove_reference<F>::type>(std::forward<F>(func));
}
