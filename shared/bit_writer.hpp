#pragma once
#include <iostream>
#include <bitset>

namespace tik
{
    namespace utils
    {
        class BitWriter
        {
        public:
            BitWriter(std::ostream& out)
                : m_out(out)
            {}

            template <std::size_t N>
            void write(std::bitset<N> value)
            {
                unsigned left = N;
                while (left > 0)
                {
                    if (left + m_length >= sizeof(char) * 8)
                    {
                        m_current |= (value << m_length).to_ulong();
                        unsigned writed = sizeof(char) * 8 - m_length;
                        m_out.put(m_current);

                        m_current = 0;
                        m_length = 0;

                        value = value >> writed;
                        left -= writed;
                    }
                    else
                    {
                        m_current |= (value << m_length).to_ulong();
                        m_length += left;

                        value = value >> left;
                        left = 0;
                    }
                }
            }

            template <typename T>
            void write(T value, std::size_t length)
            {
                unsigned left = length;
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

            ~BitWriter()
            {
                m_out.put(m_current);
            }

        private:
            unsigned m_length = 0;
            std::ostream& m_out;
            char m_current = 0;
        };
    }
}
