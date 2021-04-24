#ifndef LEXICAL_ERROR_HPP
#define LEXICAL_ERROR_HPP

#include <ostream>

namespace mli {
    class LexicalError
    {
        private:
            uint32_t m_onRow{};
            char     m_unexpectedChar{};
            char     m_expectedChar{};

        public:
            LexicalError(uint32_t a_onRow, const char a_unexpected, const char a_expected)
                : m_onRow(a_onRow), m_unexpectedChar(a_unexpected), m_expectedChar(a_expected)
            {
            }

            LexicalError(uint32_t a_onRow, const char a_unexpected)
                : m_onRow(a_onRow), m_unexpectedChar(a_unexpected)
            {
            }

            friend std::ostream& operator<<(std::ostream &a_out, const LexicalError& a_error)
            {
                if (a_error.m_unexpectedChar)
                {
                    a_out << "[LexicalError]: unexpected '" << a_error.m_unexpectedChar << "' on line: " << a_error.m_onRow;
                }
                else
                {
                    a_out << "[LexicalError]: unexpected EOF";
                }

                if (a_error.m_expectedChar)
                {
                    a_out << " ('" << a_error.m_expectedChar << "' expected)";
                }

                return a_out;
            }
    };
}

#endif // LEXICAL_ERROR_HPP
