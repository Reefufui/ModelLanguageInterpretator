#ifndef SYNTAX_ERROR_HPP
#define SYNTAX_ERROR_HPP

#include <ostream>

#include "Token.hpp"

namespace mli {
    class SyntaxError
    {
        private:
            Token       m_unexpectedToken{};

            Token::Type m_expectedTokenType{};
            Token::Type m_unexpectedTokenType{};

            uint32_t    m_onRow{};

        public:
            SyntaxError(const Token a_unexpected, const Token::Type a_expectedType)
                : m_unexpectedToken(a_unexpected), m_expectedTokenType(a_expectedType)
            {
                m_onRow               = a_unexpected.getLine();
                m_unexpectedTokenType = a_unexpected.getType();
            }

            friend std::ostream& operator<<(std::ostream &a_out, const SyntaxError& a_error)
            {
                a_out << "[SyntaxError]: unexpected " << a_error.m_unexpectedTokenType << " on line: " << a_error.m_onRow;
                a_out << " (" << a_error.m_expectedTokenType << " expected)";

                return a_out;
            }
    };
}

#endif // SYNTAX_ERROR_HPP

