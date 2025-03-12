#pragma once
#include "invoker.hpp"

#include <iterator>

namespace ereignis
{
    template <callback_list T, typename... P>
        requires callback_parameters<T, P...>
    class invoker<T, P...>::iterator
    {
        using iterator_t = typename T::iterator;

      public:
        using value_type        = typename T::mapped_type::result_type;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;

      private:
        iterator_t m_iterator;
        args_t *m_args;

      public:
        iterator() = default;

      public:
        iterator(iterator_t iterator, args_t &args) : m_iterator(iterator), m_args(&args) {}

      public:
        value_type operator*()
        {
            return std::apply(m_iterator->second, *m_args);
        }

        value_type operator*() const
        {
            return std::apply(m_iterator->second, *m_args);
        }

      public:
        iterator &operator++()
        {
            m_iterator++;
            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            m_iterator++;
            return copy;
        }

      public:
        bool operator==(const iterator &other) const
        {
            return m_iterator == other.m_iterator;
        }
    };

    template <callback_list T, typename... P>
        requires callback_parameters<T, P...>
    invoker<T, P...>::invoker(T callbacks, args_t &&args) : m_callbacks(std::move(callbacks)), m_args(std ::move(args))
    {
    }

    template <callback_list T, typename... P>
        requires callback_parameters<T, P...>
    auto invoker<T, P...>::begin()
    {
        return iterator(m_callbacks.begin(), m_args);
    }

    template <callback_list T, typename... P>
        requires callback_parameters<T, P...>
    auto invoker<T, P...>::end()
    {
        return iterator(m_callbacks.end(), m_args);
    }
} // namespace ereignis
