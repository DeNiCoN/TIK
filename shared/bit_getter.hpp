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

            explicit operator bool() const
            {
                return bool(m_in);
            }

            template <std::size_t N>
            std::bitset<N> read()
            {
                std::bitset<N> value;
                unsigned left = N;
                while (left > 0)
                {
                    if (left + m_length >= sizeof(char) * 8)
                    {
                        value |= (m_current >> m_length);
                        unsigned writed = sizeof(char) * 8 - m_length;
                        m_current = m_in.get();
                        if (!m_in)
                            m_current = 0;

                        m_length = 0;

                        value = value << writed;
                        left -= writed;
                    }
                    else
                    {
                        m_current |= (m_current >> m_length);
                        m_length += left;

                        value = value << left;
                        left = 0;
                    }
                }

                return value;
            }
        private:
            unsigned m_length = 0;
            std::istream& m_in;
            char m_current;
        };
    }
}
