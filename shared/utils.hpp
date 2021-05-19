#pragma once
#include <cstddef>
#include <iterator>
#include <iostream>
#include <bitset>
#include <climits>

namespace tik
{
    namespace utils
    {

        template<class Iterator>
        void flip_bit(const Iterator begin, const std::size_t pos)
        {
            constexpr auto unit = sizeof(std::iter_value_t<Iterator>) * 8;
            const std::size_t jump = pos / unit;
            const std::size_t remain = pos % unit;
            const auto char_it = std::next(begin, jump);
            *char_it = std::bitset<unit>(*char_it).flip(remain).to_ulong();
        }

        template<std::size_t N>
        constexpr std::uintmax_t to_uintmax(const std::bitset<N>& bitset)
        {
            std::uintmax_t result = 0;
            for (unsigned i = 0; i < N; i++)
            {
                result |= bitset[i] << i;
            }
            return result;
        }

        template <typename T>
        T swap_endian(T u)
        {
            static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

            union
            {
                T u;
                unsigned char u8[sizeof(T)];
            } source, dest;

            source.u = u;

            for (size_t k = 0; k < sizeof(T); k++)
                dest.u8[k] = source.u8[sizeof(T) - k - 1];

            return dest.u;
        }
    }
}
