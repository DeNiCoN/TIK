#include "utils.hpp"
#include <cstdint>
#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <algorithm>

namespace tik
{
    namespace crc
    {
        constexpr std::size_t BLOCK_SIZE = 1024;
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
        constexpr auto encode_span_constexpr(std::span<char, Size> span)
        {
            constexpr auto table = build_table(poly);
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            std::uintmax_t result_impl = 0;

            auto mask = utils::get_consecutive_bits(N);
            for (std::size_t i = 0; i < span.size(); i++)
            {
                auto next = (result_impl >> (N - 8)) & mask;
                unsigned char cur = next & 0xFF;
                result_impl = (result_impl << 8) & mask;
                result_impl ^= utils::to_uintmax(table[cur ^ static_cast<unsigned char>(span[i])]) & mask;
            }
            return result_impl;
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
        constexpr auto build_crc_bit_error_array()
        {
            constexpr auto Bytes = poly.size()/8;
            std::array<std::pair<unsigned long, std::size_t>, BLOCK_SIZE * 8> result;
            std::array<char, BLOCK_SIZE> buffer{};
            for (std::size_t i = 0; i < BLOCK_SIZE * 8; i++)
            {
                utils::flip_bit(buffer.begin(), i);
                result[i] = std::pair(
                    encode_span_constexpr<poly>(std::span(buffer)), i);
                utils::flip_bit(buffer.begin(), i);
            }
            std::ranges::sort(result, std::ranges::less{},
                              [](const auto& p) { return p.first; });
            std::cout << "Done building table\n";
            return result;
        }

        template <const auto& poly>
        void check(const std::filesystem::path& from, std::ostream& log_stream)
        {
            constexpr auto N = poly.size();
            constexpr auto Bytes = N/8;
            const auto bit_table = build_crc_bit_error_array<poly>();
            std::ifstream input(from, std::ios::binary);
            std::array<char, BLOCK_SIZE> block;

            auto get_crc_from_memory = [](auto span)
            {
                unsigned long result = 0;
                for (std::size_t i = span.size(); i != 0; i--)
                {
                    result |= (static_cast<unsigned char>(span[i - 1]) << ((i - 1) * 8));
                }
                return result;
            };

            auto check_and_log = [&](auto span, std::size_t i)
            {
                auto initial_crc =
                    get_crc_from_memory(span.last(Bytes));
                auto crc = encode_span<poly>(span).to_ulong();

                log_stream << "Checking block " << i << ", crc: "
                           << initial_crc << std::endl;
                if (crc != 0)
                {
                    auto range = std::ranges::equal_range(bit_table, crc, std::ranges::less {},
                                                          [](const auto& pair) { return pair.first; });
                    if (range.begin() != range.end())
                    {
                        log_stream << "possible 1 bit error at block bit "
                                   << range.begin()->second << std::endl;
                    }
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
