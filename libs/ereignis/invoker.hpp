#pragma once
#include <map>
#include <tuple>
#include <ranges>
#include <functional>

namespace ereignis
{
    template <typename T>
    concept callback_list = requires(T t) {
        []<typename A, typename B>(std::map<A, std::function<B>> &) {
        }(t);
    };

    template <typename T, typename... P>
    concept callback_parameters =
        requires(T t, std::tuple<P...> p) { callback_list<T>, std::apply(t.begin()->second, p); };

    template <callback_list T, typename... P>
        requires callback_parameters<T, P...>
    class invoker
    {
        class iterator;
        using args_t = std::tuple<P...>;

      private:
        T m_callbacks;
        args_t m_args;

      public:
        invoker(T callbacks, args_t &&args);

      public:
        auto end();
        auto begin();
    };

} // namespace ereignis

template <ereignis::callback_list T, typename... P>
    requires ereignis::callback_parameters<T, P...>
constexpr inline bool std::ranges::enable_view<ereignis::invoker<T, P...>> = true;

#include "invoker.inl"
