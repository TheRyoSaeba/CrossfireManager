#pragma once
#include "flags.hpp"

namespace flagpp
{
    template <typename T>
        requires scoped_enum<T>
    class wrapper
    {
        T m_value;

      public:
        using enum_t       = T;
        using underlying_t = std::underlying_type_t<T>;

      public:
        constexpr wrapper(T value) : m_value(value) {}

      private:
        constexpr wrapper(underlying_t value) : m_value(static_cast<T>(value)) {}

      public:
        constexpr T value() const
        {
            return m_value;
        }

        constexpr operator T() const
        {
            return value();
        }

      public:
        constexpr underlying_t underlying() const
        {
            return static_cast<underlying_t>(m_value);
        }

        constexpr operator underlying_t() const
        {
            return underlying();
        }

      public:
        constexpr wrapper operator&(wrapper other) const
        {
            return underlying() & other.underlying();
        }

        constexpr wrapper operator|(wrapper other) const
        {
            return underlying() | other.underlying();
        }

        constexpr wrapper operator^(wrapper other) const
        {
            return underlying() ^ other.underlying();
        }

        constexpr wrapper operator<<(wrapper other) const
        {
            return underlying() << other.underlying();
        }

        constexpr wrapper operator>>(wrapper other) const
        {
            return underlying() >> other.underlying();
        }

      public:
        constexpr wrapper operator~() const
        {
            return ~underlying();
        }

      public:
        constexpr wrapper &operator&=(wrapper other)
        {
            m_value = (*this) & other;
            return *this;
        }

        constexpr wrapper &operator|=(wrapper other)
        {
            m_value = (*this) | other;
            return *this;
        }

        constexpr wrapper &operator^=(wrapper other)
        {
            m_value = (*this) | other;
            return *this;
        }
    };
} // namespace flagpp
