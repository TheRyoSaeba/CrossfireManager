#pragma once
#include <concepts>
#include <type_traits>

namespace lockpp
{
    template <typename T>
    concept decayed = std::same_as<T, std::decay_t<T>>;

    template <typename T>
    concept shared_lockable = requires(T &mutex) { mutex.lock_shared(); };

    template <decayed Mutex, template <typename> typename TrueType, template <typename> typename FalseType>
    using shared_cond = std::conditional_t<shared_lockable<Mutex>, TrueType<Mutex>, FalseType<Mutex>>;

    template <typename Lock>
    consteval auto lock_mutex()
    {
        return []<template <typename> typename L, typename M>(L<M> *)
        {
            return std::type_identity<M>{};
        }(static_cast<Lock *>(nullptr));
    }

    template <typename Lock>
    using lock_mutex_t = typename decltype(lock_mutex<Lock>())::type;
} // namespace lockpp
