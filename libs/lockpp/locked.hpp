#pragma once
#include "traits.hpp"

namespace lockpp
{
    template <typename Type, typename Lock>
    class locked
    {
        template <typename, decayed>
        friend class lock;

      private:
        Lock m_lock;
        Type *m_value;

      protected:
        template <typename... LockArgs>
        locked(Type *, lock_mutex_t<Lock> &, LockArgs &&...);

      public:
        [[nodiscard]] Type &operator*() const & noexcept;
        [[nodiscard]] Type *operator->() const & noexcept;

        [[nodiscard]] Type &operator*() && noexcept = delete;
        [[nodiscard]] Type *operator->() && noexcept = delete;
    };
} // namespace lockpp

#include "locked.inl"
