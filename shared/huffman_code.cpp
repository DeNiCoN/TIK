#include "huffman_code.hpp"
#include <fstream>

namespace huffman_code
{
    void encode(const std::filesystem::path& from,
                const std::filesystem::path& to)
    {
        std::ifstream in(from, std::ios::binary);
        std::ofstream out(to, std::ios::binary);
        auto tree = Tree::from_stream(in);
        encode(in, out, tree);
    }
}
