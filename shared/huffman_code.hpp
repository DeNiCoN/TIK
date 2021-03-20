#pragma once
#include <iostream>
#include <filesystem>
#include "tree.hpp"

namespace huffman_code
{
    void encode(std::istream& from, std::ostream& to, const Tree&);
    void decode(std::istream& from, std::ostream& to, const Tree&);

    void encode(const std::filesystem::path& from,
                const std::filesystem::path& to);
    void decode(const std::filesystem::path& from,
                const std::filesystem::path& to);
}
