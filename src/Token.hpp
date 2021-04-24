#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <ostream>

#undef NULL

namespace mli {

    class Token
    {
        public:

            enum class Type {
                NULL = 0,
                ENTRY, FINISH,
                BEGIN, END,
                INT, STRING, REAL,
                VARIABLE,
                GOTO,
                CASE_OF,
                WHILE, DO,
                READ, WRITE,
                NOT, AND, OR,

                SEMICOLON, COLON, COMMA, POINT,
                ASSIGN,
                OPEN_B, CLOSE_B,
                EQ, LESS, GREATER, NEQ, LEQ, GEQ,

                PLUS, MINUS, MULTIPLY, DIVIDE,

                ID,
                POLIZ_LABEL,
                POLIZ_ADDRESS, 
                POLIZ_GO,
                POLIZ_FGO
            };

            Token(Token::Type a_type = Token::Type::NULL, uint32_t a_value = 0)
                : m_type(a_type), m_value(a_value)
            {
            }

            Token::Type getType() const
            {
                return m_type;
            }

            uint32_t getValue() const
            {
                return m_value;
            }

            bool operator==(const Token& a_other)
            {
                return (a_other.m_type == this->m_type
                        && a_other.m_value == this->m_value);
            }

            friend std::ostream& operator<<(std::ostream &a_out, const Token& a_token)
            {
                a_out << "(" << (int)a_token.m_type << ", " << a_token.m_value << ");";

                return a_out;
            }

        private:
            Token::Type m_type;
            uint32_t  m_value;
    };
}

#endif // TOKEN_HPP
