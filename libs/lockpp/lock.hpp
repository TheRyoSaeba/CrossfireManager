#pragma once
#include "traits.hpp"
#include "locked.hpp"

#include <mutex>
#include <utility>
#include <concepts>
#include <shared_mutex>

namespace lockpp
{
    template <typename Type, decayed Mutex = std::shared_mutex>
    class lock
    {
        template <typename>
        using read_lock = shared_cond<Mutex, std::shared_lock, std::lock_guard>;

        template <typename>
        using write_lock = shared_cond<Mutex, std::unique_lock, std::lock_guard>;

      private:
        Type m_value;
        mutable Mutex m_mutex;

      public:
        template <typename... Args>
        explicit lock(Args &&...);

      public:
        template <template <typename> class Lock = write_lock, typename... LockArgs>
        [[nodiscard]] locked<Type, Lock<Mutex>> write(LockArgs &&...) &;

      public:
        template <template <typename> class Lock = read_lock, typename... LockArgs>
        [[nodiscard]] locked<const Type, Lock<Mutex>> read(LockArgs &&...) const &;

        template <template <typename> class Lock = read_lock, typename... LockArgs>
        [[nodiscard]] locked<const Type, Lock<Mutex>> read(LockArgs &&...) && = delete;

      public:
        template <typename O>
            void assign(O &&value) &
                requires std::assignable_from<Type &, O>;

      public:
        [[nodiscard]] Type &get_unsafe() &;
        [[nodiscard]] Type &get_unsafe() const &;
        [[nodiscard]] Type &get_unsafe() && = delete;

      public:
        [[nodiscard]] Type copy() const &
            requires std::copyable<Type>;

        [[nodiscard]] Type copy() &&
                requires std::copyable<Type>
            = delete;
    };
} // namespace lockpp

#include "lock.inl"
