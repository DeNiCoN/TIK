#include <cstdint>
#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <span>

namespace tik
{
    namespace crc
    {
        std::bitset<16> CRC_16_ANSI(0b1100000000000101);
        std::bitset<32> CRC_32(0b100000100110000010001110110110111);
        std::bitset<32> CRC_32C(0b101110100000110111000110011010111);


        template <std::size_t N>
        constexpr std::array<std::bitset<N*8>, 256>
        build_table(const std::bitset<N*8>& poly)
        {
            constexpr auto Bits = N*8;
            std::array<std::bitset<Bits>, 256> result;
            for (std::uint8_t i = 0; i <= 255; i++)
            {
                std::uint8_t head = i;
                std::bitset<Bits> entry;
                for (int j = 7; j >= 0; j--)
                {
                    if (head & (1 << j))
                    {
                        head ^= poly >> (Bits - j + 1);
                        entry ^= poly << (j + 1);
                    }
                }
            }
        }

        template <std::size_t N>
        void encode(const std::filesystem::path& from,
                    const std::filesystem::path& to,
                    const std::bitset<N>& poly)
        {
            constexpr auto Bytes = N/8;
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(to, std::ios::binary);

            std::array<char, 4 * 1024 - Bytes> block;

            while(input.read(block.data(), block.size() - Bytes))
            {

            }
        }

        template <std::size_t Size, std::size_t N>
        std::bitset<N> encode_span(std::span<const char, Size> span,
                                   const std::bitset<N>& poly)
        {
            constexpr auto table = build_table(poly);
            constexpr auto Bytes = N/8;
            std::bitset<N> result;


        }
    }
}
