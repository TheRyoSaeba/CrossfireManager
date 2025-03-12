#pragma once
#include "locked.hpp"

#include <utility>

namespace lockpp
{
    template <typename Type, class Lock>
    template <typename... LockArgs>
    locked<Type, Lock>::locked(Type *value, lock_mutex_t<Lock> &mutex, LockArgs &&...lock_args)
        : m_lock(mutex, std::forward<LockArgs>(lock_args)...), m_value(value)
    {
    }

    template <typename Type, class Lock>
    [[nodiscard]] Type &locked<Type, Lock>::operator*() const & noexcept
    {
        return *m_value;
    }

    template <typename Type, class Lock>
    [[nodiscard]] Type *locked<Type, Lock>::operator->() const & noexcept
    {
        return m_value;
    }
} // namespace lockpp
