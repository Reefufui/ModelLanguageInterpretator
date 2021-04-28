#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <map>
#include <ostream>
#include <string>

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
                INT_CONST, STRING_CONST, REAL_CONST, VALUE,
                GOTO,
                CASE_OF,
                WHILE, DO,
                READ, WRITE,
                NOT, AND, OR,

                SEMICOLON, COLON, COMMA,
                ASSIGN,
                PARENTHESIS,
                OPEN_B, CLOSE_B,
                IF, ELSE,

                EQ, LESS, GREATER, NEQ, LEQ, GEQ, // dont change order

                PLUS, MINUS, MULTIPLY, DIVIDE,

                ID, GOTO_MARK, VARIABLE_TYPE,

                POLIZ_LABEL,
                POLIZ_ADDRESS, 
                POLIZ_GO,
                POLIZ_FALSE_GO
            };

            Token(Token::Type a_type = Token::Type::NULL, int a_line = -1, uint32_t a_value = 0)
                : m_type(a_type), m_line(a_line), m_value(a_value)
            {
            }

            Token::Type getType() const
            {
                return m_type;
            }

            int getLine() const
            {
                return m_line;
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

            friend std::ostream& operator<<(std::ostream &a_out, const Token::Type a_tokenType)
            {
                auto mapCheck = [a_tokenType](const std::pair<std::string, Token::Type>& elem) -> bool
                {
                    return elem.second == a_tokenType;
                };

                auto tokenString = [mapCheck](std::map<std::string, Token::Type> a_map) -> std::string
                {
                    auto mapPair = std::find_if(a_map.begin(), a_map.end(), mapCheck);

                    if (mapPair != std::end(a_map))
                    {
                        return (*mapPair).first;
                    }
                    else
                    {
                        return std::string("");
                    }
                };

                std::string toPrint = tokenString(Token::s_reservedWords) + tokenString(Token::s_delimeters);

                if (toPrint == std::string(""))
                {
                    if (a_tokenType == Token::Type::VARIABLE_TYPE)
                    {
                        a_out << "variable type";
                    }
                    else if (a_tokenType == Token::Type::ID)
                    {
                        a_out << "variable name";
                    }
                    else if (a_tokenType == Token::Type::GOTO_MARK)
                    {
                        a_out << "goto mark";
                    }
                    else if (a_tokenType == Token::Type::STRING_CONST)
                    {
                        a_out << "string const";
                    }
                    else if (a_tokenType == Token::Type::INT_CONST)
                    {
                        a_out << "integer const";
                    }
                    else if (a_tokenType == Token::Type::REAL_CONST)
                    {
                        a_out << "real const";
                    }
                    else if (a_tokenType == Token::Type::VALUE)
                    {
                        a_out << "value";
                    }
                    else if (a_tokenType == Token::Type::POLIZ_LABEL)
                    {
                        a_out << "poliz label";
                    }
                    else if (a_tokenType == Token::Type::POLIZ_ADDRESS)
                    {
                        a_out << "poliz address";
                    }
                    else if (a_tokenType == Token::Type::POLIZ_GO)
                    {
                        a_out << "poliz go";
                    }
                    else if (a_tokenType == Token::Type::POLIZ_FALSE_GO)
                    {
                        a_out << "poliz false go";
                    }
                    else if (a_tokenType == Token::Type::FINISH)
                    {
                        a_out << "EOF";
                    }
                    else
                    {
                        a_out << "??";
                    }
                }
                else
                {
                    a_out << "'" << toPrint << "' token";
                }

                return a_out;
            }

            friend std::ostream& operator<<(std::ostream &a_out, const Token& a_token)
            {
                a_out << a_token.m_type << "\tfrom line: " << a_token.m_line << " with value: " << a_token.m_value;

                return a_out;
            }

            static std::map<std::string, Token::Type> s_reservedWords;
            static std::map<std::string, Token::Type> s_delimeters;

        private:
            Token::Type m_type;
            int         m_line;
            uint32_t    m_value;
    };

    std::map<std::string, Token::Type> Token::s_reservedWords {
        { "",        Token::Type::NULL },
            { "program", Token::Type::ENTRY },
            { "int",     Token::Type::INT },
            { "string",  Token::Type::STRING },
            { "real",    Token::Type::REAL },
            { "goto",    Token::Type::GOTO },
            { "case_of", Token::Type::CASE_OF },
            { "while",   Token::Type::WHILE },
            { "do",      Token::Type::DO },
            { "read",    Token::Type::READ },
            { "write",   Token::Type::WRITE },
            { "not",     Token::Type::NOT },
            { "and",     Token::Type::AND },
            { "if",      Token::Type::IF },
            { "else",    Token::Type::ELSE },
            { "or",      Token::Type::OR }
    };

    std::map<std::string, Token::Type> Token::s_delimeters {
        { "",  Token::Token::Type::NULL },
            { "{",  Token::Type::BEGIN },
            { "}",  Token::Type::END },
            { ";",  Token::Type::SEMICOLON },
            { ":",  Token::Type::COLON },
            { ",",  Token::Type::COMMA },
            { "=",  Token::Type::ASSIGN },
            { "\"", Token::Type::PARENTHESIS },
            { "(",  Token::Type::OPEN_B },
            { ")",  Token::Type::CLOSE_B },
            { "==", Token::Type::EQ },
            { "<",  Token::Type::LESS },
            { ">",  Token::Type::GREATER },
            { "!=", Token::Type::NEQ },
            { "<=", Token::Type::LEQ },
            { ">=", Token::Type::GEQ },
            { "+",  Token::Type::PLUS },
            { "-",  Token::Type::MINUS },
            { "*",  Token::Type::MULTIPLY },
            { "/",  Token::Type::DIVIDE }
    };
}

#endif // TOKEN_HPP
