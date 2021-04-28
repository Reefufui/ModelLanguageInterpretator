#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Scanner.hpp"
#include "SyntaxError.hpp"
#include "Semantic.hpp"

namespace mli {

    class Parser
    {
        private:
            Token       m_currentToken;
            Token::Type m_currentType;
            int         m_currentValue;
            Scanner     m_scanner;
            Semantic    m_validator;

            std::vector<Token> m_poliz;

            void getToken()
            {
                m_currentToken = m_scanner.getToken();
                m_currentType  = m_currentToken.getType();
                m_currentValue = m_currentToken.getValue();
            }

            void checkToken(Token::Type a_expectedType)
            {
                if (m_currentType != a_expectedType)
                {
                    throw SyntaxError(m_currentToken, a_expectedType);
                }
            }

            void getToken(Token::Type a_expectedType)
            {
                getToken();
                checkToken(a_expectedType);
            }

            void value()
            {
                if (m_currentType == Token::Type::ID)
                {
                    m_validator.declarationCheck(m_currentValue);
                    getToken();
                }
                else
                {
                    constant();
                    getToken();
                }
            }

            void multiplierOperand()
            {
                if (m_currentType == Token::Type::NOT)
                {
                    getToken();
                }

                if (m_currentType == Token::Type::MINUS || m_currentType == Token::Type::PLUS)
                {
                    getToken();
                }

                if (m_currentType == Token::Type::OPEN_B)
                {
                    getToken();
                    expression();

                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                }
                else
                {
                    value();
                }
            }

            void termOperand()
            {
                multiplierOperand();

                while (m_currentType == Token::Type::MULTIPLY || m_currentType == Token::Type::DIVIDE)
                {
                    getToken();
                    multiplierOperand();
                }
            }

            void compOperand()
            {
                termOperand();

                while (m_currentType == Token::Type::PLUS || m_currentType == Token::Type::MINUS)
                {
                    getToken();
                    termOperand();
                }
            }

            void andOperand()
            {
                compOperand();

                while ( m_currentType >= Token::Type::EQ && m_currentType <= Token::Type::GEQ )
                {
                    getToken();
                    compOperand();
                }
            }

            void orOperand()
            {
                andOperand();

                while (m_currentType == Token::Type::AND)
                {
                    getToken();
                    andOperand();
                }
            }

            void assignOperand()
            {
                orOperand();

                while (m_currentType == Token::Type::OR)
                {
                    getToken();
                    orOperand();
                }
            }

            void expression()
            {
                assignOperand();

                while (m_currentType == Token::Type::ASSIGN)
                {
                    getToken();
                    assignOperand();
                }
            }

            void statement()
            {
                if ( m_currentType == Token::Type::READ )
                {
                    getToken(Token::Type::OPEN_B);
                    {
                        getToken(Token::Type::ID);
                        m_validator.declarationCheck(m_currentValue);
                    }
                    getToken(Token::Type::CLOSE_B);

                    getToken(Token::Type::SEMICOLON);
                }
                else if ( m_currentType == Token::Type::WRITE )
                {
                    getToken(Token::Type::OPEN_B);

                    do
                    {
                        getToken();
                        expression();
                    }
                    while (m_currentType == Token::Type::COMMA);

                    checkToken(Token::Type::CLOSE_B);

                    getToken(Token::Type::SEMICOLON);
                }
                else if ( m_currentType == Token::Type::WHILE )
                {
                    getToken(Token::Type::OPEN_B);
                    {
                        getToken();
                        expression();
                    }
                    checkToken(Token::Type::CLOSE_B);
                    {
                        getToken();
                        statement();
                    }
                }
                else if ( m_currentType == Token::Type::DO )
                {
                    {
                        getToken();
                        statement();
                    }
                    getToken(Token::Type::WHILE);
                    getToken(Token::Type::OPEN_B);
                    {
                        getToken();
                        expression();
                    }
                    checkToken(Token::Type::CLOSE_B);

                    getToken(Token::Type::SEMICOLON);
                }
                else if ( m_currentType == Token::Type::IF )
                {
                    getToken(Token::Type::OPEN_B);
                    {
                        getToken();
                        expression();
                    }
                    checkToken(Token::Type::CLOSE_B);
                    {
                        getToken();
                        statement();
                    }
                    getToken(Token::Type::ELSE);
                    {
                        getToken();
                        statement();
                    }
                }
                else if ( m_currentType == Token::Type::GOTO_MARK )
                {
                    m_validator.mark(m_currentValue);
                    getToken(Token::Type::COLON);
                }
                else if ( m_currentType == Token::Type::GOTO )
                {
                    getToken(Token::Type::GOTO_MARK);
                    getToken(Token::Type::SEMICOLON);
                }
                else if (m_currentType == Token::Type::BEGIN)
                {
                    getToken();
                    statements();
                }
                else
                {
                    expression();
                    checkToken(Token::Type::SEMICOLON);
                }
            }

            void statements()
            {
                while (m_currentType != Token::Type::END)
                {
                    statement();
                    getToken();
                }
            }

            void constant()
            {
                auto isNumerical = [&]() -> bool
                {
                    return m_currentType == Token::Type::INT_CONST
                        || m_currentType == Token::Type::REAL_CONST;
                };

                if (m_currentToken == Token::Type::MINUS || m_currentToken == Token::Type::PLUS)
                {
                    getToken();

                    if (!isNumerical())
                    {
                        throw SyntaxError(m_currentToken, Token::Type::INT_CONST);
                    }
                }
                else if (!isNumerical() && (m_currentType != Token::Type::STRING_CONST))
                {
                    throw SyntaxError(m_currentToken, Token::Type::VALUE);
                }
            }

            void declarations()
            {
                Token::Type variableType;

                auto isDeclaration = [&]() -> bool
                {
                    variableType = m_currentType;
                    return m_currentType == Token::Type::INT
                        || m_currentType == Token::Type::STRING
                        || m_currentType == Token::Type::REAL;
                };

                while (isDeclaration())
                {
                    do
                    {
                        getToken(Token::Type::ID);
                        uint32_t varID = m_currentValue;

                        m_validator.declaration(varID, variableType);

                        getToken();

                        if (m_currentType == Token::Type::ASSIGN)
                        {
                            getToken();
                            constant();
                            m_validator.init(varID, m_currentType);
                            getToken();
                        }
                    }
                    while (m_currentType == Token::Type::COMMA);

                    checkToken(Token::Type::SEMICOLON);
                    getToken();
                }
            }

        public:

            Parser(const std::string& a_srcFileName)
                : m_scanner(a_srcFileName)
            {
            }

            void analyze()
            {
                getToken(Token::Type::ENTRY);
                getToken(Token::Type::BEGIN);
                {
                    getToken();
                    declarations();
                    statements();
                }
                getToken(Token::Type::FINISH);
            }
    };
}

#endif // PARSER_HPP

