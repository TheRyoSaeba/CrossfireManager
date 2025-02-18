#pragma once
#include "invoker.hpp"

#include <map>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <functional>

namespace ereignis
{
    template <typename T>
    concept callback = requires(T t) { std::function{t}; };

    template <typename T, typename O>
    concept valid_callback = requires(O callback) { std::function<T>{callback}; };

    template <typename Callback, typename... T>
    concept valid_arguments = requires(Callback callback, T &&...args) { callback(std::forward<T>(args)...); };

    template <auto Id, callback Callback>
    class event
    {
      private:
        std::mutex m_mutex;
        std::atomic_size_t m_counter{0};
        std::map<std::size_t, std::function<Callback>> m_callbacks;

      public:
        void clear();

      public:
        void remove(std::size_t id);

      public:
        template <typename T>
            requires valid_callback<Callback, T>
        std::size_t add(T &&callback);

        template <typename T>
            requires valid_callback<Callback, T>
        void once(T &&callback);

      public:
        template <typename... T>
            requires valid_arguments<Callback, T...>
        auto fire(T &&...args);

        template <typename... T>
            requires valid_arguments<Callback, T...>
        auto fire(T &&...args) const;

      public:
        static constexpr auto id = Id;
    };
} // namespace ereignis

#include "event.inl"
