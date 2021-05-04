#include "hamming.hpp"
#include <bitset>
#include <fstream>
#include "bit_getter.hpp"
#include "bit_writer.hpp"

namespace tik
{
    namespace hamming
    {
        enum class Type
        {
            NORMAL,
            EXTENDED
        };

        void encode(const std::filesystem::path& from,
                    const std::filesystem::path& to)
        {}

        void encode_not_extended(const std::filesystem::path& from,
                                 const std::filesystem::path& to)
        {
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(from, std::ios::binary);

            output.put(char(Type::NORMAL));

            utils::BitGetter bit_getter(input);

            while (bit_getter)
            {
                std::bitset<4> code;
                std::bitset<16> out;

                bool current;
                current = bit_getter.get();
                for (unsigned i = 0; i < 11; ++i)
                {
                    if (bit_getter.get())
                    {
                        if (i == 0)
                            out[3] = true;
                        else if (i < 4)
                            out[i + 4] = true;
                        else
                            out[i + 5] = true;

                        code ^= i;
                    }
                }
                out[1] = code[0];
                out[2] = code[1];
                out[4] = code[2];
                out[8] = code[3];
                uint16_t o = out.to_ulong();
                output.write(reinterpret_cast<const char*>(&o), sizeof(o));
            }
        }

        void decode(const std::filesystem::path& from,
                    const std::filesystem::path& to)
        {
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(from, std::ios::binary);

            utils::BitWriter bit_writer(output);

            unsigned char type_char = input.get();
            if (type_char > 2)
                throw std::exception();

            Type type = static_cast<Type>(type_char);

            while(output)
            {
                uint16_t current;
                input.read(reinterpret_cast<char*>(&current), sizeof(current));
                std::bitset<sizeof(current) * 8> current_set = current;

                std::size_t result = 0;
                for (std::size_t i = 0; i < current_set.size(); ++i)
                {
                    result ^= current_set[i];
                }

                if (type == Type::NORMAL)
                {
                    current_set.flip(result);
                }
                else if(type == Type::EXTENDED)
                {
                    //TODO extended
                }

                bit_writer.write(current_set);
            }
        }
    }
}
