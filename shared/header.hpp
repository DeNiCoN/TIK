#pragma once
#include <unordered_map>
#include <cstdint>

namespace huffman_code
{
    template<typename Char, typename Integer>
    class Header
    {
        using Table = std::unordered_map<Char, Integer>;
    public:
    private:
        std::uint16_t m_length;
        Table m_table;
    };
}
