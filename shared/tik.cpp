#include "tik.hpp"
#include "header.hpp"

#include <fstream>

namespace tik
{
    void encode_shannon_fano(const std::filesystem::path& from,
                             const std::filesystem::path& to)
    {
        std::ifstream in(from, std::ios::binary);
        CodeTree tree = CodeTree::build_shannon_fano(in);

        std::ofstream out(to, std::ios::binary);
        CodeTree::serialize(tree, out);
        out << std::filesystem::file_size(from);

        in = std::ifstream(from, std::ios::binary);
        tree.encode(in, out);
    }

    void encode_huffman(const std::filesystem::path& from,
                        const std::filesystem::path& to)
    {
        std::ifstream in(from, std::ios::binary);
        CodeTree tree = CodeTree::build_huffman(in);

        std::ofstream out(to, std::ios::binary);
        CodeTree::serialize(tree, out);
        out << std::filesystem::file_size(from);

        in = std::ifstream(from, std::ios::binary);
        tree.encode(in, out);
    }

    void decode(const std::filesystem::path& from,
                const std::filesystem::path& to)
    {
        std::ifstream in(from, std::ios::binary);
        auto tree = CodeTree::deserialize(in);

        std::uintmax_t num_chars;
        in >> num_chars;

        std::ofstream out(to);
        tree.decode(in, out, num_chars);
    }
}
