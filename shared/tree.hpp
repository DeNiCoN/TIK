#pragma once
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

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

            bool operator==(const Code& rhs)
            {
                return (value == rhs.value) && (length == rhs.length);
            }

            bool operator!=(const Code& rhs)
            {
                return !(*this == rhs);
            }
        };

        const Code& map_char(char c) const { return m_char_code.at(c); }

        void encode(std::istream& in, std::ostream& out);
        void decode(std::istream& in, std::ostream& out, std::uintmax_t num_chars);
    private:
        struct Node
        {
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            Code code;
            char value;

            bool is_leaf() const { return !(left && right); }
        };

        class Encoder
        {
        public:
            Encoder(const CodeTree& tree, std::ostream& out)
                : m_tree(tree), m_out(out)
            {}
            ~Encoder();
            void put(CodeTree::Code code);
        private:
            const CodeTree& m_tree;
            std::ostream& m_out;
            char m_current = 0;
            unsigned m_length = 0;
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
            class BitGetter
            {
            public:
                BitGetter(std::istream& in)
                    : m_in(in), m_current(in.get())
                {}
                bool get();
            private:
                unsigned m_length = 0;
                std::istream& m_in;
                char m_current;

            };

            void update();

            const CodeTree& m_tree;
            const CodeTree::Node* m_node;
            std::size_t m_num_chars;
            BitGetter m_bit_getter;
        };

        explicit CodeTree(std::unique_ptr<Node> root);
        explicit CodeTree(std::unordered_map<char, Code> char_code);

        const Node* root() const { return m_root.get(); }

        std::unordered_map<char, Code> m_char_code;
        std::unique_ptr<Node> m_root;

        friend class Encoder;
    };
}
