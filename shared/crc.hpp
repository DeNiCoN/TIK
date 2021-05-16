#include "utils.hpp"
#include <cstdint>
#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>

namespace tik
{
    namespace crc
    {
        constexpr std::size_t BLOCK_SIZE = 4 * 1024;
        constexpr std::bitset<16> CRC_16_ANSI(0b1100000000000101);
        constexpr std::bitset<32> CRC_32(0b100000100110000010001110110110111);
        constexpr std::bitset<32> CRC_32C(0b101110100000110111000110011010111);


        template <std::size_t N>
        constexpr std::array<std::bitset<N>, 256>
        build_table(const std::bitset<N>& poly)
        {
            constexpr auto Bytes = N/8;
            std::array<std::bitset<N>, 256> result;
            auto poly_uint = utils::to_uintmax(poly);
            for (unsigned i = 0; i < 256; i++)
            {
                std::uint8_t head = i;
                std::uintmax_t entry = 0;
                for (int j = 7; j >= 0; j--)
                {
                    if (head & (1 << j))
                    {
                        head ^= poly_uint >> (N - j + 1);
                        entry ^= poly_uint << (j + 1);
                    }
                }
                result[i] = std::bitset<N>(entry);
            }
            return result;
        }

        template <const auto& poly, std::size_t Size>
        auto encode_span(std::span<char, Size> span)
        {
            constexpr auto table = build_table(poly);
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            std::bitset<N> result;

            for (std::size_t i = 0; i < span.size(); i++)
            {
                char cur = (result >> (N - 8)).to_ulong() & 0xFF;
                result <<= 8;
                result ^= table[cur ^ span[i]];
            }

            return result;
        }

        template <const auto& poly>
        void encode(const std::filesystem::path& from,
                    const std::filesystem::path& to)
        {
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(to, std::ios::binary);

            std::array<char, BLOCK_SIZE - Bytes> block;

            while(input.read(block.data(), block.size() - Bytes))
            {
                std::cout << input.gcount() << std::endl;
                auto crc = encode_span<poly>(std::span(block)).to_ulong();
                output.write(block.data(), block.size());
                output.write(reinterpret_cast<const char*>(&crc), Bytes);
            }
            std::cout << input.gcount() << " out" << std::endl;
            //Last block
            if (input.gcount())
            {
                std::span last(block.data(), input.gcount());
                auto crc = encode_span<poly>(last).to_ulong();
                output.write(last.data(), last.size());
                output.write(reinterpret_cast<const char*>(&crc), Bytes);
            }
        }

        template <const auto& poly>
        void check(const std::filesystem::path& from, std::ostream& log_stream)
        {
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            std::ifstream input(from, std::ios::binary);
            std::array<char, BLOCK_SIZE> block;

            while(input.read(block.data(), block.size() - Bytes))
            {
                auto crc = encode_span<poly>(std::span(block));
                log_stream << crc << std::endl;
            }

            if (input.gcount())
            {
                std::span last(block.data(), input.gcount());
                auto crc = encode_span<poly>(last).to_ulong();
                log_stream << crc << std::endl;
            }
        }
    }
}
