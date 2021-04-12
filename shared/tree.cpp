#include "tree.hpp"
#include <cassert>
#include <exception>
#include <map>
#include <ranges>
#include <algorithm>

namespace tik
{

    void CodeTree::serialize(const CodeTree &tree, std::ostream &out)
    {
        const auto size = tree.m_char_code.size();
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (const auto& [c, code] : tree.m_char_code)
        {
            out.put(c);
            out.write(reinterpret_cast<const char*>(&code.value), sizeof(code.value));
            out.write(reinterpret_cast<const char*>(&code.length), sizeof(code.length));
        }
    }

    CodeTree CodeTree::deserialize(std::istream& in)
    {
        std::size_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));

        std::unordered_map<char, Code> char_code;
        for (size_t i = 0; i < size; ++i)
        {
            char c = in.get();
            Code code;
            in.read(reinterpret_cast<char*>(&code.value), sizeof(code.value));
            in.read(reinterpret_cast<char*>(&code.length), sizeof(code.length));
            char_code[c] = code;
        }

        return CodeTree(std::move(char_code));
    }

    CodeTree CodeTree::build_shannon_fano(std::istream &in)
    {
        std::map<char, unsigned> counts;

        unsigned sum = 0;
        char c;
        while (in.get(c))
        {
            ++counts[c];
            ++sum;
        }

        std::vector<std::pair<double, char>> probabilities;
        probabilities.reserve(counts.size());

        for (const auto& [c, n] : counts)
        {
            probabilities.push_back(std::make_pair(n/static_cast<double>(sum), c));
        }
        std::ranges::sort(probabilities);

        auto build_tree = [] (auto begin, auto end, auto& self_ref, auto* parent)
        {
            auto back = end - 1;
            if (begin == back)
            {
                parent->value = begin->second;
                return;
            }

            auto front = begin;
            double sum_left = 0.0;
            double sum_right = 0.0;

            while (front <= back)
            {
                if (sum_left <= sum_right)
                {
                    sum_left += front->first;
                    front++;
                }
                else
                {
                    sum_right += back->first;
                    back--;
                }
            }

            const Code left_code = {static_cast<unsigned>(parent->code.value),
                parent->code.length + 1};
            const Code right_code = {static_cast<unsigned>(parent->code.value | (1 << (parent->code.length))),
                parent->code.length + 1};

            auto left = std::make_unique<Node>(Node{nullptr, nullptr,
                                                    left_code, '\0'});

            auto right = std::make_unique<Node>(Node{nullptr, nullptr,
                                                     right_code, '\0'});

            parent->left = move(left);
            parent->right = move(right);

            self_ref(begin, front, self_ref, parent->left.get());
            self_ref(front, end, self_ref, parent->right.get());
        };

        auto root = std::make_unique<Node>(Node{nullptr, nullptr, {'\0', 0}, '\0'});
        build_tree(probabilities.begin(), probabilities.end(), build_tree, root.get());

        return CodeTree(std::move(root));
    }

    CodeTree::CodeTree(std::unique_ptr<Node> root)
        : m_root(std::move(root))
    {
        auto traverse = [&](const Node* node, auto& self_ref)
        {
            if (node->is_leaf())
            {
                m_char_code[node->value] = node->code;
                return;
            }

            if (node->left)
                self_ref(node->left.get(), self_ref);

            if (node->right)
                self_ref(node->right.get(), self_ref);
        };

        traverse(m_root.get(), traverse);
    }

    CodeTree::CodeTree(std::unordered_map<char, Code> char_code)
        : m_char_code(std::move(char_code)),
          m_root(new Node {nullptr, nullptr, {'\0', 0}, '\0'})
    {
        const auto add = [](Node* parent, char c, Code code, const auto& self_ref)
        {
            if (parent->code.length == code.length)
            {
                parent->value = c;
                return;
            }
            const bool right = (code.value >> parent->code.length) & 1;
            if (right)
            {
                if (!parent->right)
                {
                    const Code right_code = {static_cast<unsigned>(parent->code.value | (1 << (parent->code.length))),
                        parent->code.length + 1};
                    parent->right = std::move(std::make_unique<Node>(Node {nullptr, nullptr, right_code, '\0'}));
                }

                self_ref(parent->right.get(), c, code, self_ref);
            }
            else
            {
                if (!parent->left)
                {
                    const Code left_code = {static_cast<unsigned>(parent->code.value),
                        parent->code.length + 1};
                    parent->left = std::move(std::make_unique<Node>(Node {nullptr, nullptr, left_code, '\0'}));
                }
                self_ref(parent->left.get(), c, code, self_ref);
            }
        };

        for (const auto& [c, code] : m_char_code)
        {
            add(m_root.get(), c, code, add);
        }
    }

    void CodeTree::Encoder::put(CodeTree::Code code)
    {
        unsigned left = code.length;
        unsigned value = code.value;
        while (left > 0)
        {
            if (left + m_length >= sizeof(char) * 8)
            {
                m_current |= (value << m_length);
                unsigned writed = sizeof(char) * 8 - m_length;
                m_out.put(m_current);

                m_current = 0;
                m_length = 0;

                value = value >> writed;
                left -= writed;

            }
            else
            {
                m_current |= (value << m_length);
                m_length += left;

                value = value >> left;
                left = 0;
            }
        }
    }

    CodeTree::Encoder::~Encoder()
    {
        m_out.put(m_current);
    }

    bool CodeTree::Decoder::BitGetter::get()
    {
        if (m_length >= 8)
        {
            m_length = 0;
            m_current = m_in.get();
        }

        return (m_current >> m_length++) & 1;
    }

    //Invariant:
    //m_node - node with char that will be returned
    char CodeTree::Decoder::get()
    {
        auto traverse = [](const Node* node, BitGetter& bit_getter, auto& self_ref)
            -> const Node*
        {
            if (!node)
                return nullptr;

            if (node->is_leaf())
                return node;

            if (bit_getter.get())
                return self_ref(node->right.get(), bit_getter, self_ref);
            else
                return self_ref(node->left.get(), bit_getter, self_ref);
        };

        m_node = traverse(m_tree.root(), m_bit_getter, traverse);

        if (!m_node)
            throw std::invalid_argument("Unknown bit sequence during decoding");

        char result = m_node->value;
        --m_num_chars;
        return result;
    }

    void CodeTree::encode(std::istream& in, std::ostream& out)
    {
        CodeTree::Encoder encoder(*this, out);
        char c;
        while(in.get(c))
        {
            encoder.put(map_char(c));
        }
    }

    void CodeTree::decode(std::istream& in, std::ostream& out, std::uintmax_t num_chars)
    {
        CodeTree::Decoder decoder(*this, in, num_chars);
        while(decoder)
        {
            out.put(decoder.get());
        }
    }
}
