/**
 * Source:
 *   https://stackoverflow.com/a/7115547
 * Description:
 *   Defines hashes for tuples, for use in sets and maps.
 */

#pragma once

#include <stddef.h>
#include "../components/descriptor.hpp"
#include "../components/epn.hpp"

namespace hash_custom
{
    /**
     * @brief Applies std::hash to the argument.
     */
    template <typename T>
    struct hash
    {
        size_t
        operator()(T const& tt) const
        {
            return std::hash<T>()(tt);
        }
    };

    namespace
    {
        /**
         * @brief Recursively combines hashes.
         */
        template <class T>
        inline void hash_combine(std::size_t& seed, T const& v)
        {
            seed ^= hash_custom::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        /**
         * @brief Uses recursion to hash every tuple element.
         */
        template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl
        {
            static void apply(size_t& seed, Tuple const& tuple)
            {
                HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
                hash_combine(seed, std::get<Index>(tuple));
            }
        };

        /**
         * @brief Hashes the first tuple element.
         */
        template <class Tuple>
        struct HashValueImpl<Tuple, 0>
        {
            static void apply(size_t& seed, Tuple const& tuple)
            {
                hash_combine(seed, std::get<0>(tuple));
            }
        };
    }

    /**
     * Hash overload for tuple.
     */
    template<typename... T>
    struct hash<std::tuple<T...>>
    {
        size_t
        operator()(std::tuple<T...> const& tt) const noexcept
        {
            size_t seed = 0;
            HashValueImpl<std::tuple<T...> >::apply(seed, tt);
            return seed;
        }
    };

    /**
     * Hash overload for vector.
     */
    template<typename... T>
    struct hash<std::vector<T...>>
    {
        size_t
        operator()(std::vector<T...> const& vec) const noexcept
        {
            std::size_t seed = vec.size();
            for(auto i : vec) {
                hash_combine(seed, i);
            }
            return seed;
        }
    };

    /**
     * Hash overload for Descriptor.
     */
    template<>
    struct hash<Descriptor>
    {
        size_t
        operator()(const Descriptor& d) const noexcept
        {
            return d.hash();
        }
    };

    /**
     * Hash overload for EPN.
     */
    template<>
    struct hash<EPN>
    {
        size_t
        operator()(const EPN& e) const noexcept
        {
            return e.hash();
        }
    };
}