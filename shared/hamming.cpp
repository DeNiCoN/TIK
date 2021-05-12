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
        {
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(to, std::ios::binary);

            output.put(char(Type::EXTENDED));

            utils::BitGetter bit_getter(input);

            while (bit_getter)
            {
                std::bitset<4> code;
                std::bitset<16> out;

                for (unsigned i = 0; i < 11; ++i)
                {
                    if (bit_getter.get())
                    {
                        unsigned mapped;
                        if (i == 0)
                            mapped = 3;
                        else if (i < 4)
                            mapped = i + 4;
                        else
                            mapped = i + 5;

                        out[mapped] = true;
                        code ^= mapped;
                    }
                }
                out[1] = code[0];
                out[2] = code[1];
                out[4] = code[2];
                out[8] = code[3];

                out[0] = out.count() % 2;
                uint16_t o = out.to_ulong();
                output.write(reinterpret_cast<const char*>(&o), sizeof(o));
            }
        }

        void encode_not_extended(const std::filesystem::path& from,
                                 const std::filesystem::path& to)
        {
            std::ifstream input(from, std::ios::binary);
            std::ofstream output(to, std::ios::binary);

            output.put(char(Type::NORMAL));

            utils::BitGetter bit_getter(input);

            while (bit_getter)
            {
                std::bitset<4> code;
                std::bitset<16> out;

                for (unsigned i = 0; i < 11; ++i)
                {
                    if (bit_getter.get())
                    {
                        unsigned mapped;
                        if (i == 0)
                            mapped = 3;
                        else if (i < 4)
                            mapped = i + 4;
                        else
                            mapped = i + 5;

                        out[mapped] = true;
                        code ^= mapped;
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
            std::ofstream output(to, std::ios::binary);

            utils::BitWriter bit_writer(output);
            bit_writer.discard_last() = true;

            unsigned char type_char = input.get();
            if (type_char > 2)
            {
                std::cerr << "Unknown type" << std::endl;
                throw std::exception();
            }

            Type type = static_cast<Type>(type_char);

            uint16_t current;
            while(input.read(reinterpret_cast<char*>(&current), sizeof(current)))
            {
                std::bitset<sizeof(current) * 8> current_set = current;

                std::size_t flip = 0;
                for (std::size_t i = 0; i < current_set.size(); ++i)
                {
                    if (current_set[i])
                    flip ^= i;
                }

                if (type == Type::NORMAL)
                {
                    current_set.flip(flip);
                }
                else if(type == Type::EXTENDED)
                {
                    if (current_set.count() % 2)
                    {
                        //single error
                        current_set.flip(flip);
                    }
                    else
                    {
                        if (flip)
                        {
                            //double
                            std::cerr << "double error" << std::endl;
                        }
                    }
                }

                std::bitset<11> result;
                result[0] = current_set[3];
                result[1] = current_set[5];
                result[2] = current_set[6];
                result[3] = current_set[7];
                result[4] = current_set[9];
                result[5] = current_set[10];
                result[6] = current_set[11];
                result[7] = current_set[12];
                result[8] = current_set[13];
                result[9] = current_set[14];
                result[10] = current_set[15];
                bit_writer.write(result);
            }
        }

        void check(const std::filesystem::path& input_file, std::ostream& log_stream)
        {
            std::ifstream input(input_file, std::ios::binary);

            unsigned char type_char = input.get();
            if (type_char > 2)
            {
                log_stream << "Wrong header\n";
                return;
            }

            Type type = static_cast<Type>(type_char);

            if (type == Type::NORMAL)
            {
                log_stream << "Hamming(15, 11)\n";
            }
            else
            {
                log_stream << "Hamming(16, 11)\n";
            }

            uint16_t current;
            std::size_t cur_code_index = 0;
            while(input.read(reinterpret_cast<char*>(&current), sizeof(current)))
            {
                std::bitset<sizeof(current) * 8> current_set = current;

                std::size_t flip = 0;
                for (std::size_t i = 0; i < current_set.size(); ++i)
                {
                    if (current_set[i])
                    flip ^= i;
                }

                if (type == Type::NORMAL)
                {
                    if (flip)
                    {
                        log_stream << "Single error at codeword " << cur_code_index + 1
                                   << " bit " << flip << " (" << cur_code_index * 16 + flip
                                   << ")\n";
                    }
                }
                else if(type == Type::EXTENDED)
                {
                    if (current_set.count() % 2)
                    {
                        //single error
                        if (flip)
                        {
                            log_stream << "Single error at codeword " << cur_code_index + 1
                                       << " bit " << flip << " (" << cur_code_index * 16 + flip
                                       << ")\n";
                        }
                    }
                    else
                    {
                        if (flip)
                        {
                            log_stream << "Double error at codeword " << cur_code_index + 1
                                       << "\n";
                        }
                    }
                }
                cur_code_index++;
            }
        }
    }
}
