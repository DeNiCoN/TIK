#include <gtest/gtest.h>
#include "tree.hpp"

using namespace tik;

TEST(Tree, Build)
{
    {
        std::string input = "AAAAABAAAABAAABBBCBBCCCACCDDDADDDEEEEAE";
        std::istringstream is(input);
        EXPECT_NO_THROW(CodeTree::build_huffman(is));
    }
    {
        std::string input = "AAAAABAAAABAAABBBCBBCCCACCDDDADDDEEEEAE";
        std::istringstream is(input);
        EXPECT_NO_THROW(CodeTree::build_shannon_fano(is));
    }
}

TEST(Tree, EncodeDecodeShannon)
{
    std::string input = "AAAAABAAAABAAABBBCBBCCCACCDDDADDDEEEEAE";
    std::istringstream is(input);

    auto tree = CodeTree::build_shannon_fano(is);

    is = std::istringstream(input);
    std::stringstream ss;
    std::ostringstream os;
    tree.encode(is, ss);
    tree.decode(ss, os, input.length());

    std::string decoded = os.str();
    EXPECT_TRUE(input == decoded);
}


TEST(Tree, EncodeDecodeHuffman)
{
    std::string input = "AAAAABAAAABAAABBBCBBCCCACCDDDADDDEEEEAE";
    std::istringstream is(input);

    auto tree = CodeTree::build_huffman(is);

    is = std::istringstream(input);
    std::stringstream ss;
    std::ostringstream os;
    tree.encode(is, ss);
    tree.decode(ss, os, input.length());

    std::string decoded = os.str();
    EXPECT_TRUE(input == decoded);
}

TEST(Tree, SerializeDeserialize)
{
    std::string input = "AAAAABAAAABAAABBBCBBCCCACCDDDADDDEEEEAE";
    std::istringstream is(input);
    std::stringstream ss;

    auto tree = CodeTree::build_huffman(is);

    CodeTree::serialize(tree, ss);
    auto deserialized_tree = CodeTree::deserialize(ss);

    EXPECT_TRUE(tree == deserialized_tree);
};
