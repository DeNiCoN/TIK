#pragma once
#include <iostream>
#include <bitset>

namespace tik
{
    namespace utils
    {
        class BitGetter
        {
        public:
            BitGetter(std::istream& in)
                : m_in(in), m_current(in.get())
            {}
            bool get()
            {
                if (m_length >= 8)
                {
                    m_length = 0;
                    m_current = m_in.get();
                }

                return (m_current >> m_length++) & 1;
            }

            /*template <std::size_t N>
            std::bitset<N> read()
            {
                unsigned left = N;
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
            }*/
        private:
            unsigned m_length = 0;
            std::istream& m_in;
            char m_current;
        };
    }
}
