#pragma once
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "bit_getter.hpp"

namespace tik
{
    class CodeTree
    {
    public:
        static void serialize(const CodeTree& tree, std::ostream& out);
        static CodeTree deserialize(std::istream& in);

        static CodeTree build_shannon_fano(std::istream& in);
        static CodeTree build_huffman(std::istream& in);

        struct Code
        {
            unsigned value;
            unsigned length;

            bool operator==(const Code& rhs) const
            {
                return (value == rhs.value) && (length == rhs.length);
            }

            bool operator!=(const Code& rhs) const
            {
                return !(*this == rhs);
            }
        };

        const Code& map_char(char c) const { return m_char_code.at(c); }

        void encode(std::istream& in, std::ostream& out);
        void decode(std::istream& in, std::ostream& out, std::uintmax_t num_chars);

        bool operator==(const CodeTree& rhs) const
        {
            return this->m_char_code == rhs.m_char_code;
        }

        bool operator!=(const CodeTree& rhs) const
        {
            return !(*this == rhs);
        }

    private:
        struct Node
        {
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            Code code;
            char value;

            bool is_leaf() const { return !(left && right); }
        };

        class Decoder
        {
        public:
            Decoder(const CodeTree& tree, std::istream& in, std::size_t num_chars)
                : m_tree(tree), m_bit_getter(in),
                  m_node(m_tree.root()), m_num_chars(num_chars)
            {
            }

            char get();
            explicit operator bool() const { return m_num_chars; }
        private:

            void update();

            const CodeTree& m_tree;
            const CodeTree::Node* m_node;
            std::size_t m_num_chars;
            utils::BitGetter m_bit_getter;
        };

        explicit CodeTree(std::unique_ptr<Node> root);
        explicit CodeTree(std::unordered_map<char, Code> char_code);

        const Node* root() const { return m_root.get(); }

        std::unordered_map<char, Code> m_char_code;
        std::unique_ptr<Node> m_root;

        friend class Encoder;
    };
}
