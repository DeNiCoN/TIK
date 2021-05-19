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
                        head ^= poly_uint >> (N - j - 1);
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
                auto next = (result >> (N - 8));
                unsigned char cur = next.to_ulong() & 0xFF;
                result <<= 8;
                result ^= table[cur ^ static_cast<unsigned char>(span[i])];
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

            auto encode_and_write = [&](auto span)
            {
                auto crc = encode_span<poly>(span).to_ulong();
                crc <<= (sizeof(crc) - Bytes)*8;
                crc = utils::swap_endian(crc);
                output.write(span.data(), span.size());
                output.write(reinterpret_cast<const char*>(&crc), Bytes);
            };

            while(input.read(block.data(), block.size()))
            {
                encode_and_write(std::span(block));
            }
            //Last block
            if (input.gcount())
            {
                encode_and_write(std::span(block.data(), input.gcount()));
            }
        }

        template <const auto& poly>
        void check(const std::filesystem::path& from, std::ostream& log_stream)
        {
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            std::ifstream input(from, std::ios::binary);
            std::array<char, BLOCK_SIZE> block;

            auto get_crc_from_memory = [](auto span)
            {
                unsigned long result = 0;
                for (std::size_t i = span.size(); i != 0; i--)
                {
                    result |= (static_cast<unsigned char>(span[i]) << (i * 8));
                }
                return result;
            };

            auto check_and_log = [&](auto span, std::size_t i)
            {
                auto initial_crc =
                    get_crc_from_memory(span.subspan(span.size() - Bytes));
                auto crc = encode_span<poly>(span).to_ulong();

                log_stream << "Checking block " << i << ", crc: "
                           << initial_crc << std::endl;
                if (crc != 0)
                {
                    log_stream << "failed: " << crc << "\n\n";
                }
                else
                {
                    log_stream << "successful: " << crc << "\n\n";
                }
            };

            std::size_t i = 0;
            while(input.read(block.data(), block.size()))
            {
                check_and_log(std::span(block), i);
                i++;
            }

            if (input.gcount())
            {
                check_and_log(std::span(block.data(), input.gcount()), i);
            }
        }

        template <const auto& poly>
        void decode(const std::filesystem::path& from,
                    const std::filesystem::path& to)
        {
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(to, std::ios::binary);
            std::array<char, BLOCK_SIZE> block;

            while(input.read(block.data(), block.size()))
            {
                output.write(block.data(), block.size() - Bytes);
            }
            output.write(block.data(), input.gcount() - Bytes);
        }
    }
}
