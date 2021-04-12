#pragma once
#include <iostream>
#include <filesystem>
#include "tree.hpp"

namespace tik
{
    void encode_shannon_fano(const std::filesystem::path& from,
                             const std::filesystem::path& to);
    void encode_huffman(const std::filesystem::path& from,
                        const std::filesystem::path& to);
    void decode(const std::filesystem::path& from,
                const std::filesystem::path& to);
}
