#pragma once
#include "lock.hpp"

namespace lockpp
{
    template <typename Type, decayed Mutex>
    template <typename... Args>
    lock<Type, Mutex>::lock(Args &&...args) : m_value(std::forward<Args>(args)...)
    {
    }

    template <typename Type, decayed Mutex>
    template <template <typename> class Lock, typename... LockArgs>
    locked<Type, Lock<Mutex>> lock<Type, Mutex>::write(LockArgs &&...lock_args) &
    {
        return {&m_value, m_mutex, std::forward<LockArgs>(lock_args)...};
    }

    template <typename Type, decayed Mutex>
    template <template <typename> class Lock, typename... LockArgs>
    locked<const Type, Lock<Mutex>> lock<Type, Mutex>::read(LockArgs &&...lock_args) const &
    {
        return {&m_value, m_mutex, std::forward<LockArgs>(lock_args)...};
    }

    template <typename Type, decayed Mutex>
        template <typename O>
        void lock<Type, Mutex>::assign(O &&value) &
            requires std::assignable_from<Type &, O>
    {
        auto locked = write();
        *locked = std::forward<O>(value);
    }

    template <typename Type, decayed Mutex>
    Type &lock<Type, Mutex>::get_unsafe() &
    {
        return m_value;
    }

    template <typename Type, decayed Mutex>
    Type &lock<Type, Mutex>::get_unsafe() const &
    {
        return m_value;
    }

    template <typename Type, decayed Mutex>
    Type lock<Type, Mutex>::copy() const &
        requires std::copyable<Type>
    {
        auto locked = read();
        return *locked;
    }
} // namespace lockpp
