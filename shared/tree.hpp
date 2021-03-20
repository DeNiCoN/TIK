#pragma once
#include <iostream>

namespace huffman_code
{
    class Tree
    {
    public:
        static Tree from_stream(std::istream& in);
    };
}
