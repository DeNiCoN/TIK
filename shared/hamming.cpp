#include "hamming.hpp"
#include <bitset>
#include <fstream>
#include "bit_getter.hpp"

namespace tik
{
    namespace hamming
    {
        void encode_not_extended(const std::filesystem::path& from,
                                 const std::filesystem::path& to, int check)
        {
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(from, std::ios::binary);

            utils::BitGetter bit_getter(input);
        }
    }
}
