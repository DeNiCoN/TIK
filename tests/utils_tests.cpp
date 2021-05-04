#include <gtest/gtest.h>
#include "bit_writer.hpp"
#include <sstream>

using namespace tik;
using namespace tik::utils;

TEST(BitWriter, write)
{
    std::string input = "ABCDEF";
    std::ostringstream os;

    BitWriter writer(os);

    writer.write(std::bitset<3>(0b010));
    writer.write(std::bitset<3>(0b011));
}
