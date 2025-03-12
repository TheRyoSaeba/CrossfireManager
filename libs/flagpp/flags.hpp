#pragma once
#include <concepts>
#include <type_traits>

namespace flagpp
{
    template <typename T>
    concept scoped_enum = requires() {
        requires std::is_enum_v<T>;
        requires not std::convertible_to<T, std::underlying_type_t<T>>;
    };

    template <typename T>
        requires scoped_enum<T>
    static constexpr bool enabled = false;

    template <typename T>
        requires scoped_enum<T>
    class wrapper;

    template <typename T>
    concept wrapped = requires(T &value) {
        []<typename O>(wrapper<O> &) {
        }(value);
    };

    template <typename Enum, typename O>
    consteval auto is_allowed()
    {
        if constexpr (wrapped<O>)
        {
            return std::same_as<Enum, typename O::enum_t>;
        }
        else
        {
            return std::same_as<Enum, O>;
        }
    }

    template <typename Enum, typename O>
    concept allowed = requires() { requires is_allowed<Enum, O>(); };
} // namespace flagpp

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto operator&(Enum left, Other right)
{
    return flagpp::wrapper{left} & right;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto operator|(Enum left, Other right)
{
    return flagpp::wrapper{left} | right;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto operator^(Enum left, Other right)
{
    return flagpp::wrapper{left} ^ right;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto operator<<(Enum left, Other right)
{
    return flagpp::wrapper{left} << right;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto operator>>(Enum left, Other right)
{
    return flagpp::wrapper{left} >> right;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto &operator&=(Enum &left, Other right)
{
    left = flagpp::wrapper{left} & right;
    return left;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto &operator|=(Enum &left, Other right)
{
    left = flagpp::wrapper{left} | right;
    return left;
}

template <typename Enum, typename Other>
    requires flagpp::enabled<Enum> and flagpp::allowed<Enum, Other>
constexpr auto &operator^=(Enum &left, Other right)
{
    left = flagpp::wrapper{left} ^ right;
    return left;
}

template <typename Enum>
    requires flagpp::enabled<Enum>
constexpr auto operator~(Enum value)
{
    return ~flagpp::wrapper{value};
}

#include "flags.inl"
