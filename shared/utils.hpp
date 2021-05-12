#pragma once
#include <cstddef>
#include <iterator>
#include <bitset>

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
    }
}
